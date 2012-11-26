
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef _WIN32

#include "platform_win32.hpp"
#include "../../../windowrendertarget.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include "../../glext.h"
#include "wglext.h"
#include <cstdlib>
#include <cstdio>

void mini3d_assert(bool expression, const char* text, ...);
using namespace mini3d;


///////// PLATFORM SPECIFIC OPENGL WRAPPER ////////////////////////////////////

#include "../opengl/openglwrapper_opengl.inl"


///////// WINDOW CALLBACKS ////////////////////////////////////////////////////

namespace {

	LRESULT CALLBACK InternalWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch(msg)
		{
			case WM_CLOSE:
				DestroyWindow(hwnd);
			break;
			case WM_DESTROY:
				PostQuitMessage(0);
			break;
			default:
				return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		return 0;
	}

	// A map matching window handles to the correct window render target object
	struct WindowInfo { IWindowRenderTarget* windowRenderTarget; WNDPROC pOrigProc; };
	
	LRESULT CALLBACK HookWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		WindowInfo* windowInfo = (WindowInfo*)GetProp(hWnd, L".mini3d_windowinfo");

		// if we did not find a matching window proc, call default window proc and return result (window is not a mini3d window)
		if (windowInfo == 0) return DefWindowProc(hWnd, msg, wParam, lParam);

		// if window has been resized update the render target size
        if (msg == WM_SIZE)
        {
			//TODO: This is checked every frame at current. Should be fixed properly for linux before this is uncommented
			//windowRenderTarget->UpdateSize();
        }

		return CallWindowProc(windowInfo->pOrigProc, hWnd, msg, wParam, lParam);
	}

}


///////// PLATFORM WIN32 //////////////////////////////////////////////////////

IOpenGlWrapper*     Platform_win32::GetOpenGlWrapper()                          { return mpOpenGlWrapper; }
float               Platform_win32::GetOpenGLVersion()                          { return mOpenGLVersion; }
bool                Platform_win32::VERSION_3_3()                               { return mVERSION_3_3; }
void                Platform_win32::SetDefaultRenderWindow()                    { wglMakeCurrent(mDeviceContext, mRenderContext); }
void                Platform_win32::SwapWindowBuffers(MINI3D_WINDOW window)     { HDC hDC = GetWindowDC((HWND)window); SwapBuffers(hDC); }
                    Platform_win32::~Platform_win32()                           { DestroyDeviceContext(); delete mpOpenGlWrapper; }

Platform_win32::Platform_win32() 
{ 
    mCurrentFormat = -1; 
    CreateInternalWindow(); 
    CreateDeviceContext(); 
    mpOpenGlWrapper = new OpenGlWrapper_opengl(); 

	mOpenGLVersion = (float)atof((const char*)::glGetString(GL_VERSION));
	mVERSION_3_3 = mOpenGLVersion >= 3.3;
}


void Platform_win32::FreeMini3dWindowToWindowRenderTargetAssociation(MINI3D_WINDOW window)
{
	// TODO: Create Atom for the ".mini3d_windowinfo" string
	WindowInfo* windowInfo = (WindowInfo*)GetProp((HWND)window, L".mini3d_windowinfo");
	RemoveProp((HWND)window, L".mini3d_windowinfo");

	(WNDPROC)SetWindowLongPtr((HWND)window, GWLP_WNDPROC, (LONG_PTR)windowInfo->pOrigProc);
	delete windowInfo;
}

