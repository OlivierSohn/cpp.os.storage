#include <string>

#include "os.log.h"

#ifdef __APPLE__

#import <Foundation/Foundation.h>

#include "file2string.h"

#elif _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#endif

#include "read.resources.h"

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
                                     NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
        
        std::string message(messageBuffer, size);
        
        LocalFree(messageBuffer);
        
        return message;
    }
    
    bool findResource(int name, std::string const &type, resource & res) {
        
        auto wtype = s2ws(type);
        res.res = FindResource(0, MAKEINTRESOURCE(name), wtype.c_str());
        if(NULL == res.res) {
            return false;
        }
        res.handle = LoadResource(0, res.res);
        if(NULL == res.handle) {
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
namespace imajuscule {
    
    bool findResource(const char * name, std::string const &type, resource & path_) {
        if(!name) {
            LG(ERR, "name of resource is null");
            return false;
        }
        
        auto b = CFBundleGetMainBundle();
        if(!b) {
            LG(ERR, "could not find bundle");
            return false;
        }
        
        auto n = [NSString stringWithUTF8String:name];
        auto t = [NSString stringWithUTF8String:type.c_str()];
        CFStringRef ref_n = (__bridge CFStringRef)n;
        CFStringRef ref_t = (__bridge CFStringRef)t;
        
        auto url = CFBundleCopyResourceURL(b, ref_n, 0, ref_t);
        if(!url) {
            LG(ERR, "could not find resource %s of type %s in bundle", name, type.c_str());
            return false;
        }

        
        CFStringRef imagePath = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
        
        // Get the system encoding method
        CFStringEncoding encodingMethod = CFStringGetSystemEncoding();
        
        // Convert the string reference into a C string
        const char *path = CFStringGetCStringPtr(imagePath, encodingMethod);
        if(!path) {
            LG(ERR, "char * is null");
            return false;
        }
        
        path_ = std::string(path);
        return true;
    }
    
    bool getResource(resource const & res, std::string &result) {
        return get_file_contents( res, result);
    }
    
    bool readResource(const char * name, std::string const &type, std::string & result) {
        resource res;
        if(!findResource(name, type, res)) {
            return false;
        }
        return getResource(res, result);
    }
    
} // namespace imajuscule
#else
# include "generated/read.resources.cpp"
#endif

