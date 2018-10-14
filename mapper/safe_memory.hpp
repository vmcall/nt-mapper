#pragma once
#include <windows.h>
#include <utility>

#include "process.hpp"
#include "logger.hpp"

class safe_memory
{
public:
	safe_memory() {}
	safe_memory(native::process* process, uintptr_t memory) : m_process(process), m_memory(memory) {}
	~safe_memory()
	{
		// IF VALID COPY OF SAFE_MEMORY: FREE MEMORY
		if (this->operator bool())
		{
			logger::log_formatted("Freeing", this->memory(), true);
			this->process()->free_memory(this->memory());
		}
	}


	// EXPLICITELY DELETE COPY FUNCTIONS TO PREVENT ANY 
	// COPYING OF THE HANDLE OBJECT

	// COPY CONSTRUCTOR
	safe_memory(const safe_memory& that) = delete;

	// COPY ASSIGNMENT OPERATOR
	safe_memory& operator= (const safe_memory& other) = delete;

	// MOVE CONSTRUCTOR
	safe_memory(safe_memory&& other) : m_process(other.m_process), m_memory(other.m_memory)
	{
		other.memory() = 0x00; // NULL OLD
	}

	// MOVE ASSIGNMENT OPERATOR
	safe_memory& operator= (safe_memory&& other) 
	{
		this->process() = other.process();
		this->memory() = other.memory();
		other.memory() = 0x00; // NULL OLD
		return *this;
	}

	explicit operator bool()
	{
		return this->memory() != 0x00;
	}

	inline auto memory() -> uintptr_t&
	{
		return this->m_memory;
	}
	inline auto process() -> native::process*&
	{
		return this->m_process;
	}

	inline void detach()
	{
		this->memory() = 0x00;
	}

private:
	uintptr_t m_memory;
	native::process* m_process;
};