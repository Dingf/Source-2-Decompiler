#include <stdint.h>
#include <fstream>
#include "keyvalues.h"

using std::ios;

//ReadOffsetString
//  Reads in the next 4 bytes as an offset to a zero-terminated string value, then
//  allocates and returns the string.
void ReadOffsetString(std::fstream& f, char *& sz)
{
	int32_t i;
	char szBuffer[4];

	f.read(szBuffer, 4);
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