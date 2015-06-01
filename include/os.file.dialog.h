
#pragma once

#include <string>
#include "os.storage.h"

bool BasicFileOpen(std::string & sPath, std::string & fileName, const std::string & sFileExt);
bool BasicFileOpen2(Storage::DirectoryPath & pathToDirectory, Storage::FileName & filename, const std::string & sFileExt);
bool BasicDirectoryOpen(std::string & sPath);
bool BasicDirectoryOpen2(Storage::DirectoryPath & pathToDirectory);

void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace);
