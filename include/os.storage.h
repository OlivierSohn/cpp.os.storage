#pragma once

#include <iostream>
#include <vector>
#include <stdint.h>

enum eResult
{
    ILE_SUCCESS = 0,
    ILE_ERROR,
    ILE_NOT_IMPLEMENTED,
    ILE_BAD_PARAMETER,
    ILE_OBJECT_INVALID
};

#define SIZE_READ_BUFFER 2048

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
    // returns true if dir exists, false otherwise
    static bool listFilenames(const std::string & dir, std::vector<std::string> & filenames);

protected:

    Storage();
    virtual ~Storage();

    //
    //          returns ILE_BAD_PARAMETER if the file could not be opened 
    //
    eResult OpenFileForOperation(const std::string & sFilePath, enum FileOperation);

    void WriteData(void * p, size_t size, size_t count);

    void ReadData(void * p, size_t size, size_t count);

    void Finalize();
    void CloseFile();

    void UpdateFileHeader();

    // child classes should call this method directly only the first time the header is written.
    // for subsequent header writes they should call instead UpdateFileHeader that will call this method at the appropriate moment
    // and then restore the file position to the position it had before writing the header
    virtual void DoUpdateFileHeader() = 0;

    void SaveBegin();
    void SaveEnd();

private:
    FILE* m_pFile;
    std::vector<unsigned char> m_writeBuffer;
    unsigned char m_freadBuffer[SIZE_READ_BUFFER];
    unsigned int m_bufferReadPos;

    int  FlushData();
    void FlushMyBuffer();

    void ReadToBuffer();

    static const char * FileOperationToString(FileOperation op);
};
