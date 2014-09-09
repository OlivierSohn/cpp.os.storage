
#include "os.storage.h"


#ifdef _WIN32
#include "windows.h"
#endif
#include <stdio.h>
#include <cassert>

Storage::Storage() :
m_pFile(NULL),
m_bufferReadPos(0)
{}


Storage::~Storage()
{
    CloseFile();
}

void Storage::CloseFile()
{
    if (m_pFile)
        fclose(m_pFile);
    m_bufferReadPos = 0;
}

void Storage::Finalize()
{
    FlushData();

    CloseFile();
}


eResult Storage::OpenFileForOperation(const std::string & sFilePath, enum FileOperation op)
{
    eResult ret = ILE_SUCCESS;

    CloseFile();

    m_pFile = fopen(sFilePath.c_str(), op == OP_READ ? "rb" : "wb");

    if (!m_pFile)
    {
        std::cerr << "fopen failed : " << errno << std::endl;
        ret = ILE_BAD_PARAMETER;
    }
    else
    {
        if (op == OP_READ)
        {
            m_bufferReadPos = 0;
            ReadToBuffer();
        }
    }

    return ret;
}

void Storage::UpdateFileHeader()
{
    // write the data for this frame
    FlushMyBuffer();

    // now that the data has been written, we can modify the file position
    fpos_t curPos;
    if (!fgetpos(m_pFile, &curPos))
    {
        rewind(m_pFile);

        DoUpdateFileHeader();

        int ret = FlushData();
        if (ret)
        {
            std::cerr << "FlushData returned " << ret << std::endl;
        }

        if (!fsetpos(m_pFile, &curPos))
        {

        }
        else
        {
            std::cerr << "fsetpos failed : " << errno << std::endl;
            assert(0);
        }
    }
    else
    {
        std::cerr << "fgetpos failed : " << errno << std::endl;
        assert(0);
    }
}

void Storage::FlushMyBuffer()
{
    size_t count = m_writeBuffer.size();
    if (count == 0)
        return;
#ifdef _WIN32
    _fwrite_nolock(m_writeBuffer.data(), 1, count, m_pFile);
#else
    fwrite(m_writeBuffer.data(), 1, count, m_pFile);
#endif

    m_writeBuffer.clear();
}

void Storage::ReadToBuffer()
{
#ifdef _WIN32
    _fread_nolock(m_freadBuffer, 1, SIZE_READ_BUFFER, m_pFile);
#else
    fread(m_freadBuffer, 1, SIZE_READ_BUFFER, m_pFile);
#endif
}

void Storage::ReadData(void * p, size_t size, size_t count)
{
    unsigned int max = m_bufferReadPos + (size * count);

    int secondRead = max - SIZE_READ_BUFFER;

    int i = 0;

    if (secondRead > 0)
    {
        for (; m_bufferReadPos < SIZE_READ_BUFFER; i++, m_bufferReadPos++)
        {
            ((unsigned char*)p)[i] = m_freadBuffer[m_bufferReadPos];
        }
        m_bufferReadPos = 0;
        ReadToBuffer();
        for (; m_bufferReadPos < secondRead; i++, m_bufferReadPos++)
        {
            ((unsigned char*)p)[i] = m_freadBuffer[m_bufferReadPos];
        }
    }
    else
    {
        for (; m_bufferReadPos < max; i++, m_bufferReadPos++)
        {
            ((unsigned char*)p)[i] = m_freadBuffer[m_bufferReadPos];
        }
    }
}

void Storage::WriteData(void * p, size_t size, size_t count)
{
    size_t add = size*count;
    m_writeBuffer.insert(m_writeBuffer.end(), (unsigned char*)p, ((unsigned char*)p) + add);
}

int Storage::FlushData()
{
    FlushMyBuffer();

#ifdef _WIN32
    return _fflush_nolock(m_pFile);
#else
    return fflush(m_pFile);
#endif
}
