#ifndef INC_S2DC_NTRO_BLOCK_H
#define INC_S2DC_NTRO_BLOCK_H

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
	     --> 4 bytes:  Offset to Indirection (?)
	       ---> 1 byte:  Unknown
	     --> 4 bytes:  Indirection depth
	     --> 4 bytes:  Type data
	     --> 2 bytes:  Type
	   -> 4 bytes:  Number of introspection fields
	   -> 4 bytes:  Struct flags
	 4 bytes:  Number of referenced structs
*/

enum IntrospectionDataType
{
	NTRO_DATA_TYPE_STRUCT = 1,
	NTRO_DATA_TYPE_ENUM = 2,
	NTRO_DATA_TYPE_HANDLE = 3,
	NTRO_DATA_TYPE_STRING = 4,
	NTRO_DATA_TYPE_BYTE = 11,
	NTRO_DATA_TYPE_INTEGER = 14,
	NTRO_DATA_TYPE_FLOAT = 18,
	NTRO_DATA_TYPE_VECTOR3 = 22,
	NTRO_DATA_TYPE_VECTOR4 = 28,
	NTRO_DATA_TYPE_BOOLEAN = 30,
	NTRO_DATA_TYPE_NAME = 31,    //Also used for notes as well? idk... seems to be some kind of special string
};

#include <fstream>
#include "keyvalues.h"

KeyValues* GetNTROResourceDataByID(uint32_t uID);
const char* GetNTROResourceNameByID(uint32_t uID);

void ProcessNTROBlock(std::fstream& f, KeyValues& sNTROInfo);
void ReadStructuredData(std::fstream& f, KeyValues& sDestination, KeyValues * sSourceStruct = NULL);

#endif