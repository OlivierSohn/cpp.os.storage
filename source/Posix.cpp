#import <Foundation/Foundation.h>

#include "platform.h"

namespace imajuscule {
    
    DirectoryPath Posix::user_path() {
        char path[1024];
#if __APPLE__
        CFStringGetCString( (CFStringRef)NSHomeDirectory() , path , sizeof(path) , kCFStringEncodingUTF8 );
#else
#endif
        return DirectoryPath(path);
    }
}
