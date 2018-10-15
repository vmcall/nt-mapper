#pragma once
#include "safe_handle.hpp"
#include "cast.hpp"

#include <windows.h>
#include <winternl.h>

namespace native
{
	class thread
	{
	public:
		thread(const HANDLE handle, const SYSTEM_THREAD_INFORMATION& info) :
			m_handle(handle),
			m_state(info.ThreadState), 
			m_wait_reason(info.WaitReason),
			m_start_address(reinterpret_cast<uintptr_t>(info.StartAddress)),
			m_thread_id(cast::pointer_convert<std::uint32_t>(info.ClientId.UniqueThread)) {}

		thread(const HANDLE handle) : 
			m_handle(handle),
			m_thread_id(GetThreadId(handle)) {}

		thread() : m_handle(nullptr) {}

		explicit operator bool()
		{
			return this->handle().unsafe_handle() != nullptr;
		}

		bool wait(const std::uint32_t max_time = 0);

		bool resume();
		bool suspend();

		// DOCUMENTED DATA
		std::uint32_t& thread_id();
		CONTEXT& context();
		safe_handle& handle();

		// UNDOCUMENTED DATA
		bool fetch();
		std::uint32_t& state();
		std::uint32_t& wait_reason();
		std::uintptr_t& start_address();


	private:
		bool get_context();
		bool set_context();

		CONTEXT m_context;
		safe_handle m_handle;
		std::uint32_t m_state;
		std::uint32_t m_thread_id;
		std::uint32_t m_wait_reason;
		std::uintptr_t m_start_address;
	};
}
