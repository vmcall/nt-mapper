#pragma once
#include "process.hpp"
#include <string>

namespace injection
{
	class loadlibrary
	{
	public:
		explicit loadlibrary(native::process& proc) noexcept : process(proc) {}
		bool inject(std::string_view buffer) noexcept;

	protected:
		native::process& process;
	};
}
