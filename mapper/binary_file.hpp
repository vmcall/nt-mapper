#pragma once
#include <vector>
#include <string>

class binary_file
{
public:
	explicit binary_file(std::string_view file_path);

	std::vector<std::byte>& buffer();
private:
	std::vector<std::byte> m_buffer;
};