#define NOMINMAX

#include "ntdll.hpp"
#include "process.hpp"
#include "manualmapper.hpp"
#include "logger.hpp"
#include "binary_file.hpp"
#include "executors.hpp"

#include <limits>
#include <ios>

int main()
{
	// INTIALISE NATIVE FUNCTIONS
	ntdll::initialise();

	//logger::log("Process name:");
	//std::string process_name{};
	//std::cin >> process_name;

	const auto process_name = std::string("notepad.exe");

	// FIND PROCESS
	auto proc = native::process(process_name.c_str(), PROCESS_ALL_ACCESS);
	logger::log_formatted("Target process id", proc.get_id(), false);

	// READ BUFFER
	binary_file image("C:\\Users\\Admin\\Google Drive\\TestPEs\\DLLTEST64.dll");

	if (image.buffer().size() == 0x00)
	{
		logger::log_error("Failed to find binary file!");
		return 0;
	}

	// CREATE MAPPER INSTANCE
	auto mapper = injection::manualmapper(proc);

	// INJECT IMAGE INTO PROCESS
	const auto injected_ctx = mapper.inject(image.buffer());
	logger::log_formatted("Injected buffer", injected_ctx.remote_image(), true);

	// PATCH TERMINATEPROCESS
	// proc.write_memory(0xC3, reinterpret_cast<std::uintptr_t>(TerminateProcess));

	// CALL IT USING SPECIFIED EXECUTOR
	const auto executed = mapper.call(injected_ctx, injection::executors::create{});

	logger::log_formatted("Executed", executed, true);

	std::cin.clear();
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cin.get();

	return 0;
}

