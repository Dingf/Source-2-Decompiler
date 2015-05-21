#include <string.h>
#include <stdint.h>
#include <math.h>
#include <string>
#include <fstream>
#include "decompiler.h"
#include "vtex.h"

using std::ios;

void ExtractRGBAImage(std::fstream& in, const std::string& szFilenameOut, uint16_t uWidth, uint16_t uHeight, uint16_t uDepth, uint8_t uMipLevels, bool bGenerateMipmaps)
{
	char szHeader[22];
	char * szBuffer = new char[uWidth * 4];

	uint16_t uRealHeight = uHeight;

	std::fstream out;

	if (uMipLevels == 0)
		throw std::string("Invalid mip levels (must be at least 1).");
	else if (uMipLevels > 1)
	{
		uRealHeight = (uint16_t)(uRealHeight * (2.0 - pow(0.5, (double)(uMipLevels - 1))));
		uRealHeight += uMipLevels - 1;
	}

	for (uint8_t i = 0; i < uDepth && i < 0xFF; i++)
	{
		if (uDepth > 1)
		{
			std::string szNewFilenameOut = szFilenameOut.substr(0, szFilenameOut.find_last_of("."));
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
		szHeader[12] = uWidth & 0x00FF;
		szHeader[13] = (uWidth & 0xFF00) >> 8;
		szHeader[14] = bGenerateMipmaps ? uRealHeight & 0x00FF : uHeight & 0x00FF;
		szHeader[15] = bGenerateMipmaps ? (uRealHeight & 0xFF00) >> 8 : (uHeight & 0xFF00) >> 8;
		szHeader[16] = 0x20;
		szHeader[17] = 0x20;
		szHeader[18] = bGenerateMipmaps ? uMipLevels : 1;
		szHeader[19] = uHeight & 0x00FF;
		szHeader[20] = (uHeight & 0xFF00) >> 8;
		szHeader[21] = 0x00;
		out.write(szHeader, 22);

		for (uint8_t j = uMipLevels; j > 0; j--)
		{
			for (uint32_t k = 0; k < uHeight / pow(2.0f, j - 1); k++)
			{
				memset(szBuffer, 0, uWidth * 4);
				in.read(szBuffer, (4 * uWidth) / pow(2.0f, j - 1));
				if ((!bGenerateMipmaps) && (j != 1))
					continue;
				out.write(szBuffer, uWidth * 4);
			}
			out.seekp((uWidth * 4), ios::cur);
		}
		out.close();
	}

	delete[] szBuffer;
}

void ExtractDXTImage(std::fstream& in, const std::string& szFilenameOut, uint16_t uWidth, uint16_t uHeight, uint16_t uDepth, uint8_t uMipLevels, uint8_t uImageFormat, bool bGenerateMipmaps)
{
	char szHeader[22];
	char szBuffer[16];

	char * szBlockBuffer = new char[uWidth * 16];

	uint8_t a[8];		//Alpha values
	uint32_t c[4];		//Color values (only the 24 LSB are used)
	uint16_t uRealHeight = uHeight;
	uint32_t uBlockOffset = 0;

	std::fstream out;

	if (uMipLevels == 0)
		throw std::string("Invalid mip levels (must be at least 1).");
	else if (uMipLevels > 1)
	{
		uRealHeight = (uint16_t)(uRealHeight * (2.0 - pow(0.5, (double)(uMipLevels - 1))));
		uRealHeight += uMipLevels - 1;
	}

	for (uint8_t i = 0; i < uDepth && i < 0xFF; i++)
	{
		if (uDepth > 1)
		{
			std::string szNewFilenameOut = szFilenameOut.substr(0, szFilenameOut.find_last_of("."));
			//There's probably a better way to do this but meh...
			szNewFilenameOut += "_z" + std::to_string((uDepth / 100) % 10) + std::to_string((uDepth / 10) % 10) + std::to_string(uDepth % 10) + ".tga";
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
		szHeader[12] = uWidth & 0x00FF;
		szHeader[13] = (uWidth & 0xFF00) >> 8;
		szHeader[14] = bGenerateMipmaps ? uRealHeight & 0x00FF : uHeight & 0x00FF;
		szHeader[15] = bGenerateMipmaps ? (uRealHeight & 0xFF00) >> 8 : (uHeight & 0xFF00) >> 8;
		szHeader[16] = 0x20;
		szHeader[17] = 0x20;
		szHeader[18] = bGenerateMipmaps ? uMipLevels : 1;
		szHeader[19] = uHeight & 0x00FF;
		szHeader[20] = (uHeight & 0xFF00) >> 8;
		szHeader[21] = uImageFormat;
		out.write(szHeader, 22);

		for (uint8_t j = uMipLevels; j > 0; j--)
		{
			for (uint32_t k = 0; k < uHeight / pow(2.0, j + 1); k++)
			{
				uBlockOffset = 0;
				memset(szBlockBuffer, 0, uWidth * 16);
				for (uint32_t l = 0; l < uWidth / pow(2.0, j + 1); l++)
				{
					if (uImageFormat == IMAGE_FORMAT_DXT5)
						in.read(szBuffer, 16);
					else if (uImageFormat == IMAGE_FORMAT_DXT1)
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

					for (uint8_t m = 0; m < 2; m++)
					{
						uint32_t ai = *(uint32_t *)&szBuffer[2 + (m * 3)];
						uint16_t ci = *(uint16_t *)&szBuffer[12 + (m * 2)];

						for (uint8_t n = 0; n < 4; n++)
						{
							memcpy(&szBlockBuffer[uBlockOffset], &c[(ci >> (n * 2)) & 0x03], 3);
							uBlockOffset += 3;
							if (uImageFormat == IMAGE_FORMAT_DXT5)
								szBlockBuffer[uBlockOffset++] = a[(ai >> (n * 3)) & 0x07];
							else if (uImageFormat == IMAGE_FORMAT_DXT1)
								szBlockBuffer[uBlockOffset++] = (char)0xFF;
						}
						uBlockOffset += (uWidth * 4) - 16;
						for (uint8_t n = 0; n < 4; n++)
						{
							memcpy(&szBlockBuffer[uBlockOffset], &c[(ci >> (n * 2)) & 0x03], 3);
							uBlockOffset += 3;
							if (uImageFormat == IMAGE_FORMAT_DXT5)
								szBlockBuffer[uBlockOffset++] = a[(ai >> (n * 3)) & 0x07];
							else if (uImageFormat == IMAGE_FORMAT_DXT1)
								szBlockBuffer[uBlockOffset++] = (char)0xFF;
						}
						uBlockOffset += (uWidth * 4) - 16;
					}
					uBlockOffset = (l + 1) * 16;
				}

				if ((bGenerateMipmaps) || (j == 1))
					out.write(szBlockBuffer, uWidth * 16);
			}
			if (bGenerateMipmaps)
				out.seekp((uWidth * 4), ios::cur);
		}
		out.close();
	}

	delete[] szBlockBuffer;
}

void ExtractFrameImage(const std::string& szImageName, const std::string& szFilenameOut, float * szCoords)
{
	char szHeader[18];

	uint16_t uImageWidth, uImageHeight;
	uint8_t uMipLevels;

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
	in.seekg(11);
	in.read((char*)&uImageWidth, 2);
	in.seekg(4, ios::cur);
	in.read((char*)&uMipLevels, 1);
	in.read((char*)&uImageHeight, 2);
	in.seekg(1, ios::cur);
	if (uMipLevels > 1)
	{
		in.seekg(uImageHeight * uImageWidth * (1.0f - pow(0.5f, (float)(uMipLevels - 1))) * 4, ios::cur);
		in.seekg(uImageWidth * 4 * (uMipLevels - 1), ios::cur);
	}

	uint32_t uFrameWidth = (uint32_t)ceil(szCoords[2] * uImageWidth) - (uint32_t)floor(szCoords[0] * uImageWidth);
	uint32_t uFrameHeight = (uint32_t)ceil(szCoords[3] * uImageHeight) - (uint32_t)floor(szCoords[1] * uImageHeight);

	memset(szHeader, 0x00, 18);
	szHeader[2] = 0x02;
	szHeader[12] = uFrameWidth & 0x00FF;
	szHeader[13] = (uFrameWidth & 0xFF00) >> 8;
	szHeader[14] = uFrameHeight & 0x00FF;
	szHeader[15] = (uFrameHeight & 0xFF00) >> 8;
	szHeader[16] = 0x20;
	szHeader[17] = 0x20;

	out.write(szHeader, 18);

	char * szBuffer = new char[uFrameWidth * 4];
	in.seekg((floor(szCoords[0] * uImageWidth) * 4) + (floor(szCoords[1] * uImageHeight) * uImageWidth * 4), ios::cur);
	for (uint32_t i = 0; i < uFrameHeight; i++)
	{
		p = in.tellg();
		in.read(szBuffer, uFrameWidth * 4);
		out.write(szBuffer, uFrameWidth * 4);
		in.seekg(p + uImageWidth * 4);
	}
	delete[] szBuffer;

	in.close();
	out.close();
}

void ExtractSheetData(std::fstream& in, const std::string& szImageName, const std::string& szFilenameOut)
{
	char szBuffer[4];

	float fDisplayTime;
	float szCoords[4];

	uint32_t uSequenceCount, uFrameCount, uImageCount;
	
	std::streamoff p1, p2;
	std::fstream out;

	out.open(szFilenameOut, ios::out);
	if (!out.is_open())
		throw "Could not open file \"" + szFilenameOut + "\" for writing.";
	
	in.read(szBuffer, 4);
	in.read((char*)&uSequenceCount, 4);
	in.seekg(*(int*)szBuffer - 8, ios::cur);
	for (uint32_t i = 0; i < uSequenceCount; i++)
	{
		in.read(szBuffer, 4);
		out << "sequence-rgba " << *(int*)szBuffer << "\n";
		in.read(szBuffer, 4);
		if (!(szBuffer[0] & 0x01))
			out << "loop\n";
		in.read(szBuffer, 4);
		in.read((char*)&uFrameCount, 4);
		in.seekg(16, ios::cur);
		p1 = in.tellg();
		in.seekg(*(int*)szBuffer - 24, ios::cur);
		for (uint32_t j = 0; j < uFrameCount; j++)
		{
			out << "frame ";
			in.read((char*)&fDisplayTime, 4);
			in.read(szBuffer, 4);
			in.read((char*)&uImageCount, 4);
			p2 = in.tellg();
			in.seekg(*(int*)szBuffer - 8, ios::cur);
			for (uint32_t k = 0; k < uImageCount; k++)
			{
				in.seekg(16, ios::cur);
				in.read((char*)szCoords, 16);

				std::string szFrameName = szFilenameOut.substr(0, szFilenameOut.find_last_of(".")) + "_" + std::to_string(i + 1) + "_" + std::to_string(j + 1) + "_" + std::to_string(k + 1) + ".tga";
				ExtractFrameImage(szImageName, szFrameName, szCoords);

				out << szFrameName.substr(szFrameName.find_last_of("\\/") + 1) << " ";
			}
			out << fDisplayTime << "\n";
			in.seekg(p2);
		}
		out << "\n";
		in.seekg(p1);
	}
}

void BuildCubeMap(const std::string& szImageName, const std::string& szFilenameOut)
{
	char * szBuffer;
	char * szBlockBuffer;
	char szHeader[22];

	uint32_t uBufferSize;
	uint32_t uBlockBufferSize;
	uint16_t uImageWidth, uImageHeight;

	std::fstream in, out;

	in.open(szImageName, ios::in | ios::binary);
	if (!in.is_open())
		throw std::string("Could not open file \"" + szImageName + "\" for reading.");

	in.read(szHeader, 22);
	if (szHeader[0] != 4)
		throw std::string("Texture metadata not found for \"" + szImageName + "\".");
	if (szHeader[16] != 0x20)
		throw std::string("Cube maps building only supports 32-bit TGA images.");

	uImageWidth = (uint8_t)szHeader[12] | ((uint8_t)szHeader[13] << 8);
	uImageHeight = (uint8_t)szHeader[14] | ((uint8_t)szHeader[15] << 8);
	uBufferSize = uImageWidth * uImageHeight * 4;
	szBuffer = new char[uBufferSize];

	memset(szBuffer, 0, uBufferSize);
	in.read(szBuffer, uBufferSize);
	in.close();

	szHeader[12] = (uImageWidth * 4) & 0x00FF;
	szHeader[13] = ((uImageWidth * 4) & 0xFF00) >> 8;
	szHeader[14] = (uImageWidth * 3) & 0x00FF;
	szHeader[15] = ((uImageWidth * 3) & 0xFF00) >> 8;

	out.open(szFilenameOut, ios::out | ios::binary);
	if (!out.is_open())
		throw std::string("Could not open file \"" + szFilenameOut + "\" for writing.");
	out.write(szHeader, 22);

	uBlockBufferSize = uImageWidth * uImageWidth * 4 * 4;
	szBlockBuffer = new char[uBlockBufferSize];

	//Write upper third
	memset(szBlockBuffer, 0, uBlockBufferSize);
	for (uint32_t i = 0; i < uImageWidth; i++)
	{
		memcpy(&szBlockBuffer[(i * uImageWidth * 16) + (uImageWidth * 4)], &szBuffer[(uImageWidth * uImageWidth * 16) + (i * uImageWidth * 4)], uImageWidth * 4);
	}
	out.write(szBlockBuffer, uBlockBufferSize);

	//Write middle third
	memset(szBlockBuffer, 0, uBlockBufferSize);
	for (uint32_t i = 0; i < uImageWidth; i++)
	{
		memcpy(&szBlockBuffer[(i * uImageWidth * 16)], &szBuffer[(uImageWidth * uImageWidth * 4) + (i * uImageWidth * 4)], uImageWidth * 4);
		memcpy(&szBlockBuffer[(i * uImageWidth * 16) + (uImageWidth * 4)], &szBuffer[(uImageWidth * uImageWidth * 8) + (i * uImageWidth * 4)], uImageWidth * 4);
		memcpy(&szBlockBuffer[(i * uImageWidth * 16) + (uImageWidth * 8)], &szBuffer[(i * uImageWidth * 4)], uImageWidth * 4);
		memcpy(&szBlockBuffer[(i * uImageWidth * 16) + (uImageWidth * 12)], &szBuffer[(uImageWidth * uImageWidth * 12) + (i * uImageWidth * 4)], uImageWidth * 4);
	}
	out.write(szBlockBuffer, uBlockBufferSize);

	//Write lower third
	memset(szBlockBuffer, 0, uBlockBufferSize);
	for (uint32_t i = 0; i < uImageWidth; i++)
	{
		memcpy(&szBlockBuffer[(i * uImageWidth * 16) + (uImageWidth * 4)], &szBuffer[(uImageWidth * uImageWidth * 20) + (i * uImageWidth * 4)], uImageWidth * 4);
	}
	out.write(szBlockBuffer, uBlockBufferSize);
	out.close();

	delete[] szBuffer;
}

void ManipulateImageChannel(const std::string& szImageName, const std::string& szFilenameOut, void(*pFunction)(char **, uint8_t, uint8_t), uint8_t uImageChannel1, uint8_t uImageChannel2)
{
	char * szBuffer;
	char szHeader[22];

	uint32_t uBufferSize;
	uint16_t uImageWidth, uImageHeight;

	std::fstream in, out;

	in.open(szImageName, ios::in | ios::binary);
	if (!in.is_open())
		throw std::string("Could not open file \"" + szImageName + "\" for reading.");

	in.read(szHeader, 22);
	if (szHeader[0] != 4)
		throw std::string("Texture metadata not found for \"" + szImageName + "\".");
	if (szHeader[16] != 0x20)
		throw std::string("Channel manipulation only available for 32-bit TGA images.");

	uImageWidth = (uint8_t)szHeader[12] | ((uint8_t)szHeader[13] << 8);
	uImageHeight = (uint8_t)szHeader[14] | ((uint8_t)szHeader[15] << 8);
	uBufferSize = uImageWidth * uImageHeight * 4;
	szBuffer = new char[uBufferSize];

	memset(szBuffer, 0, uBufferSize);
	in.read(szBuffer, uBufferSize);
	in.close();

	out.open(szFilenameOut, ios::out | ios::binary);
	if (!out.is_open())
		throw std::string("Could not open file \"" + szFilenameOut + "\" for writing.");
	out.write(szHeader, 22);

	char * szBufferPos = szBuffer;
	for (uint32_t i = 0; i < (uint32_t)uImageWidth * (uint32_t)uImageHeight; i++)
	{
		pFunction(&szBufferPos, uImageChannel1, uImageChannel2);
		szBufferPos += 4;
	}
	out.write(szBuffer, uBufferSize);
	out.close();

	delete[] szBuffer;
}

void ChannelExtract(char ** szBuffer, uint8_t uImageChannel, uint8_t uUnused)
{
	char c = (*szBuffer)[3 - uImageChannel];
	(*szBuffer)[0] = (*szBuffer)[1] = (*szBuffer)[2] = (*szBuffer)[3] = c;
}

void ExtractImageChannel(const std::string& szImageName, const std::string& szFilenameOut, ImageChannel uImageChannel)
{
	if (uImageChannel > SIZEOF_IMAGE_CHANNEL)
		throw std::string("Invalid image channel specified.");
	ManipulateImageChannel(szImageName, szFilenameOut, &ChannelExtract, uImageChannel, 0);
}

void ChannelFill(char ** szBuffer, uint8_t uImageChannel, uint8_t uValue)
{
	char c = (*szBuffer)[3 - uImageChannel];
	(*szBuffer)[3 - uImageChannel] = uValue;
}
void FillImageChannel(const std::string& szImageName, const std::string& szFilenameOut, ImageChannel uImageChannel, uint8_t uValue)
{
	if (uImageChannel > SIZEOF_IMAGE_CHANNEL)
		throw std::string("Invalid image channel specified.");
	ManipulateImageChannel(szImageName, szFilenameOut, &ChannelFill, uImageChannel, uValue);
}

void ChannelSwap(char ** szBuffer, uint8_t uImageChannel1, uint8_t uImageChannel2)
{
	char c = (*szBuffer)[3 - uImageChannel1];
	(*szBuffer)[3 - uImageChannel1] = (*szBuffer)[3 - uImageChannel2];
	(*szBuffer)[3 - uImageChannel2] = c;
}

void SwapImageChannel(const std::string& szImageName, const std::string& szFilenameOut, ImageChannel uImageChannel1, ImageChannel uImageChannel2)
{
	if ((uImageChannel1 > SIZEOF_IMAGE_CHANNEL) || (uImageChannel2 > SIZEOF_IMAGE_CHANNEL))
		throw std::string("Invalid image channel specified.");
	ManipulateImageChannel(szImageName, szFilenameOut, &ChannelSwap, uImageChannel1, uImageChannel2);
}


void S2Decompiler::DecompileVTEX(const std::string& szFilename, const std::string& szOutputDirectory, bool bGenerateVTEX, bool bGenerateMipmaps)
{
	char szBuffer[4];

	bool bHasSheet = false;

	uint32_t uNumBlocks;
	uint16_t uTextureWidth, uTextureHeight, uTextureDepth, uTextureFlags;
	uint8_t uImageFormat, uMipLevels;

	std::fstream f;
	std::streamoff p1, p2, p3;
	f.open(szFilename, ios::in | ios::binary);
	if (!f.is_open())
		throw std::string("Could not open file \"" + szFilename + "\" for writing.");

	std::string szFileExt = szFilename.substr(szFilename.length() - 20, 5);
	std::string szImageName = szFilename;

	if ((szFileExt == "_tga_") || (szFileExt == "_psd_"))
		szImageName = szFilename.substr(0, szFilename.length() - 20);
	if (szFilename.substr(szFilename.length() - 25, 5) == "_z000")
		szImageName = szImageName.substr(0, szImageName.length() - 5);
	
	szImageName = szImageName.substr(szImageName.find_last_of("\\/") + 1);
	std::string szTextureName = szImageName.substr(0, szImageName.find_last_of(".")) + ".vtex";
	std::string szSheetName = szImageName.substr(0, szImageName.find_last_of(".")) + ".mks";
	szImageName = szImageName.substr(0, szImageName.find_last_of(".")) + ".tga";

	f.seekg(12);
	f.read((char*)&uNumBlocks, 4);
	for (uNumBlocks; uNumBlocks > 0; uNumBlocks--)
	{
		f.read(szBuffer, 4);
		if (strncmp(szBuffer, "DATA", 4) == 0)
		{
			f.read(szBuffer, 4);
			p1 = f.tellg();
			f.seekg(*(int*)szBuffer - 4, ios::cur);

			f.read((char*)&uTextureWidth, 2);
			f.read((char*)&uTextureHeight, 2);
			f.read((char*)&uTextureDepth, 2);
			f.read((char*)&uImageFormat, 1);
			f.read((char*)&uMipLevels, 1);

			f.seekg(4, ios::cur);   //Unknown
			f.seekg(2, ios::cur);   //Skip Multisample Type
			f.read((char*)&uTextureFlags, 2);
			f.seekg(16, ios::cur);	//Skip Reflectivity (TODO: Figure out how to incorporate this)
			f.read(szBuffer, 4);

			//So far every cube map has this value for the flag, so we don't know which bit indicates a cube map...
			if (uTextureFlags == 0x0170)
			{
				uTextureHeight *= 6;
				bGenerateMipmaps = false;
			}

			if (*(int*)szBuffer != 0)
			{
				bHasSheet = true;
				p2 = f.tellg();
				f.seekg(*(int*)szBuffer - 4, ios::cur);
				p3 = f.tellg();
				f.seekg(p2);
				f.read(szBuffer, 4);
				f.seekg(1364 + *(int*)szBuffer, ios::cur);
				if ((uImageFormat == IMAGE_FORMAT_DXT1) || (uImageFormat == IMAGE_FORMAT_DXT5))
					ExtractDXTImage(f, szOutputDirectory + "\\" + szImageName, uTextureWidth, uTextureHeight, uTextureDepth, uMipLevels, uImageFormat, bGenerateMipmaps);
				else if (uImageFormat == IMAGE_FORMAT_RGBA8888)
					ExtractRGBAImage(f, szOutputDirectory + "\\" + szImageName, uTextureWidth, uTextureHeight, uTextureDepth, uMipLevels, bGenerateMipmaps);
				else
					throw std::string("Unsupported image format: \"" + std::to_string(uImageFormat) + "\".");
				f.seekg(p3);

				ExtractSheetData(f, szOutputDirectory + "\\" + szImageName, szOutputDirectory + "\\" + szSheetName);
			}
			else
			{
				f.read(szBuffer, 4);
				f.seekg(1364 + *(int*)szBuffer, ios::cur);
				if ((uImageFormat == IMAGE_FORMAT_DXT1) || (uImageFormat == IMAGE_FORMAT_DXT5))
					ExtractDXTImage(f, szOutputDirectory + "\\" + szImageName, uTextureWidth, uTextureHeight, uTextureDepth, uMipLevels, uImageFormat, bGenerateMipmaps);
				else if (uImageFormat == IMAGE_FORMAT_RGBA8888)
					ExtractRGBAImage(f, szOutputDirectory + "\\" + szImageName, uTextureWidth, uTextureHeight, uTextureDepth, uMipLevels, bGenerateMipmaps);
				else
					throw std::string("Unsupported image format: \"" + std::to_string(uImageFormat) + "\".");
			}

			if (uTextureFlags == 0x0170)
				BuildCubeMap(szOutputDirectory + "\\" + szImageName, szOutputDirectory + "\\" + szImageName);

			f.seekg(p1 + 4);
		}
		//We don't care about structure or external resources for textures, so skip NTRO and RERL
		//Well... maybe we do care about NTRO a little bit but whatever, it's a simple enough structure anyways
		else if ((strncmp(szBuffer, "REDI", 4) == 0) || (strncmp(szBuffer, "NTRO", 4) == 0) || (strncmp(szBuffer, "RERL", 4) == 0))
		{
			f.seekg(8, ios::cur);
		}
		else
		{
			throw std::string("Encountered invalid block type.");
		}
	}
	f.close();

	if (bGenerateVTEX)
	{
		//This is currently very limited... need to find more examples of raw VTEX format with different options
		f.open(szOutputDirectory + "\\" + szTextureName, ios::out);
		f << "<!-- This file has been auto-generated by Source 2 Decompiler -->\n";
		f << "<!-- https://github.com/Dingf/Source-2-Decompiler -->\n\n";
		f << "\"CDmeVtex\"\n{\n";
		f << "\t\"m_inputTextureArray\" \"element_array\"\n\t[\n";
		f << "\t\t\"CDmeInputTexture\"\n\t\t{\n";
		f << "\t\t\t\"m_name\"\t\"string\"\t\"0\"\n";
		f << "\t\t\t\"m_filename\"\t\"string\"\t\"" << ((bHasSheet) ? szSheetName : szImageName) << "\"\n";
		f << "\t\t\t\"m_colorSpace\"\t\"string\"\t\"srgb\"\n";
		f << "\t\t\t\"m_typeString\"\t\"string\"\t\"2D\"\n";
		f << "\t\t}\n\t]\n";
		f << "\t\"m_outputTypeString\"\t\"string\"\t\"2D\"\n";
		f << "\t\"m_outputFormat\"\t\"string\"\t\"";
		if (uImageFormat == IMAGE_FORMAT_DXT5)
			f << "DXT5\"\n";
		else if (uImageFormat == IMAGE_FORMAT_DXT1)
			f << "DXT1\"\n";	//Unconfirmed
		else if (uImageFormat == IMAGE_FORMAT_RGBA8888)
			f << "RGBA\"\n";	//Unconfirmed
		else
			f << "\"\n";
		f << "\t\"m_textureOutputChannelArray\"\t\"string\"\t\"element_array\"\n\t[\n";
		f << "\t\t\"CDmeTextureOutputChannel\"\n\t\t{\n";
		f << "\t\t\t\"m_inputTextureArray\"\t\"string_array\"\n";
		f << "\t\t\t[\n\t\t\t\t\"0\"\n\t\t\t]\n";
		f << "\t\t\t\"m_srcChannels\"\t\"string\"\t\"rgba\"\n";
		f << "\t\t\t\"m_dstChannels\"\t\"string\"\t\"rgba\"\n";
		f << "\t\t\t\"m_mipAlgorithm\"\t\"CDmeImageProcessor\"\n\t\t\t{\n";
		f << "\t\t\t\t\"m_algorithm\"\t\"string\"\t\"\"\n";
		f << "\t\t\t\t\"m_stringArg\"\t\"string\"\t\"\"\n";
		f << "\t\t\t\t\"m_vFloat4Arg\"\t\"string\"\t\"0 0 0 0\"\n";
		f << "\t\t\t}\n\t\t\t\"m_outputColorSpace\"\t\"string\"\t\"srgb\"\n";
		f << "\t\t}\n\t]\n}";
	}
}