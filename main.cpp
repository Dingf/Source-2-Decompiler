#include <stdint.h>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include "decompiler.h"

namespace bfs = boost::filesystem;

std::string szDefaultPaths[] = { "C:\\Program Files\\Steam\\SteamApps\\common\\dota 2 beta\\dota_ugc\\game\\dota_imported\\",
                                 "C:\\Program Files (x86)\\Steam\\SteamApps\\common\\dota 2 beta\\dota_ugc\\game\\dota_imported\\" };

int main(int argc, char ** argv)
{
	std::string szBasePath;
	std::vector<std::string> szFileList;

	for (int32_t i = 0; i < argc; i++)
	{
		if (strncmp(argv[i], "-d\0", 3) == 0)
			if (i + 1 < argc)
				szBasePath = std::string(argv[++i]);
		else
			szFileList.push_back(std::string(argv[i]));
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

	S2Decompiler sDecompiler(szFileList);
	sDecompiler.StartDecompile(szBasePath);
	return 0;
}
