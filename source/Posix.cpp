#if __APPLE__
# import <Foundation/Foundation.h>
#else
# include <unistd.h>
# include <sys/types.h>
# include <pwd.h>
#endif

#include "platform.h"

namespace imajuscule {
    
    DirectoryPath Posix::user_path() {
#if __APPLE__
        char path[1024];
        CFStringGetCString( (CFStringRef)NSHomeDirectory() , path , sizeof(path) , kCFStringEncodingUTF8 );
#else
        const char *path;

        if ((path = getenv("HOME")) == NULL) {
            path = getpwuid(getuid())->pw_dir;
        }
#endif
        return DirectoryPath(path);
    }
}
