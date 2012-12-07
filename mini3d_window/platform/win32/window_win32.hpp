
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifdef _WIN32

#ifndef MINI3D_WINDOW_WINDOW_WIN32_H
#define MINI3D_WINDOW_WINDOW_WIN32_H

#include "../../window.hpp"
#include "../common/eventqueue.h"

#ifndef _WINDEF_
struct HGLRC__; typedef HGLRC__ *HGLRC; // "Forward declare" windows HWND
struct HWND__; typedef HWND__ *HWND; // "Forward declare" windows HWND
struct HDC__; typedef HDC__ *HDC; // "Forward declare" windows HWND
#endif

namespace mini3d {
namespace window {

typedef class Window_win32 : public IWindow
{
public:
    WINDOW_INTERFACE(;,;)
    Window_win32(const char* title, unsigned int width, unsigned int height, WindowType windowType, unsigned int multisamples = 0);
    ~Window_win32();

    static bool processEvents();
    EventQueue mEventQueue;

private:

    HGLRC mRenderContext;
	HWND mInternalWindow;
	HGLRC mOldHglrc;
	HDC mOldHdc;
    wchar_t mWindowClassName[32];
	wchar_t mTitle[1024];

	ScreenState mScreenState;
	unsigned int mMultisamples;
	WindowType mWindowType;
	HWND hWindow;

} Window;

}
}

#endif
#endif