
namespace imajuscule
{
#if _WIN32

#   include "Windows_.h"
    using Platform = Windows;

#elif TARGET_OS_IOS

#   include "iOS.h"
    using Platform = iOS;

#else

#   include "Posix.h"
    using Platform = Posix;

#endif

  DirectoryPath root();

}
