#pragma once
#include "safe_handle.hpp"

#include <windows.h>
#include <cstdint>

namespace native
{
	class thread
	{
	public:
		thread(HANDLE handle) : m_handle(handle) {}
		thread() : m_handle(nullptr) {}

		explicit operator bool()
		{
			return static_cast<bool>(this->handle().unsafe_handle());
		}

		std::uint32_t wait(const std::uint32_t max_time);

		safe_handle& handle();


	private:
		safe_handle m_handle;
	};
}
