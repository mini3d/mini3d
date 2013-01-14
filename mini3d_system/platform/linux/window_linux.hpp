
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_WINDOW_WINDOW_LINUX_H
#define MINI3D_WINDOW_WINDOW_LINUX_H

#if defined(__linux__) && !defined(ANDROID)

#include "../../window.hpp"
#include "../common/eventqueue.h"

namespace mini3d {
namespace window {

typedef class Window_linux : public IWindow
{
public:
    WINDOW_INTERFACE(;,;)
    Window_linux(const char* title, unsigned int width, unsigned int height, WindowType windowType, unsigned int multisamples = 0);
    ~Window_linux();

public:
	void CreateLinuxWindow(const char* title, unsigned int width, unsigned int height);
	
	ScreenState mScreenState;
	unsigned int mMultisamples;
	WindowType mWindowType;
	
	struct Internal;
    Internal* mpI;

} Window;

}
}

#endif
#endif