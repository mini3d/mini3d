
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef __APPLE__
#import "TargetConditionals.h"
#endif

#if TARGET_OS_IPHONE

#include "../iplatform.hpp"
#import <QuartzCore/QuartzCore.h>


void mini3d_assert(bool expression, const char* text, ...);

@interface RenderSurface : NSObject
@property (nonatomic, assign, readonly) GLuint FrameBuffer;
@property (nonatomic, assign, readonly) GLuint ColorBuffer;
@property (nonatomic, assign, readonly) GLuint DepthBuffer;
@property (nonatomic, retain) EAGLContext* Context;
@property (nonatomic, retain) CAEAGLLayer* Layer;
@property (nonatomic, retain) UIView* View;
@property unsigned int Width;
@property unsigned int Height;
@end

@implementation RenderSurface

-(id)initWithUIView:(UIView*)view andEAGLContext:(EAGLContext*)context
{
    if ((self = [super init]) == nil) return self;

    mini3d_assert([[view layer] class] == [CAEAGLLayer class], "UIView used for window render target must have a backing layer of CAEAGLLayer type!");
    
    _View = view;
    _Layer = (CAEAGLLayer*)[view layer];
    _Context = context;

    [EAGLContext setCurrentContext:_Context];
    glGenFramebuffers(1, &_FrameBuffer);
    glGenRenderbuffers(1, &_ColorBuffer);
    glGenRenderbuffers(1, &_DepthBuffer);

    [self performSelectorOnMainThread:@selector(updateSize) withObject:nil waitUntilDone:YES];
    [view performSelectorOnMainThread:@selector(setNeedsDisplay) withObject:nil waitUntilDone:YES];
    
    return self;
}

- (void)dealloc
{
    glDeleteRenderbuffers(1, &_ColorBuffer);
    glDeleteRenderbuffers(1, &_DepthBuffer);
    glDeleteFramebuffers(1, &_FrameBuffer);
    [super dealloc];
}

-(void)updateSize
{
    [EAGLContext setCurrentContext: _Context];

    glBindFramebuffer(GL_FRAMEBUFFER, _FrameBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _ColorBuffer);
    [_Context renderbufferStorage:GL_RENDERBUFFER fromDrawable:_Layer];
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _ColorBuffer);
    
    GLint width;
    GLint height;
    
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    
    glBindRenderbuffer(GL_RENDERBUFFER, _DepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24_OES, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _DepthBuffer);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    mini3d_assert(status == GL_FRAMEBUFFER_COMPLETE, "Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    
    _Width = width;
    _Height = height;
}

@end


///////// PLATFORM ////////////////////////////////////////////////////////////

namespace mini3d {
namespace graphics {

class Platform_ios : public IPlatform
{
public:
    unsigned int GetNativeSurfaceWidth(void* nativeSurface) const   { return [(RenderSurface*)nativeSurface Width]; }
    unsigned int GetNativeSurfaceHeight(void* nativeSurface) const  { return [(RenderSurface*)nativeSurface Height]; }

    void* PrepareWindow(void* nativeWindow)                         { return [[RenderSurface alloc] initWithUIView:(UIView*)nativeWindow andEAGLContext:m_pContext]; }
    void UnPrepareWindow(void* nativeWindow, void* nativeSurface)   { [(RenderSurface*)nativeSurface release]; }
    
    ~Platform_ios()                                                 { [m_pContext release]; }

    Platform_ios()
    {
        m_pContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        [EAGLContext setCurrentContext:m_pContext];
    }

    void SwapWindowBuffers(void* nativeSurface)
    {    
        RenderSurface* pRenderSurface = (RenderSurface*)nativeSurface;

        glBindRenderbuffer(GL_RENDERBUFFER, [pRenderSurface ColorBuffer]);
        [m_pContext presentRenderbuffer:[pRenderSurface ColorBuffer]];
    }

    void MakeCurrent(void* nativeSurface)
    {
        RenderSurface* pRenderSurface = (RenderSurface*)nativeSurface;

        if (pRenderSurface == 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }
        else
        {
            if ([pRenderSurface View].frame.size.width != [pRenderSurface Width] || [pRenderSurface View].frame.size.height != [pRenderSurface Height])
                [pRenderSurface performSelectorOnMainThread:@selector(updateSize) withObject:nil waitUntilDone:YES];
            
            glBindFramebuffer(GL_FRAMEBUFFER, [pRenderSurface FrameBuffer]);
            glBindRenderbuffer(GL_RENDERBUFFER, [pRenderSurface ColorBuffer]);
        }

    }
    
private:
    EAGLContext* m_pContext;
};

IPlatform* IPlatform::New() { return new Platform_ios(); }
}
}

#endif
