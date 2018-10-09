#pragma once
#include "safe_handle.hpp"
#include <cstdint>

class memory_section
{
public:
	memory_section(uint32_t page_protection, uint64_t size);
	safe_handle handle;
	uint32_t protection;
	uint64_t size;

	explicit operator bool();
};