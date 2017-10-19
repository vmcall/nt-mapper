#include "stdafx.h"
#include "memory_section.hpp"
#include "ntdll.hpp"

memory_section::memory_section(uint32_t page_protection, uint64_t size)
{
	HANDLE new_handle;
	LARGE_INTEGER large_size;
	large_size.QuadPart = size;

	auto result = ntdll::NtCreateSection(&new_handle, GENERIC_ALL, 0, &large_size, page_protection, SEC_COMMIT, 0);

	if (!NT_SUCCESS(result))
	{
		logger::log_error("NtCreateSection failed");
		return;
	}

	this->handle = safe_handle(new_handle);
	this->protection = page_protection;
	this->size = size;
}

memory_section::operator bool()
{
	return static_cast<bool>(this->handle);
}
