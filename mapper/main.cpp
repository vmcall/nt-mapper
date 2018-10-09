#include "ntdll.hpp"
#include "process.hpp"
#include "manualmap.hpp"
#include "logger.hpp"
#include "binary_file.hpp"

int main()
{
	ntdll::initialise();

	auto process_id = process::from_name("notepad.exe");
	logger::log_formatted("Target process id", process_id);
	
	auto proc = process(process_id, PROCESS_ALL_ACCESS);
	auto injector = injection::manualmap(proc);

	auto buffer = binary_file::read_file("D:\\unsanitized\\r6s_dll\\x64\\Release\\r6s_dll.dll");

	auto address = injector.inject(buffer);

	logger::log_formatted("Injected buffer", address, true);

	std::cin.get();

    return 0;
}

