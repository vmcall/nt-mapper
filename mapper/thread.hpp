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
			return this->handle().unsafe_handle() != nullptr;
		}

		bool wait(const std::uint32_t max_time = 0);

		bool resume();
		bool suspend();

		std::uint32_t state();

		CONTEXT& context();
		safe_handle& handle();

	private:
		bool get_context();
		bool set_context();

		safe_handle m_handle;
		CONTEXT m_context;
	};
}
