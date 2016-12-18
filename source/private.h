#include "public.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#else
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
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
# include <thread>
# include <memory>
#endif
#include <sys/stat.h>

#include "platform.h"
