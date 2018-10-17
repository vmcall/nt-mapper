#pragma once
#include <vector>
#include <string>

class binary_file
{
public:
	explicit binary_file(std::string_view file_path) noexcept;

	std::vector<std::byte>& buffer() noexcept;
private:
	std::vector<std::byte> m_buffer;
};