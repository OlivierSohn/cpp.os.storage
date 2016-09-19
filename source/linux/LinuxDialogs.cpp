
#include "os.storage.h"
#include "os.file.dialog.h"

namespace imajuscule {
    using namespace StorageStuff;
bool Open(FileSystemOperation::Kind k, const std::vector<std::string> & extensions, std::string & sPath)
{
    bool bRet = false;
    A(0);
    return bRet;

}

bool BasicFileOpen(std::string & sFilePath, std::string & fileName, const std::string & fileExt)
{
    bool bRet = false;
    A(0);
    return bRet;
}

    FileSystemOperation::Nature fFileSystemOperation(FileSystemOperation::Kind k, const std::vector<std::string> & extensions, const std::string & title, std::function<void(OperationResult, const std::string &)> f, std::function<void(void)> fEnd)
    {
        A(0);
        return FileSystemOperation::Nature::BLOCKING;
    }
}
