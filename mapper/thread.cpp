#include "thread.hpp"
#include "ntdll.hpp"

bool native::thread::wait(const std::uint32_t max_time)
{
	return WaitForSingleObject(this->handle().unsafe_handle(), (max_time == 0x00) ? INFINITE : max_time) != WAIT_FAILED;
}

bool native::thread::resume()
{
	// UPDATE CONTEXT
	if (!this->set_context())
		return false;

	// RESUME
	return ResumeThread(this->handle().unsafe_handle()) != static_cast<DWORD>(0xFFFFFFFF);
}

bool native::thread::suspend()
{
	// SUSPEND THREAD
	const auto result = SuspendThread(this->handle().unsafe_handle()) != static_cast<DWORD>(0xFFFFFFFF);;

	// GET CONTEXT
	if (!this->get_context())
		return false;

	return result;
}

bool native::thread::fetch()
{

	ntdll::enumerate_threads([this](SYSTEM_THREAD_INFORMATION*) {
		if ()
	});

	return false;
}

std::uint32_t& native::thread::state()
{
	return this->m_state;
}

std::uint32_t& native::thread::wait_reason()
{
	return this->m_wait_reason;
}

std::uintptr_t& native::thread::start_address()
{
	return this->m_start_address;
}

std::uint32_t& native::thread::thread_id()
{
	return this->m_thread_id;
}

CONTEXT& native::thread::context()
{
	return this->m_context;
}

bool native::thread::get_context()
{
	return static_cast<bool>(GetThreadContext(this->handle().unsafe_handle(), &this->context()));
}

bool native::thread::set_context()
{
	return static_cast<bool>(SetThreadContext(this->handle().unsafe_handle(), &this->context()));
}

safe_handle& native::thread::handle()
{
	return this->m_handle;
}
