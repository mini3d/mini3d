
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_IWINDOWRENDERTARGET_H
#define MINI3D_IWINDOWRENDERTARGET_H

// Set the data type for the MINI3D_WINDOW macro
#include "irendertarget.hpp"

namespace mini3d {
namespace graphics {

		
struct IWindowRenderTarget : IRenderTarget
{

	enum ScreenState { SCREEN_STATE_WINDOWED = 0, SCREEN_STATE_FULLSCREEN = 1 };


#define IWINDOWRENDERTARGET_INTERFACE(PURE_VIRTUAL)\
\
	virtual void SetWindowRenderTarget(MINI3D_WINDOW windowHandle, bool depthTestEnabled) PURE_VIRTUAL;\
\
	virtual unsigned int GetWidth() const PURE_VIRTUAL;\
	virtual unsigned int GetHeight() const PURE_VIRTUAL;\
\
	virtual Viewport GetViewport() const PURE_VIRTUAL;\
	virtual void SetViewport(Viewport viewport) PURE_VIRTUAL;\
\
	virtual bool GetDepthTestEnabled() const PURE_VIRTUAL;\
	virtual void SetDepthTestEnabled(bool depthTestEnabled) PURE_VIRTUAL;\
\
	virtual MINI3D_WINDOW GetWindowHandle() const PURE_VIRTUAL;\
	virtual void Display() PURE_VIRTUAL;\


public:
	IWINDOWRENDERTARGET_INTERFACE(=0);
	virtual ~IWindowRenderTarget() {};

};
}
}

#endif
