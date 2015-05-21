#include <stdint.h>
#include <fstream>
#include "keyvalues.h"

using std::ios;

KeyValues* psLastRERLInfo = NULL;

const char* GetExternalResourceName(const char* szRefID)
{
	if (psLastRERLInfo == NULL)
		throw std::string("No RERL information was found. (Did you forget to process the RERL block first?)");

	for (int32_t i = 0; i < psLastRERLInfo->size; i++)
	{
		if (strncmp(szRefID, psLastRERLInfo->data[i], 8) == 0)
		{
			return psLastRERLInfo->name[i];
		}
	}
	return NULL;
}

void ProcessRERLBlock(std::fstream& f, KeyValues& sRERLInfo)
{
	char szBuffer[4];

	std::streamoff p1, p2;

	f.read(szBuffer, 4);
	p1 = f.tellg();
	f.seekg(*(int*)szBuffer, ios::cur);
	f.read((char*)&sRERLInfo.size, 4);
	sRERLInfo.name = new char *[sRERLInfo.size];
	sRERLInfo.data = new char *[sRERLInfo.size];
	for (int32_t i = 0; i < sRERLInfo.size; ++i)
	{
		sRERLInfo.data[i] = new char[9];
		f.read(sRERLInfo.data[i], 8);
		sRERLInfo.data[i][8] = '\0';
		p2 = f.tellg();

		ReadOffsetString(f, sRERLInfo.name[i]);
		f.seekg(p2 + 8);
	}
	f.seekg(p1 + 4);

	psLastRERLInfo = &sRERLInfo;
}