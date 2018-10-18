#include "ntdll.hpp"
#include "process.hpp"
#include "manualmapper.hpp"
#include "logger.hpp"
#include "binary_file.hpp"
#include "executors.hpp"

int main()
{
	// INTIALISE NATIVE FUNCTIONS
	ntdll::initialise();

	// FIND PROCESS
	auto proc = native::process("notepad.exe", PROCESS_ALL_ACCESS);
	logger::log_formatted("Target process id", proc.get_id(), false);

	// READ BUFFER
	binary_file image("D:\\Sync\\TestPEs\\DLLTEST64.dll");

	// CREATE MAPPER INSTANCE
	auto mapper = injection::manualmapper(proc);

	// INJECT IMAGE INTO PROCESS
	const auto injected_ctx = mapper.inject(image.buffer());
	logger::log_formatted("Injected buffer", injected_ctx.remote_image(), true);

	// CALL IT USING SPECIFIED EXECUTOR
	const auto executed = mapper.call(injected_ctx, injection::executors::hijack{});

	std::cin.get();

	return 0;
}

