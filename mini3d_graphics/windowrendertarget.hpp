
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_IWINDOWRENDERTARGET_H
#define MINI3D_IWINDOWRENDERTARGET_H

#include "irendertarget.hpp"

namespace mini3d {
namespace graphics {

struct IGraphicsService;

struct IWindowRenderTarget : public IRenderTarget
{
    static const char* TYPE;
    virtual const char* GetType() const { return TYPE; }

	enum ScreenState { SCREEN_STATE_WINDOWED = 0, SCREEN_STATE_FULLSCREEN = 1 };

    static IWindowRenderTarget* New(IGraphicsService* pGraphics, void* nativeWindow, bool depthTestEnabled);
    virtual ~IWindowRenderTarget() {};

    virtual void SetWindowRenderTarget(void* nativeWindow, bool depthTestEnabled) = 0;

    virtual void Display() = 0;

	virtual unsigned int GetWidth() const = 0;
	virtual unsigned int GetHeight() const = 0;

	virtual Viewport GetViewport() const = 0;
	virtual void SetViewport(Viewport viewport) = 0;

	virtual bool GetDepthTestEnabled() const = 0;

    virtual void* GetNativeWindow() const = 0;
};

}
}

#endif
