
#if _WIN32
namespace imajuscule {
    std::wstring s2ws(const std::string& s)
    {
        int len;
        int slength = (int)s.length() + 1;
        len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
        std::unique_ptr<wchar_t[]> buf(new wchar_t[len]);
        MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf.get(), len);
        std::wstring r(buf.get());
        return r;
    }
    std::string GetLastErrorAsString()
    {
        DWORD errorMessageID = ::GetLastError();
        if(errorMessageID == 0)
            return std::string();
        
        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                     nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);
        
        std::string message(messageBuffer, size);
        
        LocalFree(messageBuffer);
        
        return message;
    }
    
    bool findResource(int name, std::string const &type, resource & res) {
        
        auto wtype = s2ws(type);
        res.res = FindResource(0, MAKEINTRESOURCE(name), wtype.c_str());
        if(nullptr == res.res) {
            return false;
        }
        res.handle = LoadResource(0, res.res);
        if(nullptr == res.handle) {
            return false;
        }
        return true;
    }
    
    bool getResource(resource const & res, std::string & result) {
        DWORD dataSize = SizeofResource(0, res.res);
        char* data = (char*)LockResource(res.hamdle);
        result.assign(data, dataSize);
        return true;
    }
    
    bool readResource(int name, std::string const &type, std::string & result) {
        resource res;
        if(!findResource(name, type, res)) {
            return false;
        }
        return getResource(res, result);
    }
} // namespace imajuscule
#elif __APPLE__
#else
# include "generated/read.resources.cpp"
#endif

