#pragma once
#include <string>
#include <vector>

#include "portable_executable.hpp"

class map_ctx
{
public:
	map_ctx() noexcept {}
	explicit map_ctx(std::string_view new_image_name, const std::vector<std::byte>& new_buffer) noexcept :
		m_image_name(new_image_name), m_pe(new_buffer) {}

	const std::byte* pe_buffer() const noexcept;
	const portable_executable& pe() const noexcept;
	const std::string& image_name() const noexcept;
	std::uintptr_t local_image() const noexcept;
	std::uintptr_t remote_image() const noexcept;

	void set_local_image(std::uintptr_t image);
	void set_remote_image(std::uintptr_t image);

private:
	const std::string m_image_name;
	portable_executable m_pe;
	std::uintptr_t m_local_image;
	std::uintptr_t m_remote_image;
};
