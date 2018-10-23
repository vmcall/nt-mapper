#pragma once
#include "map_context.hpp"
#include "process.hpp"


namespace injection::executors
{
	class hijack
	{
	public:
		bool handle_dllmain(const map_ctx& ctx, native::process& process) const noexcept;
	};

	class create 
	{
	public:
		bool handle_dllmain(const map_ctx& ctx, native::process& process) const noexcept;
	};
	
}