
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef __APPLE__
#import "TargetConditionals.h"
#endif

#if !TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR

#include "platform_osx.hpp"
#include "../../../windowrendertarget.hpp"

#include <Cocoa/Cocoa.h>
#include <cstdlib>
#include <cstdio>

namespace mini3d {
namespace graphics {

    NSOpenGLContext* context = 0;

}
}

void mini3d_assert(bool expression, const char* text, ...);
using namespace mini3d::graphics;


///////// PLATFORM ////////////////////////////////////////////////////////////

// NOTE: SetInternalWindowPixelFormat is a workaround for platforms (windows specifically) that need all windows for a device context to be of the same pixel format.
// It will return true if the old internal pixel format was incompatible with the new one (requiring the device context to be recreated).
// Platforms without limitations on pixel formats will simply always return false.

Platform_osx::~Platform_osx()                                     { @autoreleasepool { [context release]; }}

Platform_osx::Platform_osx()
{
    @autoreleasepool {

    NSOpenGLPixelFormatAttribute attributes [] = {NSOpenGLPFADoubleBuffer, NSOpenGLPFADepthSize, 24, NSOpenGLPFAColorSize, 24, nil};
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];

    context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
    [context makeCurrentContext];
    }
}

void Platform_osx::GetWindowContentSize(MINI3D_WINDOW window, unsigned int &width, unsigned int &height) const
{
    @autoreleasepool {
    NSWindow* nswindow = (NSWindow*)window;
        NSRect rect = [[nswindow contentView] frame];
        width = rect.size.width;
        height = rect.size.height;
    }
}

void Platform_osx::SwapWindowBuffers(IWindowRenderTarget* pWindowRenderTarget)
{
    @autoreleasepool { glFlush(); [context flushBuffer]; }
}

void Platform_osx::SetRenderWindow(IWindowRenderTarget* pWindowRenderTarget)
{
    @autoreleasepool {
        if (pWindowRenderTarget == 0)
        {
            [context setView:nil];
        }
        else
        {
            NSWindow* window = (NSWindow*)pWindowRenderTarget->GetWindowHandle();
            [context setView:[window contentView]];
        }
    }
}


#endif
