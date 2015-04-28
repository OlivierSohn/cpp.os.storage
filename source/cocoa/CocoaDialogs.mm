//
//  LLVPlayerCocoaDialogs.mm
//  LLVPlayer
//
//  Created by Olivier on 09/08/2014.
//
//

#include <stdio.h>

#import <Foundation/Foundation.h>
#import <Foundation/NSUrl.h>
#import <AppKit/NSOpenPanel.h>
#include <string>

bool BasicFileOpen(std::string & sPath, std::string & sFileName, const std::string & sFileExt)
{
    bool bRet = false;
    
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowsMultipleSelection:NO];
    
    NSString *fileExt = [NSString stringWithCString:sFileExt.c_str()
                                                encoding:[NSString defaultCStringEncoding]];
    NSArray *tarr = [NSArray arrayWithObjects:fileExt, nil];
    [panel setAllowedFileTypes:tarr ];
    
    NSInteger clicked = [panel runModal];
    
    if (clicked == NSFileHandlingPanelOKButton) {
        for (NSURL *url in [panel URLs])
        {
            // do something with the url here.

            NSString *myString = [url path];
            sPath = [myString UTF8String];
            
            NSString *filename = [url lastPathComponent];
            sFileName = [filename UTF8String];
            
            bRet = true;
        }
    }
    
    return bRet;
}