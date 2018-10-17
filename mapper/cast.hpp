#pragma once

#include <cstdint>

namespace cast
{
	template <class T>
	std::uintptr_t* long_pointer(T val) noexcept
	{
		// C-CAST DUE TO UNCERTAIN USE-CASES
		// THIS IS UNDEFINED BEHAVIOUR AND 
		// STUPID,  BUT IT LOOKS REALLY GOOD 
		// :)
		return (std::uintptr_t*)(val);
	}

	template <class T>
	std::uint32_t* int_pointer(T val) noexcept
	{
		// C-CAST DUE TO UNCERTAIN USE-CASES
		// THIS IS UNDEFINED BEHAVIOUR AND 
		// STUPID,  BUT IT LOOKS REALLY GOOD 
		// :)
		return (std::uint32_t*)(val);
	}


	// THIS IS SUPER UNDEFINED BEHAVIOUR BUT EYE-CANDY
	// FOR CONVERTING VOID* TO INTEGERS THROUGH TRUNCATION
	template <class T>
	T pointer_convert(void* val) noexcept
	{
		return static_cast<T>(reinterpret_cast<std::uintptr_t>(val));
	}
}