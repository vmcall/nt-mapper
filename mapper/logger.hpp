#pragma once
#include <iostream>
#include <string_view>

namespace logger
{
	void log(std::string_view message) noexcept;
	void log_error(std::string_view message) noexcept;

	template <class T>
	void log_formatted(std::string_view variable_name, const T& variable_data, const bool hexadecimal) noexcept
	{
		std::cout << "[?] " << variable_name << ": ";
		std::cout << (hexadecimal ? std::hex : std::dec);
		std::cout << variable_data << std::dec;
		std::cout << std::endl;
	}
}