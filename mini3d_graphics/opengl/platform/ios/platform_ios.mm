
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef __APPLE__
#import "TargetConditionals.h"
#endif

#if TARGET_OS_IPHONE


#include "Platform_ios.hpp"
#include "../../../windowrendertarget.hpp"
#include <cstdlib>
#include <cstdio>

#import <QuartzCore/QuartzCore.h>

namespace mini3d {
namespace graphics {
    EAGLContext* sharedContext = 0;
}
}

void mini3d_assert(bool expression, const char* text, ...);
using namespace mini3d::graphics;

@interface Mini3dUIView : UIView

-(void)setViewController:(UIViewController*) viewController;
-(GLuint)colorRenderbuffer;
-(GLuint)framebuffer;
@end

@implementation Mini3dUIView
{
    GLuint framebuffer;
    GLuint colorRenderbuffer;
    GLuint depthRenderbuffer;
}

+(Class)layerClass {
    return [CAEAGLLayer class];
}

-(GLuint)colorRenderbuffer { return colorRenderbuffer; }
-(GLuint)framebuffer { return framebuffer; }

-(void)renderBufferStorage {
    [EAGLContext setCurrentContext: sharedContext];
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [sharedContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
}

- (void)dealloc
{
    [super dealloc];
}

-(id)initWithFrame:(CGRect)frame
{
    if ((self = [super initWithFrame:frame]) == nil)
        return self;

    self.layer.opaque = YES;
    self.userInteractionEnabled = NO;
    [self setBackgroundColor:[UIColor redColor]];
    
    [EAGLContext setCurrentContext: sharedContext];
    
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    glGenRenderbuffers(1, &colorRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [self performSelectorOnMainThread:@selector(renderBufferStorage) withObject:nil waitUntilDone:YES];
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
    
    GLint width;
    GLint height;
    
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    
    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24_OES, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }

    return self;
}

-(void)setViewController:(UIViewController*) viewController { [viewController setView:self]; }
-(void)clearViewController:(UIViewController*) viewController { [viewController setView:0]; }
@end


///////// PLATFORM ////////////////////////////////////////////////////////////

Platform_ios::~Platform_ios()
{
    [sharedContext release];
}

Platform_ios::Platform_ios()
{
    sharedContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:sharedContext];
}

void Platform_ios::PrepareWindow(IWindowRenderTarget* pWindowRenderTarget)
{
    UIWindow* window = (UIWindow*)pWindowRenderTarget->GetWindowHandle();
    mini3d_assert([window rootViewController] == 0, "The window used with a window render target must not have a rootViewController set.");
    mini3d_assert([[window subviews] count] == 0, "The window used with a window render target must not have any subviews set.");

    [window setRootViewController:[[UIViewController alloc] init]];
    Mini3dUIView* view = [[Mini3dUIView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    [view performSelectorOnMainThread:@selector(setViewController:) withObject:[window rootViewController] waitUntilDone:YES];
}

void Platform_ios::UnPrepareWindow(IWindowRenderTarget* pWindowRenderTarget)
{
    UIWindow* window = (UIWindow*)pWindowRenderTarget->GetWindowHandle();
    Mini3dUIView* view = (Mini3dUIView*)[[window rootViewController] view];
    UIViewController* viewController = [window rootViewController];
    [view performSelectorOnMainThread:@selector(clearViewController:) withObject:viewController waitUntilDone:YES];
    [viewController release];
    [view release];
}

void Platform_ios::GetWindowContentSize(IWindowRenderTarget* pWindowRenderTarget, unsigned int &width, unsigned int &height) const
{
    UIWindow* window = (UIWindow*)pWindowRenderTarget->GetWindowHandle();
    Mini3dUIView* view = (Mini3dUIView*)[[window rootViewController] view];

    glBindRenderbuffer(GL_RENDERBUFFER, [view colorRenderbuffer]);

    GLint glwidth;
    GLint glheight;
    
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &glwidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &glheight);
  
    width = glwidth;
    height = glheight;
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Platform_ios::SwapWindowBuffers(IWindowRenderTarget* pWindowRenderTarget)
{    
    UIWindow* window = (UIWindow*)pWindowRenderTarget->GetWindowHandle();
    Mini3dUIView* view = (Mini3dUIView*)[[window rootViewController] view];

    glBindRenderbuffer(GL_RENDERBUFFER, [view colorRenderbuffer]);
    [sharedContext presentRenderbuffer:[view colorRenderbuffer]];
}

void Platform_ios::SetRenderWindow(IWindowRenderTarget* pWindowRenderTarget)
{
    if (pWindowRenderTarget == 0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    else
    {
        UIWindow* window = (UIWindow*)pWindowRenderTarget->GetWindowHandle();
        Mini3dUIView* view = (Mini3dUIView*)[[window rootViewController] view];

        [EAGLContext setCurrentContext:sharedContext];
        glBindFramebuffer(GL_FRAMEBUFFER, [view framebuffer]);
        glBindRenderbuffer(GL_RENDERBUFFER, [view colorRenderbuffer]);
    }

}


#endif
