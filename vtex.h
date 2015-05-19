#ifndef INC_S2DC_VTEX_DECOMPILER_H
#define INC_S2DC_VTEX_DECOMPILER_H

/* VTEX_C File Format

   DATA Block
     2 bytes:   Texture width
	 2 bytes:   Texture height
	 2 bytes:   Texture depth
	 1 byte:    Image format (see below)
	 1 byte:    Number of mip levels
	 4 bytes:   Unknown (mip 0 size?)
	 2 bytes:   Multisample Type
	 2 bytes:   Flags
	 16 bytes:  Reflectivity (float x4)
	 4 bytes:   Offset to sheet data
	   -> 4 bytes:  Offset to sequence info
	     --> 4 bytes:  Sequence ID
		 --> 4 bytes:  Flags (?)
		   ---> 0x01:  Clamp (If not clamped, sequence animation will loop)
		   ---> 0x02:  Alpha Crop
         --> 4 bytes:  Offset to frame data
		   ---> 4 bytes:  Display time
		   ---> 4 bytes:  Offset to image data
		     ----> 8 bytes:  Cropped min (float x2)
			 ----> 8 bytes:  Cropped max (float x2)
		   ---> 4 bytes:  Number of images
		   ---> 8 bytes:  Cropped min (float x2)
		   ---> 8 bytes:  Cropped max (float x2)
		   ---> 8 bytes:  Uncropped min (float x2)
		   ---> 8 bytes:  Uncropped max (float x2)
		 --> 4 bytes:  Number of frames
		 --> 4 bytes:  Total time (float)
		 --> 4 bytes:  Offset to name
	     --> 4 bytes:  Offset to float params (unconfirmed)
	     --> 4 bytes:  Number of float params (unconfirmed)
	   -> 4 bytes:  Number of sequences
	 4 bytes:   Sheet size
	 1360 bytes:  Fallback texture bits
	 X bytes:  Texture data, as specified by image format, width, height, and mip levels
*/

#include <stdint.h>
#include <string>
#include <fstream>

// VTF Image Formats (probably the same for VTEX too... confirmed RGBA8888 = 0, DXT1 = 13, and DXT5 = 15)
enum ImageFormat
{
	IMAGE_FORMAT_NONE = -1,
	IMAGE_FORMAT_RGBA8888 = 0,
	IMAGE_FORMAT_ABGR8888 = 1,
	IMAGE_FORMAT_RGB888 = 2,
	IMAGE_FORMAT_BGR888 = 3,
	IMAGE_FORMAT_RGB565 = 4,
	IMAGE_FORMAT_I8 = 5,
	IMAGE_FORMAT_IA88 = 6,
	IMAGE_FORMAT_P8 = 7,
	IMAGE_FORMAT_A8 = 8,
	IMAGE_FORMAT_RGB888_BLUESCREEN = 9,
	IMAGE_FORMAT_BGR888_BLUESCREEN = 10,
	IMAGE_FORMAT_ARGB8888 = 11,
	IMAGE_FORMAT_BGRA8888 = 12,
	IMAGE_FORMAT_DXT1 = 13,
	IMAGE_FORMAT_DXT3 = 14,
	IMAGE_FORMAT_DXT5 = 15,
	IMAGE_FORMAT_BGRX8888 = 16,
	IMAGE_FORMAT_BGR565 = 17,
	IMAGE_FORMAT_BGRX5551 = 18,
	IMAGE_FORMAT_BGRA4444 = 19,
	IMAGE_FORMAT_DXT1_ONEBITALPHA = 20,
	IMAGE_FORMAT_BGRA5551 = 21,
	IMAGE_FORMAT_UV88 = 22,
	IMAGE_FORMAT_UVWQ8888 = 23,
	IMAGE_FORMAT_RGBA16161616F = 24,
	IMAGE_FORMAT_RGBA16161616 = 25,
	IMAGE_FORMAT_UVLX8888 = 26
};

enum ImageChannel
{
	IMAGE_CHANNEL_ALPHA = 0,
	IMAGE_CHANNEL_RED = 1,
	IMAGE_CHANNEL_GREEN = 2,
	IMAGE_CHANNEL_BLUE = 3,
	SIZEOF_IMAGE_CHANNEL
};

void ExtractRGBAImage(std::fstream& in, const std::string& szFilenameOut, uint16_t uWidth, uint16_t uHeight, uint8_t uMipLevels, bool bGenerateMipmaps);
void ExtractDXTImage(std::fstream& in, const std::string& szFilenameOut, uint16_t uWidth, uint16_t uHeight, uint8_t uMipLevels, uint8_t uImageFormat, bool bGenerateMipmaps);
void ExtractFrameImage(const std::string& szImageName, const std::string& szFilenameOut, float * szCoords);
void ExtractSheetData(std::fstream& in, const std::string& szImageName, const std::string& szFilenameOut);
void BuildCubeMap(const std::string& szImageName, const std::string& szFilenameOut);
void ExtractImageChannel(const std::string& szImageName, const std::string& szFilenameOut, ImageChannel uImageChannel);
void FillImageChannel(const std::string& szImageName, const std::string& szFilenameOut, ImageChannel uImageChannel, uint8_t uValue);
void SwapImageChannel(const std::string& szImageName, const std::string& szFilenameOut, ImageChannel uImageChannel1, ImageChannel uImageChannel2);

#endif