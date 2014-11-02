
#include "os.storage.h"
#include "os.log.h"
#include "os.log.format.h"

#ifdef _WIN32
#include <windows.h>
#include <tchar.h> 
#include <strsafe.h>
#else
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <ctime>
#include <sys/stat.h>
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
    LG(INFO, "Storage::OpenFileForOperation( %s, %s) begin", sFilePath.c_str(), FileOperationToString(op));

    eResult ret = ILE_SUCCESS;

    CloseFile();

    m_pFile = fopen(sFilePath.c_str(), op == OP_READ ? "rb" : "wb");

    if (!m_pFile)
    {
        LG(ERR, "Storage::OpenFileForOperation : fopen failed : %d", errno);
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

    LG(INFO, "Storage::OpenFileForOperation() returns %d", ret);
    return ret;
}

void Storage::SaveBegin()
{
    // to reserve the header space (it will be overwritten in ::SaveEnd())
    DoUpdateFileHeader();
}
void Storage::SaveEnd()
{
    UpdateFileHeader();
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
    LG(INFO, "Storage::ReadData(%x, %d, %d)", p, size, count);

    size_t total = size * count;
    
    do
    {
        //LG(INFO, "Storage::ReadData m_bufferReadPos = %d", m_bufferReadPos);
        
        unsigned int max = m_bufferReadPos + total;
        
        //LG(INFO, "Storage::ReadData max = %d", max);
        
        int secondRead = max - SIZE_READ_BUFFER;
        
        //LG(INFO, "Storage::ReadData secondRead = %d", secondRead);
        
        if (secondRead > 0)
        {
            //LG(INFO, "Storage::ReadData secondRead > 0");
            
            int i = SIZE_READ_BUFFER - m_bufferReadPos;
            
            memcpy(p, &m_freadBuffer[m_bufferReadPos], i);
            
            m_bufferReadPos = 0;
            
            //LG(INFO, "Storage::ReadData before ReadToBuffer");
            ReadToBuffer();
            //LG(INFO, "Storage::ReadData after ReadToBuffer");
            
            total -= i;
            p = (char*)p + i;
        }
        else
        {
            //LG(INFO, "Storage::ReadData secondRead < 0");
            
            memcpy(p, &m_freadBuffer[m_bufferReadPos], total);
            m_bufferReadPos = max;
            
            break;
        }
    }
    while(total>0);
    
    LG(INFO, "Storage::ReadData end");
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
const char * Storage::FileOperationToString(FileOperation op)
{
    switch (op)
    {
    case OP_WRITE:
        return "OP_WRITE";
        break;
    case OP_READ:
        return "OP_READ";
        break;
    default:
        return "UNKNOWN";
        break;
    }
}

#ifdef _WIN32
void Storage::string_cast(const wchar_t* pSource, unsigned int codePage, std::string & oCast)
{
    assert(pSource != 0);
    oCast.clear();
    size_t sourceLength = std::wcslen(pSource);
    if (sourceLength > 0)
    {
        int length = ::WideCharToMultiByte(codePage, 0, pSource, sourceLength, NULL, 0, NULL, NULL);
        if (length != 0)
        {
            std::vector<char> buffer(length);
            ::WideCharToMultiByte(codePage, 0, pSource, sourceLength, &buffer[0], length, NULL, NULL);
            oCast.assign(buffer.begin(), buffer.end());
        }
    }
}
#endif

bool Storage::dirExists(const std::string & path)
{
    LG(INFO, "Storage::dirExists(%s)", (path.c_str() ? path.c_str() : "NULL"));
    bool bExists = false;

    struct stat info;
    bExists = ((stat(path.c_str(), &info) == 0) && (info.st_mode & S_IFDIR));

    LG(INFO, "Storage::dirExists(%s) returns %s", (path.c_str() ? path.c_str() : "NULL"), (bExists ? "true" : "false"));
    return bExists;
}

bool Storage::fileExists(const std::string & path)
{
    LG(INFO, "Storage::fileExists(%s)", (path.c_str() ? path.c_str() : "NULL"));
    bool bExists = false;

    struct stat info;
    bExists = (stat(path.c_str(), &info) == 0) && !(info.st_mode & S_IFDIR);

    LG(INFO, "Storage::fileExists(%s) returns %s", (path.c_str() ? path.c_str() : "NULL"), (bExists ? "true" : "false"));
    return bExists;
}

bool Storage::fileCreationDate(const std::string & path, std::string & oDate)
{
    LG(INFO, "Storage::fileCreationDate(%s)", (path.c_str() ? path.c_str() : "NULL"));
    
    oDate.clear();

    bool bExists = false;

    struct stat info;
    bExists = (stat(path.c_str(), &info) == 0) && !(info.st_mode & S_IFDIR);
    if (bExists)
    {
        tm * time = gmtime((const time_t*)&(info.st_mtime));
        FormatDate(time, oDate);
    }
    else
    {
        LG(ERR, "Storage::fileCreationDate : file does not exist");
        oDate.assign("../../.. ..:..:..");
    }

    LG(INFO, "Storage::fileCreationDate(%s) returns %s", (path.c_str() ? path.c_str() : "NULL"), (bExists ? "true" : "false"));
    return bExists;
}

eResult Storage::makeDir(const std::string & path)
{
    LG(INFO, "Storage::makeDir(%s)", (path.c_str() ? path.c_str() : "NULL"));
    eResult res = ILE_SUCCESS;

#ifdef _WIN32
    std::wstring swName = std::wstring(path.begin(), path.end());
    const wchar_t * pwStr = swName.c_str();
    LG(INFO, "Storage::makeDir : before CreateDirectory");
    if (!CreateDirectory(pwStr, NULL))
    {
        DWORD dwErr = GetLastError();
        if (dwErr != ERROR_ALREADY_EXISTS)
        {
            LG(ERR, "Storage::makeDir : CreateDirectory error %x", dwErr);
            res = ILE_ERROR;
        }
        else
        {
            LG(INFO, "Storage::makeDir : directory already exists");
        }
    }
#else

    int ret;
    ret = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
    if (ret != 0)
    {
        if( errno != EEXIST)
        {
            LG(ERR, "Storage::makeDir : CreateDirectory error %x", errno);
            res = ILE_ERROR;
        }
        else
        {
            LG(INFO, "Storage::makeDir : directory already exists");
        }
    }

#endif

    LG(INFO, "Storage::makeDir(%s) returns %d", (path.c_str() ? path.c_str() : "NULL"), res);
    return res;
}

// returns true if dir exists, false otherwise
bool Storage::listFilenames(const std::string & path, std::vector<std::string> & filenames)
{
    LG(INFO, "Storage::listFilenames(%s)", (path.c_str() ? path.c_str() : "NULL"));
    bool bExists = false;
    filenames.clear();

#ifdef _WIN32
    WIN32_FIND_DATA ffd;
    TCHAR szDir[MAX_PATH];
    size_t length_of_arg;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;

    // Check that the input path plus 3 is not longer than MAX_PATH.
    // Three characters are for the "\*" plus NULL appended below.

    TCHAR *tstrTo;
    int tstrLen;
#ifdef UNICODE
    tstrLen = MultiByteToWideChar(CP_ACP, 0, path.c_str(), strlen(path.c_str()), NULL, 0);
    tstrTo = (TCHAR*)malloc((tstrLen+1) * sizeof(TCHAR));
    tstrTo[tstrLen] = 0;
    MultiByteToWideChar(CP_ACP, 0, path.c_str(), strlen(path.c_str()), tstrTo, tstrLen);
#else
    tstrTo = strdup(dllPath);
    tstrLen = strlen(tstrTo);
#endif

    StringCchLength(tstrTo, MAX_PATH, &length_of_arg);

    if (length_of_arg > (MAX_PATH - 3))
    {
        LG(ERR, "Storage::listFilenames : Directory path is too long");
    }
    else
    {
        // Prepare string for use with FindFile functions.  First, copy the
        // string to a buffer, then append '\*' to the directory name.

        StringCchCopy(szDir, MAX_PATH, tstrTo);
        StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

        // Find the first file in the directory.

        hFind = FindFirstFile(szDir, &ffd);

        if (INVALID_HANDLE_VALUE == hFind)
        {
            LG(INFO, "Storage::listFilenames : FindFirstFile returned INVALID_HANDLE_VALUE");
        }
        else
        {
            // List all the files in the directory with some info about them.

            bExists = true;

            do
            {
                if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    std::string cast;
                    string_cast(ffd.cFileName, CP_ACP, cast);
                    filenames.push_back(cast);
                }
            } while (FindNextFile(hFind, &ffd) != 0);

            dwError = GetLastError();
            if (dwError != ERROR_NO_MORE_FILES)
            {
                LG(ERR, "Storage::listFilenames : FindNextFile returned %d", dwError);
            }
        }
        FindClose(hFind);
    }

    // use tstrTo up to tstrLen characters as needed...
    free(tstrTo);
#else
    DIR           *d;
    struct dirent *dir;
    d = opendir(path.c_str());
    if (d)
    {
        bExists = true;
        while ((dir = readdir(d)) != NULL)
        {
            if (dir->d_type == DT_REG)
            {
                filenames.push_back(dir->d_name);
            }
        }

        closedir(d);
    }
#endif

    LG(INFO, "Storage::listFilenames(%s) found %d files and returns %s", (path.c_str() ? path.c_str() : "NULL"), filenames.size(), (bExists ? "true" : "false"));
    return bExists;
}

bool Storage::setCurrentDir(const char * dir)
{
    LG(INFO, "Storage::SetCurrentDirectory(%s) begin", (dir ? dir : "NULL"));
    bool bRet = false;

#ifdef _WIN32

    std::string sName(dir);
    std::wstring swName = std::wstring(sName.begin(), sName.end());
    const wchar_t * pwStr = swName.c_str();
    if (!SetCurrentDirectory(pwStr))
    {
        DWORD dwErr = GetLastError();
        LG(ERR, "Storage::SetCurrentDirectory : SetCurrentDirectory error : %x", dwErr);
    }
    else
    {
        bRet = true;
    }

#else

    if (0 != chdir(dir))
    {
        LG(ERR, "Storage::SetCurrentDirectory : chdir error : %x", errno);
    }
    else
    {
        bRet = true;
    }

#endif

    LG(INFO, "Storage::SetCurrentDirectory(%s) returns %s", (dir ? dir : "NULL"), (bRet?"true":"false"));
    return bRet;
}