#pragma once

#include <array>

namespace shellcode
{
	constexpr std::size_t call_dllmain_size = 41;
	std::array<std::byte, call_dllmain_size> call_dllmain(std::uintptr_t image, std::uintptr_t dllmain) noexcept;

	constexpr std::size_t hijack_dllmain_size = 114;
	std::array<std::byte, hijack_dllmain_size> hijack_dllmain(std::uintptr_t image, std::uintptr_t dllmain) noexcept;
}