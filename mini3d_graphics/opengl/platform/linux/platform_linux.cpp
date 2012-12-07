
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#if defined(__linux__) && !defined(ANDROID)

#include "platform_linux.hpp"
#include "../../../windowrendertarget.hpp"

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>


void mini3d_assert(bool expression, const char* text, ...);
using namespace mini3d::graphics;

namespace {
	GLint attr[] = { GLX_RGBA, GLX_RED_SIZE,8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
}

struct Platform_linux::Internal
{
	Display* dpy;
	Window window;
	GLXContext mRC;	
};


///////// PLATFORM ////////////////////////////////////////////////////////////

Platform_linux::Platform_linux() 
{
	mpI = new Internal();
	
	mpI->dpy = XOpenDisplay(NULL);
	mpI->window = DefaultRootWindow(mpI->dpy);

	XVisualInfo* vi = glXChooseVisual(mpI->dpy, 0, attr);
	mpI->mRC = glXCreateContext(mpI->dpy, vi, NULL, GL_TRUE);
	
	glXMakeCurrent(mpI->dpy, mpI->window, mpI->mRC);	

	XFree(vi);
}

Platform_linux::~Platform_linux() 
{ 
	glXDestroyContext(mpI->dpy, mpI->mRC);
	XCloseDisplay(mpI->dpy);

	delete mpI;
}


void Platform_linux::GetWindowContentSize(MINI3D_WINDOW window, unsigned int &width, unsigned int &height) const
{
	XWindowAttributes attr;
	XGetWindowAttributes(mpI->dpy, *(Window*)window, &attr);
	
	width = attr.width;
	height = attr.height;
}

void Platform_linux::SwapWindowBuffers(IWindowRenderTarget* pWindowRenderTarget)
{ 
	Window window = *(Window*)pWindowRenderTarget->GetWindowHandle();
	glXSwapBuffers(mpI->dpy, window); 
}


void Platform_linux::SetRenderWindow(IWindowRenderTarget* pWindowRenderTarget)
{
	if (pWindowRenderTarget == 0)
	{
		mini3d_assert(glXMakeCurrent(mpI->dpy, mpI->window, mpI->mRC), "Failed to set window render target! (glXMakeCurrent)");
	}
	else
	{
		Window window = *(Window*)pWindowRenderTarget->GetWindowHandle();
		mini3d_assert(glXMakeCurrent(mpI->dpy, window, mpI->mRC), "Failed to set default render target! (glXMakeCurrent)");
	}
}

#endif