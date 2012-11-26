
// Copyright(c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_GRAPHICS_IPLATFORM_H
#define MINI3D_GRAPHICS_IPLATFORM_H

#include "iopenglwrapper.hpp"

typedef void* MINI3D_WINDOW;

namespace mini3d {

struct IWindowRenderTarget;

///////// PLATFORM ////////////////////////////////////////////////////////////

// NOTE: SetInternalWindowPixelFormat is a workaround for platforms (windows specifically) that need all windows for a device context to be of the same pixel format.
// It will return true if the old internal pixel format was incompatible with the new one (requiring the device context to be recreated).
// Platforms without limitations on pixel formats will simply always return false.

struct IPlatform
{
#define IPLATFORM_INTERFACE(PURE_VIRTUAL)\
\
    virtual IOpenGlWrapper* GetOpenGlWrapper() PURE_VIRTUAL;\
    virtual void CreateDeviceContext() PURE_VIRTUAL;\
    virtual void DestroyDeviceContext() PURE_VIRTUAL;\
    virtual void RecreateDeviceContext() PURE_VIRTUAL;\
\
    virtual void SetMini3dWindowToWindowRenderTargetAssociation(IWindowRenderTarget* pWindowRenderTarget, MINI3D_WINDOW window) PURE_VIRTUAL;\
    virtual void FreeMini3dWindowToWindowRenderTargetAssociation(MINI3D_WINDOW window) PURE_VIRTUAL;\
    virtual void GetWindowContentSize(MINI3D_WINDOW window, unsigned int &width, unsigned int &height) const PURE_VIRTUAL;\
\
    virtual void SetRenderWindow(MINI3D_WINDOW window) PURE_VIRTUAL;\
    virtual void SetDefaultRenderWindow() PURE_VIRTUAL;\
    virtual void SwapWindowBuffers(MINI3D_WINDOW window) PURE_VIRTUAL;\
\
    virtual bool SetInternalWindowPixelFormat(MINI3D_WINDOW window) PURE_VIRTUAL;\
\
    virtual float GetOpenGLVersion() PURE_VIRTUAL;\
    virtual bool VERSION_3_3() PURE_VIRTUAL;\


public:
    IPLATFORM_INTERFACE(=0)
    virtual ~IPlatform() {};
};

}

// Include platform specific implementations
#include "win32/platform_win32.hpp"
#include "linux/platform_linux.hpp"

#endif // MINI3D_GRAPHICS_IPLATFORM_H