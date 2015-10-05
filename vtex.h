#ifndef INC_S2DC_VTEX_DECOMPILER_H
#define INC_S2DC_VTEX_DECOMPILER_H

#ifdef _WIN32
#pragma once
#endif

/* VTEX_C File Format

   DATA Block
     2 bytes:   VTEX version (currently 1)
     2 bytes:   Texture flags
     16 bytes:  Reflectivity (vector4)
     2 bytes:   Texture width
     2 bytes:   Texture height
     2 bytes:   Texture depth
     1 byte:    Image format (see below)
     1 byte:    Number of mip levels
     4 bytes:   Unknown (mip 0 size?)
     4 bytes:   Offset to extra data
       -> 4 bytes:  Extra data type
       -> 4 bytes:  Extra data offset
       -> 4 bytes:  Extra data size
     4 bytes:   Number of extra data
     ?? bytes:  Extra data contents
     ?? bytes:  Texture data, as specified by image format, width, height, and mip levels

   Fallback Data
     1024 bytes(?):  Texture fallback bits (might not always be 1024, last time was 1360 so...)

   Sheet Data
     4 bytes:  Offset to sequence info
       -> 4 bytes:  Sequence ID
       -> 4 bytes:  Flags
         --> 0x01:  Clamp (If not clamped, sequence animation will loop)
         --> 0x02:  Alpha Crop
       -> 4 bytes:  Offset to frame data
         --> 4 bytes:  Display time
         --> 4 bytes:  Offset to image data
           ---> 8 bytes:  Cropped min (vector2)
           ---> 8 bytes:  Cropped max (vector2)
         --> 4 bytes:  Number of images
         --> 8 bytes:  Cropped min (vector2)
         --> 8 bytes:  Cropped max (vector2)
         --> 8 bytes:  Uncropped min (vector2)
         --> 8 bytes:  Uncropped max (vector2)
       -> 4 bytes:  Number of frames
       -> 4 bytes:  Total time (float)
       -> 4 bytes:  Offset to name
       -> 4 bytes:  Offset to float params (unconfirmed)
       -> 4 bytes:  Number of float params (unconfirmed)
     4 bytes:  Number of sequences
*/

#include <stdint.h>
#include <string>
#include <fstream>

enum VTEXFlags
{
	VTEX_FLAG_SUGGEST_CLAMPS = 0x00000001,
	VTEX_FLAG_SUGGEST_CLAMPT = 0x00000002,
	VTEX_FLAG_SUGGEST_CLAMPU = 0x00000004,
	VTEX_FLAG_NO_LOD = 0x00000008,
	VTEX_FLAG_CUBE_TEXTURE = 0x00000010,
	VTEX_FLAG_VOLUME_TEXTURE = 0x00000020,
	VTEX_FLAG_TEXTURE_ARRAY = 0x00000040
};

enum VTEXFormat
{
	VTEX_FORMAT_UNKNOWN = 0,
	VTEX_FORMAT_DXT1 = 1,
	VTEX_FORMAT_DXT5 = 2,
	VTEX_FORMAT_I8 = 3,
	VTEX_FORMAT_RGBA8888 = 4,
	VTEX_FORMAT_R16 = 5,
	VTEX_FORMAT_RG1616 = 6,
	VTEX_FORMAT_RGBA16161616 = 7,
	VTEX_FORMAT_R16F = 8,
	VTEX_FORMAT_RG1616F = 9,
	VTEX_FORMAT_RGBA16161616F = 10,
	VTEX_FORMAT_R32F = 11,
	VTEX_FORMAT_RG3232F = 12,
	VTEX_FORMAT_RGB323232F = 13,
	VTEX_FORMAT_RGBA32323232F = 14,
	VTEX_FORMAT_PNG = 16
};

enum VTEXExtraData
{
	VTEX_EXTRA_DATA_UNKNOWN = 0,
	VTEX_EXTRA_DATA_FALLBACK_BITS = 1,
	VTEX_EXTRA_DATA_SHEET = 2,
};

enum ImageChannel
{
	IMAGE_CHANNEL_ALPHA = 0,
	IMAGE_CHANNEL_RED = 1,
	IMAGE_CHANNEL_GREEN = 2,
	IMAGE_CHANNEL_BLUE = 3,
	SIZEOF_IMAGE_CHANNEL
};

void ExtractImageChannel(const std::string& szImageName, const std::string& szFilenameOut, ImageChannel nImageChannel);
void FillImageChannel(const std::string& szImageName, const std::string& szFilenameOut, ImageChannel nImageChannel, uint8_t nValue);
void SwapImageChannel(const std::string& szImageName, const std::string& szFilenameOut, ImageChannel nImageChannel1, ImageChannel nImageChannel2);

#endif
