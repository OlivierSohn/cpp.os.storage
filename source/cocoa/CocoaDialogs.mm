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
#import <AppKit/NSColor.h>
#import <AppKit/NSOpenPanel.h>
#include <string>
#include <functional>
#include "os.file.dialog.h"

bool BasicFileOpen(std::string & sPathWithFileName, std::string & sFileName, const std::string & sFileExt)
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
            sPathWithFileName = [myString UTF8String];
            
            NSString *filename = [url lastPathComponent];
            sFileName = [filename UTF8String];
            
            bRet = true;
        }
    }
    
    return bRet;
}

void fAsyncDirectoryOperation(const std::string & title, std::function<void(OperationResult, const std::string &)> f, std::function<void(void)> fEnd)
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    
    [panel setTitlebarAppearsTransparent:YES];
    
    // using the button (prompt) instead
    [panel setTitleVisibility:NSWindowTitleHidden];
    [panel setPrompt:[NSString stringWithUTF8String:title.c_str()]];

    [panel setAlphaValue:0.9f];
    [panel setShowsResizeIndicator:NO];
    [panel setTreatsFilePackagesAsDirectories:YES];
    
    [panel setCanChooseFiles:NO];
    [panel setCanChooseDirectories:YES];
    [panel setCanCreateDirectories:YES];
    [panel setResolvesAliases:YES];
    [panel setAllowsMultipleSelection:NO];
    
    [panel setAllowedFileTypes:nil ];
    
    [panel beginWithCompletionHandler:^(NSInteger result) {
        if (result == NSFileHandlingPanelOKButton) {
            for (NSURL *url in [panel URLs])
            {
                NSString *myString = [url path];
                std::string sPath = [myString UTF8String];
                f(OperationResult::SUCCESS, sPath);
            }
        }
        else
            f(OperationResult::CANCELED, std::string());

        fEnd();
    }];
}

bool BasicDirectoryOpen(std::string & sPath)
{
    bool bRet = false;
    
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles:NO];
    [panel setCanChooseDirectories:YES];
    [panel setCanCreateDirectories:YES];
    [panel setResolvesAliases:YES];
    [panel setAllowsMultipleSelection:NO];
    
    [panel setAllowedFileTypes:nil ];
    
    NSInteger clicked = [panel runModal];
    
    if (clicked == NSFileHandlingPanelOKButton) {
        for (NSURL *url in [panel URLs])
        {
            // do something with the url here.
            
            NSString *myString = [url path];
            sPath = [myString UTF8String];
            
            bRet = true;
        }
    }
    
    return bRet;
}