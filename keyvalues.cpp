#include <stdint.h>
#include <fstream>
#include "keyvalues.h"

using std::ios;

#include <iostream>

//ReadOffsetString
//  Reads in the next 4 bytes as an offset to a zero-terminated string value, then
//  allocates and returns the string.
void ReadOffsetString(std::fstream& f, char *& sz)
{
	int32_t i;
	char szBuffer[4];

	f.read(szBuffer, 4);
	std::cout << f.tellg() << " " << *(int32_t*)szBuffer << "\n";
	if (*(int32_t*)szBuffer == 0)
		sz = NULL;
	else
	{
		f.seekg(*(int32_t*)szBuffer - 4, ios::cur);
		for (i = 0; f.get() != '\0'; i++);
		f.seekg(-(i + 1), ios::cur);
		sz = new char[i + 1];
		f.read(sz, i);
		sz[i] = '\0';
	}
	f.seekg(4 - *(int32_t*)szBuffer - i, ios::cur);
}

//ReadKVBlock
//  Fills in the specified KeyValues block, which consists of consecutive name-data pairs of
//  length specified.
void ReadKVBlock(std::fstream& f, KeyValues& kv, uint32_t uNameLength, uint32_t uDataLength)
{
	char szBuffer[4];
	std::streamoff p;

	f.read(szBuffer, 4);
	f.read((char *)&kv.size, 4);
	p = f.tellg();
	if (*(int32_t*)szBuffer != 0)
	{
		kv.name = new char *[kv.size];
		kv.data = new char *[kv.size];
		f.seekg(*(int32_t*)szBuffer - 8, ios::cur);
		for (uint32_t i = 0; i < kv.size; ++i)
		{
			ReadOffsetString(f, kv.name[i]);
			if (uNameLength > 4)
				f.seekg(uNameLength - 4, ios::cur);

			if (uDataLength == 0)
				ReadOffsetString(f, kv.data[i]);
			else
			{
				kv.data[i] = new char[uDataLength];
				f.read(kv.data[i], uDataLength);
			}
		}
		f.seekg(p);
	}
}