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
	const auto process_id = native::process::id_from_name("notepad.exe");
	logger::log_formatted("Target process id", process_id, false);
	auto proc = native::process(process_id, PROCESS_ALL_ACCESS);

	// READ BUFFER
	binary_file image("D:\\Sync\\TestPEs\\DLLTEST64.dll");

	// CREATE MAPPER INSTANCE
	auto mapper = injection::manualmapper(proc);

	// INJECT IMAGE INTO PROCESS
	const auto address = mapper.inject(
		image.buffer(), 
		injection::executor::mode::CREATE_THREAD);

	// PRINT INJECTED IMAGE ADDRESS
	logger::log_formatted("Injected buffer", address, true);

	std::cin.get();

	return 0;
}

