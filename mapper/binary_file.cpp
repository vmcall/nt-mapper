#include "binary_file.hpp"
#include "logger.hpp"

#include <vector>
#include <fstream>

binary_file::binary_file(std::string_view file_path)
{
	// CREATE FILE STREAM TO SPECIFIED PATH
	std::ifstream stream(file_path.data(), std::ios::binary);

	// SKIP WHITESPACES
	stream.unsetf(std::ios::skipws);

	// FIND LENGTH
	stream.seekg(0, std::ios::end);			// GO TO END OF STREAM
	const auto length = stream.tellg();		// STORE STREAM POSITION
	stream.seekg(0, std::ios::beg);			// GO TO BEGINNING OF STREAM

	// FILE NOT FOUND
	if (length == -1)
		return;

	// CREATE BUFFER
	this->m_buffer = std::vector<std::byte>(length);

	// COPY BUFFER FROM STREAM TO VECTOR
	stream.read(reinterpret_cast<char*>(this->m_buffer.data()), length);
}

const std::vector<std::byte>& binary_file::buffer() const noexcept
{
	return this->m_buffer;
}
