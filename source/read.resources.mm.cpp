
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
        
        std::string fullPath(path);
        auto where = fullPath.find_last_of('/');
        if(where != std::string::npos) {
            path_.first = {std::string(fullPath.begin(), fullPath.begin() + where)};
            path_.second = {std::string(fullPath.begin() + where + 1, fullPath.end())};
        }
        else {
            path_.first = {fullPath};
            path_.second = "";
        }
        return true;
    }
    
    bool getResource(resource const & res, std::string &result) {
        return get_file_contents( res.first.toString() + "/" + res.second, result);
    }
    
    bool readResource(const char * name, std::string const &type, std::string & result) {
        resource res;
        if(!findResource(name, type, res)) {
            return false;
        }
        return getResource(res, result);
    }
    
}
