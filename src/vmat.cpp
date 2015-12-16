#include <string.h>
#include <stdint.h>
#include <map>
#include <string>
#include <fstream>
#include <boost/assign.hpp>
#include "keyvalues.h"
#include "decompiler.h"
#include "vtex.h"

using std::ios;

std::map<std::string, std::string> szTexParamAliases = boost::assign::map_list_of
    ("g_tColor",              "TextureColor")
    ("g_tColor0",             "TextureColor0")
    ("g_tSpecular0",          "TextureReflectance0")
    ("g_tSpecular1",          "TextureReflectance1")
    ("g_tSpecular2",          "TextureReflectance2")
    ("g_tSpecular3",          "TextureReflectance3")
    ("g_tDetail",             "TextureDetail")
    ("g_tDetail2",            "TextureDetail2")
    ("g_tSpecularWarp",       "TextureSpecularWarp")
    ("g_tDiffuseWarp",        "TextureDiffuseWarp")
    ("g_tFresnelColorWarp3D", "TextureFresnelColorWarp3D")
    ("g_tCubeMap",            "TextureCubeMap")
    ("g_tEnvironmentMap",     "TextureEnvironmentMap")
    ("g_tFlow",               "TextureFlow")
    ("g_tNoise",              "TextureNoise");

void S2Decompiler::OutputVMAT(const KeyValues& DataBlock, std::fstream& f, const std::string& szOutputName)
{
    std::fstream out;
    out.open(szOutputName, ios::out);
    if (!out.is_open())
        throw std::string("Could not open file \"" + szOutputName + "\" for writing.");

    out << "Layer0\n{ \n";

    const char* szShaderName = DataBlock["m_shaderName"];
    if (szShaderName == NULL)
        throw std::string("Could not find expected key value \"m_shaderName\".");

    out << "\tshader \"" << szShaderName << "\"\n\n";

    KeyValues* pIntParams = (KeyValues*)DataBlock["m_intParams"];
    if (pIntParams == NULL)
        throw std::string("Could not find expected key value \"m_intParams\".");
    for (uint32_t i = 0; i < pIntParams->size; ++i)
    {
        KeyValues* pIntParam = (KeyValues*)pIntParams->data[i];
        out << "\t" << pIntParam->data[0] << " " << *(int32_t*)pIntParam->data[1] << "\n";
    }
    out << "\n";

    f.precision(6);
    KeyValues* pFloatParams = (KeyValues*)DataBlock["m_floatParams"];
    if (pFloatParams == NULL)
        throw std::string("Could not find expected key value \"m_floatParams\".");
    for (uint32_t i = 0; i < pFloatParams->size; ++i)
    {
        KeyValues* pFloatParam = (KeyValues*)pFloatParams->data[i];
        out << "\t" << pFloatParam->data[0] << " \"" << *(float*)pFloatParam->data[1] << "\"\n";
    }
    out << "\n";

    KeyValues* pVectorParams = (KeyValues*)DataBlock["m_vectorParams"];
    if (pVectorParams == NULL)
        throw std::string("Could not find expected key value \"m_vectorParams\".");
    for (uint32_t i = 0; i < pVectorParams->size; ++i)
    {
        KeyValues* pVectorParam = (KeyValues*)pVectorParams->data[i];
        if (strncmp(pVectorParam->data[0], "g_vTexCoordScale", 16) == 0)
            out << "\t" << pVectorParam->data[0] << std::fixed << " \"[" << *(float*)&pVectorParam->data[1][0] << " " << *(float*)&pVectorParam->data[1][4] << "]\"\n\tg_vTexCoordOffset \"[" << *(float*)&pVectorParam->data[1][8] << " " << *(float*)&pVectorParam->data[1][12] << "]\"\n";
        else if (strncmp(pVectorParam->data[0], "g_vDetailTexCoordScale", 22) == 0)
            out << "\t" << pVectorParam->data[0] << std::fixed << " \"[" << *(float*)&pVectorParam->data[1][0] << " " << *(float*)&pVectorParam->data[1][4] << "]\"\n\tg_vDetailTexCoordOffset \"[" << *(float*)&pVectorParam->data[1][8] << " " << *(float*)&pVectorParam->data[1][12] << "]\"\n";
        else
            out << "\t" << pVectorParam->data[0] << std::fixed << " \"[" << *(float*)&pVectorParam->data[1][0] << " " << *(float*)&pVectorParam->data[1][4] << " " << *(float*)&pVectorParam->data[1][8] << " " << *(float*)&pVectorParam->data[1][12] << "]\"\n";
    }
    out << "\n";

    KeyValues* pTextureParams = (KeyValues*)DataBlock["m_textureParams"];
    if (pTextureParams == NULL)
        throw std::string("Could not find expected key value \"m_textureParams\".");
    for (uint32_t i = 0; i < pTextureParams->size; ++i)
    {
        KeyValues* pTextureParam = (KeyValues*)pTextureParams->data[i];

        //Skip default resources; these will be added automatically by the Material Editor anyways
        if ((pTextureParam->data[1] == NULL) || (strncmp(pTextureParam->data[1], "materials/default/", 18) == 0))
            continue;

        std::string szDecompileDirectory = szOutputName.substr(0, szOutputName.find_last_of("\\/"));

        uint32_t nOldDecompilerFlags = _nDecompilerFlags;
        _nDecompilerFlags = _nDecompilerFlags | DECOMPILER_FLAG_SILENT_DECOMPILE | DECOMPILER_FLAG_VTEX_NO_VTEX_FILE | DECOMPILER_FLAG_VTEX_NO_MIPMAPS;
        Decompile(_szInputDirectory + pTextureParam->data[1] + "_c", szDecompileDirectory);
        _nDecompilerFlags = nOldDecompilerFlags;

        std::string szResourceString = pTextureParam->data[1];
        std::string szFileExt = szResourceString.substr(szResourceString.length() - 18, 5);
        std::string szImageName = pTextureParam->data[1];
        std::string szTexParamName = std::string(pTextureParam->data[0]);

        if ((szFileExt == "_tga_") || (szFileExt == "_psd_"))
            szImageName = szImageName.substr(0, szImageName.length() - 18);
        if (szResourceString.substr(szResourceString.length() - 23, 5) == "_z000")
            szImageName = szImageName.substr(0, szImageName.length() - 5);

        szImageName = szImageName.substr(szImageName.find_last_of("\\/") + 1);
        szImageName = szImageName.substr(0, szImageName.find_last_of(".")) + ".tga";
        if ((szTexParamName == "g_tColor1") || (szTexParamName == "g_tColor2") || (szTexParamName == "g_tColor3"))
        {
            std::string szImageName2 = szImageName.substr(0, szImageName.find_last_of(".")) + "_a.tga";
            ExtractImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName2, IMAGE_CHANNEL_ALPHA);
            out << "\tTextureColor" << szTexParamName[8] << " \"" << szImageName << "\"\n";
            out << "\tTextureRevealMask" << szTexParamName[8] << " \"" << szImageName2 << "\"\n";
        }
        else if (szTexParamName == "g_tColorTranslucency")
        {
            std::string szImageName2 = szImageName.substr(0, szImageName.find_last_of(".")) + "_a.tga";
            ExtractImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName2, IMAGE_CHANNEL_ALPHA);
            out << "\tTextureColor \"" << szImageName << "\"\n";
            out << "\tTextureTranslucency \"" << szImageName2 << "\"\n";
        }
        else if (szTexParamName == "g_tNormal")
        {
            SwapImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName, IMAGE_CHANNEL_ALPHA, IMAGE_CHANNEL_RED);
            FillImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName, IMAGE_CHANNEL_BLUE, 255);
            
            out << "\tTextureNormal \"" << szImageName << "\"\n";
        }
        else if (szTexParamName == "g_tNormalSpecularMask")
        {
            std::string szImageName2 = szImageName.substr(0, szImageName.find_last_of(".")) + "_a.tga";

            SwapImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName, IMAGE_CHANNEL_ALPHA, IMAGE_CHANNEL_RED);
            FillImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName, IMAGE_CHANNEL_BLUE, 255);
            ExtractImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName2, IMAGE_CHANNEL_ALPHA);

            out << "\tTextureNormal \"" << szImageName << "\"\n";
            out << "\tTextureSpecularMask \"" << szImageName2 << "\"\n";
        }
        else if (szTexParamName == "g_tMasks1")
        {
            std::string szImageName2 = szImageName.substr(0, szImageName.find_last_of(".")) + "_a.tga";
            std::string szImageName3 = szImageName.substr(0, szImageName.find_last_of(".")) + "_r.tga";
            std::string szImageName4 = szImageName.substr(0, szImageName.find_last_of(".")) + "_g.tga";
            std::string szImageName5 = szImageName.substr(0, szImageName.find_last_of(".")) + "_b.tga";

            ExtractImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName2, IMAGE_CHANNEL_ALPHA);
            ExtractImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName3, IMAGE_CHANNEL_RED);
            ExtractImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName4, IMAGE_CHANNEL_GREEN);
            ExtractImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName5, IMAGE_CHANNEL_BLUE);

            out << "\tTextureSelfIllumMask \"" << szImageName2 << "\"\n";
            out << "\tTextureDetailMask \"" << szImageName3 << "\"\n";
            out << "\tTextureDiffuseWarpMask \"" << szImageName4 << "\"\n";
            out << "\tTextureMetalnessMask \"" << szImageName5 << "\"\n";
        }
        else if (szTexParamName == "g_tMasks2")
        {
            std::string szImageName2 = szImageName.substr(0, szImageName.find_last_of(".")) + "_a.tga";
            std::string szImageName3 = szImageName.substr(0, szImageName.find_last_of(".")) + "_r.tga";
            std::string szImageName4 = szImageName.substr(0, szImageName.find_last_of(".")) + "_g.tga";
            std::string szImageName5 = szImageName.substr(0, szImageName.find_last_of(".")) + "_b.tga";

            ExtractImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName2, IMAGE_CHANNEL_ALPHA);
            ExtractImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName3, IMAGE_CHANNEL_RED);
            ExtractImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName4, IMAGE_CHANNEL_GREEN);
            ExtractImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName5, IMAGE_CHANNEL_BLUE);

            out << "\tTextureSpecularExponent \"" << szImageName2 << "\"\n";
            out << "\tTextureSpecularMask \"" << szImageName3 << "\"\n";
            out << "\tTextureRimMask \"" << szImageName4 << "\"\n";
            out << "\tTextureTintByBaseMask \"" << szImageName5 << "\"\n";
        }
        else if (szTexParamName == "g_tColorWarp3D")
        {
            std::string szImageName2;
            for (uint8_t j = 0; j < 4; ++j)
            {
                std::string szNewImageName = szImageName.substr(0, szImageName.find_last_of("."));
                szNewImageName += "_z" + std::to_string((j / 100) % 10) + std::to_string((j / 10) % 10) + std::to_string(j % 10) + ".tga";
                if (j == 0)
                    szImageName2 = szNewImageName;
            }
            if (szImageName2.empty())
                throw std::string("Invalid color warp 3D image.");
            out << "\tTextureColorWarp3D \"" << szImageName2 << "\"\n";
        }
        else if (szTexParamName == "g_tFresnelWarp")
        {
            std::string szImageName2 = szImageName.substr(0, szImageName.find_last_of(".")) + "_r.tga";
            std::string szImageName3 = szImageName.substr(0, szImageName.find_last_of(".")) + "_g.tga";
            std::string szImageName4 = szImageName.substr(0, szImageName.find_last_of(".")) + "_b.tga";

            ExtractImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName2, IMAGE_CHANNEL_RED);
            ExtractImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName3, IMAGE_CHANNEL_GREEN);
            ExtractImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName4, IMAGE_CHANNEL_BLUE);

            out << "\tTextureFresnelWarpRim \"" << szImageName2 << "\"\n";
            out << "\tTextureFresnelWarpColor \"" << szImageName3 << "\"\n";
            out << "\tTextureFresnelWarpSpec \"" << szImageName4 << "\"\n";
        }
        else if (szTexParamName == "g_tScrollSpeed")
        {
            std::string szImageName2 = szImageName.substr(0, szImageName.find_last_of(".")) + "_a.tga";
            std::string szImageName3 = szImageName.substr(0, szImageName.find_last_of(".")) + "_g.tga";

            ExtractImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName3, IMAGE_CHANNEL_GREEN);

            if ((*pIntParams)["F_TRANSLUCENT"] != NULL)
            {
                ExtractImageChannel(szDecompileDirectory + "\\" + szImageName, szDecompileDirectory + "\\" + szImageName2, IMAGE_CHANNEL_ALPHA);
                out << "\tTextureTranslucency \"" << szImageName2 << "\"\n";
            }
            out << "\tTextureScrollSpeed \"" << szImageName3 << "\"\n";
        }
        else if (szTexParamAliases.count(szTexParamName))
        {
            std::string szTexParamAlias = szTexParamAliases[szTexParamName];
            out << "\t" << szTexParamAlias << " \"" << szImageName << "\"\n";
        }
    }

    KeyValues* pIntAttribs = (KeyValues*)DataBlock["m_intAttributes"];
    if (pIntAttribs == NULL)
        throw std::string("Could not find expected key value \"m_intAttribs\".");

    KeyValues* pFloatAttribs = (KeyValues*)DataBlock["m_floatAttributes"];
    if (pFloatAttribs == NULL)
        throw std::string("Could not find expected key value \"m_floatAttributes\".");

    KeyValues* pVectorAttribs = (KeyValues*)DataBlock["m_vectorAttributes"];
    if (pVectorAttribs == NULL)
        throw std::string("Could not find expected key value \"m_vectorAttributes\".");

    if (pIntAttribs->size + pFloatAttribs->size + pVectorAttribs->size > 0)
    {
        out << "\n\tAttributes\n\t{\n";
        for (uint32_t i = 0; i < pIntAttribs->size; ++i)
        {
            KeyValues* pIntAttrib = (KeyValues*)pIntAttribs->data[i];
            out << "\t\t" << pIntAttrib->data[0] << " \"" << *(int32_t*)pIntAttrib->data[1] << "\"\n";
        }
        for (uint32_t i = 0; i < pFloatAttribs->size; ++i)
        {
            KeyValues* pFloatAttrib = (KeyValues*)pFloatAttribs->data[i];
            out << "\t\t" << pFloatAttrib->data[0] << " \"" << *(float*)pFloatAttrib->data[1] << "\"\n";
        }
        for (uint32_t i = 0; i < pVectorAttribs->size; ++i)
        {
            KeyValues* pVectorAttrib = (KeyValues*)pVectorAttribs->data[i];
            out << "\t\t" << pVectorAttrib->data[0] << std::fixed << " \"[" << *(float*)&pVectorAttrib->data[1][0] << " " << *(float*)&pVectorAttrib->data[1][4] << " " << *(float*)&pVectorAttrib->data[1][8] << " " << *(float*)&pVectorAttrib->data[1][12] << "]\"\n";
        }
        //Texture attributes not supported yet (mostly because I have yet to find an example of one that works...)
        out << "\t}\n";
    }
    out << "}";

    f.close();
}