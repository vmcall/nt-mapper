#pragma once
#include <string>
#include <vector>

#include "portable_executable.hpp"

class map_ctx
{
public:
	map_ctx() {}
	explicit map_ctx(std::string new_image_name, std::vector<std::byte> new_buffer) :
		m_image_name(new_image_name), m_pe(new_buffer) {}

	std::byte* pe_buffer();
	portable_executable& pe();
	std::string& image_name();
	std::uintptr_t& local_image();
	std::uintptr_t& remote_image();

private:
	std::string m_image_name;
	portable_executable m_pe;
	std::uintptr_t m_local_image;
	std::uintptr_t m_remote_image;
};
