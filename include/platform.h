#pragma once

#ifdef __APPLE__
#  include "TargetConditionals.h"
#endif

#include "os.storage.h"

namespace imajuscule
{
#if _WIN32

#   include "Windows.h"
    using Platform = Windows;

#elif TARGET_OS_IOS

#   include "iOS.h"
    using Platform = iOS;

#else

#   include "Posix.h"
    using Platform = Posix;

#endif
}
