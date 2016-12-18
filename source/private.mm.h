#include <TargetConditionals.h>

#import <Foundation/Foundation.h>

#if TARGET_OS_IOS
#else
# import <Foundation/NSUrl.h>
# import <AppKit/NSOpenPanel.h>
#endif

#include "private.h"
