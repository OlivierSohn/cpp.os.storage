#if _WIN32
# include "Windows.cpp"
#elif __linux__
# include "Posix.cpp"
# include "linux/LinuxDialogs.cpp"
#endif

#ifndef __APPLE__
# include  "read.resources.cpp"
#endif

#include "file2string.cpp"
#include "os.abstraction.cpp"
#include "os.file.dialog.cpp"
#include "os.storage.cpp"
#include "os.storage.keys.cpp"
#include "win32/Win32Dialogs.cpp"

