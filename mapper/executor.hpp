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

		explicit executor(injection::executor::mode execution_mode) noexcept : m_mode(execution_mode) {}
		executor() : m_mode(UNSPECIFIED){}

		// ACCESSORS
		injection::executor::mode& execution_mode() noexcept;

		// SANTIY
		explicit operator bool() noexcept
		{
			return this->execution_mode() != injection::executor::mode::UNSPECIFIED;
		}

		// EXECUTION HANDLER
		bool handle(map_ctx& ctx, native::process& process) noexcept;

	private:
		bool handle_hijack(map_ctx& ctx, native::process& process) noexcept;
		bool handle_create(map_ctx& ctx, native::process& process) noexcept;

		injection::executor::mode m_mode;
	};
	
}