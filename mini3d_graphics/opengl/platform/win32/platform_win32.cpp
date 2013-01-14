
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef _WIN32

#include "../../../graphicsservice.hpp"
#ifdef MINI3D_GRAPHICSSERVICE_OPENGL

#include "../iplatform.hpp"
#include "../openglwrapper.hpp"

void mini3d_assert(bool expression, const char* text, ...);
using namespace mini3d::graphics;

PIXELFORMATDESCRIPTOR defaultPixelFormatDescriptor = { sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32, 0,0,0,0,0,0,0,0,0,0,0,0,0, 24, 0, 0, PFD_MAIN_PLANE };


///////// PLATFORM WIN32 //////////////////////////////////////////////////////

namespace mini3d {
namespace graphics {

// Convenient wrapper for a win32 device handle that release the handle when it goes out of scope
struct SafeHDC { HWND hWnd; HDC hDC; SafeHDC(HWND hWnd) : hWnd(hWnd) { hDC = GetWindowDC(hWnd); } ~SafeHDC() { ReleaseDC(hWnd, hDC); } operator HDC() { return hDC; }};

class Platform_win32 : public IPlatform
{
public:

    void UnPrepareWindow(void* nativeWindow, void* nativeSurface)   { }
    unsigned int GetNativeSurfaceWidth(void* nativeSurface) const   { RECT r; GetClientRect((HWND)nativeSurface, &r); return r.right - r.left; }
    unsigned int GetNativeSurfaceHeight(void* nativeSurface) const  { RECT r; GetClientRect((HWND)nativeSurface, &r); return r.bottom - r.top; }
    void SwapWindowBuffers(void* nativeSurface) { SafeHDC hDC((HWND)nativeSurface); SwapBuffers(hDC); }
    
    Platform_win32() 
    { 
	    mInternalWindow = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", L"Mini3DHiddenWindow", 0, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, HWND_MESSAGE, 0, GetModuleHandle(NULL), 0);
        SafeHDC hDC(mInternalWindow);

        int pixelFormat = ChoosePixelFormat(hDC, &defaultPixelFormatDescriptor);
        mini3d_assert(pixelFormat != 0, "Failed to create default opengl context. CreatePixelFormat failed: (%u)", GetLastError());

        BOOL result = SetPixelFormat(hDC, pixelFormat, &defaultPixelFormatDescriptor);
	    mini3d_assert(result != 0, "Failed to create default opengl context. SetPixelFormat failed: (%u)", GetLastError());
	
        mRenderContext = wglCreateContext(hDC);
	    wglMakeCurrent(hDC, mRenderContext);
    }

    ~Platform_win32()                                               
    {  	
        wglMakeCurrent(0, 0);
        wglDeleteContext(mRenderContext);
        wglDeleteContext(mRenderContext);
	    DestroyWindow(mInternalWindow);
    }

    void* PrepareWindow(void* nativeWindow)
    {
        SafeHDC hDC((HWND)nativeWindow);
	    int format = ChoosePixelFormat(hDC, &defaultPixelFormatDescriptor);
	    SetPixelFormat(hDC, format, &defaultPixelFormatDescriptor);
        
        return nativeWindow;
    }

    void MakeCurrent(void* nativeSurface)
    {
        if (nativeSurface == 0)
        {
            SafeHDC hDC(mInternalWindow);
            wglMakeCurrent(hDC, mRenderContext);
        }
        else
        {
            SafeHDC hDC((HWND)nativeSurface);
            wglMakeCurrent(hDC, mRenderContext);
	    }
    }

    

private: 

    struct Internal;
    Internal* mpI;

	// Default window and render context
	HWND mInternalWindow;
	HGLRC mRenderContext;
};

IPlatform* IPlatform::New() { return new Platform_win32(); }

}
}


#endif
#endif
