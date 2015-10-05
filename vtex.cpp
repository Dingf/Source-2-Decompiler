#include <string.h>
#include <stdint.h>
#include <math.h>
#include <string>
#include <fstream>
#include "decompiler.h"
#include "vtex.h"

using std::ios;

void ExtractRGBAImage(std::fstream& in, const std::string& szFilenameOut, uint16_t nWidth, uint16_t nHeight, uint16_t nDepth, uint8_t nMipLevels, bool bGenerateMipmaps)
{
	char szHeader[22];
	char* szBuffer = new char[nWidth * 4];

	uint16_t nRealHeight = nHeight;

	std::fstream out;

	if (nMipLevels == 0)
		throw std::string("Invalid mip levels (must be at least 1).");
	else if (nMipLevels > 1)
	{
		nRealHeight = (uint16_t)(nRealHeight * (2.0 - pow(0.5, (double)(nMipLevels - 1))));
		nRealHeight += nMipLevels - 1;
	}

	for (uint8_t i = 0; i < nDepth && i < 0xFF; ++i)
	{
		std::string szNewFilenameOut;
		if (nDepth > 1)
		{
			szNewFilenameOut = szFilenameOut.substr(0, szFilenameOut.find_last_of("."));
			szNewFilenameOut += "_z" + std::to_string((i / 100) % 10) + std::to_string((i / 10) % 10) + std::to_string(i % 10) + ".tga";
			out.open(szNewFilenameOut, ios::out | ios::binary);
		}
		else
			out.open(szFilenameOut, ios::out | ios::binary);
		if (!out.is_open())
			throw std::string("Could not open file \"" + szFilenameOut + "\" for writing.");

		//TGA Header
		memset(szHeader, 0x00, 21);
		szHeader[0] = 0x04;
		szHeader[2] = 0x02;
		szHeader[12] = nWidth & 0x00FF;
		szHeader[13] = (nWidth & 0xFF00) >> 8;
		szHeader[14] = bGenerateMipmaps ? nRealHeight & 0x00FF : nHeight & 0x00FF;
		szHeader[15] = bGenerateMipmaps ? (nRealHeight & 0xFF00) >> 8 : (nHeight & 0xFF00) >> 8;
		szHeader[16] = 0x20;
		szHeader[17] = 0x20;
		szHeader[18] = bGenerateMipmaps ? nMipLevels : 1;
		szHeader[19] = nHeight & 0x00FF;
		szHeader[20] = (nHeight & 0xFF00) >> 8;
		szHeader[21] = 0x00;
		out.write(szHeader, 22);

		for (uint8_t j = nMipLevels; j > 0; j--)
		{
			for (uint32_t k = 0; k < nHeight / pow(2.0f, j - 1); ++k)
			{
				memset(szBuffer, 0, nWidth * 4);
				in.read(szBuffer, std::streamsize((4 * nWidth) / pow(2.0f, j - 1)));
				if ((!bGenerateMipmaps) && (j != 1))
					continue;

				char c;	
				for (uint32_t l = 0; l < (uint32_t)nWidth * 4; l += 4)
				{
					c = szBuffer[l];
					szBuffer[l] = szBuffer[l + 2];
					szBuffer[l + 2] = c;
				}
				out.write(szBuffer, nWidth * 4);
			}
			out.seekp((nWidth * 4), ios::cur);
		}
		out.close();
	}

	delete[] szBuffer;
}

