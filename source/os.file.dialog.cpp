
namespace imajuscule {

void ReplaceStringInPlace(std::string& subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

static bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

bool BasicFileOpen2(DirectoryPath & pathToDirectory, Storage::FileName & filename, const std::string & sFileExt)
{
    std::string path, file;
    if( BasicFileOpen(path, file, sFileExt) )
    {
        if(hasEnding(path, file))
        {
            path.resize(path.size() - file.size());
        }
        
        filename = file;
        pathToDirectory = DirectoryPath(path);
        
        return true;
    }
    
    return false;
}

FileSystemOperation::FileSystemOperation(const std::string & title, std::function<void(OperationResult, const std::string &)> f, const std::vector<std::string> & extensions)
{
    mNature = go(title, f, Kind::OP_FILE, extensions);
}
FileSystemOperation::FileSystemOperation(const std::string & title, std::function<void(OperationResult, const std::string &)> f, Kind k)
{
    std::vector<std::string> extensions;
    mNature = go(title, f, k, extensions);
}

auto FileSystemOperation::go(const std::string &title, std::function<void (OperationResult, const std::string &)> f, FileSystemOperation::Kind k, const std::vector<std::string> &extensions) const -> Nature
{
    if(OSAbstraction * os = OSAbstraction::edit() )
        os->PauseInteractions(true);

    return fFileSystemOperation(k, extensions, title, f, [](){
        if(OSAbstraction * os = OSAbstraction::edit() )
            os->PauseInteractions(false);
    });
}

auto FileSystemOperation::getNature () const -> Nature
{
    return mNature;
}    
}
