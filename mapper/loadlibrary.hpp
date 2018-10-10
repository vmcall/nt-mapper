#pragma once
#include "process.hpp"
#include <string>

namespace injection
{
	class loadlibrary
	{
	public:
		loadlibrary(native::process& proc) : process(proc) {}
		bool inject(std::string& buffer);

	protected:
		native::process& process;
	};
}
