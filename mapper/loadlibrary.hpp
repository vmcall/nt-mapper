#pragma once
#include "process.hpp"
#include <string>

namespace injection
{
	class loadlibrary
	{
	public:
		explicit loadlibrary(native::process& proc) : process(proc) {}
		bool inject(std::string_view buffer);

	protected:
		native::process& process;
	};
}
