#include "executor.hpp"
#include "cast.hpp"
#include "logger.hpp"

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
	return false;
}

bool injection::executor::handle_create(map_ctx& ctx, native::process& process)
{
	// dllmain_call_x64.asm
	constexpr uint8_t shellcode[] = {
		0x48, 0x83, 0xEC, 0x28, 0x48, 0xB9,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x48, 0xC7, 0xC2, 0x01, 0x00, 0x00, 0x00,
		0x4D, 0x31, 0xC0, 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xD0, 0x48, 0x83, 0xC4, 0x28, 0xC3 };

	*cast::pointer(shellcode + 0x6) = ctx.remote_image;
	*cast::pointer(shellcode + 0x1A) = ctx.remote_image + ctx.pe.get_optional_header().AddressOfEntryPoint;

	auto remote_buffer = process.raw_allocate(sizeof(shellcode));
	if (!remote_buffer)
	{
		logger::log_error("Failed to allocate shellcode");
		return false;
	}

	auto success = true;
	do
	{
		if (!process.write_raw_memory(shellcode, sizeof(shellcode), remote_buffer))
		{
			logger::log_error("Failed to write shellcode");
			success = false;
			break;
		}

		auto thread_handle = safe_handle(process.create_thread(remote_buffer, NULL));
		if (!thread_handle)
		{
			logger::log_error("Failed to create shellcode thread");
			success = false;
			break;
		}

		if (WaitForSingleObject(thread_handle.get_handle(), INFINITE) == WAIT_FAILED)
		{
			logger::log_error("Failed to wait for shellcode thread");
			success = false;
		}

	} while (false);


	// FREE SHELLCODE
	process.free_memory(remote_buffer);

	return success;
}
