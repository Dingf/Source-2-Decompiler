/*
  This file is a work in progress so don't expect anything working yet...




*/


#include "string.h"
#include <stdint.h>
#include <string>
#include <fstream>
#include "decompiler.h"
#include "keyvalues.h"

#include <iostream>

using std::ios;

void S2Decompiler::DecompileVPCF(const std::string& szFilename, const std::string& szOutputDirectory)
{
	char szBuffer[4];

	uint32_t nNumBlocks;

	KeyValues sRERLInfo, sNTROInfo;
	KeyValues sParticleInfo;

	std::fstream f;
	std::streamoff p;
	f.open(szFilename, ios::in | ios::binary);
	if (!f.is_open())
		throw std::string("Could not open file \"" + szFilename + "\" for reading.");

	f.seekg(12);
	f.read((char *)&nNumBlocks, 4);
	for (nNumBlocks; nNumBlocks > 0; nNumBlocks--)
	{
		f.read(szBuffer, 4);
		if (strncmp(szBuffer, "RERL", 4) == 0)
		{
			ProcessRERLBlock(f, sRERLInfo);
		}
		else if (strncmp(szBuffer, "DATA", 4) == 0)
		{
			f.read(szBuffer, 4);
			p = f.tellg();
			f.seekg(*(int*)szBuffer - 4, ios::cur);

			ReadStructuredData(f, sParticleInfo);

			f.seekg(p + 4);
		}
		else if (strncmp(szBuffer, "NTRO", 4) == 0)
		{
			ProcessNTROBlock(f, sNTROInfo);
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
}