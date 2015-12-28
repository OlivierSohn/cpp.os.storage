
#pragma once

#include <string>
#include <vector>
#include <functional>
#include "os.storage.h"

enum OperationResult
{
    SUCCESS,
    CANCELED
};

namespace imajuscule
{
    class FileSystemOperation
    {
    public:
        enum Kind
        {
            OP_FILE,
            OP_DIR,
            OP_BOTH
        };
        FileSystemOperation(const std::string & title, std::function<void(OperationResult, const std::string &)> f, const std::vector<std::string> & extensions);
        FileSystemOperation(const std::string & title, std::function<void(OperationResult, const std::string &)> f, Kind k);
        virtual ~FileSystemOperation();
        
        enum Nature
        {
            BLOCKING,
            NON_BLOCKING
        };
        Nature getNature() const;

    private:
        Nature go(const std::string & title, std::function<void(OperationResult, const std::string &)> f, Kind k, const std::vector<std::string> & extensions) const;
        Nature mNature;
    };

    FileSystemOperation::Nature fFileSystemOperation(FileSystemOperation::Kind, const std::vector<std::string> & extensions, const std::string & title, std::function<void(OperationResult, const std::string &)> f, std::function<void(void)> fEnd);
    
    bool BasicFileOpen(std::string & sPath, std::string & fileName, const std::string & sFileExt);
    bool BasicFileOpen2(Storage::DirectoryPath & pathToDirectory, Storage::FileName & filename, const std::string & sFileExt);
    
    void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace);
}

