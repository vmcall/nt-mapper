#include "portable_executable.hpp"
#include "transformer.hpp"
#include <vector>
#include <algorithm>

portable_executable::portable_executable(std::vector<std::byte>& new_buffer) noexcept : buffer(new_buffer)
{
	// READ HEADERS
	if (this->buffer.data())
	{
		this->dos_header = reinterpret_cast<IMAGE_DOS_HEADER*>(this->buffer.data());
		this->nt_headers = reinterpret_cast<IMAGE_NT_HEADERS64*>(this->buffer.data() + this->dos_header->e_lfanew);
		this->file_header = this->nt_headers->FileHeader;
		this->optional_header = this->nt_headers->OptionalHeader;

		// PARSE IMAGE SECTIONS FOR LATER USAGE
		parse_sections();
	}
}

IMAGE_DOS_HEADER* portable_executable::get_dos_header() noexcept
{
	return this->dos_header;
}

IMAGE_NT_HEADERS* portable_executable::get_nt_headers() noexcept
{
	return this->nt_headers;
}

IMAGE_FILE_HEADER portable_executable::get_file_header() noexcept
{
	return this->file_header;
}

IMAGE_OPTIONAL_HEADER portable_executable::get_optional_header() noexcept
{
	return this->optional_header;
}

std::uintptr_t portable_executable::get_image_base() noexcept
{
	return this->optional_header.ImageBase;
}

section_list& portable_executable::get_sections() noexcept
{
	return this->sections;
}

std::vector<std::byte>& portable_executable::get_buffer() noexcept
{
	return this->buffer;
}

void portable_executable::parse_sections() noexcept
{
	auto section_pointer = reinterpret_cast<IMAGE_SECTION_HEADER*>(this->nt_headers + 1);
	for (auto index = 0; index < this->file_header.NumberOfSections; index++)
		this->sections.push_back(section_pointer[index]);
}

relocation_list portable_executable::get_relocations(std::uintptr_t image_base) noexcept
{
	relocation_list result;
	auto section = this->optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
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

import_list portable_executable::get_imports(std::uintptr_t image_base) noexcept
{
	import_list import_modules;

	auto section = this->optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	auto import_table = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(image_base + section.VirtualAddress);


	
	for (std::uint32_t previous_name = 0; // NAMES ARE IN MEMORY ORDER
		previous_name < import_table->Name; 
		previous_name = import_table->Name, ++import_table)
	{
		auto module_name = std::string(reinterpret_cast<char*>(image_base + (std::uintptr_t)import_table->Name));

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
export_list portable_executable::get_exports(std::uintptr_t image_base) noexcept
{
	export_list exports;

	auto section = this->optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
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
