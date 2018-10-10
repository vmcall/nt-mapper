#include "map_context.hpp"

std::uint8_t* map_ctx::pe_buffer()
{
	return this->pe.get_buffer().data();
}
