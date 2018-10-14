#include "process.hpp"
#include "ntdll.hpp"
#include "logger.hpp"
#include "transformer.hpp"
#include <Psapi.h>
#include <algorithm>

native::process native::process::current_process()
{
	return process(reinterpret_cast<HANDLE>(-1));
}

uint32_t native::process::id_from_name(const std::string& process_name)
{
	DWORD process_list[516], bytes_needed;
	if (EnumProcesses(process_list, sizeof(process_list), &bytes_needed))
	{
		for (size_t index = 0; index < bytes_needed / sizeof(uint32_t); index++)
		{
			auto proc = process(process_list[index], PROCESS_ALL_ACCESS);

			if (!proc)
				continue;

			if (process_name == proc.get_name())
				return process_list[index];
		}
	}

	return 0;
}

MEMORY_BASIC_INFORMATION native::process::virtual_query(const uintptr_t address)
{
	MEMORY_BASIC_INFORMATION mbi;

	VirtualQueryEx(this->handle().unsafe_handle(), reinterpret_cast<LPCVOID>(address), &mbi, sizeof(MEMORY_BASIC_INFORMATION));

	return mbi;
}

uintptr_t native::process::raw_allocate(const SIZE_T virtual_size, const uintptr_t address)
{
	return reinterpret_cast<uintptr_t>(
		VirtualAllocEx(this->handle().unsafe_handle(), reinterpret_cast<LPVOID>(address), virtual_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE)
		);
}

bool native::process::free_memory(const uintptr_t address)
{
	return VirtualFreeEx(this->handle().unsafe_handle(), reinterpret_cast<LPVOID>(address), NULL, MEM_RELEASE);
}


bool native::process::read_raw_memory(const void* buffer, const uintptr_t address, const SIZE_T size)
{
	return ReadProcessMemory(
		this->handle().unsafe_handle(),
		reinterpret_cast<LPCVOID>(address),
		const_cast<void*>(buffer),
		size,
		nullptr);
}

bool native::process::write_raw_memory(const void* buffer, const SIZE_T size, const uintptr_t address)
{
	return WriteProcessMemory(
		this->handle().unsafe_handle(),
		reinterpret_cast<LPVOID>(address),
		buffer,
		size,
		nullptr);
}

bool native::process::virtual_protect(const uintptr_t address, uint32_t protect, uint32_t* old_protect)
{
	return VirtualProtectEx(
		this->handle().unsafe_handle(),
		reinterpret_cast<LPVOID>(address),
		0x1000,
		protect,
		reinterpret_cast<PDWORD>(old_protect));
}

uintptr_t native::process::map(memory_section& section)
{
	void* base_address = nullptr;
	SIZE_T view_size = section.size;

	auto result = ntdll::NtMapViewOfSection(
		section.handle.unsafe_handle(),
		this->handle().unsafe_handle(),
		&base_address,
		NULL, NULL, NULL,
		&view_size,
		2, 0,
		section.protection);

	if (!NT_SUCCESS(result))
	{
		logger::log_error("NtMapViewOfSection failed");
		logger::log_formatted("Error code", result, true);
	}

	return reinterpret_cast<uintptr_t>(base_address);
}

HWND native::process::get_main_window()
{
	// SETUP CONTAINER
	using window_data_t = std::pair<std::int32_t, HWND>;
	window_data_t window_data = window_data_t{ this->get_id(), 0 };

	logger::log_formatted("Process id", window_data.first, false);

	// ENUMERATE WINDOWS TO FIND 
	EnumWindows([](HWND handle, LPARAM param) -> BOOL
	{
		auto data = reinterpret_cast<window_data_t*>(param);

		logger::log_formatted("Handle", handle, false);

		std::int32_t process_id = 0;
		if (!GetWindowThreadProcessId(handle, reinterpret_cast<DWORD*>(&process_id)) || process_id != data->first)
			return TRUE; // CONTINUE

		SetLastError(static_cast<DWORD>(-1));
		data->second = handle;
		return FALSE;
	}, reinterpret_cast<LPARAM>(&window_data));

	logger::log_formatted("Main", window_data.second, false);

	// RETURN WINDOW
	return window_data.second;
}

std::int32_t native::process::get_id()
{
	return GetProcessId(this->handle().unsafe_handle());
}

std::unordered_map<std::string, uintptr_t> native::process::get_modules()
{
	auto result = std::unordered_map<std::string, uintptr_t>();

	HMODULE module_handles[1024];
	DWORD size_needed;

	if (!EnumProcessModules(this->handle().unsafe_handle(), module_handles, sizeof(module_handles), &size_needed))
		return result;

	for (auto module_index = 0; module_index < size_needed / sizeof(HMODULE); module_index++)
	{
		// INITIALISE STRING OF SIZE MAX_PATH (260)
		std::string module_name(MAX_PATH, '0');

		// GET MODULE NAME
		GetModuleBaseNameA(
			this->handle().unsafe_handle(),
			module_handles[module_index],
			const_cast<char*>(module_name.c_str()),
			static_cast<DWORD>(module_name.size()));

		// MAKE CHARACTERS LOWERCASE
		transformer::string_to_lower(module_name);

		// TRUNCATE SIZE TO NULL TERMINATOR
		transformer::truncate(module_name);

		// SET ENTRY
		result[module_name] = reinterpret_cast<uintptr_t>(module_handles[module_index]);
	}

	return result;
}

