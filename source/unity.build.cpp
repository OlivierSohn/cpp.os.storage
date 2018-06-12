#include "private.h"

#if _WIN32
# include "Windows.cpp"
#elif __linux__
# include "Posix.cpp"
# include "linux/LinuxDialogs.cpp"
#elif __APPLE__
#endif

#include "os.abstraction.cpp"
#include "os.file.dialog.cpp"
#include "os.storage.keys.cpp"

#if _WIN32
#  include "win32/Win32Dialogs.cpp"
#endif


namespace imajuscule {
  DirectoryPath root() {
    return Platform::user_path() + "grid3d";
  }
  
  
  std::string generateGuid()
  {
    std::string sGuid;
    sGuid.reserve(32);
    
#if defined (_MSC_VER)
    GUID guid;
    HRESULT hr = CoCreateGuid(&guid);
    if (unlikely(FAILED(hr)))
    {
      LG(ERR, "generateGuid : CoCreateGuid failed %x", hr);
      Assert(0);
      return sGuid;
    }
    
    OLECHAR* bstrGuid;
    hr = StringFromCLSID(guid, &bstrGuid);
    if (unlikely(FAILED(hr)))
    {
      LG(ERR, "generateGuid : StringFromCLSID failed %x", hr);
      Assert(0);
      return sGuid;
    }
    
    // First figure out our required buffer size.
    int cbData = WideCharToMultiByte(CP_ACP, 0, bstrGuid/*pszDataIn*/, -1, nullptr, 0, nullptr, nullptr);
    hr = (cbData == 0) ? HRESULT_FROM_WIN32(GetLastError()) : S_OK;
    if (likely(SUCCEEDED(hr)))
    {
      // Now allocate a buffer of the required size, and call WideCharToMultiByte again to do the actual conversion.
      std::unique_ptr<char[]> pszData( new (std::nothrow) CHAR[cbData] );
      hr = pszData.get() ? S_OK : E_OUTOFMEMORY;
      if (likely(SUCCEEDED(hr)))
      {
        hr = WideCharToMultiByte(CP_ACP, 0, bstrGuid/*pszDataIn*/, -1, pszData.get(), cbData, nullptr, nullptr)
        ? S_OK
        : HRESULT_FROM_WIN32(GetLastError());
        if (likely(SUCCEEDED(hr)))
        {
          for( int i=0; i<cbData; i++ ) {
            switch ( pszData[i] ) {
              default:
                sGuid.push_back(pszData[i]);
                break;
              case 0:
              case '-':
              case '}':
              case '{':
                break;
            }
          }
        }
      }
    }
    
    // ensure memory is freed
    ::CoTaskMemFree(bstrGuid);
#elif __ANDROID__
    LG(ERR, "generateGuid : on android, the guid should be generated in java");
#else
    uuid_t uu;
    uuid_generate(uu);
    char uuid[37];
    uuid_unparse(uu, uuid);
    for( int i=0; i<(int)sizeof(uuid); i++ ) {
      switch(uuid[i]) {
        case 0:
          break;
        case '-':
          continue;
        default:
          sGuid.push_back(toupper(uuid[i]));
          break;
      }
    }
#endif
    
    assert(sGuid.size() == 32);
    return std::move(sGuid);
  }

}
