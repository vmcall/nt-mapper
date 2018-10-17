#pragma once
#include <Windows.h>
#include <unordered_map>

struct API_SET_VALUE_ENTRY
{
	ULONG flags;
	ULONG name_offset;
	ULONG name_length;
	ULONG value_offset;
	ULONG value_length;
};

struct API_SET_VALUE_ARRAY
{
	ULONG flags;
	ULONG name_offset;
	ULONG unknown;
	ULONG name_length;
	ULONG data_offset;
	ULONG count;

	inline API_SET_VALUE_ENTRY* entry(void* api_set, SIZE_T index) noexcept
	{
		return reinterpret_cast<API_SET_VALUE_ENTRY*>(reinterpret_cast<uintptr_t>(api_set) + data_offset + index * sizeof(API_SET_VALUE_ENTRY));
	}
};

struct API_SET_NAMESPACE_ENTRY
{
	ULONG limit;
	ULONG size;
};

struct API_SET_NAMESPACE_ARRAY
{
	ULONG version;
	ULONG size;
	ULONG flags;
	ULONG count;
	ULONG start;
	ULONG end;
	ULONG unknown[2];

	inline API_SET_NAMESPACE_ENTRY* entry(SIZE_T index) noexcept
	{
		return reinterpret_cast<API_SET_NAMESPACE_ENTRY*>(reinterpret_cast<std::uintptr_t>(this) + end + index * sizeof(API_SET_NAMESPACE_ENTRY));
	}

	inline API_SET_VALUE_ARRAY* get_host(API_SET_NAMESPACE_ENTRY* entry_pointer) noexcept
	{
		return reinterpret_cast<API_SET_VALUE_ARRAY*>(reinterpret_cast<std::uintptr_t>(this) + start + sizeof(API_SET_VALUE_ARRAY) * entry_pointer->size);
	}

	inline void read_name(API_SET_NAMESPACE_ENTRY* entry_pointer, wchar_t* output) noexcept
	{
		auto array_pointer = get_host(entry_pointer);
		memcpy(output, reinterpret_cast<char*>(this) + array_pointer->name_offset, array_pointer->name_length);
	}
};


using map_api_schema = std::unordered_map<std::wstring, std::vector<std::wstring>>;
class api_set
{
public:
	api_set()  noexcept;
	bool query(std::wstring& name) noexcept;
private:
	map_api_schema schema;
};