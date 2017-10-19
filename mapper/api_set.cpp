#include "stdafx.h"
#include "api_set.hpp"

api_set::api_set()
{
	auto peb = reinterpret_cast<uintptr_t>(NtCurrentTeb()->ProcessEnvironmentBlock);

	auto api_set = *reinterpret_cast<API_SET_NAMESPACE_ARRAY**>(peb + 0x68);

	for (size_t entry_index = 0; entry_index < api_set->count; ++entry_index)
	{
		auto descriptor = api_set->entry(entry_index);

		wchar_t dll_name[MAX_PATH] = { 0 };
		api_set->read_name(descriptor, dll_name);
		std::transform(dll_name, dll_name + MAX_PATH, dll_name, ::tolower);

		auto host_data = api_set->get_host(descriptor);

		std::vector<std::wstring> hosts;
		for (auto j = 0; j < host_data->count; j++)
		{
			auto host = host_data->entry(api_set, j);

			std::wstring host_name(reinterpret_cast<wchar_t*>(reinterpret_cast<uint8_t*>(api_set) + host->value_offset),
				host->value_length / sizeof(wchar_t));

			if (!host_name.empty())
			{
				//wprintf(L"%s - %s\n", dll_name, host_name.c_str());
				hosts.push_back(host_name);
			}
		}

		this->schema.emplace(dll_name, hosts);
	}
}

bool api_set::query(std::wstring& name)
{
	// SEARCH FOR ANY ENTRIES OF OUR PROXY DLL
	auto iter = std::find_if(this->schema.begin(), this->schema.end(), [name](const map_api_schema::value_type& val)
	{
		return name.find(val.first.c_str()) != name.npos;
	});

	if (iter != this->schema.end()) // FOUND
	{
		name = iter->second.front() != name ? iter->second.front() : iter->second.back();
		return true;
	}

	return false;
}
