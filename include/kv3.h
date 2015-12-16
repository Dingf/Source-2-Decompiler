#ifndef INC_S2DC_KV3_H
#define INC_S2DC_KV3_H

/* KV3 File Format

    Header
      4 bytes:   Signature (56 4B 56 03 = VKV3)
      16 bytes:  Unknown (KV3 encoding?)
      16 bytes:  KV3 Format
      3 bytes:   File Size
      1 byte:    Flags?
        -> 0x80:   No compression
      ?? bytes:  KV3 Data (compressed unless specified above)

   Compressed Blocks
   ----------------------------------------------------------------
   The KV3 compression algorithm is a lossless dictionary algorithm
   much like LZ (Lempel-Ziv) encoding. Unlike LZ and its variants,
   KV3 does not have a static dictionary to begin with. Instead, it
   uses previously encoded data as the dictionary, resulting in a
   human-readable encoding that gradually becomes more entropic as
   it progresses.

   Additionally, KV3 also divides the data into small blocks of 16
   "sections" each. A section is composed of either a single byte
   of actual file data, or two bytes which indicates a lookup to a
   previous part of the file. The lookup is divided into two parts:
   the offset and the size. For example, a lookup of value 0x0201
   looks like this:

        12 bits       4 bits
     0000 0010 0000    0001
     [   Offset   ]   [Size]

   In this case, the offset value is 32 and the size value is 1.
   Therefore, we start at the position located 33 bytes before the
   current position (minimum offset is 1 byte), and read 4 bytes to
   obtain the actual data (minimum size is 3 bytes).

   In order to determine which sections are raw data and which
   sections are lookup values, each block has a 2 byte header,
   which is a simple bitmask of the following 16 sections. A
   header with the value 0x0408 looks like this:

   last <----------- first
     0000 0100 0000 1000

   A bit value of 0 indicates raw data, whereas a value of 1
   indicates a lookup. In the example above, the fourth and
   eleventh sections are lookup sections and the rest are normal
   data.
     

*/
#include <string>
#include <fstream>

void DecompressKV3(std::fstream& f, const std::string& szOutputName);
void OutputKV3(std::fstream& f, const std::string& szOutputName);

#endif
