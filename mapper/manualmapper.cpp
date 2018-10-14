#include "manualmapper.hpp"
#include "api_set.hpp"
#include "logger.hpp"
#include "binary_file.hpp"
#include "cast.hpp"

#include <thread>
#include <chrono>

uintptr_t injection::manualmapper::inject(const std::vector<std::byte>& buffer, injection::executor::mode execution_mode)
{
	// GET LINKED MODULES FOR LATER USE
	this->linked_modules() = this->process().get_modules();

	// INITIALISE CONTEXT
	map_ctx ctx("Main Image", buffer);

	// INITIALISE EXECUTION ENGINE (EXECUTOR)
	this->execution_engine() = { execution_mode };

	// MAP MAIN IMAGE AND ALL DEPENDENCIES
	if (!map_image(ctx))
		return 0x00;

	// CALL DEPENDENCY ENTRYPOINTS AND MAIN IMAGE ENTRYPOINT
	if (!call_entrypoint(ctx))
		return 0x00;

	// RETURN ADDRESS OF THE MAPPED IMAGE 
	return ctx.remote_image();
}

bool injection::manualmapper::map_image(map_ctx& ctx)
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
	ctx.local_image() = native::process::current_process().map(section);
	ctx.remote_image() = this->process().map(section);
	if (!ctx.local_image() || !ctx.remote_image())
	{
		logger::log_error("Failed to map section");
		return false;
	}

	// ADD MAPPED MODULE TO LIST OF MODULES
	this->mapped_modules().emplace_back(ctx);

	// MANUALMAP IMAGE
	write_headers(ctx);				// WRITE PORTABLE EXECUTABLE HEADERS, THESE CONTAIN METADATA OF THE IMAGE
	write_image_sections(ctx);		// WRITE IMAGE SECTIONS, THESE CONTAIN THE ACTUAL DATA THAT IS THE IMAGE
	fix_import_table(ctx);			// FIX IMPORTED FUNCTIONS IN THE IMPORT TABLE
	relocate_image_by_delta(ctx);	// RELOCATE IMAGE BY PARSING RELOCATION DATA AND HANDLING POINTERS FOR THE NEW IMAGE BASE

	// TODO:
	// HANDLE STATIC TLS DATA
	// HANDLE STATIC TLS CALLBACKS
	// HANDLE C++ EXCEPTIONS
	// INSERT LOADER ENTRIES (DOCUMENTED AND UNDOCUMENTED)

	return true;
}

uintptr_t injection::manualmapper::find_or_map_dependency(const std::string& image_name)
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

	// TODO: PROPER FILE SEARCHING?

	binary_file file("C:\\Windows\\System32\\" + image_name);
	auto ctx = map_ctx(image_name, file.buffer());

	// MAP DEPENDENCY
	return map_image(ctx) ? ctx.remote_image() : 0x00;
}

void injection::manualmapper::write_headers(map_ctx& ctx)
{
	// COPY OVER PE HEADERS TO MAPPED SECTION
	std::memcpy(
		reinterpret_cast<void*>(ctx.local_image()),		// DESTINATION
		ctx.pe_buffer(),								// SOURCE
		ctx.pe().get_optional_header().SizeOfHeaders);	// SIZE
}
void injection::manualmapper::write_image_sections(map_ctx& ctx)
{
	// COPY OVER EACH PE SECTION TO MAPPED SECTION
	for (const auto& section : ctx.pe().get_sections())
	{
		std::memcpy(
			reinterpret_cast<void*>(ctx.local_image() + section.VirtualAddress),	// DESTINATION
			ctx.pe_buffer() + section.PointerToRawData,								// SOURCE
			section.SizeOfRawData);													// SIZE
	}
}

bool injection::manualmapper::call_entrypoint(map_ctx& ctx)
{
	// EXECUTE THE IMAGE, HANDLED BY EXECUTION ENGINE
	return this->execution_engine().handle(ctx, this->process());
}

void injection::manualmapper::relocate_image_by_delta(map_ctx& ctx)
{
	// CHANGE IN BASE ADDRESS (DEFAULT -> MAPPED)
	const auto delta = ctx.remote_image() - ctx.pe().get_image_base();

	// UPDATE POINTERS WITH THE NEW IMAGE BASE DIFFERENCE
	for (auto&[entry, item] : ctx.pe().get_relocations(ctx.local_image()))
		*cast::pointer(ctx.local_image() + entry.page_rva + item.get_offset()) += delta;
}

void injection::manualmapper::fix_import_table(map_ctx& ctx)
{
	wstring_converter converter;
	api_set api_schema;

	for (const auto&[map_key, functions] : ctx.pe().get_imports(ctx.local_image()))
	{
		auto module_name = map_key; 

		std::wstring wide_module_name = converter.from_bytes(module_name.c_str());
		if (api_schema.query(wide_module_name))
			module_name = converter.to_bytes(wide_module_name);

		const auto module_handle = find_or_map_dependency(module_name);

		if (!module_handle)
		{
			logger::log_error("Failed to map dependency");
			return;
		}

		for (const auto& fn : functions)
		{
			*cast::pointer(ctx.local_image() + fn.function_rva) = fn.ordinal > 0 ?
				this->process().get_module_export(module_handle, reinterpret_cast<const char*>(fn.ordinal)) :	// IMPORT BY ORDINAL
				this->process().get_module_export(module_handle, fn.name .c_str());								// IMPORT BY NAME
		}
	}
}

injection::manualmapper::module_list& injection::manualmapper::linked_modules()
{
	return this->m_linked_modules;
}

std::vector<map_ctx> injection::manualmapper::mapped_modules()
{
	return this->m_mapped_modules;
}

native::process& injection::manualmapper::process()
{
	return this->m_process;
}

injection::executor& injection::manualmapper::execution_engine()
{
	return this->m_executor;
}