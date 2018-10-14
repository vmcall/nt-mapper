#include "thread.hpp"

std::uint32_t native::thread::wait(const std::uint32_t max_time)
{
	return WaitForSingleObject(this->handle().unsafe_handle(), max_time);
}

safe_handle& native::thread::handle()
{
	return this->m_handle;
}
