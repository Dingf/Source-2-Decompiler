#include <stdint.h>
#include <vector>
#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "decompiler.h"

namespace bfs = boost::filesystem;

using std::ios;

bool CheckFilenameMatch(const char* sz1, const char* sz2)
{
	if (*sz1 == *sz2)
	{
		if (*sz1 == '\0')
			return true;
		return CheckFilenameMatch(sz1 + 1, sz2 + 1);
	}
	if (*sz2 == '*')
	{
		if (*sz1 == '\0')
			return (*(sz2 + 1) == '\0');
		return CheckFilenameMatch(sz1 + 1, sz2) || CheckFilenameMatch(sz1, sz2 + 1);
	}
	return false;
}

S2Decompiler::S2Decompiler(const std::vector<std::string>& szFileList)
{
	_nDecompilerFlags = 0;
	_nSuccessCount = _nFailedCount = 0;
	_szFileList = szFileList;
	if (_szFileList.empty())
	{
		_szFileList.push_back("*.vmat_c");
		_szFileList.push_back("*.vmdl_c");
		_szFileList.push_back("*.vpcf_c");
		_szFileList.push_back("*.vsnd_c");
		_szFileList.push_back("*.vtex_c");
	}

	_OutputMap[".vmat_c"] = &S2Decompiler::OutputVMAT;
	//_OutputMap[".vmdl_c"] = &S2Decompiler::OutputVMDL;
	//_OutputMap[".vpcf_c"] = &S2Decompiler::OutputVPCF;
	//_OutputMap[".vsnd_c"] = &S2Decompiler::OutputVSND;
	_OutputMap[".vtex_c"] = &S2Decompiler::OutputVTEX;
}

void S2Decompiler::StartDecompile(const std::string& szInputDirectory, const std::string& szOutputDirectory)
{
	_szInputDirectory = szInputDirectory;
	boost::algorithm::to_lower(_szInputDirectory);
	if (_szInputDirectory.find_last_of("\\/") != _szInputDirectory.length() - 1)
		_szInputDirectory += "\\";
	_szOutputDirectory = szOutputDirectory;
	boost::algorithm::to_lower(_szOutputDirectory);
	if (_szOutputDirectory.find_last_of("\\/") != _szOutputDirectory.length() - 1)
		_szOutputDirectory += "\\";

	std::vector<std::string> szNewFileList;
	for (uint32_t i = 0; i < _szFileList.size(); ++i)
	{
		if (bfs::is_regular_file(_szFileList[i]))
		{
			try
			{
				Decompile(_szFileList[i]);
			}
			catch (const std::string& s)
			{
				std::cerr << "[S2DC]: " << s << "\n";
			}
		}
		else
			szNewFileList.push_back(_szFileList[i]);
	}
	_szFileList = szNewFileList;
	if (_szFileList.size() != 0)
	{
		std::cout << "[S2DC]: Searching for matching files...\n";
		std::cout << "[S2DC]: Please be patient, as this may take a while.\n\n";
		ProcessDirectory(_szInputDirectory);
	}
	std::cout << "\n[S2DC]: Finished decompiling!\n";
	if (_nSuccessCount + _nFailedCount == 0)
		std::cout << "[S2DC]: No matching files were found.\n";
	else
		std::cout << "[S2DC]: " << _nSuccessCount << "/" << _nSuccessCount + _nFailedCount << " files were successfully decompiled.\n";
	
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
				szFilename = szFilename.substr(szFilename.find_last_of("\\/") + 1);
				for (uint32_t i = 0; i < _szFileList.size(); ++i)
				{
					if (CheckFilenameMatch(szFilename.c_str(), _szFileList[i].c_str()) == true)
					{
						try
						{
							Decompile(current->path().string());
						}
						catch (const std::string& s)
						{
							std::cerr << "[S2DC]: " << s << "\n";
						}
						break;

					}
				}
			}
		}
	}
}

