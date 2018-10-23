#include "transformer.hpp"
#include "process.hpp"
#include "logger.hpp"
#include "ntdll.hpp"
#include "cast.hpp"

#include <algorithm>
#include <tlhelp32.h>
#include <Psapi.h>
#include <array>

native::process native::process::current_process() noexcept
{
	return process(reinterpret_cast<HANDLE>(-1));
}

std::uint32_t native::process::id_from_name(std::string_view process_name) noexcept
{
	DWORD process_list[516], bytes_needed;
	if (K32EnumProcesses(process_list, sizeof(process_list), &bytes_needed))
	{
		for (size_t index = 0; index < bytes_needed / sizeof(std::uint32_t); index++)
		{
			auto proc = process(process_list[index], PROCESS_ALL_ACCESS);

			if (proc && process_name == proc.get_name())
				return process_list[index];
		}
	}

	logger::log_error("Process not found");
	return 0x00;
}

MEMORY_BASIC_INFORMATION native::process::virtual_query(const std::uintptr_t address) const noexcept
{
	MEMORY_BASIC_INFORMATION mbi;

	VirtualQueryEx(this->handle().unsafe_handle(), reinterpret_cast<LPCVOID>(address), &mbi, sizeof(MEMORY_BASIC_INFORMATION));

	return mbi;
}

std::uintptr_t native::process::raw_allocate(const SIZE_T virtual_size, const std::uintptr_t address) noexcept
{
	return reinterpret_cast<std::uintptr_t>(
		VirtualAllocEx(this->handle().unsafe_handle(), reinterpret_cast<LPVOID>(address), virtual_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE)
		);
}

bool native::process::free_memory(const std::uintptr_t address) noexcept
{
	return VirtualFreeEx(this->handle().unsafe_handle(), reinterpret_cast<LPVOID>(address), NULL, MEM_RELEASE);
}


bool native::process::read_raw_memory(const void* buffer, const std::uintptr_t address, const std::size_t size) const noexcept
{
	return ReadProcessMemory(
		this->handle().unsafe_handle(),
		reinterpret_cast<LPCVOID>(address),
		const_cast<void*>(buffer),
		size,
		nullptr);
}

bool native::process::write_raw_memory(const void* buffer, const std::uintptr_t address, const std::size_t size) noexcept
{
	return WriteProcessMemory(
		this->handle().unsafe_handle(),
		reinterpret_cast<LPVOID>(address),
		buffer,
		size,
		nullptr);
}

bool native::process::virtual_protect(
	const std::uintptr_t address, 
	const std::uint32_t protect,
	std::uint32_t* old_protect, 
	const std::size_t page_size) noexcept
{
	return VirtualProtectEx(
		this->handle().unsafe_handle(),
		reinterpret_cast<LPVOID>(address),
		page_size,
		protect,
		reinterpret_cast<PDWORD>(old_protect));
}

std::uintptr_t native::process::map(const memory_section& section) noexcept
{
	void* base_address = nullptr;
	SIZE_T view_size = section.size();

	auto result = ntdll::NtMapViewOfSection(
		section.handle().unsafe_handle(),
		this->handle().unsafe_handle(),
		&base_address,
		NULL, NULL, NULL,
		&view_size,
		2, 0,
		section.protection());

	if (!NT_SUCCESS(result))
	{
		logger::log_error("NtMapViewOfSection failed");
		logger::log_formatted("Error code", result, true);
	}

	return reinterpret_cast<std::uintptr_t>(base_address);
}

