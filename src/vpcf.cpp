#include <string.h>
#include <stdint.h>
#include <string>
#include <fstream>
#include "decompiler.h"
#include "keyvalues.h"
#include "kv3.h"

using std::ios;

bool OutputVPCFTypeData(std::fstream& f, const KeyValues& kv, uint32_t nIndex)
{
    uint16_t nDataType = *(uint16_t*)(&kv.name[nIndex][strlen(kv.name[nIndex]) + 1]);
    if (nDataType == KV_DATA_TYPE_ENUM)
        f << "symbol " << kv.name[nIndex] << " = " << *(uint32_t*)kv.data[nIndex] << "\n";
    else if ((nDataType == KV_DATA_TYPE_HANDLE) || (nDataType == KV_DATA_TYPE_STRING) || (nDataType == KV_DATA_TYPE_NAME))
        f << "string " << kv.name[nIndex] << " = \"" << ((kv.data[nIndex] == NULL) ? "" : kv.data[nIndex]) << "\"\n";
    else if (nDataType == KV_DATA_TYPE_INTEGER)
        f << "int " << kv.name[nIndex] << " = " << *(int32_t*)kv.data[nIndex] << "\n";
    else if (nDataType == KV_DATA_TYPE_FLOAT)
        f << "float " << kv.name[nIndex] << " = " << std::fixed << *(float*)kv.data[nIndex] << "\n";
    else if (nDataType == KV_DATA_TYPE_VECTOR3)
        f << "float(3) " << kv.name[nIndex] << " = ( " << std::fixed << *(float*)&kv.data[nIndex][0] << ", " << *(float*)&kv.data[nIndex][4] << ", " << *(float*)&kv.data[nIndex][8] << " )\n";
    else if (nDataType == KV_DATA_TYPE_VECTOR4)
        f << "float(4) " << kv.name[nIndex] << " = ( " << std::fixed << *(float*)&kv.data[nIndex][0] << ", " << *(float*)&kv.data[nIndex][4] << ", " << *(float*)&kv.data[nIndex][8] << ", " << *(float*)&kv.data[nIndex][12] << " )\n";
    else if (nDataType == KV_DATA_TYPE_COLOR)
        f << "int(4) " << kv.name[nIndex] << " = ( " << std::fixed << (uint32_t)*(uint8_t*)&kv.data[nIndex][0] << ", " << (uint32_t)*(uint8_t*)&kv.data[nIndex][1] << ", " << (uint32_t)*(uint8_t*)&kv.data[nIndex][2] << ", " << (uint32_t)*(uint8_t*)&kv.data[nIndex][3] << " )\n";
    else if (nDataType == KV_DATA_TYPE_BOOLEAN)
        f << "bool " << kv.name[nIndex] << " = " << (*(bool*)kv.data[nIndex] ? "true" : "false") << "\n";
    else
        return false;
    return true;
}

void OutputLegacyVPCF(const KeyValues& DataBlock, std::fstream& f, const std::string& szOutputName)
{
    std::fstream out;
    out.open(szOutputName, ios::out);
    if (!out.is_open())
        throw std::string("Could not open file \"" + szOutputName + "\" for writing.");

    f.precision(6);
    out << "<!-- schema text{7e125a45-3d83-4043-b292-9e24f8ef27b4} generic {198980d8-3a93-4919-b4c6-dd1fb07a3a4b} -->\n\n";
    out << "CParticleSystemDefinition CParticleSystemDefinition_0\n{\n";
    for (uint32_t i = 0; i < DataBlock.size; ++i)
    {
        uint16_t nDataType = *(uint16_t*)(&DataBlock.name[i][strlen(DataBlock.name[i]) + 1]);

        out << "\t";
        if ((!OutputVPCFTypeData(out, DataBlock, i)) && (nDataType == KV_DATA_TYPE_STRUCT))
        {
            out.seekp(-1, ios::cur);
            if (strncmp(DataBlock.name[i], "m_Children\0", 11) == 0)
            {
                KeyValues* pChildren = (KeyValues*)DataBlock.data[i];
                out << "\tParticleChildrenInfo_t[] m_Children =\n\t[\n";
                for (uint32_t j = 0; j < pChildren->size; ++j)
                {
                    out << "\t\t" << pChildren->name[j] << "\n\t\t{\n";
                    KeyValues* pChild = (KeyValues*)pChildren->data[j];
                    for (uint32_t k = 0; k < pChild->size; ++k)
                    {
                        out << "\t\t\t";
                        OutputVPCFTypeData(out, *pChild, k);
                    }
                    out << "\t\t}" << ((j == pChildren->size - 1) ? "\n" : ",\n");
                }
                out << "\t]\n";
            }
            else
            {
                KeyValues* pOperators = (KeyValues*)DataBlock.data[i];
                out << "\tCParticleOperator*[] " << DataBlock.name[i] << " =\n\t[\n";
                for (uint32_t j = 0; j < pOperators->size; ++j)
                {
                    KeyValues* pOperator = (KeyValues*)pOperators->data[j];
                    out << "\t\t&" << pOperator->name[0] << "_" << j << ((j < pOperators->size - 1) ? ",\n" : "\n");
                }
                out << "\t]\n";
            }
        }
    }
    out << "}\n";


    for (uint32_t i = 0; i < DataBlock.size; ++i)
    {
        uint16_t nDataType = *(uint16_t*)(&DataBlock.name[i][strlen(DataBlock.name[i]) + 1]);
        if ((nDataType == KV_DATA_TYPE_STRUCT) && (strncmp(DataBlock.name[i], "m_Children\0", 11) != 0))
        {
            KeyValues* pOperators = (KeyValues*)DataBlock.data[i];
            for (uint32_t j = 0; j < pOperators->size; ++j)
            {
                KeyValues* pOperator = (KeyValues*)pOperators->data[j];
                KeyValues* pOperatorData = (KeyValues*)pOperator->data[0];
                out << "\n" << pOperator->name[0] << " " << pOperator->name[0] << "_" << j << "\n{\n";

                bool bLastOutputSuccess = true;
                for (uint32_t k = 0; k < pOperatorData->size; ++k)
                {
                    if (bLastOutputSuccess)
                        out << "\t";
                    bLastOutputSuccess = OutputVPCFTypeData(out, *pOperatorData, k);
                }
                out << "}\n";
            }
        }
    }
    out.close();
}

void S2Decompiler::OutputVPCF(const KeyValues& DataBlock, std::fstream& f, const std::string& szOutputName)
{
    uint32_t* pSignature = (uint32_t*)DataBlock["m_Signature"];
    if ((pSignature != NULL) && (*pSignature == 55987030))
        OutputKV3(f, szOutputName);
    else if (pSignature == NULL)
        OutputLegacyVPCF(DataBlock, f, szOutputName);
    else
        throw std::string("Invalid KV3 signature.");
}