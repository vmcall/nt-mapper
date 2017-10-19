#include "stdafx.h"
#include "ntdll.hpp"

int main()
{
	ntdll::initialise();

	auto process_id = process::from_name("notepad.exe");
	logger::log_formatted("Target process id", process_id);
	
	auto proc = process(process_id, PROCESS_ALL_ACCESS);
	auto injector = injection::manualmap(proc);

	auto buffer = binary_file::read_file("D:\\Sync\\TestPEs\\DLLTEST_CONSOLE.dll");

	injector.inject(buffer);

	std::cin.get();

    return 0;
}

