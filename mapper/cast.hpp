#pragma once

#include <cstdint>

namespace cast
{
	template <class T>
	uintptr_t* pointer(T val)
	{
		return (uintptr_t*)(val);
	}
}