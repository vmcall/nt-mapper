#pragma once
#include "stdafx.h"

namespace logger
{
	inline void log(std::string message)
	{
		std::cout << "[+] " << message << std::endl;
	}
	inline void log_error(std::string message)
	{
		std::cout << "[!] " << message << std::endl;
	}

	template <class T>
	inline void log_formatted(std::string variable_name, T variable_data, bool hexadecimal = false)
	{
		auto format = hexadecimal ? std::hex : std::dec;
		std::cout << "[?] " << variable_name << ": " << format << variable_data << std::dec << std::endl;
	}
}