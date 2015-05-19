#include <stdint.h>
#include <fstream>
#include "keyvalues.h"
#include "ntro.h"

using std::ios;

KeyValues * sLastNTROInfo = NULL;

void ProcessNTROBlock(std::fstream& f, KeyValues& sNTROInfo)
{
	char szBuffer[4];

	int32_t i, j, k;
	std::streamoff p1, p2, p3;

	f.read(szBuffer, 4);
	p1 = f.tellg();
	f.seekg(*(int*)szBuffer + 4, ios::cur);
	f.read((char*)&sNTROInfo.size, 4);
	sNTROInfo.name = new char *[sNTROInfo.size];
	sNTROInfo.data = new char *[sNTROInfo.size];

	f.seekg(-8, ios::cur);
	f.read(szBuffer, 4);
	f.seekg(*(int*)szBuffer - 4, ios::cur);

	for (i = 0; i < sNTROInfo.size; i++)
	{
		uint16_t uDiskSize;
		uint16_t uDiskOffset = 0;
		uint32_t uResourceID;

		f.seekg(4, ios::cur);
		f.read((char*)&uResourceID, 4);
		f.read(szBuffer, 4);
		p2 = f.tellg();
		f.seekg(*(int*)szBuffer - 4, ios::cur);
		for (j = 0; f.get() != '\0'; j++);
		f.seekg(-(j + 1), ios::cur);
		sNTROInfo.name[i] = new char[j + 5];
		f.read(sNTROInfo.name[i], j);
		sNTROInfo.name[i][j] = '\0';
		memcpy(&sNTROInfo.name[i][j + 1], &uResourceID, 4);
		f.seekg(p2 + 8);
		f.read((char*)&uDiskSize, 2);
		f.seekg(10, ios::cur);
		f.read(szBuffer, 4);

		KeyValues* sResourceDiskStruct = new KeyValues;
		sResourceDiskStruct->size = *(int*)szBuffer;
		sResourceDiskStruct->name = new char *[sResourceDiskStruct->size];
		sResourceDiskStruct->data = new char *[sResourceDiskStruct->size];
		sNTROInfo.data[i] = (char *)sResourceDiskStruct;

		p2 = f.tellg();
		f.seekg(-8, ios::cur);
		f.read(szBuffer, 4);
		f.seekg(*(int*)szBuffer - 4, ios::cur);

		for (j = 0; j < sResourceDiskStruct->size; j++)
		{
			f.read(szBuffer, 4);
			p3 = f.tellg();
			f.seekg(*(int*)szBuffer - 4, ios::cur);
			for (k = 0; f.get() != '\0'; k++);
			f.seekg(-(k + 1), ios::cur);
			sResourceDiskStruct->name[j] = new char[k + 1];
			f.read(sResourceDiskStruct->name[j], k);
			sResourceDiskStruct->name[j][k] = '\0';

			f.seekg(p3 + 2);
			f.read(szBuffer, 2);
			sResourceDiskStruct->data[j] = new char[8];
			if (j != 0)
				*(uint16_t*)&sResourceDiskStruct->data[j - 1][0] = *(uint16_t*)szBuffer - uDiskOffset;
			uDiskOffset = *(uint16_t*)szBuffer;
			f.seekg(8, ios::cur);
			f.read(szBuffer, 4);
			*(uint32_t*)&sResourceDiskStruct->data[j][4] = *(uint32_t*)szBuffer;
			f.read(szBuffer, 4);
			*(uint16_t*)&sResourceDiskStruct->data[j][2] = *(uint16_t*)szBuffer;
		}

		if (j != 0)
			*(uint16_t*)&sResourceDiskStruct->data[j - 1][0] = uDiskSize - uDiskOffset - 4;

		f.seekg(p2 + 4);
	}

	f.seekg(p1 + 4);

	sLastNTROInfo = &sNTROInfo;
}

//This still needs debugging, avoid using for now
void ReadStructuredData(std::fstream& f, KeyValues * sDestination, KeyValues * sSourceStruct)
{
	char szBuffer[4];

	int32_t i, j, k;

	std::streamoff p1, p2, p3;

	if (sLastNTROInfo == NULL)
		throw std::string("No NTRO information was found. (Did you forget to process the NTRO block first?)");

	sDestination->name = new char *[sSourceStruct->size];
	sDestination->data = new char *[sSourceStruct->size];
	memset(sDestination->name, 0, sSourceStruct->size);

	for (uint32_t i = 0; i < sSourceStruct->size; i++)
	{
		uint16_t uNameLength = strlen(sSourceStruct->name[i]);
		uint16_t uDataSize = *(uint16_t*)&sSourceStruct->data[i][0];
		uint16_t uDataType = *(uint16_t*)&sSourceStruct->data[i][2];
		uint32_t uResourceType = *(uint32_t*)&sSourceStruct->data[i][4];

		sDestination->name[i] = new char[uNameLength + 1];
		memcpy(sDestination->name[i], sSourceStruct->name[i], uNameLength);
		sDestination->name[i][uNameLength] = '\0';
		if ((uDataType == NTRO_DATA_TYPE_STRING) || (uDataType == NTRO_DATA_TYPE_NAME))
		{
			p2 = f.tellg();
			f.read(szBuffer, 4);
			f.seekg(*(int*)szBuffer - 4, ios::cur);
			for (j = 0; f.get() != '\0'; j++);
			f.seekg(-(j + 1), ios::cur);
			sDestination->data[i] = new char[j + 1];
			f.read(sDestination->data[i], j);
			sDestination->data[i][j] = '\0';
			f.seekg(p2 + uDataSize);
		}
		else if (uDataType == NTRO_DATA_TYPE_STRUCT)
		{
			KeyValues* sParticleInfoStruct = new KeyValues;

			p2 = f.tellg();
			sDestination->data[i] = (char *)sParticleInfoStruct;
			f.seekg(4, ios::cur);
			f.read((char*)&sParticleInfoStruct->size, 4);

			if (sParticleInfoStruct->size != 0)
			{
				sParticleInfoStruct->name = new char *[sParticleInfoStruct->size];
				sParticleInfoStruct->data = new char *[sParticleInfoStruct->size];

				f.seekg(-8, ios::cur);
				f.read(szBuffer, 4);
				f.seekg(*(int*)szBuffer - 4, ios::cur);

				for (j = 0; j < sLastNTROInfo->size; j++)
				{
					if (*(uint32_t*)&sLastNTROInfo->name[j][strlen(sLastNTROInfo->name[j]) + 1] == uResourceType)
					{
						for (k = 0; k < sParticleInfoStruct->size; k++)
						{
							sParticleInfoStruct->name[k] = new char[strlen(sLastNTROInfo->name[j]) + 1];
							memcpy(sParticleInfoStruct->name[k], sLastNTROInfo->name[j], strlen(sLastNTROInfo->name[j]) + 1);
							sParticleInfoStruct->data[k] = (char*)new KeyValues;
							ReadStructuredData(f, (KeyValues*)sParticleInfoStruct->data[k], (KeyValues*)sLastNTROInfo->data[j]);
						}
						break;
					}
				}
			}
			f.seekg(p2);
			f.seekg(8, ios::cur);
		}
		else
		{
			sDestination->data[i] = new char[uDataSize];
			f.read(sDestination->data[i], uDataSize);
		}
	}
}