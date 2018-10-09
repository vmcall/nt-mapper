#pragma once
#include <windows.h>
#include <utility>

class safe_handle
{
public:
	safe_handle() : handle(nullptr) {}
	safe_handle(HANDLE new_handle) : handle(new_handle) {}
	~safe_handle();

	// COPY CONSTRUCTOR
	safe_handle(const safe_handle& that) = delete;

	// COPY ASSIGNMENT OPERATOR
	safe_handle& operator= (const safe_handle& other) = delete;

	// MOVE CONSTRUCTOR
	safe_handle(safe_handle&& other)
		: handle(std::move(other.handle)) {
		other.handle = nullptr; // COPY TO NEW INSTANCE AND NULL OLD
	} 

	// MOVE ASSIGNMENT OPERATOR
	safe_handle& operator= (safe_handle&& other)
		//: handle(std::move(other.handle)) 
	{
		this->handle = std::move(other.handle); // COPY TO NEW INSTANCE AND NULL OLD
		other.handle = nullptr;
		return *this;
	}
	
	void set_handle(HANDLE handle);
	HANDLE get_handle();

	explicit operator bool(); 

private:
	HANDLE handle;
};