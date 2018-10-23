#include "portable_executable.hpp"
#include "transformer.hpp"
#include "logger.hpp"

#include <vector>
#include <algorithm>

portable_executable::portable_executable(const std::vector<std::byte>& new_buffer) noexcept : m_buffer(new_buffer)
{
	// READ HEADERS
	if (this->m_buffer.data())
	{
		const auto buffer_address = this->m_buffer.data();

		this->m_dos_header = reinterpret_cast<IMAGE_DOS_HEADER*>(buffer_address);
		this->m_nt_headers = reinterpret_cast<IMAGE_NT_HEADERS64*>(buffer_address + this->get_dos_header()->e_lfanew);
		this->m_file_header = this->get_nt_headers()->FileHeader;
		this->m_optional_header = this->get_nt_headers()->OptionalHeader;

		// PARSE IMAGE SECTIONS FOR LATER USAGE
		parse_sections();
	}
}

const IMAGE_DOS_HEADER* portable_executable::get_dos_header() const noexcept
{
	return this->m_dos_header;
}

const IMAGE_NT_HEADERS* portable_executable::get_nt_headers() const noexcept
{
	return this->m_nt_headers;
}

const IMAGE_FILE_HEADER portable_executable::get_file_header() const noexcept
{
	return this->m_file_header;
}

const IMAGE_OPTIONAL_HEADER portable_executable::get_optional_header() const noexcept
{
	return this->m_optional_header;
}

const std::uintptr_t portable_executable::get_image_base() const noexcept
{
	return this->m_optional_header.ImageBase;
}

const section_list& portable_executable::get_sections() const noexcept
{
	return this->m_sections;
}

const std::vector<std::byte>& portable_executable::get_buffer() const noexcept
{
	return this->m_buffer;
}

void portable_executable::parse_sections() noexcept
{
	const auto section_pointer = reinterpret_cast<const IMAGE_SECTION_HEADER*>(this->get_nt_headers() + 1);
	for (auto index = 0; index < this->get_file_header().NumberOfSections; index++)
		this->m_sections.push_back(pe_section(section_pointer[index]));
}

relocation_list portable_executable::get_relocations(std::uintptr_t image_base) const noexcept
{
	relocation_list result;
	auto section = this->get_optional_header().DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	auto base_relocation = reinterpret_cast<IMAGE_BASE_RELOCATION*>(image_base + section.VirtualAddress);
	auto end_relocation = reinterpret_cast<std::uintptr_t>(base_relocation) + section.Size;
	auto reloc = reinterpret_cast<reloc_data*>(base_relocation);

	while (reinterpret_cast<std::uintptr_t>(reloc) < end_relocation && reloc->block_size > 0)
	{
		auto count = (reloc->block_size - 8) >> 1;
		for (size_t relocation_index = 0; relocation_index < count; ++relocation_index)
		{
			auto type = reloc->item[relocation_index].type;
			if (type == IMAGE_REL_BASED_HIGHLOW || type == IMAGE_REL_BASED_DIR64)
				result.emplace_back(*reloc, reloc->item[relocation_index]);
		}

		reloc = reinterpret_cast<reloc_data*>(reinterpret_cast<std::uintptr_t>(reloc) + reloc->block_size);
	}

	return result;
}

import_list portable_executable::get_imports(std::uintptr_t image_base) const noexcept
{
	import_list import_modules;

	auto section = this->get_optional_header().DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

	if (section.VirtualAddress == 0x00 || section.Size == 0x00)
		return import_modules;

	auto import_table = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(image_base + section.VirtualAddress);

	for (std::uint32_t previous_name = 0; // NAMES ARE IN MEMORY ORDER
		previous_name < import_table->Name; 
		previous_name = import_table->Name, ++import_table)
	{
		auto module_name = std::string(reinterpret_cast<char*>(image_base + (std::uintptr_t)import_table->Name));

		if (module_name.empty())
		{
			logger::log_error("Empty module name in import table");
		}


		// CONVERT TO LOWER
		transformer::string_to_lower(module_name);

		auto entry = reinterpret_cast<IMAGE_THUNK_DATA64*>(image_base + import_table->OriginalFirstThunk);
		for (std::uintptr_t index = 0; entry->u1.AddressOfData; index += sizeof(std::uintptr_t), ++entry)
		{
			auto import_by_name = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(image_base + entry->u1.AddressOfData);

			import_data data;
			data.function_rva = import_table->FirstThunk + index;

			if (entry->u1.Ordinal < IMAGE_ORDINAL_FLAG64 && import_by_name->Name[0])
				data.name = reinterpret_cast<const char*>(import_by_name->Name); // IMPORT BY NAME
			else
				data.ordinal = IMAGE_ORDINAL64(entry->u1.AddressOfData); // IMPORT BY ORDINAL

			import_modules[module_name].emplace_back(data);
		}
	}

	return import_modules;
}

// THX DAAX <3
export_list portable_executable::get_exports(std::uintptr_t image_base) const noexcept
{
	export_list exports;

	auto section = this->get_optional_header().DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	auto export_dir = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(image_base + section.VirtualAddress);

	for (unsigned int iter = 0; iter < export_dir->NumberOfFunctions; ++iter)
	{
		export_data data = { 0 };
		data.name = reinterpret_cast<char*>(image_base + reinterpret_cast<std::uint32_t*>(image_base + export_dir->AddressOfNames)[iter]);
		data.ordinal = reinterpret_cast<std::uint16_t*>(image_base + export_dir->AddressOfNameOrdinals)[iter];
		data.function_rva = image_base + reinterpret_cast<std::uint32_t*>(image_base + export_dir->AddressOfFunctions)[iter];

		exports[data.name].emplace_back(data);
	}

	return exports;
}
