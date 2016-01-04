#pragma once

#include <string>
#include <vector>
#include <set>

namespace imajuscule {
    
enum eResult
{
    ILE_SUCCESS = 0,
    ILE_RECURSIVITY,
    ILE_ERROR,
    ILE_NOT_IMPLEMENTED,
    ILE_BAD_PARAMETER,
    ILE_OBJECT_INVALID
};

#define SIZE_READ_BUFFER 2048

typedef std::vector<std::string> DirectoryPath;

class Storage
{
public:

    enum FileOperation
    {
        OP_WRITE,
        OP_READ
    };

#ifdef _WIN32
    static void string_cast(const wchar_t* pSource, unsigned int codePage /*= CP_ACP*/, std::string & oString);
#endif

    static bool dirExists(const std::string & path);
    static bool fileExists(const std::string & path);
    static bool fileCreationDate(const std::string & path, std::string & oDate);
    static eResult makeDir(const std::string & path);
    static bool setCurrentDir(const char * dir);
    static std::vector< std::string > listFilenames( const std::string & path );

    static bool isGUID(std::string const & str);

    eResult Save();

    typedef std::string FileName;
    static DirectoryPath curDir();
    static DirectoryPath toDirPath(const std::string &);
    static std::string toString(const DirectoryPath &);
    
protected:
    
    static DirectoryPath m_curDir;
    
    Storage(DirectoryPath, FileName);
    virtual ~Storage();

    eResult OpenForRead();
    eResult OpenForWrite();
    
    virtual void WriteData(void * p, size_t size, size_t count);

    virtual void ReadData(void * p, size_t size, size_t count);

    void Finalize();
    void CloseFile();

    void UpdateFileHeader();

    // child classes should call this method directly only the first time the header is written.
    // for subsequent header writes they should call instead UpdateFileHeader that will call this method at the appropriate moment
    // and then restore the file position to the position it had before writing the header
    virtual void DoUpdateFileHeader() = 0;

    eResult doSaveBegin();
private:
    bool isBeingSaved();
    virtual eResult doSave();
    void doSaveEnd();

    void* m_pFile;
    std::vector<unsigned char> m_writeBuffer;
    unsigned char m_freadBuffer[SIZE_READ_BUFFER];
    size_t m_bufferReadPos;
    
    DirectoryPath m_directoryPath;
    FileName m_filename;
    
    static std::set<std::string> g_openedForWrite;
    std::string m_filePath;
    
    int  FlushData();
    void FlushMyBuffer();

    void ReadToBuffer();
    eResult OpenFileForOperation(const std::string & sFilePath, enum FileOperation);

    static const char * FileOperationToString(FileOperation op);
    
    static DirectoryPath getOSCurrentDir();
    static bool getOSCurrentDir( DirectoryPath& );
};
    
    
}
