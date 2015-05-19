#include <stdint.h>
#include <iostream>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include "decompiler.h"

namespace bfs = boost::filesystem;

bool CheckFilenameMatch(const char * sz1, const char * sz2)
{
	if (*sz1 == *sz2)
	{
		if (*sz1 == '\0')
			return true;
		return CheckFilenameMatch(sz1 + 1, sz2 + 1);
	}
	if (*sz2 == '*')
	{
		if ((*(sz2 + 1) != '\0') && (*sz1 == '\0'))
			return false;
		return CheckFilenameMatch(sz1 + 1, sz2) || CheckFilenameMatch(sz1, sz2 + 1);
	}
	return false;
}

S2Decompiler::S2Decompiler()
{
	_szFileList.push_back("*.vtex_c");
	_szFileList.push_back("*.vmat_c");
	_szFileList.push_back("*.vpcf_c");
	_szFileList.push_back("*.vmdl_c");
	_szFileList.push_back("*.vsnd_c");
}

S2Decompiler::S2Decompiler(const std::vector<std::string>& szFileList)
{
	_szFileList = szFileList;
	if (_szFileList.empty())
	{
		_szFileList.push_back("*.vtex_c");
		_szFileList.push_back("*.vmat_c");
		_szFileList.push_back("*.vpcf_c");
		_szFileList.push_back("*.vmdl_c");
		_szFileList.push_back("*.vsnd_c");
	}
}

void S2Decompiler::StartDecompile(const std::string& szDirectory)
{
	_szBaseDirectory = szDirectory;
	ProcessDirectory(_szBaseDirectory);
}

void S2Decompiler::ProcessDirectory(const std::string& szDirectory)
{
	if (bfs::is_directory(szDirectory))
	{
		bfs::path p(szDirectory);
		bfs::directory_iterator current(p), end;
		for (current; current != end; ++current)
		{
			if (bfs::is_directory(current->path()))
			{
				ProcessDirectory(current->path().string());
			}
			else if (bfs::is_regular_file(current->path()))
			{
				std::string szFilename = current->path().string();
				for (uint32_t i = 0; i < _szFileList.size(); i++)
				{
					if (CheckFilenameMatch(szFilename.c_str(), _szFileList[i].c_str()) == true)
					{
						Decompile(szFilename);
						break;
					}
				}
			}
		}
	}
}

DecompileResult S2Decompiler::Decompile(const std::string& szPathname)
{
	std::string szExtension = szPathname.substr(szPathname.find_last_of("."));
	std::string szFilename = szPathname.substr(_szBaseDirectory.length());
	std::string szNewDirectory = ".\\" + szFilename.substr(0, szFilename.find_last_of("."));
	if (!bfs::is_directory(szNewDirectory))
	{
		if (!bfs::create_directories(bfs::path(szNewDirectory)))
		{
			std::cerr << "[S2DC]: Could not create directory \"" + szNewDirectory + "\".";
			return DECOMPILE_COULD_NOT_CREATE_DIRECTORY;
		}
	}

	std::cout << "[S2DC]: Decompiling " << szFilename << "... ";
	if (szExtension == ".vtex_c")
	{
		try
		{
			DecompileVTEX(_szBaseDirectory + szFilename, szNewDirectory);
			std::cout << "done!\n";
		}
		catch (std::string& s)
		{
			std::cout << "failed!\n";
			std::cerr << "[VTEX]: " << s << "\n";
		}
	}
	else if (szExtension == ".vmat_c")
	{
		try
		{
			DecompileVMAT(_szBaseDirectory + szFilename, szNewDirectory);
			std::cout << "done!\n";
		}
		catch (std::string& s)
		{
			std::cout << "failed!\n";
			std::cerr << "[VMAT]: " << s << "\n";
		}
	}
	else if (szExtension == ".vpcf_c")
	{
		//This doesn't work... yet
		try
		{
			DecompileVPCF(_szBaseDirectory + szFilename, szNewDirectory);
			std::cout << "done!\n";
		}
		catch (std::string& s)
		{
			std::cout << "failed!\n";
			std::cerr << "[VPCF]: " << s << "\n";
		}/**/
	}
	else if (szExtension == ".vmdl_c")
	{
	}
	else if (szExtension == ".vsnd_c")
	{
	}
	else
	{
		bfs::remove_all(szNewDirectory);
		return DECOMPILE_FILE_TYPE_NOT_SUPPORTED;
	}
}