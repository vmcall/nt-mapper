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
	logger::log_formatted("Target process id", process_id);
	auto proc = native::process(process_id, PROCESS_ALL_ACCESS);

	// READ BUFFER
	const auto buffer = binary_file::read_file("D:\\Sync\\TestPEs\\DLLTEST_CONSOLE.dll");
	const auto address = injection::manualmapper(proc).inject(buffer, injection::executor::mode::CREATE_THREAD);

	logger::log_formatted("Injected buffer", address, true);

	std::cin.get();

    return 0;
}

