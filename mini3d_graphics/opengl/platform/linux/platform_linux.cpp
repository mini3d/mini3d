
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#if defined(__linux__) && !defined(ANDROID) && !defined(__APPLE__)

#include "platform_linux.hpp"
#include "../../../windowrendertarget.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <cstdlib>
#include <cstdio>

namespace mini3d {
struct Internal
{
	Display* mDisplay;
	XVisualInfo* mVinfo;
	XSetWindowAttributes mSwattr;
	Window mWindow;
	GLXContext mRenderingContext;	
};
}

void mini3d_assert(bool expression, const char* text, ...);
using namespace mini3d;

///////// PLATFORM SPECIFIC OPENGL WRAPPER ////////////////////////////////////

#include "../opengl/openglwrapper_opengl.inl"


///////// PLATFORM ////////////////////////////////////////////////////////////

// NOTE: SetInternalWindowPixelFormat is a workaround for platforms (windows specifically) that need all windows for a device context to be of the same pixel format.
// It will return true if the old internal pixel format was incompatible with the new one (requiring the device context to be recreated).
// Platforms without limitations on pixel formats will simply always return false.

IOpenGlWrapper*     Platform_linux::GetOpenGlWrapper()                                  { return mpOpenGlWrapper; }
void                Platform_linux::DestroyDeviceContext()                              { }
void                Platform_linux::RecreateDeviceContext()                             { }
bool                Platform_linux::SetInternalWindowPixelFormat(MINI3D_WINDOW window)  {	return false; } /* TODO: Are all formats compatible? */

void                Platform_linux::SetDefaultRenderWindow()                            { glXMakeCurrent(mpInternal->mDisplay, mpInternal->mWindow, mpInternal->mRenderingContext); }
void                Platform_linux::SwapWindowBuffers(MINI3D_WINDOW window)             { glXSwapBuffers(mpInternal->mDisplay, *(Window*)window); }
float               Platform_linux::GetOpenGLVersion()                                  { return mOpenGLVersion; }
bool                Platform_linux::VERSION_3_3()                                       { return mVERSION_3_3; }
                    Platform_linux::~Platform_linux()                             		{ DestroyDeviceContext(); delete mpOpenGlWrapper; delete mpInternal;}

Platform_linux::Platform_linux() 
{
	mpInternal = new Internal();
    CreateDeviceContext(); 
    CreateInternalWindow(); 
    mpOpenGlWrapper = new OpenGlWrapper_opengl();
	mOpenGLVersion = (float)atof((const char*)::glGetString(GL_VERSION));
	mVERSION_3_3 = mOpenGLVersion >= 3.3;
}

void Platform_linux::GetWindowContentSize(MINI3D_WINDOW window, unsigned int &width, unsigned int &height) const
{
	XWindowAttributes attr;
	XGetWindowAttributes(mpInternal->mDisplay, *(Window*)window, &attr);
	
	width = attr.width;
	height = attr.height;
}

void Platform_linux::FreeMini3dWindowToWindowRenderTargetAssociation(MINI3D_WINDOW window)
{
}

void Platform_linux::SetMini3dWindowToWindowRenderTargetAssociation(IWindowRenderTarget* pWindowRenderTarget, MINI3D_WINDOW window)
{
}

void Platform_linux::CreateDeviceContext()
{
	int attrList[20];
	int indx=0;

	if (!(mpInternal->mDisplay = XOpenDisplay(0)))
	{
		// todo: throw and error
	}

	attrList[indx] = GLX_USE_GL; indx++;
	attrList[indx] = GLX_DEPTH_SIZE; indx++;
	attrList[indx] = 1; indx++;
	attrList[indx] = GLX_RGBA; indx++;
	attrList[indx] = GLX_RED_SIZE; indx++;
	attrList[indx] = 1; indx++;
	attrList[indx] = GLX_GREEN_SIZE; indx++;
	attrList[indx] = 1; indx++;
	attrList[indx] = GLX_BLUE_SIZE; indx++;
	attrList[indx] = 1; indx++;
	attrList[indx] = None;

	mpInternal->mVinfo = glXChooseVisual(mpInternal->mDisplay, DefaultScreen(mpInternal->mDisplay), attrList);

	if (mpInternal->mVinfo == NULL)
	{
	    // todo: throw error
	}

	mpInternal->mSwattr.colormap=XCreateColormap (mpInternal->mDisplay ,RootWindow (mpInternal->mDisplay, mpInternal->mVinfo->screen), mpInternal->mVinfo->visual, AllocNone);
	mpInternal->mSwattr.background_pixel = BlackPixel (mpInternal->mDisplay, mpInternal->mVinfo->screen);
	mpInternal->mSwattr.border_pixel = BlackPixel (mpInternal->mDisplay, mpInternal->mVinfo->screen);
}

void Platform_linux::SetRenderWindow(MINI3D_WINDOW window)
{
	// TODO: How do we test for null here?
	if (mpInternal->mWindow == 0)
	{
		if(!glXMakeCurrent(mpInternal->mDisplay, None, 0))
		{
			// todo: throw error
		}
	}
	else
	{
		//TODO: Switch to windowed mode

		mpOpenGlWrapper->glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if (!glXMakeCurrent(mpInternal->mDisplay, *(Window*)window, mpInternal->mRenderingContext))
		{
			// todo: throw error
		}
	}
}

void Platform_linux::CreateInternalWindow()
{
	mpInternal->mWindow = XCreateWindow(mpInternal->mDisplay, RootWindow(mpInternal->mDisplay, mpInternal->mVinfo->screen), 30, 30, 64, 64, 0, mpInternal->mVinfo->depth, CopyFromParent, mpInternal->mVinfo->visual,CWBackPixel | CWBorderPixel | CWColormap, &mpInternal->mSwattr);
	mpInternal->mRenderingContext = glXCreateContext(mpInternal->mDisplay, mpInternal->mVinfo, 0, true);

	if (mpInternal->mRenderingContext == 0)
	{
		// todo: throw error
	}

	if (!glXMakeCurrent(mpInternal->mDisplay, mpInternal->mWindow, mpInternal->mRenderingContext))
	{
		// todo: throw error
	}
}




#endif