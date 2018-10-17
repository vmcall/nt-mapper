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
		explicit thread(const HANDLE handle, const SYSTEM_THREAD_INFORMATION& info) noexcept :
			m_handle(handle),
			m_state(static_cast<state_t>(info.ThreadState)), 
			m_wait_reason(static_cast<wait_reason_t>(info.WaitReason)),
			m_start_address(reinterpret_cast<std::uintptr_t>(info.StartAddress)),
			m_thread_id(cast::pointer_convert<std::uint32_t>(info.ClientId.UniqueThread)) {}

		explicit thread(const HANDLE handle) noexcept :
			m_handle(handle),
			m_thread_id(GetThreadId(handle)) {}

		thread() noexcept : m_handle(nullptr) {}

		explicit operator bool() noexcept
		{
			return this->handle().unsafe_handle() != nullptr;
		}

		bool wait(const std::uint32_t max_time = 0) noexcept;

		bool resume() noexcept;
		bool suspend() noexcept;

		// DOCUMENTED DATA
		std::uint32_t& thread_id() noexcept;
		CONTEXT& context() noexcept;
		safe_handle& handle() noexcept;

		// UNDOCUMENTED DATA
		enum wait_reason_t : std::uint8_t
		{
			EXECUTIVE,
			FREE_PAGE,
			PAGE_IN, 
			POOL_ALLOCATION,
			DELAY_EXECUTION,
			SUSPENDED,
			USER_REQUEST,
			WR_EXECUTIVE,
			WR_FREE_PAGE,
			WR_PAGE_IN,
			WR_POOL_ALLOCATION,
			WR_DELAY_EXECUTION,
			WR_SUSPENDED,
			WR_USER_REQUEST
		};
		enum state_t : std::uint8_t
		{
			INITIALIZED,
			READY,
			RUNNING,
			STANDBY,
			TERMINATED,
			WAITING,
			TRANSITION,
			DEFERRED_READY,
			GATE_WAIT
		};

		bool fetch() noexcept;
		state_t& state() noexcept;
		wait_reason_t& wait_reason() noexcept;
		std::uintptr_t& start_address() noexcept;


	private:
		bool get_context() noexcept;
		bool set_context() noexcept;

		state_t m_state;
		CONTEXT m_context;
		safe_handle m_handle;
		std::uint32_t m_thread_id;
		wait_reason_t m_wait_reason;
		std::uintptr_t m_start_address;
	};
}
