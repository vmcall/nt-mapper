#include "binary_file.hpp"

#include <vector>
#include <fstream>

binary_file::binary_file(std::string_view file_path) noexcept
{
	// CREATE FILE STREAM TO SPECIFIED PATH
	std::ifstream stream(file_path.data(), std::ios::binary);

	// SKIP WHITESPACES
	stream.unsetf(std::ios::skipws);

	// FIND LENGTH
	stream.seekg(0, std::ios::end);			// GO TO END OF STREAM
	const auto length = stream.tellg();		// STORE STREAM POSITION
	stream.seekg(0, std::ios::beg);			// GO TO BEGINNING OF STREAM

	// CREATE BUFFER
	this->buffer() = std::vector<std::byte>(length);

	// COPY BUFFER FROM STREAM TO VECTOR
	stream.read(reinterpret_cast<char*>(this->buffer().data()), length);
}

std::vector<std::byte>& binary_file::buffer() noexcept
{
	return this->m_buffer;
}
