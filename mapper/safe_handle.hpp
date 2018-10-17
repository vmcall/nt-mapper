#pragma once
#include <windows.h>
#include <utility>

class safe_handle
{
public:
	safe_handle() : m_handle(nullptr) {}
	explicit safe_handle(HANDLE new_handle) noexcept : m_handle(new_handle) {}
	~safe_handle() 
	{
		if (this->unsafe_handle())
		{
			//logger::log_formatted("Closing", this->handle, true);
			CloseHandle(this->unsafe_handle());
		}
	}


	// EXPLICITELY DELETE COPY FUNCTIONS TO PREVENT ANY 
	// COPYING OF THE HANDLE OBJECT

	// COPY CONSTRUCTOR
	safe_handle(const safe_handle& that) = delete;

	// COPY ASSIGNMENT OPERATOR
	safe_handle& operator= (const safe_handle& other) = delete;

	// MOVE CONSTRUCTOR
	safe_handle(safe_handle&& other) noexcept : m_handle(other.m_handle)
	{
		other.unsafe_handle() = nullptr; // COPY TO NEW INSTANCE AND NULL OLD
	} 

	// MOVE ASSIGNMENT OPERATOR
	safe_handle& operator= (safe_handle&& other) noexcept
	{
		this->unsafe_handle() = other.m_handle; // COPY TO NEW INSTANCE AND NULL OLD
		other.unsafe_handle() = nullptr;
		return *this;
	}

	explicit operator bool() noexcept
	{
		return this->unsafe_handle() != nullptr;
	}

	inline auto unsafe_handle() noexcept -> HANDLE&
	{
		return this->m_handle;
	}

private:
	HANDLE m_handle;
};