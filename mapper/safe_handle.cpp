#include "stdafx.h"
#include "safe_handle.hpp"

safe_handle::~safe_handle()
{
	if (this->handle)
	{
		//logger::log_formatted("Closing", this->handle, true);
		CloseHandle(this->handle);
	}
}

void safe_handle::set_handle(HANDLE new_handle)
{
	this->handle = new_handle;
}

HANDLE safe_handle::get_handle()
{
	return this->handle;
}

safe_handle::operator bool()
{
	return this->handle != nullptr;
}
