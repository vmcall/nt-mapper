#include "stdafx.h"

std::vector<uint8_t> binary_file::read_file(const std::string& file_path)
{
	std::ifstream stream(file_path, std::ios::binary);

	stream.unsetf(std::ios::skipws);

	auto buffer = std::vector<uint8_t>();
	stream.seekg(0, std::ios::end);
	buffer.reserve(stream.tellg());
	stream.seekg(0, std::ios::beg);

	std::copy(std::istream_iterator<uint8_t>{stream}, {}, std::back_inserter(buffer));

	return buffer;
}
