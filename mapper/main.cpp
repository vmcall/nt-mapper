#include "ntdll.hpp"
#include "process.hpp"
#include "manualmapper.hpp"
#include "logger.hpp"
#include "binary_file.hpp"

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
	const auto address = mapper.inject(
		image.buffer(), 
		injection::executor::mode::HIJACK_THREAD);

	// PRINT INJECTED IMAGE ADDRESS
	logger::log_formatted("Injected buffer", address, true);

	std::cin.get();

	return 0;
}

