#pragma once
#include "process.hpp"
#include <string>

namespace injection
{
	class loadlibrary
	{
	public:
		loadlibrary(process& proc) : process(proc) {}
		bool inject(std::string& buffer);

	protected:
		process& process;
	};
}
