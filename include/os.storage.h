
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
        eResult removeDir(const std::string & path);
        eResult removeFile(const std::string & path);
        
        bool isGUID(std::string const & str);
    }
    
    class Storage;
    class DirectoryPath {
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
        
        auto begin() const { return vec.begin(); }
        auto end() const { return vec.end(); }

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

    using FileName = std::string;

    class Storage;
    
    struct ReadableStorage {
        friend class Storage;
        
        DirectoryPath const & directory() {
            return m_directoryPath;
        }

        enum class FileMode
        {
            WRITE,
            READ
        };
        
    protected:
        ReadableStorage(DirectoryPath const &, FileName const &);
        virtual ~ReadableStorage();

        eResult OpenForRead();
        void CloseFile();
        
        void ReadData(void * p, size_t size, size_t count);
        

    protected:
        void* m_pFile;
    private:
        unsigned char m_freadBuffer[SIZE_READ_BUFFER];
        size_t m_bufferReadPos;

    protected:
        DirectoryPath m_directoryPath;
        FileName m_filename;

    private:
        eResult OpenFileForOperation(const std::string & sFilePath, FileMode);
        void ReadToBuffer();
    };
    
    class Storage : public ReadableStorage
    {
    public:
        
        eResult Save();
        
    protected:

        Storage(DirectoryPath const & d, FileName const & f) : ReadableStorage(d, f) {}

        eResult OpenForWrite();
        
        virtual void WriteData(void * p, size_t size, size_t count);
        
        void Finalize();
        
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
        
        std::vector<unsigned char> m_writeBuffer;
        
        static std::set<std::string> g_openedForWrite;
        std::string m_filePath;
        
        int  FlushData();
        void FlushMyBuffer();
        
        
    };
    namespace StorageStuff {
        std::vector< std::string > listFilenames( const DirectoryPath & path );
        std::vector< std::string > listFilenames( const std::string & path );
        
        const char * FileOperationToString(Storage::FileMode op);
    }
    
}
