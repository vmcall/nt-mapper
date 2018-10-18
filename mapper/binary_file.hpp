#pragma once
#include <vector>
#include <string>

class binary_file
{
public:
	explicit binary_file(std::string_view file_path);

	const std::vector<std::byte>& buffer() const noexcept;
private:
	std::vector<std::byte> m_buffer;
};