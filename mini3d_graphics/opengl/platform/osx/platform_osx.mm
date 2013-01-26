
// Copyright (c) <2011-2013> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef __APPLE__
#import "TargetConditionals.h"
#endif

#if !TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR

#include "../iplatform.hpp"

#include <Cocoa/Cocoa.h>
#include <cstdlib>
#include <cstdio>

@interface OpenGLNSView : NSView { }
@property (nonatomic, assign) NSOpenGLContext* context;
@property (nonatomic, assign) NSRect rect;

- (BOOL) acceptsFirstResponder;
- (BOOL) becomeFirstResponder;

@end

@implementation OpenGLNSView { }
- (BOOL) acceptsFirstResponder { return NO; }
- (BOOL) becomeFirstResponder { return NO; }
@end

namespace mini3d {
namespace graphics {

void mini3d_assert(bool expression, const char* text, ...);

    
///////// PLATFORM ////////////////////////////////////////////////////////////

const NSOpenGLPixelFormatAttribute attributes[] = { NSOpenGLPFADoubleBuffer, NSOpenGLPFADepthSize, 24, NSOpenGLPFAColorSize, 24, nil };
    
class Platform_osx : public IPlatform
{
public:
    unsigned int GetNativeSurfaceWidth(void* nativeSurface) const   { return ((NSView*)nativeSurface).frame.size.width; }
    unsigned int GetNativeSurfaceHeight(void* nativeSurface) const  { return ((NSView*)nativeSurface).frame.size.height; }

    void SwapWindowBuffers(void* nativeSurface)                     { @autoreleasepool { glFlush(); [[(OpenGLNSView*)nativeSurface context] flushBuffer]; }}
    
    ~Platform_osx()                                                 { @autoreleasepool { [m_pPixelFormat release]; [m_pContext release]; }}

    Platform_osx()
    {
        @autoreleasepool {
            m_pPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];

            m_pContext = [[NSOpenGLContext alloc] initWithFormat:m_pPixelFormat shareContext:nil];
            [m_pContext makeCurrentContext];
        }
    }

    void* PrepareWindow(void* nativeWindow)
    {
        @autoreleasepool {
            NSWindow* window = (NSWindow*)nativeWindow;
            NSRect rect = [window contentRectForFrameRect:[window frame]];
            
            OpenGLNSView* contentView = [[OpenGLNSView alloc] initWithFrame:NSMakeRect(0, 0, rect.size.width, rect.size.height)];
            [window setContentView:contentView];

            NSOpenGLContext* pContext = [[NSOpenGLContext alloc] initWithFormat:m_pPixelFormat shareContext:m_pContext];
            [pContext setView:contentView];

            [contentView setContext:pContext];
            [contentView setRect:NSMakeRect(0,0,0,0)];
            
            return contentView;
        }    
    }
    
    void UnPrepareWindow(void* nativeWindow, void* nativeSurface)
    {
        @autoreleasepool {
            NSWindow* window = (NSWindow*)nativeWindow;
            [window setContentView: nil];

            OpenGLNSView* view = (OpenGLNSView*)nativeSurface;
            [[view context] release];
            [view release];
        }
    }

    void MakeCurrent(void* nativeSurface)
    {
        @autoreleasepool {
            OpenGLNSView* view = (OpenGLNSView*)nativeSurface;
            
            if (!NSEqualRects([view rect], [view frame]))
            {
                [[view context] update];
                [view setRect:[view frame]];
            }
            
            [[view context] makeCurrentContext];
        }
    }

    
private:
    NSOpenGLPixelFormat* m_pPixelFormat;
    NSOpenGLContext* m_pContext = 0;
};
    
    IPlatform* IPlatform::New() { return new Platform_osx(); }
    
}
}


#endif
