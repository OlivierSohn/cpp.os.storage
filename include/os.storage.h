#pragma once

#include <string>
#include <vector>
#include <set>
#include <initializer_list>

#include "os.log.h"
#include "os.log.format.h"

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
    namespace StorageStuff {
#ifdef _WIN32
        void string_cast(const wchar_t* pSource, unsigned int codePage /*= CP_ACP*/, std::string & oString);
#endif
        
        bool dirExists(const std::string & path);
        bool fileExists(const std::string & path);
        bool fileCreationDate(const std::string & path, std::string & oDate);
        eResult makeDir(const std::string & path);
        
        bool isGUID(std::string const & str);
    }

    class Storage;
    class DirectoryPath {
        friend class Storage;
        std::vector<std::string> vec;
        static DirectoryPath referentiablesPath;
        static DirectoryPath capturePath;
    public:
        DirectoryPath() {}
        DirectoryPath( const std::string & path );
        DirectoryPath( const char * path );
        DirectoryPath( std::initializer_list<std::string> vec ) :
        vec(vec) {}
        
        static DirectoryPath root();
        static void setReferentiablesDir(DirectoryPath const &);
        static bool getReferentiablesDir(DirectoryPath &);
        static void setCaptureImageDir(DirectoryPath const &);
        static bool getCaptureImageDir(DirectoryPath &);
        
        bool empty() const { return vec.empty(); }
        bool isFile() const { return StorageStuff::fileExists(toString()); }
        bool isDir() const { return StorageStuff::dirExists(toString()); }
        
        std::string toString() const;
        void set(const std::string & path);
        
        DirectoryPath operator + ( const DirectoryPath & other ) const {
            DirectoryPath ret = *this;
            ret.vec.insert( ret.vec.end(), other.vec.begin(), other.vec.end() );
            return ret;
        }
        void operator += ( const DirectoryPath & other ) {
            vec.insert( vec.end(), other.vec.begin(), other.vec.end() );
        }
        DirectoryPath & append(const char * p) {
            vec.push_back(p);
            return *this;
        }
    };

class Storage
{
public:

    enum FileOperation
    {
        OP_WRITE,
        OP_READ
    };

    eResult Save();

    typedef std::string FileName;
    DirectoryPath const & directory() {
        return m_directoryPath;
    }
    
protected:
    
    Storage(DirectoryPath const &, FileName const &);
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
    virtual void DoUpdateFileHeader() { A(0); }

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
    
};
    namespace StorageStuff {
        std::vector< std::string > listFilenames( const DirectoryPath & path );
        std::vector< std::string > listFilenames( const std::string & path );

        const char * FileOperationToString(Storage::FileOperation op);
    }

}
