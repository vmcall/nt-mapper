#include "loadlibrary.hpp"
#include "process.hpp"
#include "logger.hpp"
#include "safe_memory.hpp"


bool injection::loadlibrary::inject(std::string_view buffer) noexcept
{
	// ALLOCATE AND INITIALISE MEMORY HANDLER (RAII)
	auto remote_buffer = safe_memory(
		&process,
		process.raw_allocate(buffer.size()));

	// FAILED TO ALLOCATE?
	if (!remote_buffer)
	{
		logger::log_error("Failed to allocate path");
		return false;
	}

	// FAILED TO WRITE?
	if (!process.write_raw_memory(buffer.data(), remote_buffer.memory(), buffer.length()))
	{
		logger::log_error("Failed to write path");
		return false;
	}

	// FIND LOADLIBRARY
	const auto module_handle = GetModuleHandle(L"kernel32.dll");
	const auto function_pointer = reinterpret_cast<std::uintptr_t>(GetProcAddress(module_handle, "LoadLibraryA"));

	// FAILED TO CREATE THREAD?
	auto thread = process.create_thread(function_pointer, remote_buffer.memory());
	if (!thread)
	{
		logger::log_error("Failed to create loadlibrary thread");
		return false;
	}

	// FAILED TO WAIT FOR THREAD?
	if (!thread.wait())
	{
		logger::log_error("Failed to wait for loadlibrary thread");
		return false;
	}

	return true;
}
