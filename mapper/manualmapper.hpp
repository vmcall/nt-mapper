#pragma once
#include "process.hpp"
#include "portable_executable.hpp"
#include "map_context.hpp"
#include "executor.hpp"

#include <string>
#include <vector>
#include <locale>
#include <codecvt>
#include <unordered_map>

#pragma warning(disable:4996) // DEPRECATED LIBRARY :(
using wstring_converter = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>;

namespace injection
{
	class manualmapper
	{
	public:
		manualmapper(native::process& proc) : m_process(proc) { }
		uintptr_t inject(const std::vector<uint8_t>& buffer, injection::executor::mode execution_mode);

		using module_list = std::unordered_map<std::string, uintptr_t>;
		module_list& linked_modules();
		std::vector<map_ctx> mapped_modules();
		native::process& process();
		injection::executor& execution_engine();

	private:
		bool map_image(map_ctx& ctx);
		uintptr_t find_or_map_dependency(const std::string& image_name);
		void write_headers(map_ctx& ctx);
		bool call_entrypoint(map_ctx& ctx);
		void write_image_sections(map_ctx& ctx);
		void relocate_image_by_delta(map_ctx& ctx);
		void fix_import_table(map_ctx& ctx);

		module_list m_linked_modules;
		std::vector<map_ctx> m_mapped_modules;
		native::process& m_process;
		injection::executor m_executor;
	};
}