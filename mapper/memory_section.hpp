#pragma once
#include "safe_handle.hpp"
#include <cstdint>

class memory_section
{
public:
	explicit memory_section(std::uint32_t page_protection, std::uint64_t size) noexcept;
	safe_handle handle;
	std::uint32_t protection;
	std::uint64_t size;

	explicit operator bool() noexcept
	{
		return static_cast<bool>(this->handle);
	}
};