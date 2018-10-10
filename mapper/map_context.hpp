#pragma once
#include <string>
#include <vector>

#include "portable_executable.hpp"

struct map_ctx
{
	std::string image_name;
	portable_executable pe;

	// UNDEFINED BEHAVIOUR
	union {
		uintptr_t local_image;
		void* local_image_void;

	};

	// UNDEFINED BEHAVIOUR
	union {
		uintptr_t remote_image;
		void* remote_image_void;
	};

	uint8_t* pe_buffer();

	map_ctx() : pe(std::vector<uint8_t>()), remote_image(0) {}
	map_ctx(std::string new_image_name, std::vector<uint8_t> new_buffer) :
		image_name(new_image_name), pe(new_buffer) {}
};
