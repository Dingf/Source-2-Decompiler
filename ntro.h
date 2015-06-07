#ifndef INC_S2DC_NTRO_BLOCK_H
#define INC_S2DC_NTRO_BLOCK_H

#ifdef _WIN32
#pragma once
#endif

/*NTRO Block
     4 bytes:  Introspection version
	 4 bytes:  Offset to referenced structs
	   -> 4 bytes:  Introspection version
	   -> 4 bytes:  ID
	   -> 4 bytes:  Offset to name
	   -> 4 bytes:  Disk CRC
	   -> 4 bytes:  User version
	   -> 2 bytes:  Disk size
	   -> 2 bytes:  Alignment
	   -> 4 bytes:  Base struct ID
	   -> 4 bytes:  Offset to introspection fields
	     --> 4 bytes:  Offset to field name
		 --> 2 bytes:  Count
	     --> 2 bytes:  On-disk offset
	     --> 4 bytes:  Offset to Indirection Bytes
	       ---> 1 byte:  Indirection Byte (0x04 = array, 0x03 = pointer)
	     --> 4 bytes:  Indirection level
	     --> 4 bytes:  Type data
	     --> 2 bytes:  Type
	   -> 4 bytes:  Number of introspection fields
	   -> 4 bytes:  Struct flags
	 4 bytes:  Number of referenced structs
*/

#include <fstream>
#include "keyvalues.h"

void ProcessNTROBlock(std::fstream& f, KeyValues& NTROInfo);
void ClearLastNTROEntry();

KeyValues * ReadIndirectionData(std::fstream& f, char * szArgs, uint32_t nDepth);
void ReadDataField(std::fstream& f, char *& szDestination, char * szArgs, uint32_t nDepth);
void ReadStructuredData(std::fstream& f, KeyValues& Destination, KeyValues * pSourceStruct = NULL);

#endif