void ExtractDXTImage(std::fstream& in, const std::string& szFilenameOut, uint16_t nWidth, uint16_t nHeight, uint16_t nDepth, uint8_t nMipLevels, uint8_t nImageFormat, bool bGenerateMipmaps)
{
	char szHeader[22];
	char szBuffer[16];

	char* szBlockBuffer = new char[nWidth * 16];

	uint8_t a[8];		//Alpha values
	uint32_t c[4];		//Color values (only the 24 LSB are used)
	uint16_t nRealHeight = nHeight;
	uint32_t nBlockOffset = 0;

	std::fstream out;

	if (nMipLevels == 0)
		throw std::string("Invalid mip levels (must be at least 1).");
	else if (nMipLevels > 1)
	{
		nRealHeight = (uint16_t)(nRealHeight * (2.0 - pow(0.5, (double)(nMipLevels - 1))));
		nRealHeight += nMipLevels - 1;
	}

	for (uint8_t i = 0; i < nDepth && i < 0xFF; ++i)
	{
		if (nDepth > 1)
		{
			std::string szNewFilenameOut = szFilenameOut.substr(0, szFilenameOut.find_last_of("."));
			//There's probably a better way to do this but meh...
			szNewFilenameOut += "_z" + std::to_string((nDepth / 100) % 10) + std::to_string((nDepth / 10) % 10) + std::to_string(nDepth % 10) + ".tga";
			out.open(szNewFilenameOut, ios::out | ios::binary);
		}
		else
			out.open(szFilenameOut, ios::out | ios::binary);
		if (!out.is_open())
			throw std::string("Could not open file \"" + szFilenameOut + "\" for writing.");

		//TGA Header
		memset(szHeader, 0x00, 21);
		szHeader[0] = 0x04;
		szHeader[2] = 0x02;
		szHeader[12] = nWidth & 0x00FF;
		szHeader[13] = (nWidth & 0xFF00) >> 8;
		szHeader[14] = bGenerateMipmaps ? nRealHeight & 0x00FF : nHeight & 0x00FF;
		szHeader[15] = bGenerateMipmaps ? (nRealHeight & 0xFF00) >> 8 : (nHeight & 0xFF00) >> 8;
		szHeader[16] = 0x20;
		szHeader[17] = 0x20;
		szHeader[18] = bGenerateMipmaps ? nMipLevels : 1;
		szHeader[19] = nHeight & 0x00FF;
		szHeader[20] = (nHeight & 0xFF00) >> 8;
		szHeader[21] = nImageFormat;
		out.write(szHeader, 22);

		for (uint8_t j = nMipLevels; j > 0; j--)
		{
			for (uint32_t k = 0; k < nHeight / pow(2.0, j + 1); ++k)
			{
				nBlockOffset = 0;
				memset(szBlockBuffer, 0, nWidth * 16);
				for (uint32_t l = 0; l < nWidth / pow(2.0, j + 1); ++l)
				{
					if (nImageFormat == VTEX_FORMAT_DXT5)
						in.read(szBuffer, 16);
					else if (nImageFormat == VTEX_FORMAT_DXT1)
						in.read(&szBuffer[8], 8);

					if (in.eof())
						throw std::string("Encountered unexpected EOF while extracting texture data.");

					if ((!bGenerateMipmaps) && (j != 1))
						continue;

					a[0] = szBuffer[0];
					a[1] = szBuffer[1];
					if (a[0] > a[1])
					{
						a[2] = (6 * a[0] + 1 * a[1]) / 7;
						a[3] = (5 * a[0] + 2 * a[1]) / 7;
						a[4] = (4 * a[0] + 3 * a[1]) / 7;
						a[5] = (3 * a[0] + 4 * a[1]) / 7;
						a[6] = (2 * a[0] + 5 * a[1]) / 7;
						a[7] = (1 * a[0] + 6 * a[1]) / 7;
					}
					else
					{
						a[2] = (4 * a[0] + 1 * a[1]) / 5;
						a[3] = (3 * a[0] + 2 * a[1]) / 5;
						a[4] = (2 * a[0] + 3 * a[1]) / 5;
						a[5] = (1 * a[0] + 4 * a[1]) / 5;
						a[6] = 0x00;
						a[7] = 0xFF;
					}

					c[0] = (((szBuffer[9] >> 3) & 0x1F) << 19) | ((((szBuffer[9] & 0x07) << 3) | ((szBuffer[8] >> 5) & 0x07)) << 10) | ((szBuffer[8] & 0x1F) << 3);
					c[1] = (((szBuffer[11] >> 3) & 0x1F) << 19) | ((((szBuffer[11] & 0x07) << 3) | ((szBuffer[10] >> 5) & 0x07)) << 10) | ((szBuffer[10] & 0x1F) << 3);
					if (c[0] > c[1])
					{
						c[2] = ((2 * (c[0] & 0x000000FF) + 1 * (c[1] & 0x000000FF)) / 3) |
							(((2 * ((c[0] & 0x0000FF00) >> 8) + 1 * ((c[1] & 0x0000FF00) >> 8)) / 3) << 8) |
							(((2 * ((c[0] & 0x00FF0000) >> 16) + 1 * ((c[1] & 0x00FF0000) >> 16)) / 3) << 16);

						c[3] = ((1 * (c[0] & 0x000000FF) + 2 * (c[1] & 0x000000FF)) / 3) |
							(((1 * ((c[0] & 0x0000FF00) >> 8) + 2 * ((c[1] & 0x0000FF00) >> 8)) / 3) << 8) |
							(((1 * ((c[0] & 0x00FF0000) >> 16) + 2 * ((c[1] & 0x00FF0000) >> 16)) / 3) << 16);
					}
					else
					{
						c[2] = (((c[0] & 0x000000FF) + (c[1] & 0x000000FF)) / 2) |
							(((((c[0] & 0x0000FF00) >> 8) + ((c[1] & 0x0000FF00) >> 8)) / 2) << 8) |
							(((((c[0] & 0x00FF0000) >> 16) + ((c[1] & 0x00FF0000) >> 16)) / 2) << 16);

						c[3] = 0x00000000;
					}

					for (uint8_t m = 0; m < 2; ++m)
					{
						uint32_t ai = *(uint32_t*)&szBuffer[2 + (m * 3)];
						uint16_t ci = *(uint16_t*)&szBuffer[12 + (m * 2)];

						for (uint8_t n = 0; n < 4; ++n)
						{
							memcpy(&szBlockBuffer[nBlockOffset], &c[(ci >> (n * 2)) & 0x03], 3);
							nBlockOffset += 3;
							if (nImageFormat == VTEX_FORMAT_DXT5)
								szBlockBuffer[nBlockOffset++] = a[(ai >> (n * 3)) & 0x07];
							else if (nImageFormat == VTEX_FORMAT_DXT1)
								szBlockBuffer[nBlockOffset++] = (char)0xFF;
						}
						nBlockOffset += (nWidth * 4) - 16;
						for (uint8_t n = 0; n < 4; ++n)
						{
							memcpy(&szBlockBuffer[nBlockOffset], &c[(ci >> (n * 2)) & 0x03], 3);
							nBlockOffset += 3;
							if (nImageFormat == VTEX_FORMAT_DXT5)
								szBlockBuffer[nBlockOffset++] = a[(ai >> (n * 3)) & 0x07];
							else if (nImageFormat == VTEX_FORMAT_DXT1)
								szBlockBuffer[nBlockOffset++] = (char)0xFF;
						}
						nBlockOffset += (nWidth * 4) - 16;
					}
					nBlockOffset = (l + 1) * 16;
				}

				if ((bGenerateMipmaps) || (j == 1))
					out.write(szBlockBuffer, nWidth * 16);
			}
			if (bGenerateMipmaps)
				out.seekp((nWidth * 4), ios::cur);
		}
		out.close();
	}

	delete[] szBlockBuffer;
}

