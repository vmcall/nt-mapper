#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <unordered_map>

struct reloc_item
{
	std::uint16_t offset : 12;
	std::uint16_t type : 4;

	std::uint16_t get_offset() noexcept
	{
		return offset % 4096;
	}
};
struct reloc_data
{
	std::uint32_t page_rva;
	std::uint32_t block_size;
	reloc_item item[1];
};

struct import_data
{
	std::string name;
	std::uintptr_t function_rva;
	std::uintptr_t ordinal = 0;
};

struct export_data
{
	std::string name;
	std::uintptr_t function_rva;
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
	explicit portable_executable(std::vector<std::byte>& new_buffer) noexcept;

	// INFORMATION
	IMAGE_DOS_HEADER* get_dos_header() noexcept;
	IMAGE_NT_HEADERS* get_nt_headers() noexcept;
	IMAGE_FILE_HEADER get_file_header() noexcept;
	IMAGE_OPTIONAL_HEADER get_optional_header() noexcept;
	std::uintptr_t get_image_base() noexcept;

	// RESOURCES
	std::vector<std::byte>& get_buffer() noexcept;
	section_list& get_sections() noexcept;
	relocation_list get_relocations(std::uintptr_t image_base) noexcept;
	import_list get_imports(std::uintptr_t image_base) noexcept;
	export_list get_exports(std::uintptr_t image_base) noexcept;

private:
	void parse_sections() noexcept;
	IMAGE_DOS_HEADER* dos_header;
	IMAGE_NT_HEADERS* nt_headers;
	IMAGE_OPTIONAL_HEADER optional_header;
	IMAGE_FILE_HEADER file_header;
	std::vector<std::byte> buffer;
	section_list sections;
};