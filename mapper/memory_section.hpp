#pragma once
#include "safe_handle.hpp"
#include <cstdint>

class memory_section
{
public:
	memory_section(std::uint32_t page_protection, std::uint64_t size);
	safe_handle handle;
	std::uint32_t protection;
	std::uint64_t size;

	explicit operator bool()
	{
		return static_cast<bool>(this->handle);
	}
};