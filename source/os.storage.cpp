
#include "os.storage.h"
#include "os.log.h"
#include "os.log.format.h"
#include "os.file.dialog.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h> 
#include <strsafe.h>
#else
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#endif

#include <ctime>
#include <sys/stat.h>
#include <stdio.h>
#include <cstring> // memcpy
#include <sstream>
#include <iostream>

Storage::DirectoryPath Storage::m_curDir = std::list<std::string>();
Storage::DirectoryPath Storage::curDir()
{
    return m_curDir;
}

Storage::Storage(DirectoryPath d, FileName f) :
m_pFile(NULL),
m_bufferReadPos(0),
m_directoryPath(d),
m_filename(f)
{}


Storage::~Storage()
{
    CloseFile();
}

void Storage::CloseFile()
{
    if (m_pFile)
        fclose((FILE*)m_pFile);
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

#ifndef WIN32
    const int sizeBuf = PATH_MAX+1;
    char bufCurDirectory[sizeBuf];
    if (getcwd(bufCurDirectory, sizeBuf))
    {
        LG(INFO, "Storage::OpenFileForOperation : current directory %s", bufCurDirectory);
    }
    else
    {
        LG(INFO, "Storage::OpenFileForOperation : getcwd error %d", errno);
    }
#else
#endif
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

eResult Storage::OpenForRead()
{
    std::string filePath;
    
    auto it = m_directoryPath.begin();
    auto end = m_directoryPath.end();
    for(;it!=end;++it)
    {
        filePath.append(*it);
        filePath.append("/");
    }
    
    filePath.append(m_filename);
    std::string search("//");
    std::string replace("/");
    ReplaceStringInPlace(filePath, search, replace );
    eResult ret = OpenFileForOperation(filePath, OP_READ);
    if (ret != ILE_SUCCESS)
    {
        LG(ERR, "Storage::OpenForRead : OpenFileForOperation returned %d", ret);
        goto end;
    }

end:
    return ret;
}
eResult Storage::OpenForWrite()
{
    eResult ret = ILE_SUCCESS;
    {
        std::string filePath;
        filePath.append("./");
        
        auto it = m_directoryPath.begin();
        auto end = m_directoryPath.end();
        for(;it!=end;++it)
        {
            filePath.append(*it);
            filePath.append("/");
            if (!Storage::dirExists(filePath))
            {
                ret = Storage::makeDir(filePath);
                if (ILE_SUCCESS != ret)
                {
                    LG(ERR, "Storage::OpenForWrite : Storage::makeDir(%s) error : %d", filePath.c_str(), ret);
                    goto end;
                }
            }
        }
        
        filePath.append(m_filename);
        
        ret = OpenFileForOperation(filePath, OP_WRITE);
        if (ret != ILE_SUCCESS)
        {
            LG(ERR, "Storage::OpenForWrite : OpenFileForOperation returned %d", ret);
            goto end;
        }
    }
    
end:
    return ret;
}

eResult Storage::Save()
{
    eResult ret = doSaveBegin();
    if (ret != ILE_SUCCESS)
    {
        LG(ERR, "Storage::Save : doSaveBegin returned %d", ret);
        goto end;
    }
    
    ret = doSave();
    if (ret != ILE_SUCCESS)
    {
        LG(ERR, "Storage::Save : doSave returned %d", ret);
        goto end;
    }

    doSaveEnd();
    
end:
    return ret;
}

eResult Storage::doSaveBegin()
{
    eResult ret = OpenForWrite();
    if (ret != ILE_SUCCESS)
    {
        LG(ERR, "Storage::SaveBegin : OpenForWrite returned %d", ret);
        goto end;
    }
    
    // to reserve the header space (it will be overwritten in ::SaveEnd())
    DoUpdateFileHeader();
    
end:
    return ret;
}
eResult Storage::doSave()
{
    return ILE_SUCCESS;
}
void Storage::doSaveEnd()
{
    UpdateFileHeader();
}

void Storage::UpdateFileHeader()
{
    // write the data for this frame
    FlushMyBuffer();

    // now that the data has been written, we can modify the file position
    fpos_t curPos;
    if (!fgetpos((FILE*)m_pFile, &curPos))
    {
        rewind((FILE*)m_pFile);

        DoUpdateFileHeader();

        int ret = FlushData();
        if (ret)
        {
            LG(ERR, "Storage::UpdateFileHeader : FlushData returned %d", ret );
        }

        if (!fsetpos((FILE*)m_pFile, &curPos))
        {

        }
        else
        {
            LG(ERR, "Storage::UpdateFileHeader : fsetpos failed : %d", errno);
            A(0);
        }
    }
    else
    {
        LG(ERR, "Storage::UpdateFileHeader : fgetpos failed : %d", errno);
        A(0);
    }
}

void Storage::FlushMyBuffer()
{
    size_t count = m_writeBuffer.size();
    if (count == 0)
        return;
#ifdef _WIN32
    _fwrite_nolock(m_writeBuffer.data(), 1, count, (FILE*)m_pFile);
#else
    fwrite(m_writeBuffer.data(), 1, count, (FILE*)m_pFile);
#endif

    m_writeBuffer.clear();
}

void Storage::ReadToBuffer()
{
#ifdef _WIN32
    _fread_nolock(m_freadBuffer, 1, SIZE_READ_BUFFER, (FILE*)m_pFile);
#else
    fread(m_freadBuffer, 1, SIZE_READ_BUFFER, (FILE*)m_pFile);
#endif
}

void Storage::ReadData(void * p, size_t size, size_t count)
{
    //LG(INFO, "Storage::ReadData(%x, %d, %d)", p, size, count);

    size_t total = size * count;
    
    do
    {
        //LG(INFO, "Storage::ReadData m_bufferReadPos = %d", m_bufferReadPos);
        
        size_t max = m_bufferReadPos + total;
        
        //LG(INFO, "Storage::ReadData max = %d", max);
        
        long secondRead = max - SIZE_READ_BUFFER;
        
        //LG(INFO, "Storage::ReadData secondRead = %d", secondRead);
        
        if (secondRead > 0)
        {
            //LG(INFO, "Storage::ReadData secondRead > 0");
            
            long i = SIZE_READ_BUFFER - m_bufferReadPos;
            
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
    
    //LG(INFO, "Storage::ReadData end");
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
    return _fflush_nolock((FILE*)m_pFile);
#else
    return fflush((FILE*)m_pFile);
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
    A(pSource != 0);
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

    if(bRet)
    {
        std::string path(dir);
        m_curDir = toDirPath(path);
    }
    
    LG(INFO, "Storage::SetCurrentDirectory(%s) returns %s", (dir ? dir : "NULL"), (bRet?"true":"false"));
    return bRet;
}

Storage::DirectoryPath Storage::toDirPath(const std::string & sInput)
{
    DirectoryPath strings;
    std::istringstream f(sInput);
    std::string s;
    while (getline(f, s, ';')) {
        strings.push_back(s);
    }
    
    return strings;
}