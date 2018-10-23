#include "manualmapper.hpp"
#include "logger.hpp"
#include "binary_file.hpp"
#include "cast.hpp"

#include <thread>
#include <chrono>

map_ctx injection::manualmapper::inject(const std::vector<std::byte>& buffer) noexcept
{
	// GET LINKED MODULES FOR LATER USE
	this->linked_modules() = this->process().get_modules();

	// INITIALISE CONTEXT
	map_ctx ctx("Main Image", buffer);

	// MAP MAIN IMAGE AND ALL DEPENDENCIES
	return map_image(ctx) ? ctx : map_ctx{};
}

bool injection::manualmapper::map_image(map_ctx& ctx) noexcept
{
	// CREATE A MEMORY SECTION THAT CAN BE MAPPED INTO SEPERATE PROCESSES
	// AND REFLECT EACH OTHER, ALLOWING FOR EASY MEMORY ACCESS
	auto section = memory_section(PAGE_EXECUTE_READWRITE, ctx.pe().get_optional_header().SizeOfImage);
	if (!section)
	{
		logger::log_error("Failed to create section");
		return false;
	}

	// MAP SECTION INTO BOTH LOCAL AND REMOTE PROCESS
	ctx.set_local_image(native::process::current_process().map(section));
	ctx.set_remote_image(this->process().map(section));

	if (!ctx.local_image() || !ctx.remote_image())
	{
		logger::log_error("Failed to map section");
		return false;
	}

	// ADD MAPPED MODULE TO LIST OF MODULES
	this->mapped_modules().emplace_back(ctx);

	logger::log_formatted("Mapping image", ctx.image_name(), false);

	// MANUALMAP IMAGE
	write_headers(ctx);				// WRITE PORTABLE EXECUTABLE HEADERS, THESE CONTAIN METADATA OF THE IMAGE
	write_image_sections(ctx);		// WRITE IMAGE SECTIONS, THESE CONTAIN THE ACTUAL DATA THAT IS THE IMAGE
	fix_import_table(ctx);			// FIX IMPORTED FUNCTIONS IN THE IMPORT TABLE
	relocate_image_by_delta(ctx);	// RELOCATE IMAGE BY PARSING RELOCATION DATA AND HANDLING POINTERS FOR THE NEW IMAGE BASE

	// TODO:
	// HANDLE TLS CALLBACKS
	// HANDLE STATIC TLS DATA
	// HANDLE C++ EXCEPTIONS
	// INSERT LOADER ENTRIES (DOCUMENTED AND UNDOCUMENTED)

	return true;
}

uintptr_t injection::manualmapper::find_or_map_dependency(const std::string& image_name) noexcept
{
	// HAVE WE MAPPED THIS DEPENDENCY ALREADY?
	for (auto& module : this->mapped_modules())
	{
		if (module.image_name() == image_name)
			return module.remote_image();
	}

	// WAS THIS DEPENDENCY ALREADY LOADED BY LDR?
	if (auto iterator = this->linked_modules().find(image_name); iterator != this->linked_modules().end())
		return iterator->second;

	logger::log_formatted("Failed to find loaded dependency", image_name, false);

	// TODO: PROPER FILE SEARCHING?
	binary_file file("C:\\Windows\\System32\\" + image_name);
	auto ctx = map_ctx(image_name, file.buffer());

	// MAP DEPENDENCY
	return map_image(ctx) ? ctx.remote_image() : 0x00;
}

void injection::manualmapper::write_headers(map_ctx& ctx) noexcept
{
	// COPY OVER PE HEADERS TO MAPPED SECTION
	std::memcpy(
		reinterpret_cast<void*>(ctx.local_image()),		// DESTINATION
		ctx.pe_buffer(),								// SOURCE
		ctx.pe().get_optional_header().SizeOfHeaders);	// SIZE
}

