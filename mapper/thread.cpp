#include "thread.hpp"
#include "ntdll.hpp"

bool native::thread::wait(const std::uint32_t max_time) noexcept
{
	return WaitForSingleObject(this->handle().unsafe_handle(), (max_time == 0x00) ? INFINITE : max_time) != WAIT_FAILED;
}

bool native::thread::resume() noexcept
{
	// UPDATE CONTEXT
	if (!this->set_context())
		return false;

	// RESUME
	return ResumeThread(this->handle().unsafe_handle()) != 0xFFFFFFFF;
}

bool native::thread::suspend() noexcept
{
	// SUSPEND THREAD
	if (SuspendThread(this->handle().unsafe_handle()) == 0xFFFFFFFF)
		return false;

	// GET CONTEXT
	if (!this->get_context())
		return false;

	return true;
}

bool native::thread::fetch() noexcept
{
	ntdll::enumerate_threads([this](SYSTEM_THREAD_INFORMATION* thread_info) {

		const auto this_thread_id = cast::pointer_convert<std::uint32_t>(thread_info->ClientId.UniqueThread);

		if (this_thread_id != this->thread_id())
			return false;

		// FETCH
		this->state() = static_cast<native::thread::state_t>(thread_info->ThreadState);
		this->wait_reason() = static_cast<native::thread::wait_reason_t>(thread_info->WaitReason);
		this->start_address() = reinterpret_cast<std::uintptr_t>(thread_info->StartAddress);

		return true;
	});

	return false;
}

native::thread::state_t& native::thread::state() noexcept
{
	return this->m_state;
}

native::thread::wait_reason_t& native::thread::wait_reason() noexcept
{
	return this->m_wait_reason;
}

std::uintptr_t& native::thread::start_address() noexcept
{
	return this->m_start_address;
}

std::uint32_t& native::thread::thread_id() noexcept
{
	return this->m_thread_id;
}

CONTEXT& native::thread::context() noexcept
{
	return this->m_context;
}

bool native::thread::get_context() noexcept
{
	// SPECIFY ALL REGISTERS
	this->m_context = CONTEXT();
	this->m_context.ContextFlags = CONTEXT_FULL;

	return static_cast<bool>(GetThreadContext(this->handle().unsafe_handle(), &this->m_context));
}

bool native::thread::set_context() noexcept
{
	return static_cast<bool>(SetThreadContext(this->handle().unsafe_handle(), &this->m_context));
}

safe_handle& native::thread::handle() noexcept
{
	return this->m_handle;
}
