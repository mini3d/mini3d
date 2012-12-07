
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_WINDOW_WINDOW_H
#define MINI3D_WINDOW_WINDOW_H

#include "event.hpp"

namespace mini3d {
namespace window {

typedef void* MINI3D_WINDOW;

struct IWindow
{
	enum ScreenState { SCREEN_STATE_WINDOWED, SCREEN_STATE_FULLSCREEN };
	enum WindowType{ WINDOW_TYPE_OPENGL, WINDOW_TYPE_DIRECTX };

#define WINDOW_INTERFACE(virtual, PURE_VIRTUAL) \
	virtual int GetMultisamples() const PURE_VIRTUAL;\
	virtual MINI3D_WINDOW GetWindow() const PURE_VIRTUAL;\
	virtual WindowType GetWindowType() const PURE_VIRTUAL;\
\
	virtual void Show() PURE_VIRTUAL;\
	virtual void Hide() PURE_VIRTUAL;\
\
	virtual const Event* WaitForNextMessage() PURE_VIRTUAL;\
	virtual const Event* GetNextMessage() PURE_VIRTUAL;\
\
	virtual void GetWindowContentSize(unsigned int &width, unsigned int &height) const PURE_VIRTUAL;\
\
	virtual ScreenState GetScreenState() const PURE_VIRTUAL;\
	virtual void SetScreenStateFullscreen() PURE_VIRTUAL;\
	virtual void SetScreenStateWindowed() PURE_VIRTUAL;\

public:
    WINDOW_INTERFACE(virtual, =0)
    virtual ~IWindow() {};
};

}
}

#include "platform/win32/window_win32.hpp"
#include "platform/linux/window_linux.hpp"
#include "platform/osx/window_osx.hpp"

#endif