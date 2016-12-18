
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
    
}
