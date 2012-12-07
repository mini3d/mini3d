
// Copyright(c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_GRAPHICS_IPLATFORM_H
#define MINI3D_GRAPHICS_IPLATFORM_H

typedef void* MINI3D_WINDOW;

namespace mini3d {
namespace graphics {

struct IWindowRenderTarget;

///////// PLATFORM ////////////////////////////////////////////////////////////

struct IPlatform
{
#define IPLATFORM_INTERFACE(PURE_VIRTUAL)\
\
    virtual void GetWindowContentSize(MINI3D_WINDOW window, unsigned int &width, unsigned int &height) const PURE_VIRTUAL;\
\
    virtual void SetRenderWindow(IWindowRenderTarget* pWindowRenderTarget) PURE_VIRTUAL;\
    virtual void SwapWindowBuffers(IWindowRenderTarget* pWindowRenderTarget) PURE_VIRTUAL;\

public:
    IPLATFORM_INTERFACE(=0)
    virtual ~IPlatform() {};
};

}
}

// Include platform specific implementations
#include "win32/platform_win32.hpp"
#include "linux/platform_linux.hpp"
#include "osx/platform_osx.hpp"

#endif // MINI3D_GRAPHICS_IPLATFORM_H