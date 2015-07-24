
#pragma once

#include <string>
#include <functional>
#include "os.storage.h"

enum OperationResult
{
    SUCCESS,
    CANCELED
};

namespace imajuscule
{
    class AsyncDirectoryOperation
    {
    public:
        AsyncDirectoryOperation(const std::string & title, std::function<void(OperationResult, const std::string &)> f);
        virtual ~AsyncDirectoryOperation();
    };
}

void fAsyncDirectoryOperation(const std::string & title, std::function<void(OperationResult, const std::string &)> f, std::function<void(void)> fEnd);

bool BasicFileOpen(std::string & sPath, std::string & fileName, const std::string & sFileExt);
bool BasicFileOpen2(Storage::DirectoryPath & pathToDirectory, Storage::FileName & filename, const std::string & sFileExt);
bool BasicDirectoryOpen(std::string & sPath);
bool BasicDirectoryOpen2(Storage::DirectoryPath & pathToDirectory);

void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace);
