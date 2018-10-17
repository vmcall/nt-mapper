#pragma once
#include <string>
#include <vector>

#include "portable_executable.hpp"

class map_ctx
{
public:
	map_ctx() noexcept {}
	explicit map_ctx(std::string new_image_name, std::vector<std::byte> new_buffer) noexcept :
		m_image_name(new_image_name), m_pe(new_buffer) {}

	std::byte* pe_buffer() noexcept;
	portable_executable& pe() noexcept;
	std::string& image_name() noexcept;
	std::uintptr_t& local_image() noexcept;
	std::uintptr_t& remote_image() noexcept;

private:
	std::string m_image_name;
	portable_executable m_pe;
	std::uintptr_t m_local_image;
	std::uintptr_t m_remote_image;
};
