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

#define DATA_TYPE_BOOL         'b' // fixed length : char
#define DATA_TYPE_DOUBLE_ARRAY 'D'
#define DATA_TYPE_FLOAT_ARRAY  'F'
#define DATA_TYPE_STRING       's' // variable length

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
    static eResult makeDir(const std::string & path);
    // returns true if dir exists, flase otherwise
    static bool listFilenames(const std::string & dir, std::vector<std::string> & filenames);

protected:

    Storage();
    virtual ~Storage();

    //
    //          returns ILE_BAD_PARAMETER if the file could not be opened 
    //
    eResult OpenFileForOperation(const std::string & sFilePath, enum FileOperation);
    void WriteData(void * p, size_t size, size_t count);
    int32_t WriteKeyData(char key, std::string & sValue);
    int32_t WriteKeyData(char key, bool bValue);
    int32_t WriteKeyData(char key, int32_t iValue);
    int32_t WriteKeyData(char key, double dValue);
    int32_t WriteKeyData(char key, double * bValueArray, int size);
    int32_t WriteKeyData(char key, float * bValueArray, int size);
    void ReadData(void * p, size_t size, size_t count);

    void Finalize();
    void CloseFile();

    void UpdateFileHeader();

    // child classes should call this method directly only the first time the header is written.
    // for subsequent header writes they should call instead UpdateFileHeader that will call this method at the appropriate moment
    // and then restore the file position to the position it had before writing the header
    virtual void DoUpdateFileHeader() = 0;

private:
    FILE* m_pFile;
    std::vector<unsigned char> m_writeBuffer;
    unsigned char m_freadBuffer[SIZE_READ_BUFFER];
    unsigned int m_bufferReadPos;

    int32_t m_totalSizeInBytes;

    int  FlushData();
    void FlushMyBuffer();

    void ReadToBuffer();

    static const char * FileOperationToString(FileOperation op);
};
