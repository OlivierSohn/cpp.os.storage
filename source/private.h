#include "../include/public.h"

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  define STRICT_TYPED_ITEMIDS
#  include <SDKDDKVer.h>
#  include <windows.h>      // For common windows data types and function headers
#  include <tchar.h>
#  include <Shobjidl.h>
#  include "Shobjidl.h"
#  include <shlobj.h>
#  include <objbase.h>      // For COM headers
#  include <shobjidl.h>     // for IFileDialogEvents and IFileDialogControlEvents
#  include <shlwapi.h>
#  include <knownfolders.h> // for KnownFolder APIs/datatypes/function headers
#  include <propvarutil.h>  // for PROPVAR-related functions
#  include <propkey.h>      // for the Property key APIs/datatypes
#  include <propidl.h>      // for the Property System APIs
#  include <shtypes.h>      // for COMDLG_FILTERSPEC
#  pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#  include <dirent.h>
#  include <sys/types.h>
#  include <unistd.h>
#  include <pwd.h>
#endif

#include <ctime>
#include <cstring>
#include <cstddef>
#include <cerrno>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#if __linux__
#  include <thread>
#  include <memory>
#endif
#include <sys/stat.h>
