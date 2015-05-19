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

	int32_t i;
	uint32_t nNumBlocks;

	KeyValues sRERLInfo, sNTROInfo;
	KeyValues* psParticleSystemTemplate = NULL;
	KeyValues* psParticleOpTemplate = NULL;

	KeyValues sParticleInfo;

	std::fstream f;
	std::streamoff p1, p2, p3;
	f.open(szFilename, ios::in | ios::binary);
	if (!f.is_open())
		throw std::string("Could not open file \"" + szFilename + "\" for reading.");

	std::cout << "\n";
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
			p1 = f.tellg();
			f.seekg(*(int*)szBuffer - 4, ios::cur);

			f.seekg(4, ios::cur);    //0xAAA29DC3, the .vpcf data block ID

			if ((psParticleSystemTemplate == NULL) || (psParticleOpTemplate == NULL))
				throw std::string("Introspection data not found.");

			ReadStructuredData(f, &sParticleInfo, psParticleSystemTemplate);

			f.seekg(p1 + 4);
		}
		else if (strncmp(szBuffer, "NTRO", 4) == 0)
		{
			ProcessNTROBlock(f, sNTROInfo);
			for (uint32_t i = 0; i < sNTROInfo.size; i++)
			{
				if (strncmp(sNTROInfo.name[i], "CParticleSystemDefinition", 25) == 0)
					psParticleSystemTemplate = (KeyValues*)sNTROInfo.data[i];
				else if (strncmp(sNTROInfo.name[i], "CParticleOperatorInstance", 25) == 0)
					psParticleOpTemplate = (KeyValues*)sNTROInfo.data[i];
			}
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