void ExtractPNGImage(std::fstream& f, const std::string& szFilenameOut)
{
	std::fstream out;
	out.open(szFilenameOut, ios::out | ios::binary);
	if (!out.is_open())
		throw std::string("Could not open file \"" + szFilenameOut + "\" for writing.");
	else
	{
		out << f.rdbuf();
		out.close();
	}
}

void ExtractFrameImage(const std::string& szImageName, const std::string& szFilenameOut, float * fCoords)
{
	char* szBuffer;
	char szHeader[18];

	uint16_t nWidth, nHeight;
	uint8_t nMipLevels;

	std::streamoff p;
	std::fstream in, out;

	in.open(szImageName, ios::in | ios::binary);
	if (!in.is_open())
		throw std::string("Could not open file \"" + szImageName + "\" for reading.");

	out.open(szFilenameOut, ios::out | ios::binary);
	if (!out.is_open())
		throw std::string("Could not open file \"" + szFilenameOut + "\" for writing.");

	in.read(szHeader, 1);
	if (szHeader[0] != 4)
		throw std::string("Texture metadata not found for \"" + szImageName + "\".");
	in.seekg(12);
	in.read((char*)&nWidth, 2);
	in.seekg(4, ios::cur);
	in.read((char*)&nMipLevels, 1);
	in.read((char*)&nHeight, 2);
	in.seekg(1, ios::cur);
	if (nMipLevels > 1)
	{
		in.seekg(std::streamsize(nHeight * nWidth * (1.0f - pow(0.5f, (float)(nMipLevels - 1))) * 4), ios::cur);
		in.seekg(std::streamsize(nWidth * 4 * (nMipLevels - 1)), ios::cur);
	}

	uint32_t nFrameWidth = (uint32_t)ceil(fCoords[2] * nWidth) - (uint32_t)floor(fCoords[0] * nWidth);
	uint32_t nFrameHeight = (uint32_t)ceil(fCoords[3] * nHeight) - (uint32_t)floor(fCoords[1] * nHeight);

	memset(szHeader, 0x00, 18);
	szHeader[2] = 0x02;
	szHeader[12] = nFrameWidth & 0x00FF;
	szHeader[13] = (nFrameWidth & 0xFF00) >> 8;
	szHeader[14] = nFrameHeight & 0x00FF;
	szHeader[15] = (nFrameHeight & 0xFF00) >> 8;
	szHeader[16] = 0x20;
	szHeader[17] = 0x20;

	out.write(szHeader, 18);

	szBuffer = new char[nFrameWidth * 4];
	in.seekg((int32_t(fCoords[0] * nWidth) * 4) + (int32_t(fCoords[1] * nHeight) * nWidth * 4), ios::cur);
	for (uint32_t i = 0; i < nFrameHeight; ++i)
	{
		p = in.tellg();
		in.read(szBuffer, nFrameWidth * 4);
		out.write(szBuffer, nFrameWidth * 4);
		in.seekg(p + nWidth * 4);
	}

	in.close();
	out.close();

	delete[] szBuffer;
}

