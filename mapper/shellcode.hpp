#pragma once

#include <array>

namespace shellcode
{
	constexpr size_t call_dllmain_size = 41;
	std::array<std::byte, call_dllmain_size> call_dllmain(uintptr_t image, uintptr_t dllmain);
}