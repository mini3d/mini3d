
// Copyright(c) <2011-2013> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_GRAPHICS_IPLATFORM_H
#define MINI3D_GRAPHICS_IPLATFORM_H

typedef void* MINI3D_WINDOW;

namespace mini3d {
namespace graphics {

struct IWindowRenderTarget;
struct IRenderTarget;

///////// PLATFORM ////////////////////////////////////////////////////////////

struct IPlatform
{
    static IPlatform* New();

    virtual unsigned int GetNativeSurfaceWidth(void* nativeSurface) const = 0;
    virtual unsigned int GetNativeSurfaceHeight(void* nativeSurface) const = 0;

    virtual void MakeCurrent(void* nativeSurface) = 0;
    virtual void SwapWindowBuffers(void* nativeSurface) = 0;

    virtual void* PrepareWindow(void* nativeWindow) = 0; // Returns opaque pointer to nativeSurface object
    virtual void UnPrepareWindow(void* nativeWindow, void* nativeSurface) = 0;
    
public:
    virtual ~IPlatform() {};

};

}
}

#endif // MINI3D_GRAPHICS_IPLATFORM_H
