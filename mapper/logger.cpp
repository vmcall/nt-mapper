#include "logger.hpp"

void logger::log(std::string_view message)
{
	std::cout << "[+] " << message << std::endl;
}

void logger::log_error(std::string_view message)
{
	std::cout << "[!] " << message << std::endl;
}