std::string native::process::get_name()
{
	char buffer[MAX_PATH];
	GetModuleBaseNameA(this->handle().unsafe_handle(), nullptr, buffer, MAX_PATH);

	return std::string(buffer);
}

// thx blackbone :)
uintptr_t native::process::get_module_export(uintptr_t module_handle, const char* function_ordinal)
{
	IMAGE_DOS_HEADER dos_header;
	IMAGE_NT_HEADERS64 nt_header;
	this->read_memory(&dos_header, module_handle);
	this->read_memory(&nt_header, module_handle + dos_header.e_lfanew);

	auto export_base = nt_header.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	auto export_base_size = nt_header.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
	if (!export_base) // CONTAINS EXPORTED FUNCTIONS
	{
		logger::log_error("Module does not contain exports!");
		return 0x00;
	}

	std::unique_ptr<IMAGE_EXPORT_DIRECTORY> export_data_raw(reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(malloc(export_base_size)));
	auto export_data = export_data_raw.get();

	// READ EXPORTED DATA FROM TARGET PROCESS FOR LATER PROCESSING
	if (!this->read_raw_memory(export_data, module_handle + export_base, export_base_size))
		logger::log_error("failed to read export data");

	// BLACKBONE PASTE, NEVER EXPERIENCED THIS BUT WHO KNOWS?
	if (export_base_size <= sizeof(IMAGE_EXPORT_DIRECTORY))
	{
		export_base_size = static_cast<DWORD>(export_data->AddressOfNameOrdinals - export_base
			+ max(export_data->NumberOfFunctions, export_data->NumberOfNames) * 255);

		export_data_raw.reset(reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(malloc(export_base_size)));
		export_data = export_data_raw.get();

		if (!this->read_raw_memory(export_data, module_handle + export_base, export_base_size))
			logger::log_error("failed to read export data");
	}

	// GET DATA FROM READ MEMORY
	auto delta = reinterpret_cast<uintptr_t>(export_data) - export_base;
	auto address_of_ordinals = reinterpret_cast<WORD*>(export_data->AddressOfNameOrdinals + delta);
	auto address_of_names = reinterpret_cast<DWORD*>(export_data->AddressOfNames + delta);
	auto address_of_functions = reinterpret_cast<DWORD*>(export_data->AddressOfFunctions + delta);

	// NO EXPORTED FUNCTIONS? DID WE FUCK UP?
	if (export_data->NumberOfFunctions <= 0)
		logger::log_error("No exports found!");

	auto ptr_function_ordinal = reinterpret_cast<uintptr_t>(function_ordinal);

	// ITERATE EXPORTED FUNCTIONS
	for (size_t export_index = 0; export_index < export_data->NumberOfFunctions; export_index++)
	{
		WORD ordinal;
		std::string function_name;
		auto is_import_by_ordinal = ptr_function_ordinal <= 0xFFFF;

		// GET EXPORT INFORMATION
		ordinal = static_cast<WORD>(is_import_by_ordinal ? export_index : address_of_ordinals[export_index]);
		function_name = reinterpret_cast<char*>(address_of_names[export_index] + delta);

		// IS IT THE FUNCTION WE ASKED FOR?
		auto found_via_ordinal =
			is_import_by_ordinal &&
			static_cast<WORD>(ptr_function_ordinal) == (ordinal + export_data->Base);
		auto found_via_name =
			!is_import_by_ordinal && function_name == function_ordinal;

		if (!found_via_ordinal && !found_via_name)
		{
			continue;
			//logger::log_error("Export not found!");
			//return 0;
		}

		// FORWARDED EXPORT?
		// IF FUNCTION POINTER IS INSIDE THE EXPORT DIRECTORY, IT IS *NOT* A FUNCTION POINTER!
		// FUCKING SHIT MSVCPxxx
		// FUCK YOU
		const auto function_pointer = module_handle + address_of_functions[ordinal];
		const auto directory_start = module_handle + export_base;
		const auto directory_end = module_handle + export_base + export_base_size;
		if (function_pointer >= directory_start && function_pointer <= directory_end)
		{
			// READ STRING
			std::uint8_t forwarded_name[255] = { 0 };
			this->read_raw_memory(forwarded_name, function_pointer, sizeof(forwarded_name));

			// PARSE STRING
			std::string forward(reinterpret_cast<char*>(forwarded_name));
			std::string library_name = forward.substr(0, forward.find(".")) + ".dll";
			function_name = forward.substr(forward.find(".") + 1, function_name.npos);

			// LOWERCASE THANKS
			transformer::string_to_lower(library_name);

			// FIND FORWARDED MODULE
			auto modules = this->get_modules();
			auto search = modules.find(library_name);
			if (search != modules.end())
			{
				// RECURSIVELY HANDLE FORWARDED MODULE :)
				return this->get_module_export(search->second, function_name.c_str());
			}

			// TODO: HANDLE THIS? WHO CARES
			logger::log_error("Forwarded module not loaded"); 
		}

		return function_pointer;
	}

	logger::log_error("Export not found!");
	return 0;
}

native::thread native::process::create_thread(const uintptr_t address, const uintptr_t argument)
{
	const auto casted_function = reinterpret_cast<LPTHREAD_START_ROUTINE>(address);
	const auto casted_argument = reinterpret_cast<LPVOID>(argument);

	const auto thread_handle = CreateRemoteThread(this->handle().unsafe_handle(), nullptr, 0, casted_function, casted_argument, 0, nullptr);

	return native::thread(thread_handle);
}

safe_handle& native::process::handle()
{
	return this->m_handle;
}
