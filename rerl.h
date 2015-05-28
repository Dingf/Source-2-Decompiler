#ifndef INC_S2DC_RERL_BLOCK_H
#define INC_S2DC_RERL_BLOCK_H

#ifdef _WIN32
#pragma once
#endif

/*RERL Block (8 bytes)
     4 bytes:  Offset to resource entry list (always 0x00000008, the size of the RERL block)
       -> 8 bytes:  Resource ID
       -> 8 bytes:  Offset to resource name
     4 bytes:  Number of resource entries
*/

#include <fstream>
#include "keyvalues.h"

const char * GetExternalResourceName(const char * szRefID);

void ProcessRERLBlock(std::fstream& f, KeyValues& CRERLInfo);
void ClearLastRERLEntry();

#endif