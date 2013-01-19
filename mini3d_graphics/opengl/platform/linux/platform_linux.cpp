
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#if defined(__linux__) && !defined(ANDROID)

#include "../iplatform.hpp"
#include "../../../windowrendertarget.hpp"

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

void mini3d_assert(bool expression, const char* text, ...);


namespace mini3d {
namespace graphics {
    
class Platform_linux : public IPlatform
{
public:
    unsigned int GetNativeSurfaceWidth(void* nativeWindow) const    { XWindowAttributes attr; XGetWindowAttributes(m_pDisplay, *(Window*)nativeWindow, &attr); return attr.width; }
    unsigned int GetNativeSurfaceHeight(void* nativeWindow) const   { XWindowAttributes attr; XGetWindowAttributes(m_pDisplay, *(Window*)nativeWindow, &attr); return attr.height; }

    virtual void MakeCurrent(void* nativeSurface)                   { mini3d_assert(glXMakeCurrent(m_pDisplay, *(Window*)nativeSurface, m_rC), "Failed to set window render target! (glXMakeCurrent)"); }
    virtual void SwapWindowBuffers(void* nativeSurface)             { glXSwapBuffers(m_pDisplay, *(Window*)nativeSurface); }

    ~Platform_linux()                                               { glXDestroyContext(m_pDisplay, m_rC); XCloseDisplay(m_pDisplay); }

    void* PrepareWindow(void* nativeWindow)                         { return nativeWindow; } // TODO: Make sure the window is compatible with the context visual
    void UnPrepareWindow(void* nativeWindow, void* nativeSurface)   { }

    Platform_linux() 
    {
        m_pDisplay = XOpenDisplay(NULL);
        m_window = DefaultRootWindow(m_pDisplay);

        static const GLint attr[] = { GLX_RGBA, GLX_RED_SIZE,8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

        XVisualInfo* vi = glXChooseVisual(m_pDisplay, 0, (int*)&attr);
        m_rC = glXCreateContext(m_pDisplay, vi, NULL, GL_TRUE);
    
        glXMakeCurrent(m_pDisplay, m_window, m_rC);    
        XFree(vi);
    }


private:
    void CreateInternalWindow();
    void CreateDeviceContext();
    void DestroyDeviceContext();
    void Init();

    Display* m_pDisplay;
    Window m_window;
    GLXContext m_rC;    

};

IPlatform* IPlatform::New() { return new Platform_linux(); }

}
}


#endif
