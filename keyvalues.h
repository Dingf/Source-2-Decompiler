#ifndef INC_S2DC_KEY_VALUES_H
#define INC_S2DC_KEY_VALUES_H

#ifdef _WIN32
#pragma once
#endif

/* KeyValues struct

   A simple struct that contains a fixed number of name/generic data pairs. When the struct
   is destroyed, it frees all memory that it allocated.

   To prevent memory leaks and/or invalid deallocations, make sure that any children KeyValues
   inserted as data are marked as such (using is_kv).
*/

#include <stdint.h>
#include <vector>
#include <fstream>

enum KeyValueType
{
	KV_TYPE_PLAIN_OLD_DATA = 0,
	KV_TYPE_CHILD_KEYVALUE = 1,
};

struct KeyValues
{
	KeyValues() : size(0), name(0), data(0), type(0) { id = guid_counter++; };
	KeyValues(uint32_t nSize)
	{
		id = guid_counter++;
		size = nSize;
		name = new char *[nSize];
		memset(name, 0, sizeof(char *) * nSize);
		data = new char *[nSize];
		memset(data, 0, sizeof(char *) * nSize);
		type = new uint8_t[nSize];
		memset(type, 0, sizeof(uint8_t) * nSize);
	}

	~KeyValues()
	{
		if (data)
		{
			for (uint32_t i = 0; i < size; i++)
			{
				if (data[i])
				{
					if ((type) && (type[i] & KV_TYPE_CHILD_KEYVALUE))
						delete (KeyValues*)data[i];
					else
						delete[] data[i];
				}
			}
			delete[] data;
		}
		if (name)
		{
			for (uint32_t i = 0; i < size; i++)
			{
				if (name[i])
					delete[] name[i];
			}
			delete[] name;
		}
		if (type)
			delete[] type;
		//size = 0;
	}

	KeyValues& operator = (const KeyValues& kv)
	{
		size = kv.size;
		name = new char *[size];
		memcpy(name, kv.name, sizeof(char *) * size);
		data = new char *[size];
		memcpy(data, kv.data, sizeof(char *) * size);
		type = new uint8_t[size];
		memcpy(type, kv.type, sizeof(uint8_t) * size);
		return *this;
	}
	
	const char* operator[](const char *szName) const
	{
		for (uint32_t i = 0; i < size; i++)
		{
			if (strcmp(name[i], szName) == 0)
			{
				return data[i];
			}
		}
		return NULL;
	}

	char* operator[](const char *szName)
	{
		for (uint32_t i = 0; i < size; i++)
		{
			if (strcmp(name[i], szName) == 0)
			{
				return data[i];
			}
		}
		return NULL;
	}

	int32_t id;
	static uint32_t guid_counter;

	uint32_t size;
	uint8_t * type;
	char ** name;
	char ** data;
};

void ReadOffsetString(std::fstream& f, char *& sz);

#endif