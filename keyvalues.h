#ifndef INC_S2DC_KEY_VALUES_H
#define INC_S2DC_KEY_VALUES_H

/* KeyValues struct

   A simple struct that contains a fixed number of name/generic data pairs. When the struct
   is destroyed, it frees all memory that it allocated.

   To prevent memory leaks and/or invalid deallocations, make sure that all non-NULL name
   and data values have been dynamically allocated with new[]. 
*/

#include <stdint.h>
#include <fstream>

struct KeyValues
{
	KeyValues() : size(0), name(0), data(0) {};
	~KeyValues()
	{
		if (name)
		{
			for (int32_t i = 0; i < size; i++)
			{
				if (name[i])
					delete[] name[i];
			}
			delete[] name;
		}
		if (data)
		{
			for (int32_t i = 0; i < size; i++)
			{
				if (data[i])
					delete[] data[i];
			}
			delete[] data;
		}
	}
	
	const char* operator[](const char *szName) const
	{
		for (int32_t i = 0; i < size; i++)
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
		for (int32_t i = 0; i < size; i++)
		{
			if (strcmp(name[i], szName) == 0)
			{
				return data[i];
			}
		}
		return NULL;
	}

	int32_t size;
	char ** name;
	char ** data;
};

void ReadOffsetString(std::fstream& f, char *& sz);
void ReadKVBlock(std::fstream& f, KeyValues& kv, uint32_t nNameLength, uint32_t nDataLength);

#endif