void injection::manualmapper::write_image_sections(map_ctx& ctx) noexcept
{
	// APPLY READ-ONLY PROTECTION TO PE-HEADERS
	//std::uint32_t old_protection;
	//this->process().virtual_protect(ctx.remote_image(), PAGE_READONLY, &old_protection, 0x1000);

	// COPY EACH PE SECTION TO MAPPED SECTION
	for (const auto& section : ctx.pe().get_sections())
	{
		// COPY RAW DATA OF SECTION
		std::memcpy(
			reinterpret_cast<void*>(ctx.local_image() + section.virtual_address()),	// DESTINATION
			ctx.pe_buffer() + section.raw_data_offset(),							// SOURCE
			section.raw_size());													// SIZE

		// PARSE PROTECTION FROM CHARACTERISTICS
		std::uint32_t virtual_protection = 0x00;

		// NO ACCESSS
		if (!section.is_readable())
		{
			virtual_protection = PAGE_NOACCESS;
		}

		// WRITABLE
		if (section.is_writable())
		{
			virtual_protection = section.is_executable() ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE;
		}
		else
		{
			virtual_protection = section.is_executable() ? PAGE_EXECUTE_READ : PAGE_READONLY;
		}

		// APPLY VIRTUAL FLAGS TO REMOTE PROCESS
		//auto result = this->process().virtual_protect(
		//	ctx.remote_image() + section.virtual_address(), 
		//	virtual_protection, &old_protection,
		//	section.virtual_size());
		
		//if (!result)
		//{
		//	logger::log_error("Failed to protect section");
		//	logger::log_formatted("Last error", GetLastError(), true);
		//	logger::log_formatted("Section name", section.raw_header().Name, false);
		//	logger::log_formatted("Protection", virtual_protection, true);
		//
		//	logger::log("Arguments:");
		//	logger::log_formatted("1", ctx.remote_image() + section.virtual_address(), true);
		//	logger::log_formatted("2", virtual_protection, true);
		//	logger::log_formatted("3", &old_protection, true);
		//	logger::log_formatted("4", section.raw_size(), true);
		//}
	}
}

void injection::manualmapper::relocate_image_by_delta(map_ctx& ctx) noexcept
{
	// CHANGE IN BASE ADDRESS (DEFAULT -> MAPPED)
	const auto delta = ctx.remote_image() - ctx.pe().get_image_base();

	// UPDATE POINTERS WITH THE NEW IMAGE BASE DIFFERENCE
	for (auto&[entry, item] : ctx.pe().get_relocations(ctx.local_image()))
		*cast::long_pointer(ctx.local_image() + entry.page_rva + item.get_offset()) += delta;
}

void injection::manualmapper::fix_import_table(map_ctx& ctx) noexcept
{
	wstring_converter_t converter;
	api_set api_schema;

	for (const auto&[map_key, functions] : ctx.pe().get_imports(ctx.local_image()))
	{
		auto module_name = map_key; 

		// QUERY API SCHEMA FOR NAME
		std::wstring wide_module_name = converter.from_bytes(module_name.c_str());
		if (api_schema.query(wide_module_name))
			module_name = converter.to_bytes(wide_module_name);

		const auto module_handle = find_or_map_dependency(module_name);
		if (!module_handle)
		{
			logger::log_error("Failed to map dependency");
			return;
		}

		// ITERATE IMPORTED FUNCTIONS
		for (const auto& fn : functions)
		{
			// GET THE FUNCTION EXPORT
			const auto function_ordinal = fn.ordinal > 0 ? reinterpret_cast<const char*>(fn.ordinal) : fn.name.c_str();
			auto exported_function = this->process().get_module_export(module_handle, function_ordinal);

			// HANDLE FORWARDED EXPORTS RECURSIVELY
			while (exported_function.forwarded)
			{
				exported_function = handle_forwarded_export(exported_function, api_schema);
			}
			
			// UPDATE IMPORTED FUNCTION POINTER
			if (exported_function.function != 0x00)
			{
				*cast::long_pointer(ctx.local_image() + fn.function_rva) = exported_function.function;
			}
			else
			{
				logger::log_error("Failed to handle imported function.");
				logger::log_formatted("Function name", function_ordinal, false);
			}
		}
	}
}

native::process::module_export injection::manualmapper::handle_forwarded_export(native::process::module_export& exported_function, api_set& api_schema) noexcept
{
	// QUERY API SCHEMA FOR NAME
	wstring_converter_t converter;
	auto library_name = exported_function.forwarded_library;
	std::wstring wide_forwarded_library_name = converter.from_bytes(library_name.c_str());
	if (api_schema.query(wide_forwarded_library_name))
		library_name = converter.to_bytes(wide_forwarded_library_name);

	logger::log_formatted("Library name", library_name, false);
	logger::log_formatted("Function name", exported_function.forwarded_name, false);

	// TODO: PROPER FILE SEARCHING?
	binary_file file("C:\\Windows\\System32\\" + library_name);
	auto forwarded_ctx = map_ctx(library_name, file.buffer());

	// MAP FORWARDED LIBRARY
	if (map_image(forwarded_ctx))
	{
		logger::log_error("Failed to map forwarded library.");
		return native::process::module_export(0x00);
	}

	// FIND FORWARDED EXPORT
	return this->process().get_module_export(forwarded_ctx.remote_image(), exported_function.forwarded_name.c_str());
}

native::process::module_list_t& injection::manualmapper::linked_modules() noexcept
{
	return this->m_linked_modules;
}

std::vector<map_ctx>& injection::manualmapper::mapped_modules() noexcept
{
	return this->m_mapped_modules;
}

native::process& injection::manualmapper::process() noexcept
{
	return this->m_process;
}