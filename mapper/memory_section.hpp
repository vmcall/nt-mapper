#pragma once
#include "safe_handle.hpp"
#include "ntdll.hpp"
#include "logger.hpp"

#include <windows.h>

class memory_section
{
public:
	explicit memory_section(std::uint32_t page_protection, std::uint64_t size) noexcept
	{
		LARGE_INTEGER large_size{};
		large_size.QuadPart = size;

		HANDLE new_handle;
		auto result = ntdll::NtCreateSection(&new_handle, GENERIC_ALL, 0, &large_size, page_protection, SEC_COMMIT, 0);

		if (!NT_SUCCESS(result))
		{
			logger::log_error("NtCreateSection failed");
			return;
		}

		this->m_handle = safe_handle(new_handle);
		this->m_protection = page_protection;
		this->m_size = size;
	}

	explicit operator bool() const noexcept
	{
		return static_cast<bool>(this->m_handle);
	}

	inline auto handle() const noexcept -> const safe_handle&
	{
		return this->m_handle;
	}

	inline auto protection() const noexcept -> std::uint32_t
	{
		return this->m_protection;
	}

	inline auto size() const noexcept -> std::uint64_t
	{
		return this->m_size;
	}

private:
	safe_handle m_handle;
	std::uint32_t m_protection;
	std::uint64_t m_size;
};