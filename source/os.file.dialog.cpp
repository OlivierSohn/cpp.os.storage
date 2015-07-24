//
//  os.file.cpp
//  os.storage
//
//  Created by Olivier on 16/05/2015.
//  Copyright (c) 2015 Olivier. All rights reserved.
//

#include "os.file.dialog.h"
#include "os.abstraction.h"

using namespace imajuscule;

void ReplaceStringInPlace(std::string& subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

bool BasicFileOpen2(Storage::DirectoryPath & pathToDirectory, Storage::FileName & filename, const std::string & sFileExt)
{
    std::string path, file;
    bool bRet = BasicFileOpen(path, file, sFileExt);
    if(bRet)
    {
        if(hasEnding(path, file))
        {
            path.resize(path.size() - file.size());
        }
        
        filename = file;
        pathToDirectory = Storage::toDirPath(path);
    }
    
    return bRet;
}

bool BasicDirectoryOpen2(Storage::DirectoryPath & pathToDirectory)
{
    std::string path;
    bool bRet = BasicDirectoryOpen(path);
    if(bRet)
    {
        pathToDirectory = Storage::toDirPath(path);
    }
    
    return bRet;
}

AsyncDirectoryOperation::AsyncDirectoryOperation(const std::string & title, std::function<void(OperationResult, const std::string &)> f)
{
    if(OSAbstraction * os = OSAbstraction::get() )
        os->PauseInteractions(true);

    fAsyncDirectoryOperation(title, f, [](){
        if(OSAbstraction * os = OSAbstraction::get() )
            os->PauseInteractions(false);
    });
}

AsyncDirectoryOperation::~AsyncDirectoryOperation()
{
    
}
