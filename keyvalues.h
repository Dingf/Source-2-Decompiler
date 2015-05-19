#ifndef INC_S2DC_KEY_VALUES_H
#define INC_S2DC_KEY_VALUES_H

/* KeyValues struct

   A simple struct that contains a fixed number of name/generic data pairs. When the struct
   is destroyed, it frees all memory that it allocated.
*/

#include <fstream>

struct KeyValues
{
	KeyValues() : size(0), name(0), data(0) {};
	~KeyValues()
	{
		if (name)
		{
			for (int i = 0; i < size; ++i)
			{
				if (name[i])
					delete[] name[i];
			}
			delete[] name;
		}
		if (data)
		{
			for (int i = 0; i < size; ++i)
			{
				if (data[i])
					delete[] data[i];
			}
			delete[] data;
		}
	}

	int size;
	char ** name;
	char ** data;
};

void ReadOffsetString(std::fstream& f, char *& sz);
void ReadKVBlock(std::fstream& f, KeyValues& kv, int nNameLength, int nDataLength);

#endif