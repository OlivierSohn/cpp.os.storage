#include "Windows.h"

#include "platform.h"

namespace imajuscule {
    
    DirectoryPath Windows::user_path() {

        TCHAR szPath[MAX_PATH];

        if ( SUCCEEDED(SHGetFolderPath(NULL,
            CSIDL_PERSONAL | CSIDL_FLAG_CREATE,
            NULL,
            0,
            szPath)) )
        {
            char path[1024];
            size_t s;
            if ( auto err = wcstombs_s(&s, path, 1024, szPath, MAX_PATH) ) {
                LG(ERR, "wcstombs_s error %d", err);
                return DirectoryPath();
            }
            return DirectoryPath(path);
        }

        return DirectoryPath();
    }
}
