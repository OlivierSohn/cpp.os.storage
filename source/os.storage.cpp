#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

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

#include "platform.h"
#include "os.storage.h"
#include "os.log.h"
#include "os.log.format.h"
#include "os.file.dialog.h"


using namespace imajuscule;
using namespace StorageStuff;

namespace imajuscule {
    DirectoryPath DirectoryPath::referentiablesPath;
    DirectoryPath DirectoryPath::capturePath;
}

std::set<std::string> Storage::g_openedForWrite;

void DirectoryPath::setReferentiablesDir(DirectoryPath const & d) {
    referentiablesPath = d;
}
bool DirectoryPath::getReferentiablesDir(DirectoryPath & p) {
    p = referentiablesPath;
    return !referentiablesPath.empty();
}

void DirectoryPath::setCaptureImageDir(DirectoryPath const & d) {
    capturePath = d;
}
bool DirectoryPath::getCaptureImageDir(DirectoryPath & p) {
    p = capturePath;
    return !capturePath.empty();
}

Storage::Storage(DirectoryPath const &d, FileName const &f) :
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
    //LG(INFO, "Storage::OpenFileForOperation( %s, %s) begin", sFilePath.c_str(), FileOperationToString(op));
    
    eResult ret = ILE_SUCCESS;
    
    CloseFile();
    
    m_pFile = fopen(sFilePath.c_str(), op == OP_READ ? "rb" : "wb");
    
    if ( unlikely(!m_pFile))
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
    
    //LG(INFO, "Storage::OpenFileForOperation() returns %d", ret);
    return ret;
}

eResult Storage::OpenForRead()
{
    std::string filePath;
    
    for( auto const & directory_name : m_directoryPath.vec)
    {
        filePath.append(directory_name);
        filePath.append("/");
    }
    
    filePath.append(m_filename);
    std::string search("//");
    std::string replace("/");
    ReplaceStringInPlace(filePath, search, replace );
    eResult ret = OpenFileForOperation(filePath, OP_READ);
    if ( unlikely(ret != ILE_SUCCESS))
    {
        LG(ERR, "Storage::OpenForRead : OpenFileForOperation returned %d", ret);
    }
    
    return ret;
}
eResult Storage::OpenForWrite()
{
    eResult ret = ILE_SUCCESS;
    {
        for( auto const & directory_name : m_directoryPath.vec)
        {
            m_filePath.append( directory_name );
            m_filePath.append("/");
            if (!dirExists(m_filePath))
            {
                ret = makeDir(m_filePath);
                if ( unlikely(ILE_SUCCESS != ret))
                {
                    LG(ERR, "Storage::OpenForWrite : Storage::makeDir(%s) error : %d", m_filePath.c_str(), ret);
                    return ret;
                }
            }
        }
        
        m_filePath.append(m_filename);
        
        auto it2 = g_openedForWrite.find(m_filePath);
        if(it2 != g_openedForWrite.end())
        {
            return ILE_RECURSIVITY;
        }
        g_openedForWrite.insert(m_filePath);
        
        ret = OpenFileForOperation(m_filePath, OP_WRITE);
        if ( unlikely(ret != ILE_SUCCESS))
        {
            LG(ERR, "Storage::OpenForWrite : OpenFileForOperation returned %d", ret);
        }
    }
    
    return ret;
}

eResult Storage::Save()
{
    m_filePath.clear();
    
    eResult ret = doSaveBegin();
    if (ret != ILE_SUCCESS)
    {
        if( unlikely(ret != ILE_RECURSIVITY))
            LG(ERR, "Storage::Save : doSaveBegin returned %d", ret);
        return ret;
    }
    
    ret = doSave();
    if (unlikely(ret != ILE_SUCCESS))
    {
        LG(ERR, "Storage::Save : doSave returned %d", ret);
        return ret;
    }
    
    doSaveEnd();
    
    return ILE_SUCCESS;
}

eResult Storage::doSaveBegin()
{
    {
        eResult ret = OpenForWrite();
        if ( ret != ILE_SUCCESS )
        {
            if ( unlikely(ret != ILE_RECURSIVITY) )
                LG(ERR, "Storage::SaveBegin : OpenForWrite returned %d", ret);
            return ret;
        }
    }
    
    // to reserve the header space (it will be overwritten in ::SaveEnd())
    DoUpdateFileHeader();
    
    return ILE_SUCCESS;
}
eResult Storage::doSave()
{
    return ILE_SUCCESS;
}
void Storage::doSaveEnd()
{
    UpdateFileHeader();
    
    if(!m_filePath.empty())
        g_openedForWrite.erase(m_filePath);
}

