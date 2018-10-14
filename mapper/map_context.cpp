#include "map_context.hpp"

std::byte* map_ctx::pe_buffer()
{
	return this->pe().get_buffer().data();
}

portable_executable& map_ctx::pe()
{
	return this->m_pe;
}

std::string& map_ctx::image_name()
{
	return this->m_image_name;
}

uintptr_t& map_ctx::local_image()
{
	return this->m_local_image;
}

uintptr_t& map_ctx::remote_image()
{
	return this->m_remote_image;
}
