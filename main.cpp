#include <stdint.h>
#include <iostream>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "decompiler.h"

namespace bfs = boost::filesystem;

std::string szDefaultPaths[] = { "c:\\program files\\steam\\steamapps\\common\\dota 2 beta\\dota_ugc\\game\\dota_imported\\",
                                 "c:\\program files (x86)\\steam\\steamapps\\common\\dota 2 beta\\dota_ugc\\game\\dota_imported\\" };

int main(int argc, char ** argv)
{
	std::string szBasePath;
	std::string szOutputPath = ".\\";
	std::vector<std::string> szFileList;

	for (int32_t i = 1; i < argc; i++)
	{
		if (strncmp(argv[i], "-d\0", 3) == 0)
		{
			if ((i + 1 < argc) && (bfs::is_directory(argv[i + 1])))
				szBasePath = std::string(argv[++i]);
		}
		else if (strncmp(argv[i], "-o\0", 3) == 0)
		{
			if (i + 1 < argc)
				szOutputPath = std::string(argv[++i]);
		}
		else
		{
			std::string szInput(argv[i]);
			boost::algorithm::to_lower(szInput);
			szFileList.push_back(szInput);
		}
	}

	if (szFileList.empty())
	{
		std::cout << "Usage: s2dc [-d <input path>] [-o <output path>] <input(s)>\n\n";
		std::cout << "\t[Input]\n\t-d\tSpecifies the path to search for input files.\n\t\tIf an absolute pathname is given, this path is ignored.\n\n";
		std::cout << "\t[Output]\n\t-o\tSpecifies the output path. This path only serves as the\n\t\troot folder for the output. The directory structure\n\t\trelative to the input path is still preserved.";
		std::cout << "\n\n\nSource 2 Resource Decompiler\n\n";
		std::cout << "S2DC searches the input path (or dota_ugc\\game\\dota_imported, if not specified) for all files that match the input file(s). Wildcards in the input (*) are accepted, and will correspond to any file which also matches the pattern. Currently supports the .vtex_c, .vmat_c, and .vpcf_c file formats.\n\n";
		std::cout << "Decompiled resources will be written to the output path (or the local directory, if not specified), but their directory structure relative to the input path is still preserved. If the input is an absolute path, the output will be found in the root folder (the output path) instead.\n\n";
		return 1;
	}

	if (szBasePath.empty())
	{
		for (uint32_t i = 0; i < sizeof(szDefaultPaths) / sizeof(char *); ++i)
		{
			if (bfs::is_directory(szDefaultPaths[i]))
			{
				szBasePath = szDefaultPaths[i];
				break;
			}
		}
	}

	boost::algorithm::to_lower(szBasePath);
	boost::algorithm::to_lower(szOutputPath);
	S2Decompiler sDecompiler(szFileList);
	sDecompiler.StartDecompile(szBasePath, szOutputPath);
	return 0;
}
