#pragma once
#include <windows.h>
#include <cstdint>

class pe_section
{
public:
	explicit pe_section(const IMAGE_SECTION_HEADER& header) noexcept : m_header(header) {};


	// SECTION FLAGS
	enum data_section_flags : std::uint32_t
	{
		type_reg = 0x00000000,
		type_d_Sect = 0x00000001,
		type_no_load = 0x00000002,
		type_group = 0x00000004,
		type_no_padded = 0x00000008,
		type_copy = 0x00000010,
		content_code = 0x00000020,
		content_initialized_data = 0x00000040,
		content_uninitialized_data = 0x00000080,
		link_other = 0x00000100,
		link_info = 0x00000200,
		type_over = 0x00000400,
		link_remove = 0x00000800,
		link_com_dat = 0x00001000,
		no_defer_spec_exceptions = 0x00004000,
		relative_gp = 0x00008000,
		mem_purgeable = 0x00020000,
		memory_16_bit = 0x00020000,
		memory_locked = 0x00040000,
		memory_preload = 0x00080000,
		align_1_bytes = 0x00100000,
		align_2_bytes = 0x00200000,
		align_4_bytes = 0x00300000,
		align_8_bytes = 0x00400000,
		align_16_bytes = 0x00500000,
		align_32_bytes = 0x00600000,
		align_64_bytes = 0x00700000,
		align_128_bytes = 0x00800000,
		align_256_bytes = 0x00900000,
		align_512_bytes = 0x00A00000,
		align_1024_bytes = 0x00B00000,
		align_2048_bytes = 0x00C00000,
		align_4096_bytes = 0x00D00000,
		align_8192_bytes = 0x00E00000,
		link_extended_relocation_overflow = 0x01000000,
		memory_discardable = 0x02000000,
		memory_not_cached = 0x04000000,
		memory_not_paged = 0x08000000,
		memory_shared = 0x10000000,
		memory_execute = 0x20000000,
		memory_read = 0x40000000,
		memory_write = 0x80000000
	};

	const bool is_writable() const noexcept
	{
		return this->has_flag_set(data_section_flags::memory_write);
	}
	const bool is_readable() const noexcept
	{
		return this->has_flag_set(data_section_flags::memory_read);
	}
	const bool is_executable() const noexcept
	{
		return this->has_flag_set(data_section_flags::memory_execute);
	}
	const bool is_discardable() const noexcept
	{
		return this->has_flag_set(data_section_flags::memory_discardable);
	}
	const bool has_flag_set(const data_section_flags& flag) const noexcept
	{
		return (this->raw_header().Characteristics & flag) == flag;
	}

	// SECTION INFORMATION
	const DWORD& virtual_address() const noexcept
	{
		return this->m_header.VirtualAddress;
	}
	const DWORD& virtual_size() const noexcept
	{
		return this->m_header.Misc.VirtualSize;
	}
	const DWORD& raw_data_offset() const noexcept
	{
		return this->m_header.PointerToRawData;
	}
	const DWORD& raw_size() const noexcept
	{
		return this->m_header.SizeOfRawData;
	}

	const IMAGE_SECTION_HEADER& raw_header() const noexcept
	{
		return this->m_header;
	}

private:
	IMAGE_SECTION_HEADER m_header;
};