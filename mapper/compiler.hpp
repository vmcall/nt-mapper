#pragma once

// USED TO LABEL VARIABLES AS UNREFERENCED FOR TEMPORARY /W4 COMPLIANCE
namespace compiler
{
	namespace detail
	{
		template <typename T>
		constexpr void unreferenced_variable(T& val)
		{
			static_cast<void>(val);
		}

		template <typename T, typename ...Rest>
		constexpr void unreferenced_variable(T& val, Rest&... rest)
		{
			compiler::detail::unreferenced_variable(val);
			compiler::detail::unreferenced_variable(rest...);
		}
	}

	template <typename ...Rest>
	constexpr void unreferenced_variable(Rest&... rest)
	{
		compiler::detail::unreferenced_variable(rest...);
	}
}