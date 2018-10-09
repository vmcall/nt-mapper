#pragma once
#include <vector>
#include <string>

class binary_file
{
public:
	static std::vector<uint8_t> read_file(const std::string& file_path);
};