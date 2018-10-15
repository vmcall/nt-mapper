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


	// THIS IS SUPER UNDEFINED BEHAVIOUR BUT EYE-CANDY
	// FOR CONVERTING VOID* TO INTEGERS THROUGH TRUNCATION
	template <class T>
	__forceinline T pointer_convert(void* val)
	{
		return static_cast<T>(reinterpret_cast<uintptr_t>(val));
	}
}