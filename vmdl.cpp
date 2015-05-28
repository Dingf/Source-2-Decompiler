/*#include <string>
#include <fstream>
#include "decompiler.h"
#include "vmdl.h"

#include <iostream>

using std::ios;

void S2Decompiler::DecompileVMDL(const std::string& szFilename, const std::string& szOutputDirectory)
{
	char szBuffer[4];

	uint32_t uNumBlocks;

	KeyValues sRERLInfo, sNTROInfo;
	KeyValues sVMDLData;

	std::fstream f;
	std::streamoff p1;
	f.open(szFilename, ios::in | ios::binary);
	if (!f.is_open())
		throw std::string("Could not open file \"" + szFilename + "\" for reading.");

	f.seekg(12);
	f.read((char *)&uNumBlocks, 4);
	for (uNumBlocks; uNumBlocks > 0; uNumBlocks--)
	{
		f.read(szBuffer, 4);
		if (strncmp(szBuffer, "RERL", 4) == 0)
		{
			ProcessRERLBlock(f, sRERLInfo);
		}
		else if (strncmp(szBuffer, "NTRO", 4) == 0)
		{
			ProcessNTROBlock(f, sNTROInfo);
		}
		else if (strncmp(szBuffer, "DATA", 4) == 0)
		{
			f.read(szBuffer, 4);
			p1 = f.tellg();
			f.seekg(*(int*)szBuffer - 4, ios::cur);
			ReadStructuredData(f, sVMDLData, (KeyValues*)sNTROInfo.data[0]);
			f.seekg(p1 + 4);
		}
		else if (strncmp(szBuffer, "REDI", 4) == 0)
		{
			f.seekg(8, ios::cur);
		}
		else
		{
			throw std::string("Encountered invalid block type.");
		}
	}
	f.close();

	for (uint32_t i = 0; i < sVMDLData.size; i++)
	{
		std::cout << sVMDLData.name[i] << "\n";
	}
}*/