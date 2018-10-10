#pragma once
#include "map_context.hpp"
#include "process.hpp"

namespace injection
{
	class executor
	{
	public:
		enum mode
		{
			CREATE_THREAD,
			HIJACK_THREAD,
			UNSPECIFIED
		};

		executor(injection::executor::mode execution_mode) : m_mode(execution_mode) {}
		executor() : m_mode(UNSPECIFIED){}

		// ACCESSORS
		injection::executor::mode& execution_mode();

		// SANTIY
		explicit operator bool();

		// EXECUTION HANDLER
		bool handle(map_ctx& ctx, native::process& process);

	private:
		bool handle_hijack(map_ctx& ctx, native::process& process);
		bool handle_create(map_ctx& ctx, native::process& process);

		injection::executor::mode m_mode;
	};
	
}