#pragma once
#include "stdafx.h"
#include "process.hpp"

namespace injection
{
	class loadlibrary
	{
	public:
		loadlibrary(process& proc) : process(proc) {}
		bool inject(std::string buffer);

	protected:
		process& process;
	};
}