void ExtractSheetData(KeyValues* pSheetData, const std::string& szImageName, const std::string& szFilenameOut)
{
	uint32_t i, j, k;

	std::fstream out;
	out.open(szFilenameOut, ios::out | ios::binary);
	if (!out.is_open())
		throw std::string("Could not open file \"" + szFilenameOut + "\" for writing.");

	KeyValues* pSequenceData = (KeyValues*)(*pSheetData)["m_Sequences"];
	for (i = 0; i < pSequenceData->size; ++i)
	{
		KeyValues* pSequence = (KeyValues*)pSequenceData->data[i];
		KeyValues* pFrames = (KeyValues*)(*pSequence)["m_Frames"];

		out << "sequence-rgba " << *(uint32_t*)(*pSequence)["m_nId"] << "\n";
		if (!(*(bool *)(*pSequence)["m_bClamp"]))
			out << "loop\n";

		for (j = 0; j < pFrames->size; ++j)
		{
			KeyValues* pFrame = (KeyValues*)(pFrames->data[j]);
			KeyValues* pImages = (KeyValues*)(*pFrame)["m_Images"];
			out << "frame ";
			for (k = 0; k < pImages->size; ++k)
			{
				KeyValues* pImage = (KeyValues*)(pImages->data[k]);
				std::string szFrameName = szFilenameOut.substr(0, szFilenameOut.find_last_of(".")) + "_" + std::to_string(i + 1) + "_" + std::to_string(j + 1) + "_" + std::to_string(k + 1) + ".tga";
				ExtractFrameImage(szImageName, szFrameName, (float *)(*pImage)["uvUncropped"]);

				out << szFrameName.substr(szFrameName.find_last_of("\\/") + 1) << " ";
			}
			out << *(float *)(*pFrame)["m_flDisplayTime"] << "\n";
		}
	}
}

