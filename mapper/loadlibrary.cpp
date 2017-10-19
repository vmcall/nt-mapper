#include "stdafx.h"
#include "loadlibrary.hpp"
#include "process.hpp"
#include "logger.hpp"


bool injection::loadlibrary::inject(std::string buffer)
{
	// ALLOCATE LIBRARY PATH TO PROCESS MEMORY
	auto path_pointer = this->process.raw_allocate(buffer.length());
	if (!path_pointer)
	{
		logger::log_error("Failed to allocate memory!");
		return false;
	}

	// WRITE LIBRARY PATH TO PROCESS MEMORY
	if (!this->process.write_raw_memory(buffer.data(), buffer.length(), path_pointer))
	{
		logger::log_error("Failed to write library path!");
		return false;
	}

	// FIND LOADLIBRARY FUNCTION POINTER
	auto module_handle = GetModuleHandle(L"kernel32.dll");
	auto function_pointer = reinterpret_cast<uintptr_t>(GetProcAddress(module_handle, "LoadLibraryA"));

	// EXECUTE LOADLIBRARY IN REMOTE PROCESS
	auto thread_handle = this->process.create_thread(function_pointer, path_pointer);

	if (!thread_handle)
	{
		logger::log_error("Failed to create thread!");
		return false;
	}

	// WAIT FOR THREAD TO FINISH
	WaitForSingleObject(thread_handle, INFINITE);

	// FREE LIBRARY PATH FROM PROCESS MEMORY
	this->process.free_memory(path_pointer);

	return false;
}
