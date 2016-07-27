#pragma once

#include <string>

namespace imajuscule {
    
#ifdef __APPLE__
    bool readResource(const char * name, std::string const &type, std::string & result);
#else
    bool readResource(const char * name, std::string const &type, std::string & result);
    bool readResource(int name, std::string const &type, std::string & result);
#endif
}
