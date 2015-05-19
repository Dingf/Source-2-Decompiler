#include <fstream>
#include "keyvalues.h"

using std::ios;

//ReadOffsetString
//  Reads in the next 4 bytes as an offset to a zero-terminated string value, then
//  allocates and returns the string.
void ReadOffsetString(std::fstream& f, char *& sz)
{
	int i;
	char szBuffer[4];

	f.read(szBuffer, 4);
	f.seekg(*(int*)szBuffer - 4, ios::cur);
	for (i = 0; f.get() != '\0'; ++i);
	f.seekg(-(i + 1), ios::cur);
	sz = new char[i + 1];
	f.read(sz, i);
	sz[i] = '\0';
	f.seekg(4 - *(int*)szBuffer - i, ios::cur);
}

//ReadKVBlock
//  Fills in the specified KeyValues block, which consists of consecutive name-data pairs of
//  length specified.
void ReadKVBlock(std::fstream& f, KeyValues& kv, int nNameLength, int nDataLength)
{
	char szBuffer[4];
	std::streamoff p;

	f.read(szBuffer, 4);
	f.read((char *)&kv.size, 4);
	p = f.tellg();
	if (*(int*)szBuffer != 0)
	{
		kv.name = new char *[kv.size];
		kv.data = new char *[kv.size];
		f.seekg(*(int*)szBuffer - 8, ios::cur);
		for (int i = 0; i < kv.size; ++i)
		{
			ReadOffsetString(f, kv.name[i]);
			if (nNameLength > 4)
				f.seekg(nNameLength - 4, ios::cur);

			if (nDataLength == 0)
				ReadOffsetString(f, kv.data[i]);
			else
			{
				kv.data[i] = new char[nDataLength];
				f.read(kv.data[i], nDataLength);
			}
		}
		f.seekg(p);
	}
}