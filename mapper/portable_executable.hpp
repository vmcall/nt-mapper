#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "pe_section.hpp"

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
using section_list = std::vector<pe_section>;
using import_list = std::unordered_map<std::string, std::vector<import_data>>;
using export_list = std::unordered_map<std::string, std::vector<export_data>>;

class portable_executable
{
public:
	portable_executable() {};
	explicit portable_executable(const std::vector<std::byte>& new_buffer) noexcept;

	// INFORMATION
	 const IMAGE_DOS_HEADER* get_dos_header() const noexcept;
	 const IMAGE_NT_HEADERS* get_nt_headers() const noexcept;
	 const IMAGE_FILE_HEADER get_file_header() const noexcept;
	 const IMAGE_OPTIONAL_HEADER get_optional_header() const noexcept;
	 const std::uintptr_t get_image_base() const noexcept;

	// RESOURCES
	const std::vector<std::byte>& get_buffer() const noexcept;
	const section_list& get_sections() const noexcept;
	
	relocation_list get_relocations(std::uintptr_t image_base)  const noexcept;
	import_list get_imports(std::uintptr_t image_base) const noexcept;
	export_list get_exports(std::uintptr_t image_base) const noexcept;

private:
	void parse_sections() noexcept;
	IMAGE_DOS_HEADER* m_dos_header;
	IMAGE_NT_HEADERS* m_nt_headers;
	IMAGE_OPTIONAL_HEADER m_optional_header;
	IMAGE_FILE_HEADER m_file_header;
	std::vector<std::byte> m_buffer;
	section_list m_sections;
};