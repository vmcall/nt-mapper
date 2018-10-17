#pragma once

#include <array>
#include <type_traits>

namespace maker
{
	template<typename... T>
	__forceinline std::array<std::byte, sizeof...(T)> create_byte_array(T&&... bytes) noexcept {
		return { std::byte(bytes)... };
	}
}