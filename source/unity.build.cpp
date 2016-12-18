#include "private.h"

#if _WIN32
# include "Windows.cpp"
#elif __linux__
# include "Posix.cpp"
# include "linux/LinuxDialogs.cpp"
#elif __APPLE__
#endif

#include "read.resources.cpp"

#include "file2string.cpp"
#include "os.abstraction.cpp"
#include "os.file.dialog.cpp"
#include "os.storage.cpp"
#include "os.storage.keys.cpp"
#include "win32/Win32Dialogs.cpp"

