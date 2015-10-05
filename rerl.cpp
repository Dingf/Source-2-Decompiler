#include <stdint.h>
#include <fstream>
#include "keyvalues.h"

using std::ios;

KeyValues* pLastRERLInfo = NULL;

const char* GetExternalResourceName(const char* szRefID)
{
	if (pLastRERLInfo == NULL)
		throw std::string("No RERL information was found. (Did you forget to process the RERL block first?)");

	for (uint32_t i = 0; i < pLastRERLInfo->size; ++i)
	{
		if (strncmp(szRefID, pLastRERLInfo->data[i], 8) == 0)
		{
			return pLastRERLInfo->name[i];
		}
	}
	return NULL;
}

void ProcessRERLBlock(std::fstream& f, KeyValues& RERLInfo)
{
	char szBuffer[4];

	std::streamoff p1, p2;

	f.read(szBuffer, 4);
	p1 = f.tellg();
	f.seekg(*(int32_t*)szBuffer - 4, ios::cur);
	f.seekg(4, ios::cur);
	f.read(szBuffer, 4);
	RERLInfo = KeyValues(*(uint32_t*)szBuffer);
	f.seekg(-8, ios::cur);
	f.read(szBuffer, 4);
	f.seekg(*(int32_t*)szBuffer - 4, ios::cur);
	for (uint32_t i = 0; i < RERLInfo.size; ++i)
	{
		RERLInfo.data[i] = new char[9];
		f.read(RERLInfo.data[i], 8);
		RERLInfo.data[i][8] = '\0';
		p2 = f.tellg();

		ReadOffsetString(f, RERLInfo.name[i]);
		f.seekg(p2 + 8);
	}
	f.seekg(p1 + 4);

	pLastRERLInfo = &RERLInfo;
}

void ClearLastRERLEntry()
{
	pLastRERLInfo = NULL;
}