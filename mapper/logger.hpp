#pragma once
#include <iostream>
#include <string_view>

namespace logger
{
	void log(std::string_view message);
	void log_error(std::string_view message);

	template <class T>
	__forceinline void log_formatted(std::string_view variable_name, const T& variable_data, const bool hexadecimal)
	{
		std::cout << "[?] " << variable_name << ": ";
		std::cout << (hexadecimal ? std::hex : std::dec);
		std::cout << variable_data << std::dec;
		std::cout << std::endl;
	}
}