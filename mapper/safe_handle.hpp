#pragma once
#include <windows.h>
#include <utility>

class safe_handle
{
public:
	safe_handle() : m_handle(nullptr) {}
	explicit safe_handle(HANDLE new_handle) noexcept : m_handle(new_handle) {}
	~safe_handle() noexcept
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
		other.invalidate();
	} 

	// MOVE ASSIGNMENT OPERATOR
	safe_handle& operator= (safe_handle&& other) noexcept 
	{
		this->m_handle = other.m_handle; // COPY TO NEW INSTANCE AND NULL OLD
		other.invalidate();
		return *this;
	}

	explicit operator bool() const noexcept
	{
		return this->unsafe_handle() != nullptr;
	}

	inline auto unsafe_handle() const noexcept -> const HANDLE&
	{
		return this->m_handle;
	}

	inline auto invalidate() noexcept -> void
	{
		this->m_handle = nullptr;
	}

private:
	HANDLE m_handle;
};