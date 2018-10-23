#pragma once
#include "process.hpp"
#include "portable_executable.hpp"
#include "map_context.hpp"
#include "api_set.hpp"

#include <string>
#include <vector>
#include <unordered_map>

namespace injection
{
	class manualmapper
	{
	public:
		explicit manualmapper(native::process& proc) noexcept : m_process(proc) { }

		// INJECT IMAGE INTO PROCESS
		map_ctx inject(const std::vector<std::byte>& buffer) noexcept;

		template <typename Executor>
		bool call(const map_ctx& ctx, const Executor&& executor) noexcept
		{
			// EXECUTE THE IMAGE, HANDLED BY EXECUTION ENGINE
			return executor.handle_dllmain(ctx, this->process());
		}

	private:
		void write_headers(map_ctx& ctx) noexcept;
		bool map_image(map_ctx& ctx) noexcept;
		std::uintptr_t find_or_map_dependency(const std::string& image_name) noexcept;
		void write_image_sections(map_ctx& ctx) noexcept;
		void relocate_image_by_delta(map_ctx& ctx) noexcept;
		void fix_import_table(map_ctx& ctx) noexcept;
		native::process::module_export handle_forwarded_export(native::process::module_export& exported_function, api_set& api_schema) noexcept;

		native::process::module_list_t& linked_modules() noexcept;
		std::vector<map_ctx>& mapped_modules() noexcept;
		native::process& process() noexcept;

		native::process::module_list_t m_linked_modules;
		std::vector<map_ctx> m_mapped_modules;
		native::process& m_process;
	};
}