void BuildCubeMap(const std::string& szImageName, const std::string& szFilenameOut)
{
	char* szBuffer;
	char* szBlockBuffer;
	char szHeader[22];

	uint32_t nBufferSize;
	uint32_t nBlockBufferSize;
	uint16_t nWidth, nHeight;

	std::fstream in, out;

	in.open(szImageName, ios::in | ios::binary);
	if (!in.is_open())
		throw std::string("Could not open file \"" + szImageName + "\" for reading.");

	in.read(szHeader, 22);
	if (szHeader[0] != 4)
		throw std::string("Texture metadata not found for \"" + szImageName + "\".");
	if (szHeader[16] != 0x20)
		throw std::string("Cube maps building only supports 32-bit TGA images.");

	nWidth = (uint8_t)szHeader[12] | ((uint8_t)szHeader[13] << 8);
	nHeight = (uint8_t)szHeader[14] | ((uint8_t)szHeader[15] << 8);
	nBufferSize = nWidth * nHeight * 4;
	szBuffer = new char[nBufferSize];

	memset(szBuffer, 0, nBufferSize);
	in.read(szBuffer, nBufferSize);
	in.close();

	szHeader[12] = (nWidth * 4) & 0x00FF;
	szHeader[13] = ((nWidth * 4) & 0xFF00) >> 8;
	szHeader[14] = (nWidth * 3) & 0x00FF;
	szHeader[15] = ((nWidth * 3) & 0xFF00) >> 8;

	out.open(szFilenameOut, ios::out | ios::binary);
	if (!out.is_open())
		throw std::string("Could not open file \"" + szFilenameOut + "\" for writing.");
	out.write(szHeader, 22);

	nBlockBufferSize = nWidth * nWidth * 4 * 4;
	szBlockBuffer = new char[nBlockBufferSize];

	//Write upper third
	memset(szBlockBuffer, 0, nBlockBufferSize);
	for (uint32_t i = 0; i < nWidth; ++i)
	{
		memcpy(&szBlockBuffer[(i * nWidth * 16) + (nWidth * 4)], &szBuffer[(nWidth * nWidth * 16) + (i * nWidth * 4)], nWidth * 4);
	}
	out.write(szBlockBuffer, nBlockBufferSize);

	//Write middle third
	memset(szBlockBuffer, 0, nBlockBufferSize);
	for (uint32_t i = 0; i < nWidth; ++i)
	{
		uint32_t nIndex = i * nWidth * 16;
		uint32_t nOffset = i * nWidth * 4;
		memcpy(&szBlockBuffer[nIndex], &szBuffer[nOffset + (nWidth * nWidth * 4) ], nWidth * 4);
		memcpy(&szBlockBuffer[nIndex + (nWidth * 4)], &szBuffer[nOffset + (nWidth * nWidth * 8)], nWidth * 4);
		memcpy(&szBlockBuffer[nIndex + (nWidth * 8)], &szBuffer[nOffset], nWidth * 4);
		memcpy(&szBlockBuffer[nIndex + (nWidth * 12)], &szBuffer[nOffset + (nWidth * nWidth * 12)], nWidth * 4);
	}
	out.write(szBlockBuffer, nBlockBufferSize);

	//Write lower third
	memset(szBlockBuffer, 0, nBlockBufferSize);
	for (uint32_t i = 0; i < nWidth; ++i)
	{
		memcpy(&szBlockBuffer[(i * nWidth * 16) + (nWidth * 4)], &szBuffer[(nWidth * nWidth * 20) + (i * nWidth * 4)], nWidth * 4);
	}
	out.write(szBlockBuffer, nBlockBufferSize);
	out.close();

	delete[] szBlockBuffer;
	delete[] szBuffer;
}


void ManipulateImageChannel(const std::string& szImageName, const std::string& szFilenameOut, void (*pFunction)(char**, uint8_t, uint8_t), uint8_t nImageChannel1, uint8_t nImageChannel2)
{
	char* szBuffer;
	char szHeader[22];

	uint32_t nBufferSize;
	uint16_t nWidth, nHeight;

	std::fstream in, out;

	in.open(szImageName, ios::in | ios::binary);
	if (!in.is_open())
		throw std::string("Could not open file \"" + szImageName + "\" for reading.");

	in.read(szHeader, 22);
	if (szHeader[0] != 4)
		throw std::string("Texture metadata not found for \"" + szImageName + "\".");
	if (szHeader[16] != 0x20)
		throw std::string("Channel manipulation only available for 32-bit TGA images.");

	nWidth = (uint8_t)szHeader[12] | ((uint8_t)szHeader[13] << 8);
	nHeight = (uint8_t)szHeader[14] | ((uint8_t)szHeader[15] << 8);
	nBufferSize = nWidth * nHeight * 4;
	szBuffer = new char[nBufferSize];

	memset(szBuffer, 0, nBufferSize);
	in.read(szBuffer, nBufferSize);
	in.close();

	out.open(szFilenameOut, ios::out | ios::binary);
	if (!out.is_open())
		throw std::string("Could not open file \"" + szFilenameOut + "\" for writing.");
	out.write(szHeader, 22);

	char* szBufferPos = szBuffer;
	for (uint32_t i = 0; i < (uint32_t)nWidth * (uint32_t)nHeight; ++i)
	{
		pFunction(&szBufferPos, nImageChannel1, nImageChannel2);
		szBufferPos += 4;
	}
	out.write(szBuffer, nBufferSize);
	out.close();

	delete[] szBuffer;
}

void ChannelExtract(char** szBuffer, uint8_t nImageChannel, uint8_t uUnused)
{
	char c = (*szBuffer)[3 - nImageChannel];
	(*szBuffer)[0] = (*szBuffer)[1] = (*szBuffer)[2] = (*szBuffer)[3] = c;
}

