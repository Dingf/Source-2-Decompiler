#include <stdint.h>
#include <stack>
#include <fstream>
#include "keyvalues.h"
#include "ntro.h"
#include "rerl.h"

//#include <iostream>

using std::ios;

KeyValues * psLastNTROInfo = NULL;

KeyValues* GetNTROResourceDataByID(uint32_t uID)
{
	if (psLastNTROInfo == NULL)
		throw std::string("No NTRO information was found. (Did you forget to process the NTRO block first?)");

	for (uint32_t i = 0; i < psLastNTROInfo->size; i++)
	{
		if (uID == *(uint32_t*)&psLastNTROInfo->name[i][strlen(psLastNTROInfo->name[i])+1])
		{
			return (KeyValues*)psLastNTROInfo->data[i];
		}
	}
	return NULL;
}

const char* GetNTROResourceNameByID(uint32_t uID)
{
	if (psLastNTROInfo == NULL)
		throw std::string("No NTRO information was found. (Did you forget to process the NTRO block first?)");

	for (uint32_t i = 0; i < psLastNTROInfo->size; i++)
	{
		if (uID == *(uint32_t*)&psLastNTROInfo->name[i][strlen(psLastNTROInfo->name[i]) + 1])
		{
			return psLastNTROInfo->name[i];
		}
	}
	return NULL;
}


void ProcessNTROBlock(std::fstream& f, KeyValues& sNTROInfo)
{
	char szBuffer[4];

	int32_t i, j, k;
	uint32_t uStructSize, uEnumSize;

	std::streamoff p1, p2, p3, p4;

	f.read(szBuffer, 4);
	p1 = f.tellg();
	f.seekg(*(int*)szBuffer + 4, ios::cur);
	f.read((char*)&uStructSize, 4);
	p4 = f.tellg();
	f.seekg(4, ios::cur);
	f.read((char*)&uEnumSize, 4);

	f.seekg(-16, ios::cur);
	f.read(szBuffer, 4);
	f.seekg(*(int*)szBuffer - 4, ios::cur);

	sNTROInfo.size = uStructSize + uEnumSize;
	sNTROInfo.name = new char *[sNTROInfo.size];
	sNTROInfo.data = new char *[sNTROInfo.size];

	for (i = 0; i < uStructSize; i++)
	{
		uint16_t uDiskSize;
		uint16_t uDiskOffset = 0;
		uint32_t uResourceID;
		uint32_t uBaseStructID;

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
		f.seekg(2, ios::cur);
		f.read((char*)&uBaseStructID, 4);
		f.seekg(4, ios::cur);
		f.read(szBuffer, 4);

		KeyValues* sResourceDiskStruct = new KeyValues;
		sNTROInfo.data[i] = (char *)sResourceDiskStruct;

		sResourceDiskStruct->size = *(int*)szBuffer;
		sResourceDiskStruct->name = new char*[sResourceDiskStruct->size+1];
		sResourceDiskStruct->data = new char*[sResourceDiskStruct->size+1];

		//Stick the base struct onto the end as "hidden" data
		sResourceDiskStruct->name[sResourceDiskStruct->size] = new char[16];
		memcpy(sResourceDiskStruct->name[sResourceDiskStruct->size], "m_nBaseStructId", 15);
		sResourceDiskStruct->name[sResourceDiskStruct->size][15] = '\0';
		sResourceDiskStruct->data[sResourceDiskStruct->size] = (char*)uBaseStructID;

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

			//ResourceDiskStruct data:
			//  2 bytes: disk offset
			//  2 bytes: size of the struct
			//  2 bytes: struct type
			//  4 bytes: resource type
			//  4 bytes: indirection level
			f.seekg(p3 + 2);
			f.read(szBuffer, 2);
			sResourceDiskStruct->data[j] = new char[14];
			*(uint16_t*)&sResourceDiskStruct->data[j][0] = *(uint16_t*)szBuffer;
			if (j != 0)
				*(uint16_t*)&sResourceDiskStruct->data[j - 1][2] = *(uint16_t*)szBuffer - uDiskOffset;
			uDiskOffset = *(uint16_t*)szBuffer;
			f.seekg(4, ios::cur);
			f.read(szBuffer, 4);
			*(uint32_t*)&sResourceDiskStruct->data[j][10] = *(uint32_t*)szBuffer;
			f.read(szBuffer, 4);
			*(uint32_t*)&sResourceDiskStruct->data[j][6] = *(uint32_t*)szBuffer;
			f.read(szBuffer, 4);
			*(uint16_t*)&sResourceDiskStruct->data[j][4] = *(uint16_t*)szBuffer;
		}

		if (j != 0)
			*(uint16_t*)&sResourceDiskStruct->data[j - 1][2] = uDiskSize - uDiskOffset;

		f.seekg(p2 + 4);
	}

	f.seekg(p4);
	f.read(szBuffer, 4);
	f.seekg(*(int*)szBuffer - 4, ios::cur);
	for (i = uStructSize; i < uStructSize + uEnumSize; i++)
	{
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
		f.seekg(p2 + 12);
		f.read(szBuffer, 4);

		KeyValues* sResourceDiskEnum = new KeyValues;
		sNTROInfo.data[i] = (char *)sResourceDiskEnum;

		sResourceDiskEnum->size = *(int*)szBuffer;
		sResourceDiskEnum->name = new char*[sResourceDiskEnum->size+1];
		sResourceDiskEnum->data = new char*[sResourceDiskEnum->size+1];

		f.seekg(-8, ios::cur);
		f.read(szBuffer, 4);
		p2 = f.tellg();
		f.seekg(*(int*)szBuffer - 4, ios::cur);
		for (j = 0; j < sResourceDiskEnum->size; j++)
		{
			uint32_t uEnumValue;
			ReadOffsetString(f, sResourceDiskEnum->name[j]);
			f.read((char*)&uEnumValue, 4);
			sResourceDiskEnum->data[j] = (char*)uEnumValue;
		}
		f.seekg(p2 + 4);
	}

	f.seekg(p1 + 4);

	psLastNTROInfo = &sNTROInfo;
}

