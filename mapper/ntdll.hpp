#pragma once
#include "logger.hpp"

#include <Windows.h>
#include <winternl.h>
#include <memory>

using NtCreateSection_t = NTSTATUS(NTAPI*)(
	PHANDLE sectionHandle,
	ULONG                DesiredAccess,
	POBJECT_ATTRIBUTES   ObjectAttributes OPTIONAL,
	PLARGE_INTEGER       MaximumSize OPTIONAL,
	ULONG                PageAttributess,
	ULONG                SectionAttributes,
	HANDLE               FileHandle OPTIONAL);

using NtMapViewOfSection_t = NTSTATUS(NTAPI*)(
	HANDLE          SectionHandle,
	HANDLE          ProcessHandle,
	PVOID           *BaseAddress,
	ULONG_PTR       ZeroBits,
	SIZE_T          CommitSize,
	PLARGE_INTEGER  SectionOffset,
	PSIZE_T         ViewSize,
	int InheritDisposition,
	ULONG           AllocationType,
	ULONG           Win32Protect);

using NtQuerySystemInformation_t = NTSTATUS(NTAPI*)(
	SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	ULONG* ReturnLength);

namespace ntdll
{
	void initialise() noexcept;
	extern NtCreateSection_t NtCreateSection;
	extern NtMapViewOfSection_t NtMapViewOfSection;
	extern NtQuerySystemInformation_t NtQuerySystemInformation;


	// NATIVE ENUMERATORS
	template <class Fn>
	void enumerate_processes(Fn callback) noexcept
	{
		constexpr auto size_mismatch = 0xC0000004;
		constexpr auto process_type = SystemProcessInformation;

		// QUERY SIZE
		auto allocation = std::make_unique<std::byte[]>(0x1);
		std::uint32_t size_needed = 0;
		const auto size_needed_pointer = reinterpret_cast<DWORD*>(&size_needed);
		if (ntdll::NtQuerySystemInformation(process_type, allocation.get(), 0x1, size_needed_pointer) != size_mismatch)
		{
			logger::log_error("NtQuerySystemInformation undefined behaviour.");
			return;
		}

		allocation = std::make_unique<std::byte[]>(size_needed);
		auto info = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(allocation.get());

		// QUERY DATA
		if (auto result = ntdll::NtQuerySystemInformation(process_type, info, size_needed, size_needed_pointer);
			result != 0x00)
		{
			logger::log_error("NtQuerySystemInformation failed - SYSTEM_PROCESS_INFORMATION");
			logger::log_formatted("Result", result, true);
			return;
		}

		for (
			auto info_casted = reinterpret_cast<std::uintptr_t>(info);
			info->NextEntryOffset;
			info = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(info_casted + info->NextEntryOffset),
			info_casted = reinterpret_cast<std::uintptr_t>(info))
		{
			if (callback(info))
				break;
		}
	}

	template <class Fn>
	void enumerate_threads(Fn callback) noexcept
	{
		ntdll::enumerate_processes([=](SYSTEM_PROCESS_INFORMATION* info) {
			const auto info_casted = reinterpret_cast<std::uintptr_t>(info);
			const auto thread_info = reinterpret_cast<SYSTEM_THREAD_INFORMATION*>(info_casted + sizeof(SYSTEM_PROCESS_INFORMATION));
			for (std::uint32_t thread_index = 0; thread_index < info->NumberOfThreads; ++thread_index)
			{
				if (callback(&thread_info[thread_index]))
					return true;
			}

			return false;
		});
	}
}