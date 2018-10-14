#pragma once
#include <windows.h>
#include <utility>

class safe_handle
{
public:
	safe_handle() : m_handle(nullptr) {}
	safe_handle(HANDLE new_handle) : m_handle(new_handle) {}
	~safe_handle() 
	{
		if (this->handle())
		{
			//logger::log_formatted("Closing", this->handle, true);
			CloseHandle(this->handle());
		}
	}


	// EXPLICITELY DELETE COPY FUNCTIONS TO PREVENT ANY 
	// COPYING OF THE HANDLE OBJECT

	// COPY CONSTRUCTOR
	safe_handle(const safe_handle& that) = delete;

	// COPY ASSIGNMENT OPERATOR
	safe_handle& operator= (const safe_handle& other) = delete;

	// MOVE CONSTRUCTOR
	safe_handle(safe_handle&& other) : m_handle(other.m_handle)
	{
		other.handle() = nullptr; // COPY TO NEW INSTANCE AND NULL OLD
	} 

	// MOVE ASSIGNMENT OPERATOR
	safe_handle& operator= (safe_handle&& other)
	{
		this->handle() = other.m_handle; // COPY TO NEW INSTANCE AND NULL OLD
		other.handle() = nullptr;
		return *this;
	}

	explicit operator bool()
	{
		return this->handle() != nullptr;
	}

	inline auto handle() -> HANDLE&
	{
		return this->m_handle;
	}

private:
	HANDLE m_handle;
};