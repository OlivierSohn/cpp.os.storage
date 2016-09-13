#import <Foundation/Foundation.h>

#include "platform.h"

namespace imajuscule {
    
    DirectoryPath iOS::user_path() {
        char path[1024];
        CFStringGetCString( (CFStringRef)NSHomeDirectory() , path , sizeof(path) , kCFStringEncodingUTF8 );
        return DirectoryPath(path);
    }
}
