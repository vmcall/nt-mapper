#include "executors.hpp"
#include "cast.hpp"
#include "logger.hpp"
#include "safe_memory.hpp"
#include "shellcode.hpp"

#include <thread>
#include <chrono>


bool injection::executors::hijack::handle_dllmain(const map_ctx& ctx, native::process& process) const noexcept
{
	// CREATE SHELLCODE FOR IMAGE
	const auto entrypoint_offset = ctx.pe().get_optional_header().AddressOfEntryPoint;
	auto shellcode = shellcode::hijack_dllmain(ctx.remote_image(), ctx.remote_image() + entrypoint_offset);

	// ALLOCATE AND INITIALISE MEMORY HANDLER (RAII)
	auto remote_buffer = safe_memory(
		&process,
		process.raw_allocate(shellcode.size()));

	logger::log_formatted("Shellcode", remote_buffer.memory(), true);

	// FAILED TO ALLOCATE?
	if (!remote_buffer)
	{
		logger::log_error("Failed to allocate shellcode");
		return false;
	}

	// FAILED TO WRITE?
	if (!process.write_raw_memory(shellcode.data(), remote_buffer.memory(), shellcode.size()))
	{
		logger::log_error("Failed to write shellcode");
		return false;
	}

	for (auto& thread : process.threads())
	{
		// FIND A THREAD THAT IS IDLE
		const auto is_waiting = thread.state() == native::thread::state_t::WAITING;

		const auto is_user_requested_delay =
			thread.wait_reason() == native::thread::wait_reason_t::USER_REQUEST || thread.wait_reason() == native::thread::wait_reason_t::WR_USER_REQUEST;

		if (!is_waiting || !is_user_requested_delay)
			continue;

		logger::log_formatted("Hijackable Thread", thread.thread_id(), false);

		// SUSPEND THREAD TO MODIFY IT'S CONTEXT
		if (!thread.suspend())
		{
			logger::log_error("Failed to suspend hijackable thread.");
			return false;
		}

		// ALLOCATE AND WRITE A OLD INSTRUCTION POINTER ON STACK
		thread.context().Rsp -= sizeof(std::uintptr_t);

		// ALIGN STACK TO DEFAULT 8-BYTE ALIGNMENT ?
		// thread.context().Rsp &= 0xFFFFFFFFFFFFFFF8;

		if (!process.write_memory(thread.context().Rip, thread.context().Rsp))
		{
			logger::log_error("Failed to write instruction pointer to stack.");
			return false;
		}

		// SET NEW INSTRUCTION POINTER
		thread.context().Rip = remote_buffer.memory();

		logger::log_formatted("New Stack pointer", thread.context().Rsp, true);
		logger::log_formatted("Instruction pointer", thread.context().Rip, true);
		logger::log_formatted("Entrypoint", ctx.remote_image() + entrypoint_offset, true);

		// RESUME THREAD TO RUN OUR SHELLCODE
		if (!thread.resume())
		{
			logger::log_error("Failed to resume hijackable thread.");
			return false;
		}

		// WAIT FOR THREAD TO FINISH
		// ...
		// AT THE END OF THE SHELLCODE, IT WRITES 0x1 TO SHELLCODE+6a TO ALERT EXECUTOR
		// OF IT'S SUCCESS :)
		const auto marker_address = remote_buffer.memory() + shellcode.size() - 8;
		for (std::uint8_t marker = 0x00; marker != 0x01; process.read_memory(&marker, marker_address))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		return true;
	}

	return false;
}

bool injection::executors::create::handle_dllmain(const map_ctx& ctx, native::process& process) const noexcept
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
	if (!process.write_raw_memory(shellcode.data(), remote_buffer.memory(), shellcode.size()))
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
	if (!thread.wait())
	{
		logger::log_error("Failed to wait for shellcode thread");
		return false;
	}

	// SUCCESSFULLY RAN SHELLCODE, MEMORY GET'S FREED BY HANDLER
	// WHEN IT GOES OUT OF SCOPE
	return true;
}
