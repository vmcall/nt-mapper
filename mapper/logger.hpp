#pragma once
#include "stdafx.h"

namespace logger
{
	inline void log(const std::string& message)
	{
		std::cout << "[+] " << message << std::endl;
	}
	inline void log_error(const std::string& message)
	{
		std::cout << "[!] " << message << std::endl;
	}

	template <class T>
	inline void log_formatted(const std::string& variable_name, const T& variable_data, bool hexadecimal = false)
	{
		auto format = hexadecimal ? std::hex : std::dec;
		std::cout << "[?] " << variable_name << ": " << format << variable_data << std::dec << std::endl;
	}
}