void ReadStructuredData(std::fstream& f, KeyValues& sDestination, KeyValues * psSourceStruct)
{
	char szBuffer[4];

	int32_t i, j, k;
	uint32_t uIndex = 0;
	uint32_t uTotalOffset = 0;

	std::streamoff p1, p2;

	if (psLastNTROInfo == NULL)
		throw std::string("No NTRO information was found. (Did you forget to process the NTRO block first?)");

	if (psSourceStruct == NULL)
	{
		f.read(szBuffer, 4);
		psSourceStruct = GetNTROResourceDataByID(*(uint32_t*)szBuffer);
		if (psSourceStruct == NULL)
			throw std::string("No matching NTRO structure was found for the header.");
		uTotalOffset += 4;
	}

	//KeyValues aren't dynamically sized, so we need to know how much to allocate ahead of time
	//This includes the struct and all base structs that it encompasses
	uint32_t uTotalStructSize = 0;
	std::stack<KeyValues*> sStructStack;
	sStructStack.push(psSourceStruct);
	while (sStructStack.top() != NULL)
	{
		uTotalStructSize += sStructStack.top()->size;
		sStructStack.push(GetNTROResourceDataByID((uint32_t)sStructStack.top()->data[sStructStack.top()->size]));
	}
	sStructStack.pop();    //Get rid of the terminating null value

	sDestination.size = uTotalStructSize;
	sDestination.name = new char*[uTotalStructSize];
	sDestination.data = new char*[uTotalStructSize];
	memset(sDestination.name, 0, uTotalStructSize);

	while (!sStructStack.empty())
	{
		psSourceStruct = sStructStack.top();
		for (i = 0; i < psSourceStruct->size; i++)
		{
			uint16_t uNameLength = strlen(psSourceStruct->name[i]);
			uint16_t uDiskOffset = *(uint16_t*)&psSourceStruct->data[i][0];
			uint16_t uDataSize = *(uint16_t*)&psSourceStruct->data[i][2];
			uint16_t uDataType = *(uint16_t*)&psSourceStruct->data[i][4];
			uint32_t uStructID = *(uint32_t*)&psSourceStruct->data[i][6];
			uint32_t uIndirectionLevel = *(uint32_t*)&psSourceStruct->data[i][10];

			sDestination.name[uIndex] = new char[uNameLength + 1];
			memcpy(sDestination.name[uIndex], psSourceStruct->name[i], uNameLength);
			sDestination.name[uIndex][uNameLength] = '\0';

			if (uTotalOffset != uDiskOffset)
				f.seekg((int32_t)uDiskOffset - (int32_t)uTotalOffset, ios::cur);

			if ((uDataType == NTRO_DATA_TYPE_STRING) || (uDataType == NTRO_DATA_TYPE_NAME))
			{
				p1 = f.tellg();
				ReadOffsetString(f, sDestination.data[uIndex]);
				f.seekg(p1 + uDataSize);
			}
			else if (uDataType == NTRO_DATA_TYPE_STRUCT)
			{
				KeyValues* sParticleInfoStruct = new KeyValues;

				p1 = f.tellg();
				sDestination.data[uIndex] = (char*)sParticleInfoStruct;
				f.seekg(4, ios::cur);
				f.read((char*)&sParticleInfoStruct->size, 4);

				if ((sParticleInfoStruct->size != 0) || (uIndirectionLevel == 0))
				{
					sParticleInfoStruct->name = new char*[sParticleInfoStruct->size];
					sParticleInfoStruct->data = new char*[sParticleInfoStruct->size];

					memset(sParticleInfoStruct->name, 0, sizeof(char*) * sParticleInfoStruct->size);
					memset(sParticleInfoStruct->data, 0, sizeof(char*) * sParticleInfoStruct->size);

					f.seekg(-8, ios::cur);
					f.read(szBuffer, 4);
					f.seekg(*(int*)szBuffer - 4, ios::cur);

					if (uIndirectionLevel > 1)
					{
						for (j = 0; j < sParticleInfoStruct->size; j++)
						{
							p2 = f.tellg();
							//Unconfirmed for indirection level > 2...
							//Then again, what kind of idiot stores a pointer to a pointer to a pointer anyways?
							for (k = 1; k < uIndirectionLevel; k++)
							{
								f.read(szBuffer, 4);
								f.seekg(*(int*)szBuffer - 4, ios::cur);
							}
							sParticleInfoStruct->data[j] = (char*)new KeyValues;
							ReadStructuredData(f, *(KeyValues*)sParticleInfoStruct->data[j]);

							f.seekg(p2 + 4);
						}
					}
					else if (uIndirectionLevel == 1)
					{
						KeyValues* psResourceStruct = GetNTROResourceDataByID(uStructID);
						const char* szResourceName = GetNTROResourceNameByID(uStructID);
						if (psResourceStruct != NULL)
						{
							for (k = 0; k < sParticleInfoStruct->size; k++)
							{
								sParticleInfoStruct->name[k] = new char[strlen(szResourceName) + 1];
								memcpy(sParticleInfoStruct->name[k], szResourceName, strlen(szResourceName) + 1);
								sParticleInfoStruct->data[k] = (char*)new KeyValues;
								ReadStructuredData(f, *(KeyValues*)sParticleInfoStruct->data[k], psResourceStruct);
							}
						}
					}
					else if (uIndirectionLevel == 0)
					{
						KeyValues* psResourceStruct = GetNTROResourceDataByID(uStructID);
						const char* szResourceName = GetNTROResourceNameByID(uStructID);
						if (psResourceStruct != NULL)
						{
							sParticleInfoStruct->size = psResourceStruct->size;
							sParticleInfoStruct->name = new char*[sParticleInfoStruct->size];
							sParticleInfoStruct->data = new char*[sParticleInfoStruct->size];

							for (j = 0; j < psResourceStruct->size; j++)
							{
								uint32_t uStructNameLength = strlen(psResourceStruct->name[j]);
								uint16_t uStructFieldSize = *(uint16_t*)&psResourceStruct->data[j][2];

								sParticleInfoStruct->name[j] = new char[uStructNameLength + 1];
								memcpy(sParticleInfoStruct->name[j], psResourceStruct->name[j], uStructNameLength);
								sParticleInfoStruct->name[j][uStructNameLength] = '\0';
								sParticleInfoStruct->data[j] = new char[uStructFieldSize];
								f.read(sParticleInfoStruct->data[j], uStructFieldSize);
							}
						}

						p1 = f.tellg() - (std::streampos)uDataSize;
					}
				}
				f.seekg(p1 + uDataSize);
			}
			else if (uDataType == NTRO_DATA_TYPE_HANDLE)
			{
				char szBuffer2[8];

				f.read(szBuffer2, uDataSize);
				const char * szResourceName = GetExternalResourceName(szBuffer2);
				if (szResourceName != NULL)
				{
					uint32_t uResourceNameLength = strlen(szResourceName);
					sDestination.data[uIndex] = new char[uResourceNameLength + 1];
					memcpy(sDestination.data[uIndex], szResourceName, uResourceNameLength);
					sDestination.data[uIndex][uResourceNameLength] = '\0';
				}
				else
				{
					sDestination.data[uIndex] = new char[1];
					sDestination.data[uIndex][0] = '\0';
				}
			}
			else
			{
				sDestination.data[uIndex] = new char[uDataSize];
				f.read(sDestination.data[uIndex], uDataSize);
			}

			uIndex++;
			uTotalOffset += uDataSize;
		}
		sStructStack.pop();
	}
}