HWND native::process::get_main_window() const noexcept
{
	// SETUP CONTAINER
	using window_data_t = std::pair<std::uint32_t, HWND>;
	window_data_t window_data = window_data_t{ this->get_id(), 0x00 };

	logger::log_formatted("Process id", window_data.first, false);

	// ENUMERATE WINDOWS TO FIND 
	EnumWindows([](HWND handle, LPARAM param) -> BOOL
	{
		auto data = reinterpret_cast<window_data_t*>(param);

		logger::log_formatted("Handle", handle, false);

		std::uint32_t process_id = 0;
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

std::uint32_t native::process::get_id() const noexcept
{
	return GetProcessId(this->handle().unsafe_handle());
}

native::process::module_list_t native::process::get_modules() const noexcept
{
	native::process::module_list_t result{};
	//std::array<HMODULE, 200> modules{};
	//
	//std::uint32_t size_needed;
	//if (!EnumProcessModulesEx(
	//	this->handle().unsafe_handle(),
	//	modules.data(), 
	//	static_cast<DWORD>(modules.size()),
	//	reinterpret_cast<DWORD*>(&size_needed), LIST_MODULES_ALL))
	//{
	//	return result;
	//}
	//
	//for (auto module_index = 0; module_index < size_needed / sizeof(HMODULE); module_index++)
	//{
	//	// INITIALISE STRING OF SIZE MAX_PATH (260)
	//	std::string module_name(MAX_PATH, '\00');
	//
	//	// GET MODULE NAME
	//	GetModuleBaseNameA(
	//		this->handle().unsafe_handle(),
	//		modules.at(module_index),
	//		const_cast<char*>(module_name.c_str()),
	//		static_cast<DWORD>(module_name.size()));
	//
	//	// MAKE CHARACTERS LOWERCASE
	//	transformer::string_to_lower(module_name);
	//
	//	// TRUNCATE SIZE TO NULL TERMINATOR
	//	transformer::truncate(module_name);
	//
	//	// SET ENTRY
	//	result[module_name] = reinterpret_cast<std::uintptr_t>(modules.at(module_index));
	//}


	const auto snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, this->get_id());

	MODULEENTRY32 module_entry{};
	module_entry.dwSize = sizeof(MODULEENTRY32);

	if (!Module32FirstW(snapshot_handle, &module_entry))
	{
		logger::log_error("Failed to get module");
		return result;
	}

	auto success = TRUE;

	wstring_converter_t converter;

	for (; success; success = Module32NextW(snapshot_handle, &module_entry))
	{
		// INITIALISE STRING OF SIZE MAX_PATH (260)
		std::wstring module_name(module_entry.szModule);

		// MAKE CHARACTERS LOWERCASE
		transformer::string_to_lower(module_name);

		// TRUNCATE SIZE TO NULL TERMINATOR
		transformer::truncate(module_name);

		// CONVERT TO STRING
		const auto narrow_string = converter.to_bytes(module_name);

		// SET ENTRY
		result[narrow_string] = reinterpret_cast<std::uintptr_t>(module_entry.modBaseAddr);
	}

	return result;
}

std::string native::process::get_name() const noexcept
{
	char buffer[MAX_PATH];
	GetModuleBaseNameA(this->handle().unsafe_handle(), nullptr, buffer, MAX_PATH);

	return std::string(buffer);
}

native::process::module_export native::process::get_module_export(std::uintptr_t module_handle, const char* function_ordinal) const noexcept
{
	IMAGE_DOS_HEADER dos_header;
	IMAGE_NT_HEADERS64 nt_header;
	this->read_memory(&dos_header, module_handle);
	this->read_memory(&nt_header, module_handle + dos_header.e_lfanew);

	auto export_base = nt_header.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	auto export_base_size = nt_header.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
	if (export_base == 0x00 || export_base_size == 0x00) // CONTAINS EXPORTED FUNCTIONS?
	{
		logger::log_error("Module does not contain exports!");
		return native::process::module_export(0x00);
	}

	auto export_data_raw = std::make_unique<std::byte[]>(export_base_size);
	auto export_data = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(export_data_raw.get());

	// READ EXPORTED DATA FROM TARGET PROCESS FOR LATER PROCESSING
	if (!this->read_raw_memory(export_data, module_handle + export_base, export_base_size))
		logger::log_error("failed to read export data");

	// BLACKBONE HAD THIS ... NEVER EXPERIENCED THIS BUT WHO KNOWS?
	if (export_base_size <= sizeof(IMAGE_EXPORT_DIRECTORY))
	{
		export_base_size = static_cast<std::uint32_t>(export_data->AddressOfNameOrdinals - export_base
			+ max(export_data->NumberOfFunctions, export_data->NumberOfNames) * 255);

		// UPDATE EXPORT DATA
		export_data_raw = std::make_unique<std::byte[]>(export_base_size);
		export_data = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(export_data_raw.get());

		if (!this->read_raw_memory(export_data, module_handle + export_base, export_base_size))
			logger::log_error("failed to read export data");
	}

	// NO EXPORTED FUNCTIONS? DID WE FUCK UP?
	if (export_data->NumberOfFunctions <= 0)
		logger::log_error("No exports found!");

	// GET DATA FROM READ MEMORY
	const auto delta = reinterpret_cast<std::uintptr_t>(export_data) - export_base;
	const auto address_of_ordinals = reinterpret_cast<std::uint16_t*>(export_data->AddressOfNameOrdinals + delta);
	const auto address_of_names = reinterpret_cast<std::uint32_t*>(export_data->AddressOfNames + delta);
	const auto address_of_functions = reinterpret_cast<std::uint32_t*>(export_data->AddressOfFunctions + delta);

	// ITERATE EXPORTED FUNCTIONS
	const auto ptr_function_ordinal = reinterpret_cast<std::uintptr_t>(function_ordinal);
	for (size_t export_index = 0; export_index < export_data->NumberOfFunctions; export_index++)
	{
		const auto is_import_by_ordinal = ptr_function_ordinal <= 0xFFFF;

		// GET EXPORT INFORMATION
		const auto ordinal = static_cast<std::uint16_t>(is_import_by_ordinal ? export_index : address_of_ordinals[export_index]);
		std::string function_name = reinterpret_cast<char*>(address_of_names[export_index] + delta);

		// IS IT THE FUNCTION WE ASKED FOR?
		const auto found_via_ordinal =
			is_import_by_ordinal &&
			static_cast<std::uint16_t>(ptr_function_ordinal) == (ordinal + export_data->Base);

		const auto found_via_name =
			!is_import_by_ordinal && function_name == function_ordinal;

		if (!found_via_ordinal && !found_via_name)
			continue;

		// FORWARDED EXPORT?
		// IF FUNCTION POINTER IS INSIDE THE EXPORT DIRECTORY, IT IS *NOT* A FUNCTION POINTER!
		// FUCKING SHIT MSVCPxxx
		// FUCK YOU
		const auto function_pointer = module_handle + address_of_functions[ordinal];
		const auto directory_start = module_handle + export_base;
		const auto directory_end = module_handle + export_base + export_base_size;
		if (function_pointer >= directory_start && function_pointer <= directory_end)
		{
			// READ FORWARD
			std::byte forwarded_name[50] = {};
			this->read_raw_memory(forwarded_name, function_pointer, sizeof(forwarded_name));

			// PARSE FUNCTION NAME
			std::string forward(reinterpret_cast<char*>(forwarded_name));
			function_name = forward.substr(forward.find(".") + 1, function_name.npos);

			// PARSE LIBRARY NAME
			auto library_name = forward.substr(0, forward.find(".")) + ".dll";
			transformer::string_to_lower(library_name);

			// FIND FORWARDED MODULE
			auto modules = this->get_modules();
			const auto search = modules.find(library_name);
			if (search == modules.end())
			{
				logger::log_error("Forwarded module not loaded.");
				return native::process::module_export(library_name, function_name);
			}

			// RECURSIVELY HANDLE FORWARDED MODULE :)
			return this->get_module_export(search->second, function_name.c_str());
		}

		return native::process::module_export(function_pointer);
	}

	logger::log_error("Export not found!");
	return native::process::module_export(0x00);
}

native::thread native::process::create_thread(const std::uintptr_t address, const std::uintptr_t argument) noexcept
{
	const auto casted_function = reinterpret_cast<LPTHREAD_START_ROUTINE>(address);
	const auto casted_argument = reinterpret_cast<LPVOID>(argument);

	const auto thread_handle = CreateRemoteThread(
		this->handle().unsafe_handle(), 
		nullptr, 0x00, 
		casted_function, casted_argument,
		0x00, nullptr);

	return native::thread(thread_handle);
}

std::vector<native::thread> native::process::threads() const noexcept
{
	std::vector<native::thread> thread_list{};

	const auto current_pid = this->get_id();

	ntdll::enumerate_threads([=, &thread_list](SYSTEM_THREAD_INFORMATION* info) 
	{
		if (cast::pointer_convert<std::uint32_t>(info->ClientId.UniqueProcess) != current_pid)
			return false;

		const auto this_thread = info->ClientId.UniqueThread;
		auto handle = OpenThread(THREAD_ALL_ACCESS, false, cast::pointer_convert<std::uint32_t>(this_thread));

		if (handle == INVALID_HANDLE_VALUE)
		{
			logger::log_error("Failed to open handle to thread.");
			logger::log_formatted("Thread Id", this_thread, true);
			return false;
		}

		thread_list.emplace_back(handle, *info);
		return false;
	});

	return thread_list;
}

const safe_handle& native::process::handle() const noexcept
{
	return this->m_handle;
}
