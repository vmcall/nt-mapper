#pragma once

#include <algorithm>
#include <string>

// USED TO TRANSFORM CHARACTERS IN STRINGS, /W4 COMPLIANT :)
namespace transformer
{
	namespace detail
	{
		__forceinline char character_to_lower(char character)
		{
			return static_cast<char>(tolower(character));
		}
		__forceinline wchar_t wide_character_to_lower(wchar_t character)
		{
			return static_cast<wchar_t>(towlower(character));
		}
	}

	__forceinline void string_to_lower(std::string& data)
	{
		std::transform(data.begin(), data.end(), data.begin(), transformer::detail::character_to_lower);
	}
	__forceinline void string_to_lower(std::wstring& data)
	{
		std::transform(data.begin(), data.end(), data.begin(), transformer::detail::wide_character_to_lower);
	}
	__forceinline void string_to_lower(std::wstring& data, std::size_t size)
	{
		std::transform(data.begin(), data.begin() + size, data.begin(), transformer::detail::wide_character_to_lower);
	}
	__forceinline void truncate(std::string& data)
	{
		data.erase(std::find(data.begin(), data.end(), '\0'), data.end());
	}
}