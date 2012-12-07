
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef _WIN32

#include "platform_win32.hpp"
#include "../../../windowrendertarget.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdlib>
#include <cstdio>

void mini3d_assert(bool expression, const char* text, ...);
using namespace mini3d::graphics;

namespace {
    PIXELFORMATDESCRIPTOR defaultPixelFormatDescriptor = { sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32, 0,0,0,0,0,0,0,0,0,0,0,0,0, 24, 0, 0, PFD_MAIN_PLANE };
}

///////// PLATFORM WIN32 //////////////////////////////////////////////////////

Platform_win32::Platform_win32() 
{ 
	mInternalWindow = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", L"Mini3DHiddenWindow", 0, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, HWND_MESSAGE, 0, GetModuleHandle(NULL), 0);
    mDeviceContext = GetWindowDC(mInternalWindow);

    mCurrentFormat = ChoosePixelFormat(mDeviceContext, &defaultPixelFormatDescriptor);
    mini3d_assert(mCurrentFormat != 0, "Failed to create default opengl context. CreatePixelFormat failed: (%u)", GetLastError());

    BOOL result = SetPixelFormat(mDeviceContext, mCurrentFormat, &defaultPixelFormatDescriptor);
	mini3d_assert(result != 0, "Failed to create default opengl context. SetPixelFormat failed: (%u)", GetLastError());
	
    mRenderContext = wglCreateContext(mDeviceContext);
	wglMakeCurrent(mDeviceContext, mRenderContext);
}

Platform_win32::~Platform_win32()                                               
{  	
    wglMakeCurrent(0, 0);
    wglDeleteContext(mRenderContext);
    wglDeleteContext(mRenderContext);

    ReleaseDC(mInternalWindow, mDeviceContext);
	DestroyWindow(mInternalWindow);
}

void Platform_win32::GetWindowContentSize(const MINI3D_WINDOW window, unsigned int &width, unsigned int &height) const
{
	RECT rect;
	GetClientRect((HWND)window, &rect);

	width = (rect.right - rect.left);
	height = (rect.bottom - rect.top);

    // get the width and height (must be bigger than 0)
    if (width == 0) width = 1;
    if (height == 0) height = 1;
}

void Platform_win32::SetRenderWindow(IWindowRenderTarget* pWindowRenderTarget)
{
	// TODO: Add comment about render contexts and the connection to the operating system.
	if (pWindowRenderTarget == 0)
	{
		wglMakeCurrent(mDeviceContext, mRenderContext);
	}
	else
	{
        HWND hWnd = (HWND)pWindowRenderTarget->GetWindowHandle();
        HDC hDC = GetWindowDC(hWnd);
        wglMakeCurrent(hDC, mRenderContext);
        ReleaseDC(hWnd, hDC);
	}
}

void Platform_win32::SwapWindowBuffers(IWindowRenderTarget* pWindowRenderTarget)     
{ 
    HWND hWnd = (HWND)pWindowRenderTarget->GetWindowHandle();
    HDC hDC = GetWindowDC(hWnd); 
    SwapBuffers(hDC);
    ReleaseDC(hWnd, hDC);
}


#endif