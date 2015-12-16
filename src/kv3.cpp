#include <stdint.h>
#include <string>
#include <fstream>
#include <iomanip>
#include "kv3.h"

using std::ios;

char szKV3Encoding[] = { 0x46, 0x1A, 0x79, 0x95, 0xBC, 0x95, 0x6C, 0x4F, 0xA7, 0x0B, 0x05, 0xBC, 0xA1, 0xB7, 0xDF, 0xD2 };
char szKV3Format[]   = { 0x7C, 0x16, 0x12, 0x74, 0xE9, 0x06, 0x98, 0x46, 0xAF, 0xF2, 0xE6, 0x3E, 0xB5, 0x90, 0x37, 0xE7 };

void DecompressKV3(std::fstream& f, const std::string& szOutputName)
{
    char szBuffer[18];

    int16_t nBlockMask, nOffset, nSize;

    std::fstream out;
    std::streamoff p;

    out.open(szOutputName, ios::out | ios::in | ios::trunc | ios::binary);
    if (!out.is_open())
        throw std::string("Could not open file \"" + szOutputName + "\" for writing.");

    f.read(szBuffer, 4);
    if (*(uint32_t*)szBuffer != 55987030)
        throw std::string("Invalid KV3 data.");
    out.write(szBuffer, 4);

    f.read(szBuffer, 16);
    if (strncmp(szBuffer, szKV3Encoding, 16) != 0)
        throw std::string("Unrecognized KV3 encoding.");
    out.write(szBuffer, 16);

    f.read(szBuffer, 16);
    if (strncmp(szBuffer, szKV3Format, 16) != 0)
        throw std::string("Unrecognized KV3 format.");
    out.write(szBuffer, 16);

    f.read(szBuffer, 4);
    out.write(szBuffer, 4);
    if (szBuffer[3] & 0x80)
    {
        out << f.rdbuf();
    }
    else
    {
        while (!f.eof())
        {
            f.read(szBuffer, 2);
            nBlockMask = *(uint16_t*)szBuffer;
            for (uint8_t i = 0; i < 16; i++)
            {
                if (nBlockMask & (1 << i))
                {
                    f.read(szBuffer, 2);

                    nOffset = ((*(uint16_t*)szBuffer & 0xFFF0) >> 4) + 1;
                    nSize = (*(uint16_t*)szBuffer & 0x000F) + 3;

                    int32_t nLookupSize = (nOffset < nSize) ? nOffset : nSize;

                    p = out.tellp();
                    out.seekg(p - (std::streamoff)nOffset);
                    out.read(szBuffer, nLookupSize);
                    out.seekp(p);

                    while (nSize > 0)
                    {
                        out.write(szBuffer, (nLookupSize < nSize) ? nLookupSize : nSize);
                        nSize -= nLookupSize;
                    }
                }
                else
                {
                    f.read(szBuffer, 1);
                    out.write(szBuffer, 1);
                }
            }
        }
    }
    
    out.close();
}

void WriteKV3Data(std::fstream& in, std::fstream& out, const char** szStringData, uint8_t nDepth = 0, uint8_t nLastType = 0)
{
    char szBuffer[8];

    int32_t i;
    uint8_t nDataType = 0;

    int x = in.tellg();
    for (i = 0; i < nDepth; i++) out.put('\t');
    if ((nDepth == 0) || (nLastType == 0x08))
    {
        in.read(szBuffer, 1);
        nDataType = szBuffer[0];
    }
    else
    {
        in.read(szBuffer, 5);
        nDataType = szBuffer[4];
        if (nDataType != 0x09)
            out << szStringData[*(uint32_t*)szBuffer] << " = ";
    }

    if (nDataType & 0x80)
    {
        nDataType = nDataType & 0x7F;
        in.read(szBuffer, 1);
        if (szBuffer[0] == 1)
            out << "resource:";
        else if (szBuffer[0] == 2)
            out << "deferred_resource:";
        else
            throw std::string("Unknown KV3 resource type \"" + std::to_string(szBuffer[0]) + "\".");
    }
    switch (nDataType)
    {
        case 0x02:
        {
            in.read(szBuffer, 1);
            out << ((szBuffer[0] == 0) ? "false" : "true");
            break;
        }
        case 0x03:
        {
            in.read(szBuffer, 8);
            out << *(int64_t*)szBuffer;
            break;
        }
        case 0x04:        //unconfirmed, but likely
        {
            in.read(szBuffer, 8);
            out << *(uint64_t*)szBuffer;
            break;
        }
        case 0x05:
        {
            in.read(szBuffer, 8);
            out << std::fixed << *(double*)szBuffer;
            break;
        }
        case 0x06:
        {
            in.read(szBuffer, 4);
            const char* szString = (*(int32_t*)szBuffer == -1) ? "" : szStringData[*(int32_t*)szBuffer];
            out << "\"" << szString << "\"";
            break;
        }
        case 0x08:
        {
            in.read(szBuffer, 4);
            out << "\n";
            for (i = 0; i < nDepth; i++) out.put('\t');
            out << "[\n";
            for (i = 0; i < *(int32_t*)szBuffer; i++)
            {
                WriteKV3Data(in, out, szStringData, nDepth + 1, nDataType);
                out << ",\n";
            }
            for (i = 0; i < nDepth; i++) out.put('\t');
            out << "]";
            break;
        }
        case 0x09:
        {
            in.read(szBuffer, 4);
            out << "{\n";
            for (i = 0; i < *(int32_t*)&szBuffer; i++)
            {
                WriteKV3Data(in, out, szStringData, nDepth + 1, nDataType);
                out << "\n";
            }
            for (i = 0; i < nDepth; i++) out.put('\t');
            out << "}";
            break;
        }
        default:
        {
            throw std::string("Unknown KV3 data type \"" + std::to_string(nDataType) + "\".");
            break;
        }
    }
}

void OutputKV3(std::fstream& f, const std::string& szOutputName)
{
    int32_t i, j;
    char szBuffer[16];

    std::fstream out;
    out.open(szOutputName, ios::out);
    if (!out.is_open())
        throw std::string("Could not open file \"" + szOutputName + "\" for writing.");

    f.read(szBuffer, 4);
    if (*(uint32_t*)szBuffer != 55987030)
        throw std::string("Invalid KV3 data.");

    f.read(szBuffer, 16);
    if (strncmp(szBuffer, szKV3Encoding, 16) != 0)
        throw std::string("Unrecognized KV3 encoding.");

    f.read(szBuffer, 16);
    if (strncmp(szBuffer, szKV3Format, 16) != 0)
        throw std::string("Unrecognized KV3 format.");

    f.seekg(4, ios::cur);
    f.read(szBuffer, 4);

    int32_t nStringCount = *(int32_t*)szBuffer;
    char** szStringData = new char*[nStringCount];
    for (i = 0; i < nStringCount; i++)
    {
        for (j = 0; f.get() != '\0'; ++j);
        f.seekg(-(j + 1), ios::cur);
        szStringData[i] = new char[j + 1];
        f.read(szStringData[i], j + 1);
    }

    out.precision(6);
    out << "<!-- kv3 ";
    out << "encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} ";        //TODO: Make these not fixed values
    out << "format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} ";
    out << "-->\n";
    WriteKV3Data(f, out, (const char**)szStringData);

    out.close();

    for (i = 0; i < nStringCount; i++)
        delete[] szStringData[i];
    delete[] szStringData;
}