void S2Decompiler::Decompile(const std::string& szPathname, const std::string& szOverrideDirectory)
{
	char szBuffer[4];

	bool bSilentDecompile = _nDecompilerFlags & DECOMPILER_FLAG_SILENT_DECOMPILE;

	uint32_t nFileSize, nBlockCount;
	KeyValues RERLBlock, NTROBlock, DATABlock;

	std::string szFilename;
	std::string szExtension = szPathname.substr(szPathname.find_last_of("."));
	if (szPathname.find(_szInputDirectory) != std::string::npos)
		szFilename = szPathname.substr(_szInputDirectory.length());
	else
		szFilename = szPathname.substr(szPathname.find_last_of("\\/") + 1);

	if (_OutputMap.find(szExtension) == _OutputMap.end())
		throw std::string("Unsupported file extension \"" + szExtension + "\"");

	std::string szResourceName = szFilename.substr(0, szFilename.length() - 7);
	if (szFilename.length() >= 20)
	{
		std::string szFileExt = szFilename.substr(szFilename.length() - 20, 5);
		if ((szFileExt == "_tga_") || (szFileExt == "_png_") || (szFileExt == "_psd_"))
			szResourceName = szResourceName.substr(0, szResourceName.length() - 13);
		if ((szResourceName.length() >= 5) && (szResourceName.substr(szResourceName.length() - 5, 5) == "_z000"))
			szResourceName = szResourceName.substr(0, szResourceName.length() - 5);
	}

	std::string szDecompileDirectory;
	if (szOverrideDirectory.empty())
		szDecompileDirectory = _szOutputDirectory + szResourceName.substr(0, szResourceName.find_last_of("."));
	else
		szDecompileDirectory = szOverrideDirectory;

	if (!bfs::is_directory(szDecompileDirectory) && !bfs::create_directories(bfs::path(szDecompileDirectory)))
		throw std::string("Could not create directory \"" + szDecompileDirectory + "\".");

	std::fstream f;
	f.open(_szInputDirectory + szFilename, ios::in | ios::binary);
	if (!f.is_open())
		throw std::string("Could not open file \"" + szFilename + "\" for reading.");

	if (!bSilentDecompile)
		std::cout << "[S2DC]: Decompiling " << szFilename << "... ";

	f.read((char*)&nFileSize, 4);
	f.seekg(12);
	f.read((char*)&nBlockCount, 4);
	for (nBlockCount; nBlockCount > 0; nBlockCount--)
	{
		f.read(szBuffer, 4);
		if (strncmp(szBuffer, "RERL", 4) == 0)
		{
			ProcessRERLBlock(f, RERLBlock);
		}
		else if (strncmp(szBuffer, "NTRO", 4) == 0)
		{
			ProcessNTROBlock(f, NTROBlock);
		}
		else if (strncmp(szBuffer, "DATA", 4) == 0)
		{
			if (!NTROBlock.data)
			{
				std::fstream f2;
				f2.open(".\\ntro\\" + szExtension.substr(1) + ".ntro", ios::in | ios::binary);
				if (!f.is_open())
					throw std::string("Could not find NTRO information for file type \"" + szExtension + "\".");
				else
					ProcessNTROBlock(f2, NTROBlock);
			}
			f.read(szBuffer, 4);
			std::streamoff p = f.tellg();
			f.seekg(*(int32_t*)szBuffer - 4, ios::cur);
			ReadStructuredData(f, DATABlock, (KeyValues*)NTROBlock.data[0]);
		}
		else if (strncmp(szBuffer, "REDI", 4) == 0)
		{
			f.seekg(8, ios::cur);
		}
		else
		{
			throw std::string("Encountered invalid block type");
		}
	}
	f.seekg(nFileSize);

	if (DATABlock.size == 0)
		throw std::string("No data was found");

	ClearLastRERLEntry();
	ClearLastNTROEntry();

	std::map<std::string, OutputFunction>::iterator i = _OutputMap.find(szExtension);
	if (i != _OutputMap.end())
	{
		try
		{
			(this->*(i->second))(DATABlock, NTROBlock, f, szDecompileDirectory + "\\" + szResourceName.substr(szResourceName.find_last_of("\\/") + 1) + i->first.substr(0, i->first.length() - 2));
			if (!bSilentDecompile)
			{
				std::cout << "done!\n";
				_nSuccessCount++;
			}
		}
		catch (const std::string& s)
		{
			if (!bSilentDecompile)
			{
				std::cout << "failed!\n";
				_nFailedCount++;
			}
			throw s;
		}
	}
	else
	{
		bfs::remove_all(szDecompileDirectory);
		if (!bSilentDecompile)
		{
			std::cout << "failed!\n";
			_nFailedCount++;
		}
		throw std::string("Unsupported file type \"" + szExtension + "\".\n");
	}

	f.close();
}