void ExtractImageChannel(const std::string& szImageName, const std::string& szFilenameOut, ImageChannel nImageChannel)
{
	if (nImageChannel > SIZEOF_IMAGE_CHANNEL)
		throw std::string("Invalid image channel specified.");
	ManipulateImageChannel(szImageName, szFilenameOut, &ChannelExtract, nImageChannel, 0);
}

void ChannelFill(char** szBuffer, uint8_t nImageChannel, uint8_t uValue)
{
	char c = (*szBuffer)[3 - nImageChannel];
	(*szBuffer)[3 - nImageChannel] = uValue;
}
void FillImageChannel(const std::string& szImageName, const std::string& szFilenameOut, ImageChannel nImageChannel, uint8_t uValue)
{
	if (nImageChannel > SIZEOF_IMAGE_CHANNEL)
		throw std::string("Invalid image channel specified.");
	ManipulateImageChannel(szImageName, szFilenameOut, &ChannelFill, nImageChannel, uValue);
}

void ChannelSwap(char** szBuffer, uint8_t nImageChannel1, uint8_t nImageChannel2)
{
	char c = (*szBuffer)[3 - nImageChannel1];
	(*szBuffer)[3 - nImageChannel1] = (*szBuffer)[3 - nImageChannel2];
	(*szBuffer)[3 - nImageChannel2] = c;
}

void SwapImageChannel(const std::string& szImageName, const std::string& szFilenameOut, ImageChannel nImageChannel1, ImageChannel nImageChannel2)
{
	if ((nImageChannel1 > SIZEOF_IMAGE_CHANNEL) || (nImageChannel2 > SIZEOF_IMAGE_CHANNEL))
		throw std::string("Invalid image channel specified.");
	ManipulateImageChannel(szImageName, szFilenameOut, &ChannelSwap, nImageChannel1, nImageChannel2);
}

