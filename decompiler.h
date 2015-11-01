#ifndef INC_S2DC_DECOMPILER_H
#define INC_S2DC_DECOMPILER_H

#ifdef _WIN32
#pragma once
#endif

/* General Decompiler Notes:

   *Values are in little endian order and are integer values, unless stated otherwise.
   *Size and offset values are measured in bytes, unless stated otherwise.
   *All offsets are from the current stream position, not from the start of the file.
   *Blocks are padded to the next 16 bytes, including additional compiled data. This is not included in the block size.
   --------------------------------------------------------------------------------------------------------------------------------
   Header(16 bytes)
   4 bytes:  Size of file
   4 bytes : Unknown(constant 0x0000000c)
   4 bytes : Offset to block information
   4 bytes : Number of blocks

   Block information(12 bytes)
   4 bytes : Block name(i.e.RERL, REDI, NTRO, DATA)
   4 bytes : Offset to block start
   4 bytes : Size of block
*/

#include <stdint.h>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include "keyvalues.h"
#include "rerl.h"
#include "ntro.h"

enum DecompilerFlag
{
	DECOMPILER_FLAG_SILENT_DECOMPILE = 1,        //Don't print output and don't count the result towards success/failures
	DECOMPILER_FLAG_VTEX_NO_VTEX_FILE = 2,       //Don't generate a VTEX file for this decompile (used mostly by VMAT)
	DECOMPILER_FLAG_VTEX_NO_MIPMAPS = 4,         //Don't generate mipmaps for this decomile (used mostly by VMAT)

	DECOMPILER_FLAG_VTEX_SPLIT_MIPMAPS = 8,      //Split each mip level into its own image
	DECOMPILER_FLAG_VTEX_SPLIT_SEQUENCES = 16,   //Split each frame in each sequence into its own image
};

class S2Decompiler
{
	public:
		typedef void (S2Decompiler::*OutputFunction)(const KeyValues&, std::fstream&, const std::string&);

		S2Decompiler(const std::vector<std::string>& szFileList);

		void StartDecompile(const std::string& szLocalDirectory, const std::string& szInputDirectory, const std::string& szOutputDirectory);
	private:
		void ProcessDirectory(const std::string& szDirectory);
		void Decompile(const std::string& szPathname, const std::string& szOverrideDirectory = "");

		void OutputVMAT(const KeyValues& DataBlock, std::fstream& f, const std::string& szOutputName);
		void OutputVMDL(const KeyValues& DataBlock, std::fstream& f, const std::string& szOutputName);
		void OutputVPCF(const KeyValues& DataBlock, std::fstream& f, const std::string& szOutputName);
		void OutputVTEX(const KeyValues& DataBlock, std::fstream& f, const std::string& szOutputName);

		uint32_t _nDecompilerFlags;
		uint32_t _nSuccessCount, _nFailedCount;
		std::string _szLocalDirectory, _szInputDirectory, _szOutputDirectory;
		std::vector<std::string> _szFileList;
		std::map<std::string, OutputFunction> _OutputMap;
};

#endif