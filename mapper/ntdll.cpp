#include "stdafx.h"
#include "ntdll.hpp"

fnNtCreateSection ntdll::NtCreateSection = nullptr;
fnNtMapViewOfSection ntdll::NtMapViewOfSection = nullptr;

void ntdll::initialise()
{
	auto module_handle = GetModuleHandle(L"ntdll.dll");
	ntdll::NtCreateSection = reinterpret_cast<fnNtCreateSection>(GetProcAddress(module_handle, "NtCreateSection"));
	ntdll::NtMapViewOfSection = reinterpret_cast<fnNtMapViewOfSection>(GetProcAddress(module_handle, "NtMapViewOfSection"));

}