void S2Decompiler::OutputVTEX(const KeyValues& DataBlock, const KeyValues& NTROBlock, std::fstream& f, const std::string& szOutputName)
{
	uint16_t nWidth      = *(uint16_t*)DataBlock["m_nWidth"];
	uint16_t nHeight     = *(uint16_t*)DataBlock["m_nHeight"];
	uint16_t nDepth      = *(uint16_t*)DataBlock["m_nDepth"];
	uint8_t nImageFormat = *(uint8_t*) DataBlock["m_nImageFormat"];
	uint8_t nMipLevels   = *(uint8_t*) DataBlock["m_nNumMipLevels"];
	uint16_t nFlags      = *(uint16_t*)DataBlock["m_nFlags"];

	KeyValues* pSheetData = NULL;
	if (DataBlock["m_extraData"])
	{
		for (uint32_t i = 0; i < ((KeyValues*)DataBlock["m_extraData"])->size; ++i)
		{
			KeyValues* pExtraData = (KeyValues*)((KeyValues*)DataBlock["m_extraData"])->data[i];
			if (strncmp(pExtraData->name[0], "Sheet_t\0", 8) == 0)
			{
				pSheetData = (KeyValues*)pExtraData->data[0];
				break;
			}
		}
	}

	bool bBuildCubeMap = (nFlags & VTEX_FLAG_CUBE_TEXTURE) == VTEX_FLAG_CUBE_TEXTURE;
	bool bGenerateVTEX = !(_nDecompilerFlags & DECOMPILER_FLAG_VTEX_NO_VTEX_FILE);
	bool bGenerateMipmaps = !(_nDecompilerFlags & DECOMPILER_FLAG_VTEX_NO_MIPMAPS) && !bBuildCubeMap && pSheetData;

	std::string szBaseName = szOutputName.substr(0, szOutputName.length() - 5);
	if ((szBaseName.substr(szBaseName.length() - 4) == "_png") || (szBaseName.substr(szBaseName.length() - 4) == "_tga"))
		szBaseName = szBaseName.substr(0, szBaseName.length() - 4);

	std::string szImageName = szBaseName + ((nImageFormat == VTEX_FORMAT_PNG) ? ".png" : ".tga");
	std::string szSheetName = szBaseName + ".mks";

	if (bBuildCubeMap)
		nHeight *= 6;

	if ((nImageFormat == VTEX_FORMAT_DXT1) || (nImageFormat == VTEX_FORMAT_DXT5))
		ExtractDXTImage(f, szImageName, nWidth, nHeight, nDepth, nMipLevels, nImageFormat, bGenerateMipmaps);
	else if (nImageFormat == VTEX_FORMAT_RGBA8888)
		ExtractRGBAImage(f, szImageName, nWidth, nHeight, nDepth, nMipLevels, bGenerateMipmaps);
	else if (nImageFormat == VTEX_FORMAT_PNG)
		ExtractPNGImage(f, szImageName);
	else
		throw std::string("Unsupported image format: \"" + std::to_string(nImageFormat) + "\".");

	if (bBuildCubeMap)
		BuildCubeMap(szImageName, szImageName);

	if (pSheetData)
		ExtractSheetData(pSheetData, szImageName, szSheetName);

	if (bGenerateVTEX)
	{
		std::fstream out;
		out.open(szOutputName, ios::out);
		if (!out.is_open())
			throw std::string("Could not open file \"" + szImageName + "\" for writing.");

		//This is currently very limited... need to find more examples of raw VTEX format with different options
		out << "<!-- dmx encoding keyvalues2_noids 1 format vtex 1 -->\n";
		out << "// This file has been auto-generated by Source 2 Decompiler\n";
		out << "// https://github.com/Dingf/Source-2-Decompiler\n\n";
		out << "\"CDmeVtex\"\n{\n";
		out << "\t\"m_inputTextureArray\" \"element_array\"\n\t[\n";
		out << "\t\t\"CDmeInputTexture\"\n\t\t{\n";
		out << "\t\t\t\"m_name\"\t\"string\"\t\"0\"\n";

		std::string szInputName = ((pSheetData) ? szSheetName : szImageName);
		for (uint32_t i = 0; i < szInputName.size(); ++i)
			szInputName[i] = (szInputName[i] == '\\') ? '/' : szInputName[i];
		if (szInputName.substr(0, 2) == "./")
			szInputName = szInputName.substr(2);
		if ((nDepth > 1) && (szInputName.substr(szInputName.length() - 4) == ".tga"))
			szInputName = szInputName.substr(0, szInputName.length() - 4) + "_z000" + szInputName.substr(szInputName.length() - 4);

		out << "\t\t\t\"m_fileName\"\t\"string\"\t\"" << szInputName << "\"\n";
		out << "\t\t\t\"m_colorSpace\"\t\"string\"\t\"srgb\"\n";
		out << "\t\t\t\"m_typeString\"\t\"string\"\t\"" << (nDepth > 1 ? "3D" : "2D") << "\"\n";
		out << "\t\t}\n\t]\n";
		out << "\t\"m_outputTypeString\"\t\"string\"\t\"" << (nDepth > 1 ? "3D" : "2D") << "\"\n";
		out << "\t\"m_outputFormat\"\t\"string\"\t\"";
		if (nImageFormat == VTEX_FORMAT_DXT5)
			out << "DXT5\"\n";
		else if (nImageFormat == VTEX_FORMAT_DXT1)
			out << "DXT1\"\n";
		else if ((nImageFormat == VTEX_FORMAT_RGBA8888) || (nImageFormat == VTEX_FORMAT_PNG))
			out << "RGBA8888\"\n";
		else
			out << "\"\n";
		out << "\t\"m_textureOutputChannelArray\"\t\"element_array\"\n\t[\n";
		out << "\t\t\"CDmeTextureOutputChannel\"\n\t\t{\n";
		out << "\t\t\t\"m_inputTextureArray\"\t\"string_array\"\n";
		out << "\t\t\t[\n\t\t\t\t\"0\"\n\t\t\t]\n";
		out << "\t\t\t\"m_srcChannels\"\t\"string\"\t\"rgba\"\n";
		out << "\t\t\t\"m_dstChannels\"\t\"string\"\t\"rgba\"\n";
		out << "\t\t\t\"m_mipAlgorithm\"\t\"CDmeImageProcessor\"\n\t\t\t{\n";
		out << "\t\t\t\t\"m_algorithm\"\t\"string\"\t\"\"\n";
		out << "\t\t\t\t\"m_stringArg\"\t\"string\"\t\"\"\n";
		out << "\t\t\t\t\"m_vFloat4Arg\"\t\"vector4\"\t\"0 0 0 0\"\n";
		out << "\t\t\t}\n\t\t\t\"m_outputColorSpace\"\t\"string\"\t\"srgb\"\n";
		out << "\t\t}\n\t]\n}";
	}
}