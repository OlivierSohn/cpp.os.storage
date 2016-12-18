
namespace imajuscule {

#if _WIN32
    struct resource_ {
        HRSRC res;
        HGLOBAL handle;
    };
    using resource = resource_;

    bool readResource(int name, std::string const &type, std::string & result);
    bool findResource(int name, std::string const &type, resource & res);
    bool getResource(resource const & res, std::string & result);
#else
    using resource = std::string;
    
    bool readResource(const char * name, std::string const &type, std::string & result);
    bool findResource(const char * name, std::string const &type, resource & path_);
    bool getResource(resource const & res, std::string &result);
#endif
}
