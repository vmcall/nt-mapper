#pragma once

#include <cstdint>

namespace cast
{
	template <class T>
	__forceinline uintptr_t* pointer(T val)
	{
		// C-CAST DUE TO UNCERTAIN USE-CASES
		// THIS IS UNDEFINED BEHAVIOUR AND 
		// STUPID,  BUT IT LOOKS REALLY GOOD 
		// :)
		return (uintptr_t*)(val);
	}
}