#include "executor.hpp"
#include "cast.hpp"
#include "logger.hpp"
#include "compiler.hpp"
#include "safe_memory.hpp"
#include "shellcode.hpp"

injection::executor::mode& injection::executor::execution_mode()
{
	return this->m_mode;
}

injection::executor::operator bool()
{
	return this->execution_mode() != injection::executor::mode::UNSPECIFIED;
}

bool injection::executor::handle(map_ctx& ctx, native::process& process)
{
	switch (this->execution_mode())
	{
	case injection::executor::CREATE_THREAD:
		return this->handle_create(ctx, process);

	case injection::executor::HIJACK_THREAD:
		return this->handle_hijack(ctx, process);

	default:
		return false;
	}
}

bool injection::executor::handle_hijack(map_ctx& ctx, native::process& process)
{
	compiler::unreferenced_variable(process, ctx);

	return false;
}

bool injection::executor::handle_create(map_ctx& ctx, native::process& process)
{
	// CREATE SHELLCODE FOR IMAGE
	const auto entrypoint_offset = ctx.pe().get_optional_header().AddressOfEntryPoint;
	auto shellcode = shellcode::call_dllmain(ctx.remote_image(), ctx.remote_image() + entrypoint_offset);

	// ALLOCATE AND INITIALISE MEMORY HANDLER (RAII)
	auto remote_buffer = safe_memory(
		&process,
		process.raw_allocate(shellcode.size()));

	// FAILED TO ALLOCATE?
	if (!remote_buffer)
	{
		logger::log_error("Failed to allocate shellcode");
		return false;
	}

	// FAILED TO WRITE?
	if (!process.write_raw_memory(shellcode.data(), shellcode.size(), remote_buffer.memory()))
	{
		logger::log_error("Failed to write shellcode");
		return false;
	}

	// FAILED TO CREATE THREAD?
	auto thread = process.create_thread(remote_buffer.memory(), 0x00);
	if (!thread)
	{
		logger::log_error("Failed to create shellcode thread");
		return false;
	}

	// FAILED TO WAIT FOR THREAD?
	if (thread.wait(INFINITE) == WAIT_FAILED)
	{
		logger::log_error("Failed to wait for shellcode thread");
		return false;
	}

	// SUCCESSFULLY RAN SHELLCODE, MEMORY GET'S FREED BY HANDLER
	// WHEN IT GOES OUT OF SCOPE
	return true;
}
