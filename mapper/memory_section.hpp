#pragma once
#include "stdafx.h"
#include "process.hpp"

class memory_section
{
public:
	memory_section(uint32_t page_protection, uint64_t size);
	safe_handle handle;
	uint32_t protection;
	uint64_t size;

	explicit operator bool();
};