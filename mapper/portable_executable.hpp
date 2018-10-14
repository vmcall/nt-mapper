#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <unordered_map>

struct reloc_item
{
	WORD offset : 12;
	WORD type : 4;

	WORD get_offset()
	{
		return offset % 4096;
	}
};
struct reloc_data
{
	ULONG page_rva;
	ULONG block_size;
	reloc_item item[1];
};

struct import_data
{
	std::string name;
	uintptr_t function_rva;
	uintptr_t ordinal = 0;
};

struct export_data
{
	std::string name;
	uintptr_t function_rva;
	uint16_t ordinal;
};

using relocation_list = std::vector<std::pair<reloc_data, reloc_item>>;
using section_list = std::vector<IMAGE_SECTION_HEADER>;
using import_list = std::unordered_map<std::string, std::vector<import_data>>;
using export_list = std::unordered_map<std::string, std::vector<export_data>>;

class portable_executable
{
public:
	portable_executable() {};
	portable_executable(std::vector<std::byte>& new_buffer);

	// INFORMATION
	IMAGE_DOS_HEADER* get_dos_header();
	IMAGE_NT_HEADERS* get_nt_headers();
	IMAGE_FILE_HEADER get_file_header();
	IMAGE_OPTIONAL_HEADER get_optional_header();
	uintptr_t get_image_base();

	// RESOURCES
	std::vector<std::byte>& get_buffer();
	section_list& get_sections();
	relocation_list get_relocations(uintptr_t image_base);
	import_list get_imports(uintptr_t image_base);
	export_list get_exports(uintptr_t image_base);

private:
	void parse_sections();
	IMAGE_DOS_HEADER* dos_header;
	IMAGE_NT_HEADERS* nt_headers;
	IMAGE_OPTIONAL_HEADER optional_header;
	IMAGE_FILE_HEADER file_header;
	std::vector<std::byte> buffer;
	section_list sections;
};