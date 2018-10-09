#pragma once

#include <string>
#include <algorithm>

namespace transformer
{
	__forceinline void to_lower(std::string& data)
	{
		std::transform(data.begin(), data.end(), data.begin(), ::tolower);
	}
	__forceinline void truncate(std::string& data)
	{
		data.erase(std::find(data.begin(), data.end(), '\0'), data.end());
	}
}