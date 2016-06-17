
#include <fstream>
#include <string>
#include <cerrno>

#include "os.log.h"

#include "file2string.h"

namespace imajuscule {
    
    bool get_file_contents(const std::string &filename, std::string & str )
    {
        std::ifstream in(filename, std::ios::in | std::ios::binary);
        if (!in)
        {
            LG(WARN, "file not found : %s", filename.c_str());
            return false;
        }
        in.seekg(0, std::ios::end);
        str.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&str[0], str.size());
        in.close();
        return true;
    }
}
