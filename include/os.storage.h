#pragma once

#include <iostream>
#include <vector>

enum eResult
{
    ILE_SUCCESS = 0,
    ILE_GENERAL_ERROR,
    ILE_NOT_IMPLEMENTED,
    ILE_BAD_PARAMETER,
};

#define SIZE_READ_BUFFER 256

class Storage
{
public:

    enum FileOperation
    {
        OP_WRITE,
        OP_READ
    };

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

private:
    FILE* m_pFile;
    std::vector<unsigned char> m_writeBuffer;
    unsigned char m_freadBuffer[SIZE_READ_BUFFER];
    unsigned int m_bufferReadPos;

    int  FlushData();
    void FlushMyBuffer();

    void ReadToBuffer();

    static char * FileOperationToString(FileOperation op);
};
