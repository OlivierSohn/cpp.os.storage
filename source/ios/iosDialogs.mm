//
//  LLVPlayerCocoaDialogs.mm
//  LLVPlayer
//
//  Created by Olivier on 09/08/2014.
//
//

// for ios, see https://developer.apple.com/library/ios/documentation/FileManagement/Conceptual/DocumentPickerProgrammingGuide/AccessingDocuments/AccessingDocuments.html#//apple_ref/doc/uid/TP40014451-CH2-SW4

#include <string>
#include <vector>
#include <functional>

#include "os.file.dialog.h"

bool BasicFileOpen(std::string & sPathWithFileName, std::string & sFileName, const std::string & sFileExt)
{
    bool bRet = false;
    return bRet;
}

namespace imajuscule {
    FileSystemOperation::Nature fFileSystemOperation(FileSystemOperation::Kind k, const std::vector<std::string> & extensions, const std::string & title, std::function<void(OperationResult, const std::string &)> f, std::function<void(void)> fEnd)
    {
        return FileSystemOperation::Nature::NON_BLOCKING;
    }
}
