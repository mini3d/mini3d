
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_WINDOW_WINDOW_H
#define MINI3D_WINDOW_WINDOW_H

#include "event.hpp"


namespace mini3d {
namespace system {

struct IWindow
{
	enum ScreenState { SCREEN_STATE_WINDOWED, SCREEN_STATE_FULLSCREEN };

    static IWindow* New(const wchar_t* title, unsigned int width, unsigned int height);
    virtual ~IWindow() {};

    virtual void Show() = 0;
	virtual void Hide() = 0;

    virtual ScreenState GetScreenState() const = 0;
	virtual void SetScreenStateFullscreen() = 0;
	virtual void SetScreenStateWindowed() = 0;

    virtual void GetWindowContentSize(unsigned int &width, unsigned int &height) const = 0;

    virtual const bool GetEvent(Event &ev) = 0;

	virtual void* GetNativeWindow() const = 0;
};

}
}

#endif
