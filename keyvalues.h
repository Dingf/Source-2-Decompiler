#ifndef INC_S2DC_KEY_VALUES_H
#define INC_S2DC_KEY_VALUES_H

#ifdef _WIN32
#pragma once
#endif

/* KeyValues struct

   A simple struct that contains a fixed number of name/generic data pairs. When the struct
   is destroyed, it frees all memory that it allocated.

   To prevent memory leaks and/or invalid deallocations, make sure that any children KeyValues
   inserted as data are marked as such (by setting the type to KV_DATA_TYPE_STRUCT).
*/

#include <stdint.h>
#include <vector>
#include <fstream>

enum KeyValueDataType
{
	KV_DATA_TYPE_STRUCT = 1,
	KV_DATA_TYPE_ENUM = 2,
	KV_DATA_TYPE_HANDLE = 3,
	KV_DATA_TYPE_STRING = 4,
	KV_DATA_TYPE_BYTE = 11,
	KV_DATA_TYPE_SHORT = 12,
	KV_DATA_TYPE_USHORT = 13,
	KV_DATA_TYPE_INTEGER = 14,
	KV_DATA_TYPE_UINTEGER = 15,
	KV_DATA_TYPE_INT64 = 16,
	KV_DATA_TYPE_UINT64 = 17,
	KV_DATA_TYPE_FLOAT = 18,
	KV_DATA_TYPE_VECTOR2 = 21,
	KV_DATA_TYPE_VECTOR3 = 22,
	KV_DATA_TYPE_VECTOR4 = 23,
	KV_DATA_TYPE_QUATERNION = 25,
	KV_DATA_TYPE_COLOR = 28,   //Standard RGBA, 1 byte per channel
	KV_DATA_TYPE_BOOLEAN = 30,
	KV_DATA_TYPE_NAME = 31,    //Also used for notes as well? idk... seems to be some kind of special string
};

struct KeyValues
{
	KeyValues() : size(0), name(NULL), data(NULL), type(NULL) { };
	KeyValues(uint32_t nSize)
	{
		size = nSize;
		if (size > 0)
		{
			name = new char*[nSize];
			memset(name, 0, sizeof(char*) * nSize);
			data = new char*[nSize];
			memset(data, 0, sizeof(char*) * nSize);
			type = new uint8_t[nSize];
			memset(type, 0, sizeof(uint8_t) * nSize);
		}
		else
		{
			name = NULL;
			data = NULL;
			type = NULL;
		}
	}

	~KeyValues()
	{
		if (data)
		{
			for (uint32_t i = 0; i < size; ++i)
			{
				if (data[i])
				{
					if ((type) && (type[i] == KV_DATA_TYPE_STRUCT))
						delete (KeyValues*)data[i];
					else
						delete[] data[i];
				}
			}
			delete[] data;
		}
		if (name)
		{
			for (uint32_t i = 0; i < size; ++i)
			{
				if (name[i])
					delete[] name[i];
			}
			delete[] name;
		}
		if (type)
			delete[] type;
		size = 0;
	}

	KeyValues& operator = (const KeyValues& kv)
	{
		size = kv.size;
		if (name)
			delete[] name;
		name = new char*[size];
		memcpy(name, kv.name, sizeof(char*) * size);
		if (data)
			delete[] data;
		data = new char*[size];
		memcpy(data, kv.data, sizeof(char*) * size);
		if (type)
			delete[] type;
		type = new uint8_t[size];
		memcpy(type, kv.type, sizeof(uint8_t) * size);
		return *this;
	}
	
	const char* operator[](const char* szName) const
	{
		if ((name) && (data))
		{
			for (uint32_t i = 0; i < size; ++i)
			{
				if (strcmp(name[i], szName) == 0)
				{
					return data[i];
				}
			}
		}
		return NULL;
	}

	char* operator[](const char* szName)
	{
		if ((name) && (data))
		{
			for (uint32_t i = 0; i < size; ++i)
			{
				if (strcmp(name[i], szName) == 0)
				{
					return data[i];
				}
			}
		}
		return NULL;
	}

	uint32_t size;
	uint8_t* type;
	char** name;
	char** data;

	static uint8_t _snDataSize[];
};

void ReadOffsetString(std::fstream& f, char*& sz);

#endif