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

#define SIZE_READ_BUFFER 256

// data types

#define DATA_TYPE_INT32        'a'
#define DATA_TYPE_BOOL         'b'
#define DATA_TYPE_DOUBLE       'd'
#define DATA_TYPE_DOUBLE_ARRAY 'D'
#define DATA_TYPE_FLOAT        'f'
#define DATA_TYPE_FLOAT_ARRAY  'F'
#define DATA_TYPE_CHAR         's'
#define DATA_TYPE_CHAR_ARRAY   'S'

// keys

#define KEY_IC_DQROT     'a' // 4 double
#define KEY_IC_DPOS      'b' // 3 double
#define KEY_IC_SET       'c' // bool
#define KEY_DATE_CREA    'd' // n * char
#define KEY_GUID         'e' // n * char
#define KEY_RAWPATH_GUID 'f' // n * char
#define KEY_INTPATH_GUID 'g' // n * char
#define KEY_REGPATH_GUID 'h' // n * char
#define KEY_NAME         'i' // n * char
#define KEY_ITGR_MODE    'j' // int32
#define KEY_FILTER_RATE  'k' // double
#define KEY_FILTER_CUTOFF 'l' // double
#define KEY_FILTER_ADAPT 'm' // bool
#define KEY_VEC_ACC_POS  'P' // n * 4 double
#define KEY_VEC_INT_POS  'p' // n * 7 double
#define KEY_VEC_ACC_ROT  'R' // n * 4 double
#define KEY_VEC_INT_ROT  'r' // n * 5 double


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