void Storage::UpdateFileHeader()
{
    // write the data for this frame
    FlushMyBuffer();
    
    // now that the data has been written, we can modify the file position
    fpos_t curPos;
    if (likely(!fgetpos((FILE*)m_pFile, &curPos)))
    {
        rewind((FILE*)m_pFile);
        
        DoUpdateFileHeader();
        
        int ret = FlushData();
        if (unlikely(ret))
        {
            LG(ERR, "Storage::UpdateFileHeader : FlushData returned %d", ret );
        }
        
        if (likely(!fsetpos((FILE*)m_pFile, &curPos)))
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
    auto result = _fread_nolock(m_freadBuffer, 1, SIZE_READ_BUFFER, (FILE*)m_pFile);
#else
    auto result = fread(m_freadBuffer, 1, SIZE_READ_BUFFER, (FILE*)m_pFile);
#endif
  // we don't check the result and it's okay
  //if (result != SIZE_READ_BUFFER) {fputs ("Reading error",stderr); exit (3);}
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

namespace imajuscule {
    
    DirectoryPath DirectoryPath::root() {
        return Platform::user_path() + "grid3d";
    }
    
    namespace StorageStuff {
    const char * FileOperationToString(Storage::FileOperation op)
    {
        switch (op)
        {
            case Storage::OP_WRITE:
                return "OP_WRITE";
                break;
            case Storage::OP_READ:
                return "OP_READ";
                break;
            default:
                return "UNKNOWN";
                break;
        }
    }
    
#ifdef _WIN32
    void string_cast(const wchar_t* pSource, unsigned int codePage, std::string & oCast)
    {
        A(pSource != 0);
        oCast.clear();
        size_t sourceLength = std::wcslen(pSource);
        if (likely(sourceLength > 0))
        {
            int length = ::WideCharToMultiByte(codePage, 0, pSource, sourceLength, NULL, 0, NULL, NULL);
            if (likely(length != 0))
            {
                std::vector<char> buffer(length);
                ::WideCharToMultiByte(codePage, 0, pSource, sourceLength, &buffer[0], length, NULL, NULL);
                oCast.assign(buffer.begin(), buffer.end());
            }
        }
    }
#endif
    
    bool dirExists(const std::string & path)
    {
        //LG(INFO, "dirExists(%s)", (path.c_str() ? path.c_str() : "NULL"));
        bool bExists = false;
        
        struct stat info;
        bExists = ((stat(path.c_str(), &info) == 0) && (info.st_mode & S_IFDIR));
        
        //LG(INFO, "dirExists(%s) returns %s", (path.c_str() ? path.c_str() : "NULL"), (bExists ? "true" : "false"));
        return bExists;
    }
    
    bool fileExists(const std::string & path)
    {
        //LG(INFO, "fileExists(%s)", (path.c_str() ? path.c_str() : "NULL"));
        bool bExists = false;
        
        struct stat info;
        bExists = (stat(path.c_str(), &info) == 0) && !(info.st_mode & S_IFDIR);
        
        //LG(INFO, "fileExists(%s) returns %s", (path.c_str() ? path.c_str() : "NULL"), (bExists ? "true" : "false"));
        return bExists;
    }
    
    bool fileCreationDate(const std::string & path, std::string & oDate)
    {
        //LG(INFO, "fileCreationDate(%s)", (path.c_str() ? path.c_str() : "NULL"));
        
        oDate.clear();
        
        bool bExists = false;
        
        struct stat info;
        bExists = (stat(path.c_str(), &info) == 0) && !(info.st_mode & S_IFDIR);
        if (likely(bExists))
        {
            tm * time = gmtime((const time_t*)&(info.st_mtime));
            FormatDate(time, oDate);
        }
        else
        {
            LG(ERR, "fileCreationDate : file does not exist");
            oDate.assign("../../.. ..:..:..");
        }
        
        //LG(INFO, "fileCreationDate(%s) returns %s", (path.c_str() ? path.c_str() : "NULL"), (bExists ? "true" : "false"));
        return bExists;
    }
    
    eResult makeDir(const std::string & path)
    {
        //LG(INFO, "makeDir(%s)", (path.c_str() ? path.c_str() : "NULL"));
        eResult res = ILE_SUCCESS;
        
#ifdef _WIN32
        std::wstring swName = std::wstring(path.begin(), path.end());
        const wchar_t * pwStr = swName.c_str();
        //LG(INFO, "makeDir : before CreateDirectory");
        if (!CreateDirectory(pwStr, NULL))
        {
            DWORD dwErr = GetLastError();
            if (unlikely(dwErr != ERROR_ALREADY_EXISTS))
            {
                LG(ERR, "makeDir : CreateDirectory error %x", dwErr);
                res = ILE_ERROR;
            }
            else
            {
                //LG(INFO, "makeDir : directory already exists");
            }
        }
#else
        
        int ret;
        ret = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
        if (ret != 0)
        {
            if( unlikely(errno != EEXIST))
            {
                LG(ERR, "makeDir : CreateDirectory error %x", errno);
                res = ILE_ERROR;
            }
            else
            {
                //LG(INFO, "makeDir : directory already exists");
            }
        }
        
#endif
        
        //LG(INFO, "makeDir(%s) returns %d", (path.c_str() ? path.c_str() : "NULL"), res);
        return res;
    }
    
        
        std::vector< std::string > listFilenames( const DirectoryPath & path ) {
            return listFilenames(path.toString());
        }
    std::vector< std::string > listFilenames( const std::string & path )
    {
        //LG(INFO, "listFilenames(%s)", (path.c_str() ? path.c_str() : "NULL"));
        bool bExists = false;
        std::vector<std::string> filenames;
        
#ifdef _WIN32
        WIN32_FIND_DATA ffd;
        TCHAR szDir[MAX_PATH];
        size_t length_of_arg;
        HANDLE hFind = INVALID_HANDLE_VALUE;
        DWORD dwError = 0;
        
        // Check that the input path plus 3 is not longer than MAX_PATH.
        // Three characters are for the "\*" plus NULL appended below.
        
        TCHAR tstrTo[MAX_PATH*2];
        const int nMax = sizeof(tstrTo) / sizeof(tstrTo[0]);
        int tstrLen;
#ifdef UNICODE
        tstrLen = MultiByteToWideChar(CP_ACP, 0, path.c_str(), strlen(path.c_str()), NULL, 0);
        if ( unlikely(tstrLen >= nMax) ) {
            LG(ERR, "listFilenames : string %s is tool long", path.c_str());
            A(0);
            return filenames;
        }
        tstrTo[tstrLen] = 0;
        MultiByteToWideChar(CP_ACP, 0, path.c_str(), strlen(path.c_str()), tstrTo, tstrLen);
#else
        int err = strcpy_s( tstrTo, nMax, path.c_str() );
        if ( err != 0 )
        {
            LG(ERR, "listFilenames : strcpy_s error %d", err);
            A(0);
            return filenames;
        }
        tstrLen = strlen( tstrTo );
#endif
        
        HRESULT hr=StringCchLength(tstrTo, MAX_PATH, &length_of_arg);
        
        if (unlikely(FAILED(hr)))
        {
            LG(ERR, "listFilenames : StringCchLength failed (%x)", hr);
        }
        else if (unlikely(length_of_arg > (MAX_PATH - 3)))
        {
            // can fix this by using unicode version of FindFirstFile and prepending \\?\ to the path
            LG(ERR, "listFilenames : Directory path is too long");
        }
        else
        {
            // Prepare string for use with FindFile functions.  First, copy the
            // string to a buffer, then append '\*' to the directory name.
            
            StringCchCopy(szDir, MAX_PATH, tstrTo);
            StringCchCat(szDir, MAX_PATH, TEXT("\\*"));
            
            // Find the first file in the directory.
            
            hFind = FindFirstFile(szDir, &ffd);
            
            if (unlikely(INVALID_HANDLE_VALUE == hFind))
            {
                LG(INFO, "listFilenames : FindFirstFile returned INVALID_HANDLE_VALUE");
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
                if (unlikely(dwError != ERROR_NO_MORE_FILES))
                {
                    LG(ERR, "listFilenames : FindNextFile returned %d", dwError);
                }
            }
            FindClose(hFind);
        }
        
#else
        DIR           *d;
        struct dirent *dir;
        d = opendir(path.c_str());
        if (likely(d))
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
        
        //LG(INFO, "listFilenames(%s) found %d files and returns %s", (path.c_str() ? path.c_str() : "NULL"), filenames.size(), (bExists ? "true" : "false"));
        return filenames;
    }
    
    bool isGUID(std::string const & str)
    {
        bool bIsGUID = true;
        
        int count = -1;
        int idx_parenthesis_open = -1;
        int idx_parenthesis_close = -1;
        for(char const &c:str)
        {
            count++;
            
            switch(c)
            {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F':
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                    break;
                case '-':
                    break;
                case '{':
                    if(idx_parenthesis_open != -1)
                        bIsGUID = false;
                    else
                        idx_parenthesis_open = count;
                    break;
                case '}':
                    if(idx_parenthesis_close != -1)
                        bIsGUID = false;
                    else
                        idx_parenthesis_close = count;
                    break;
                default:
                    bIsGUID = false;
                    break;
            }
            if (!bIsGUID) {
                break;
            }
        }
        
        if(bIsGUID)
        {
            if( idx_parenthesis_close != -1 ) {
                if( idx_parenthesis_open != -1 ) {
                    if( ( idx_parenthesis_open != 0 ) || ( idx_parenthesis_close != count ) ) {
                        bIsGUID = false;
                    }
                } else {
                    bIsGUID = false;
                }
            } else if( idx_parenthesis_open != -1 ) {
                bIsGUID = false;
            }
        }
        return bIsGUID;
    }
    
    } // namespace StorageStuff
} // namespace imajuscule

DirectoryPath::DirectoryPath(const std::string & sInput) {
    set(sInput);
}

DirectoryPath::DirectoryPath(const char * sInput) {
    set(std::string(sInput));
}
void DirectoryPath::set(const std::string & sInput)
{
    std::istringstream f(sInput);
    std::string s;
    while (getline(f, s, '/')) {
        vec.push_back(s);
    }
}

std::string DirectoryPath::toString() const
{
    std::string ret;
    for( auto & st : vec )
    {
        ret.append(st);
        ret.append("/");
    }
    
    if(ret.size() > 1) {
        // remove trailing '/'
        ret.pop_back();
    }
    
    return ret;
}
