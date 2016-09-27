#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <thread>
#include <algorithm>

#include "os.storage.h"
#include "os.file.dialog.h"

namespace imajuscule {
    using namespace StorageStuff;
    bool Open(FileSystemOperation::Kind k, const std::vector<std::string> & extensions, std::string & sPath)
    {
        bool bRet = false;
        A(0);
        return bRet;

    }

    bool BasicFileOpen(std::string & sFilePath, std::string & fileName, const std::string & fileExt)
    {
        bool bRet = false;
        A(0);
        return bRet;
    }


    namespace {
        bool exec(const char* cmd, std::string & result) {
            char buffer[128];
            LG(INFO, "executing \"%s\"", cmd);
            std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
            if (!pipe) {
                result = "popen failed";
                return false;    
            }
            while (!feof(pipe.get())) {
                if (fgets(buffer, 128, pipe.get()) != NULL) {
                    result += buffer;
                }
            }
            return true;
        }        
    }

    FileSystemOperation::Nature
    fFileSystemOperation(FileSystemOperation::Kind k, 
                         const std::vector<std::string> & extensions,
                         const std::string & title,
                         std::function<void(OperationResult, const std::string &)> f,
                         std::function<void(void)> fEnd)
    {
        std::thread thread([=](){
            std::string cmd("python -c \"import tkFileDialog; print 'begin_results'; print ");
            if(k == FileSystemOperation::Kind::OP_DIR) {
                cmd += "tkFileDialog.askdirectory()\" ";                
            } else {
                cmd += "'\\n'.join(tkFileDialog.askopenfilenames())\" ";
            }
            cmd += "2>&1";

            bool res;
            std::string result;
            if( !exec( cmd.c_str(), result ) ) {
                f(OperationResult::CANCELED, result);
            } else {
                auto lines = split_in_lines(result);
                lines.erase(std::remove(lines.begin(), lines.end(), ""), lines.end());

                bool errors = true;
                bool ok = false;
                for (auto const & line : lines) {
                    if(line == "begin_results") {
                        errors = false;
                        continue;
                    }
                    if( errors ) {
                        LG(ERR, "%s", line.c_str());
                        continue;
                    }
                    ok = true;
                    f(OperationResult::SUCCESS, line);
                }
                if(!ok) {
                    f(OperationResult::CANCELED, result);
                }
            }
            
            fEnd();
        });
        
        thread.detach();

        return FileSystemOperation::Nature::NON_BLOCKING;
    }
}
