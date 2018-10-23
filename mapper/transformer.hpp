#pragma once

#include <algorithm>
#include <string>

// USED TO TRANSFORM CHARACTERS IN STRINGS, /W4 COMPLIANT :)
namespace transformer
{
	namespace detail
	{
		inline char character_to_lower(char character) noexcept
		{
			return static_cast<char>(tolower(character));
		}
		inline wchar_t wide_character_to_lower(wchar_t character) noexcept
		{
			return static_cast<wchar_t>(towlower(character));
		}
	}

	inline void string_to_lower(std::string& data) noexcept
	{
		std::transform(data.begin(), data.end(), data.begin(), transformer::detail::character_to_lower);
	}
	inline void string_to_lower(std::wstring& data) noexcept
	{
		std::transform(data.begin(), data.end(), data.begin(), transformer::detail::wide_character_to_lower);
	}
	inline void string_to_lower(std::wstring& data, std::size_t size) noexcept
	{
		std::transform(data.begin(), data.begin() + size, data.begin(), transformer::detail::wide_character_to_lower);
	}
	inline void truncate(std::string& data) noexcept
	{
		data.erase(std::find(data.begin(), data.end(), '\0'), data.end());
	}
	inline void truncate(std::wstring& data) noexcept
	{
		data.erase(std::find(data.begin(), data.end(), '\0'), data.end());
	}
}