void Platform_win32::SetMini3dWindowToWindowRenderTargetAssociation(IWindowRenderTarget* pWindowRenderTarget, MINI3D_WINDOW window)
{
	WindowInfo* windowInfo = (WindowInfo*)GetProp((HWND)window, L".mini3d_windowinfo");
	if (windowInfo)	{ /* TODO: ERROR! ALREADY REGISTERED TO A DIFFERENT pWindowRenderTarget */ }
		
	// overwrite the window process for the window (our override window process will call the original window process saved in mpOrigProc)
	WNDPROC pOrigProc = (WNDPROC)SetWindowLongPtr((HWND)window, GWLP_WNDPROC, (LONG_PTR)HookWndProc);
	
	// add new window
	windowInfo = new WindowInfo(); 
	windowInfo->windowRenderTarget = pWindowRenderTarget; 
	windowInfo->pOrigProc = pOrigProc;
	
	SetProp((HWND)window, L".mini3d_windowinfo", windowInfo);
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

void Platform_win32::CreateDeviceContext()
{
	// get the pixel format for the device context
	// query desktop video settings
	DEVMODE desktop_mode;
	EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &desktop_mode);
	
	DEVMODE devMode; devMode.dmSize = sizeof(devMode); // read the monitor information from win32
	EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &devMode); // get monitor settings from Windows

	int colorBits, depthBits;
	depthBits = colorBits = (devMode.dmBitsPerPel == 16) ? 16 : 
							(devMode.dmBitsPerPel == 32) ? 24 : 16;

	// TODO: Error handling for this setup! (What if the depth or color format is unsupported!)
    PIXELFORMATDESCRIPTOR pfd = {0};
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; // Make the pixel format compatible with opengl
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = colorBits;
	pfd.cDepthBits = depthBits;
	pfd.iLayerType = PFD_MAIN_PLANE;

	// If current format is not set, set it to a default pixel format (happens when we run for the first time)
    // Get the pixel format index for the new pixel format
	if (mCurrentFormat == -1)
		mCurrentFormat = ChoosePixelFormat(mDeviceContext, &pfd);

	// Set the pixel format for the device context
	BOOL result = SetPixelFormat(mDeviceContext, mCurrentFormat, &pfd);
	// TODO: Handle Error

	// Create a default reder context and set it as current so we can start loading graphics data to the graphics card
	mRenderContext = wglCreateContext(mDeviceContext);
	wglMakeCurrent(mDeviceContext, mRenderContext);
}

void Platform_win32::DestroyDeviceContext()
{
	DeleteDC(mDeviceContext);
	mDeviceContext = 0;

	DestroyWindow(mInternalWindow);
	mInternalWindow = 0;

    wglMakeCurrent(0, 0);
	wglDeleteContext(mRenderContext);
}

void Platform_win32::RecreateDeviceContext()
{
	DestroyDeviceContext();
			
	// Create a new rendering context with the correct pixel format
	HINSTANCE hInstance = GetModuleHandle(NULL);
	UnregisterClass(L"Mini3DInternalWindowClass", hInstance);
	CreateInternalWindow();
	CreateDeviceContext();
}

bool Platform_win32::SetInternalWindowPixelFormat(MINI3D_WINDOW window)
{
	HDC hdc = GetWindowDC((HWND)window);
	int pixelFormat = GetPixelFormat(hdc);

	if (pixelFormat == mCurrentFormat) return false;

	mCurrentFormat = pixelFormat;
	return true;
}

void Platform_win32::SetRenderWindow(MINI3D_WINDOW window)
{
	// TODO: Add comment about render contexts and the connection to the operating system.
	if (window == 0)
	{
		wglMakeCurrent(mDeviceContext, mRenderContext);
	}
	else
	{
		HDC hDC = GetWindowDC((HWND)window);
		mpOpenGlWrapper->glBindFramebuffer(GL_FRAMEBUFFER, 0);
		wglMakeCurrent(hDC, mRenderContext);
	}
}

void Platform_win32::CreateInternalWindow()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	WNDCLASSEX wc = {};
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.lpfnWndProc   = InternalWindowProc;
	wc.hInstance     = hInstance;
	wc.lpszClassName = L"Mini3DInternalWindowClass";
	RegisterClassEx(&wc);
	mInternalWindow = CreateWindowEx(WS_EX_CLIENTEDGE, L"Mini3DInternalWindowClass", L"Mini3DHiddenWindow", 0, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, HWND_MESSAGE, 0, hInstance, 0);
	mDeviceContext = GetDC(mInternalWindow);
}

#endif