
// Copyright (c) <2011-2013> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef __ANDROID__

#include "../iplatform.hpp"

#include <android/native_window.h>
#include <EGL/egl.h>
#include <unistd.h>

// #include <GLES2/gl2.h> //TODO: Can be removed?

void mini3d_assert(bool expression, const char* text, ...);

namespace mini3d {
namespace graphics {

/*
EGLContext CreateMainContext()
{
	sleep(4);

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);
    int err = eglGetError();
    eglBindAPI(EGL_OPENGL_ES_API);
    int err2 = eglGetError();

    EGLint numConfigs;
    EGLConfig config;
	static const EGLint attribs[] =    { EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PIXMAP_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 16, EGL_NONE };
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    int err3 = eglGetError();

    EGLint format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    int err4 = eglGetError();

    // TODO: found valid config?

    static const EGLint pbufferAttribs[] =   { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_TEXTURE_TARGET, EGL_NO_TEXTURE, EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE, EGL_NONE };
    EGLSurface surface = eglCreatePbufferSurface(display, config, pbufferAttribs);
    int err5 = eglGetError();

    EGLint contextAttrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    EGLContext context = eglCreateContext(display, config, NULL, contextAttrs);
    int err6 = eglGetError();

	mini3d_assert(eglMakeCurrent(display, surface, surface, context) == EGL_TRUE, "Failed to create parent opengl context! (eglMakeCurrent)");
    int err7 = eglGetError();
	return context;
}

EGLContext m_shareContext = CreateMainContext();
*/
///////// PLATFORM ////////////////////////////////////////////////////////////

class Platform_android : public IPlatform
{
public:

    unsigned int GetNativeSurfaceWidth(void* nativeSurface) const   { EGLint width; eglQuerySurface(m_display, nativeSurface, EGL_WIDTH, &width); return width; };
    unsigned int GetNativeSurfaceHeight(void* nativeSurface) const  { EGLint height; eglQuerySurface(m_display, nativeSurface, EGL_HEIGHT, &height); return height; };
    void SwapWindowBuffers(void* nativeSurface)                     { eglSwapBuffers(m_display, nativeSurface); }

Platform_android()
{
    m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(m_display, 0, 0);
    eglBindAPI(EGL_OPENGL_ES_API);

    EGLint numConfigs;
	static const EGLint attribs[] =    { EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PIXMAP_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 16, EGL_NONE };
    eglChooseConfig(m_display, attribs, &m_config, 1, &numConfigs);
    eglGetConfigAttrib(m_display, m_config, EGL_NATIVE_VISUAL_ID, &m_format);
    // TODO: found valid config?

    static const EGLint pbufferAttribs[] =   { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_TEXTURE_TARGET, EGL_NO_TEXTURE, EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE, EGL_NONE };
    m_surface = eglCreatePbufferSurface(m_display, m_config, pbufferAttribs);

    EGLint contextAttrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    m_context = eglCreateContext(m_display, m_config, NULL, contextAttrs);

	mini3d_assert(eglMakeCurrent(m_display, m_surface, m_surface, m_context) == EGL_TRUE, "Failed to set default render target! (eglMakeCurrent)");
}

~Platform_android()
{ 
	eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroySurface(m_display, m_surface);
	eglDestroyContext(m_display, m_context);
	eglTerminate(m_display);
}

void* PrepareWindow(void* nativeWindow)
{
    ANativeWindow_setBuffersGeometry((ANativeWindow*)nativeWindow, 0, 0, m_format);
	// TODO: Verify pixel format!

	/*
    int32_t format = ANativeWindow_getFormat(nativeWindow);
    mini3d_assert(ANativeWindow_getFormat(nativeWindow) == WINDOW_FORMAT_RGBA_8888,
		"Platform specific: Android window must have format WINDOW_FORMAT_RGBA_8888. "
		"Before creating a window render target, set the native window format with: "
		"void ANativeActivity_setWindowFormat(ANativeActivity* activity, int32_t format) ");
*/
    
	return eglCreateWindowSurface(m_display, m_config, (ANativeWindow*)nativeWindow, NULL);
}

void UnPrepareWindow(void* nativeWindow, void* nativeSurface)
{
    eglDestroySurface(m_display, nativeSurface);
}

void MakeCurrent(void* nativeSurface)
{
    if (nativeSurface == 0)
        mini3d_assert(eglMakeCurrent(m_display, m_surface, m_surface, m_context) == EGL_TRUE, "Failed to set default render target! (glXMakeCurrent)");
    else
        mini3d_assert(eglMakeCurrent(m_display, nativeSurface, nativeSurface, m_context) == EGL_TRUE, "Failed to set default render target! (glXMakeCurrent)");
}

private:

    EGLDisplay m_display;
    EGLConfig m_config;
    EGLint m_format;
    EGLSurface m_surface;
    EGLContext m_context;
};

IPlatform* IPlatform::New() { return new Platform_android(); }

}
}


#endif
