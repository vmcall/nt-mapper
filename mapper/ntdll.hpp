#pragma once
#include <Windows.h>
#include <winternl.h>


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
	void initialise();
	extern NtCreateSection_t NtCreateSection;
	extern NtMapViewOfSection_t NtMapViewOfSection;
	extern NtQuerySystemInformation_t NtQuerySystemInformation;
}