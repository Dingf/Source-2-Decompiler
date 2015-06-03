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

enum IntrospectionDataType
{
	NTRO_DATA_TYPE_STRUCT = 1,
	NTRO_DATA_TYPE_ENUM = 2,
	NTRO_DATA_TYPE_HANDLE = 3,
	NTRO_DATA_TYPE_STRING = 4,
	NTRO_DATA_TYPE_BYTE = 11,
	NTRO_DATA_TYPE_SHORT = 12,
	NTRO_DATA_TYPE_USHORT = 13,
	NTRO_DATA_TYPE_INTEGER = 14,
	NTRO_DATA_TYPE_UINTEGER = 15,
	NTRO_DATA_TYPE_INT64 = 16,
	NTRO_DATA_TYPE_UINT64 = 17,
	NTRO_DATA_TYPE_FLOAT = 18,
	NTRO_DATA_TYPE_VECTOR3 = 22,
	NTRO_DATA_TYPE_QUATERNION = 25,
	NTRO_DATA_TYPE_VECTOR4 = 27,
	NTRO_DATA_TYPE_COLOR = 28,   //Standard RGBA, 1 byte per channel
	NTRO_DATA_TYPE_BOOLEAN = 30,
	NTRO_DATA_TYPE_NAME = 31,    //Also used for notes as well? idk... seems to be some kind of special string
};

void ProcessNTROBlock(std::fstream& f, KeyValues& NTROInfo);
void ClearLastNTROEntry();

KeyValues * ReadIndirectionData(std::fstream& f, char * szArgs, uint32_t nDepth);
void ReadDataField(std::fstream& f, char *& szDestination, char * szArgs, uint32_t nDepth);
void ReadStructuredData(std::fstream& f, KeyValues& Destination, KeyValues * pSourceStruct = NULL);

#endif