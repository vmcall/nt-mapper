#include "map_context.hpp"

const std::byte* map_ctx::pe_buffer() const noexcept
{
	return this->pe().get_buffer().data();
}

const portable_executable& map_ctx::pe() const noexcept
{
	return this->m_pe;
}

const std::string& map_ctx::image_name() const noexcept
{
	return this->m_image_name;
}

std::uintptr_t map_ctx::local_image() const noexcept
{
	return this->m_local_image;
}

std::uintptr_t map_ctx::remote_image() const noexcept
{
	return this->m_remote_image;
}

void map_ctx::set_local_image(std::uintptr_t image)
{
	this->m_local_image = image;
}

void map_ctx::set_remote_image(std::uintptr_t image)
{
	this->m_remote_image = image;
}
