#pragma once

#include <string>

namespace imajuscule {

#if _WIN32
    bool readResource(int name, std::string const &type, std::string & result);
#else
    bool readResource(const char * name, std::string const &type, std::string & result);
#endif
}
