
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef _WIN32

#ifndef MINI3D_GRAPHICS_PLATFORM_WIN32_H
#define MINI3D_GRAPHICS_PLATFORM_WIN32_H

#include "../iplatform.hpp"

#ifndef _WINGDI_
struct tagPIXELFORMATDESCRIPTOR;
typedef tagPIXELFORMATDESCRIPTOR PIXELFORMATDESCRIPTOR;
#endif

#ifndef _WINDEF_
struct HGLRC__; typedef HGLRC__ *HGLRC; // "Forward declare" windows HWND
struct HWND__; typedef HWND__ *HWND; // "Forward declare" windows HWND
struct HDC__; typedef HDC__ *HDC; // "Forward declare" windows HWND
#endif


namespace mini3d {
namespace graphics {

typedef class Platform_win32 : public IPlatform
{
public:
    IPLATFORM_INTERFACE(;)

    Platform_win32();
    ~Platform_win32();

private:

	// Default window and render context
	HWND mInternalWindow;
	HDC mDeviceContext;
	HGLRC mRenderContext;
	
	int mCurrentFormat; // Stores the index of the currently used pixel format

} Platform;

}
}

#endif // MINI3D_GRAPHICS_PLATFORM_WIN32_H
#endif // WIN32