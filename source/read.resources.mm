#include <string>

#ifdef __APPLE__

#import <Foundation/Foundation.h>

#elif _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#endif

namespace imajuscule {

#ifdef __APPLE__
bool readResource(int name, int type, std::string & result) {
    std::string filePath = appleResourcePath(name, type);
    filePath = ResourcePath(filePath);
    return get_file_contents(filePath, result );
}
#elif __ANDROID__
bool readResource(int name, int type, std::string & result) {
    A(!"TODO");
	return false;
}
#elif _WIN32
std::wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}
std::string GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}
bool readResource(int name, std::string const &type, std::string & result) {
    
    auto wtype = s2ws(type);
    HRSRC hRes = FindResource(0, MAKEINTRESOURCE(name), wtype.c_str());
    if(NULL != hRes)
    {
        HGLOBAL hData = LoadResource(0, hRes);
        if(NULL != hData)
        {
	        DWORD dataSize = SizeofResource(0, hRes);
            char* data = (char*)LockResource(hData);
	        result.assign(data, dataSize);
            return true;
        }
    }
    return false;
}
#endif

} // namespace imajuscule
