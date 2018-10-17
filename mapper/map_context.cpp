#include "map_context.hpp"

std::byte* map_ctx::pe_buffer() noexcept
{
	return this->pe().get_buffer().data();
}

portable_executable& map_ctx::pe() noexcept
{
	return this->m_pe;
}

std::string& map_ctx::image_name() noexcept
{
	return this->m_image_name;
}

std::uintptr_t& map_ctx::local_image() noexcept
{
	return this->m_local_image;
}

std::uintptr_t& map_ctx::remote_image() noexcept
{
	return this->m_remote_image;
}
