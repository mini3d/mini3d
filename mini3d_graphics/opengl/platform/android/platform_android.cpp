
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef __ANDROID__

#include "Platform_android.hpp"
#include "../../../windowrendertarget.hpp"
#include "../../opengl.hpp"
#include "../openglwrapper.hpp"

#include <android/native_window.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

void mini3d_assert(bool expression, const char* text, ...);
using namespace mini3d::graphics;

namespace {
	const EGLint attribs[] = 			{ EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PIXMAP_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 24, EGL_STENCIL_SIZE, 0, EGL_NONE };
    EGLint pbufferAttribs[] = 			{ EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_TEXTURE_TARGET, EGL_NO_TEXTURE, EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE, EGL_NONE };
}

struct Platform_android::Internal
{
    EGLDisplay display;
    EGLConfig config;
    EGLint format;
    EGLSurface surface;
    EGLContext context;
};


///////// PLATFORM ////////////////////////////////////////////////////////////

Platform_android::Platform_android()
{
	mpI = new Internal();
	
    mpI->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(mpI->display, 0, 0);

    EGLint numConfigs;
    eglChooseConfig(mpI->display, attribs, &mpI->config, 1, &numConfigs);
    eglGetConfigAttrib(mpI->display, mpI->config, EGL_NATIVE_VISUAL_ID, &mpI->format);
    // TODO: found valid config?

    mpI->surface = eglCreatePbufferSurface(mpI->display, mpI->config, pbufferAttribs);

    EGLint contextAttrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    mpI->context = eglCreateContext(mpI->display, mpI->config, NULL, contextAttrs);

	mini3d_assert(eglMakeCurrent(mpI->display, mpI->surface, mpI->surface, mpI->context) == EGL_TRUE, "Failed to set default render target! (glXMakeCurrent)");

    pOgl = new OpenGlWrapper();
}

Platform_android::~Platform_android()
{ 
	eglMakeCurrent(mpI->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroySurface(mpI->display, mpI->surface);
	eglDestroyContext(mpI->display, mpI->context);
	eglTerminate(mpI->display);

	delete mpI;
}

void Platform_android::PrepareWindow(IWindowRenderTarget* pWindowRenderTarget)
{
	WindowRenderTarget_OpenGL* openglWindow = (WindowRenderTarget_OpenGL*) pWindowRenderTarget;
    ANativeWindow* window = (ANativeWindow*)pWindowRenderTarget->GetWindowHandle();
    ANativeWindow_setBuffersGeometry(window, 0, 0, mpI->format);
    int32_t format = ANativeWindow_getFormat(window);
	// TODO: Verify pixel format!

	/*
	mini3d_assert(ANativeWindow_getFormat(window) == WINDOW_FORMAT_RGBA_8888,
		"Platform specific: Android window must have format WINDOW_FORMAT_RGBA_8888. "
		"Before creating a window render target, set the native window format with: "
		"void ANativeActivity_setWindowFormat(ANativeActivity* activity, int32_t format) ");
*/
    
	openglWindow->m_pWindowSurface = eglCreateWindowSurface(mpI->display, mpI->config, window, NULL);
}

void Platform_android::UnPrepareWindow(IWindowRenderTarget* pWindowRenderTarget)
{
	WindowRenderTarget_OpenGL* openglWindow = (WindowRenderTarget_OpenGL*) pWindowRenderTarget;
    eglDestroySurface(mpI->display, openglWindow->m_pWindowSurface);
}

void Platform_android::GetWindowContentSize(IWindowRenderTarget* pWindowRenderTarget, unsigned int &width, unsigned int &height) const
{
	WindowRenderTarget_OpenGL* openglWindow = (WindowRenderTarget_OpenGL*) pWindowRenderTarget;

    EGLint w, h;

    eglQuerySurface(mpI->display, openglWindow->m_pWindowSurface, EGL_WIDTH, &w);
    eglQuerySurface(mpI->display, openglWindow->m_pWindowSurface, EGL_HEIGHT, &h);

    width = w;
    height = h;
}

void Platform_android::SwapWindowBuffers(IWindowRenderTarget* pWindowRenderTarget)
{ 
	WindowRenderTarget_OpenGL* openglWindow = (WindowRenderTarget_OpenGL*) pWindowRenderTarget;
	ANativeWindow* window = (ANativeWindow*)pWindowRenderTarget->GetWindowHandle();
	eglSwapBuffers(mpI->display, openglWindow->m_pWindowSurface);
}

void Platform_android::SetRenderTarget(IRenderTarget* pRenderTarget)
{
    if (pRenderTarget == 0)
    {
        pOgl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
        mini3d_assert(eglMakeCurrent(mpI->display, mpI->surface, mpI->surface, mpI->context) == EGL_TRUE, "Failed to set default render target! (glXMakeCurrent)");
        return;
    }

	if(pRenderTarget->GetType() == IRenderTargetTexture::TYPE)
    {
		RenderTargetTexture_OpenGL* pRenderTargetTexture = (RenderTargetTexture_OpenGL*)pRenderTarget;
		pOgl->glBindFramebuffer(GL_FRAMEBUFFER, pRenderTargetTexture->m_pRenderTarget);
        pOgl->glViewport(0,0,pRenderTargetTexture->GetWidth(), pRenderTargetTexture->GetHeight());
        return;
    }

	if(pRenderTarget->GetType() == IWindowRenderTarget::TYPE)
	{
		WindowRenderTarget_OpenGL* pWindowRenderTarget = (WindowRenderTarget_OpenGL*)pRenderTarget;

		pOgl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
		mini3d_assert(eglMakeCurrent(mpI->display, pWindowRenderTarget->m_pWindowSurface, pWindowRenderTarget->m_pWindowSurface, mpI->context) == EGL_TRUE, "Failed to set window render target! (glXMakeCurrent)");

		pWindowRenderTarget->UpdateSize();
        pOgl->glViewport(0,0, pWindowRenderTarget->GetWidth(), pWindowRenderTarget->GetHeight());
        return;
	}
}

#endif
