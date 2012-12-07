
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_WINDOW_WINDOW_OSX_H
#define MINI3D_WINDOW_WINDOW_OSX_H

#if defined(__APPLE__)

#include "../../window.hpp"
#include "../common/eventqueue.h"

namespace mini3d {
namespace window {

struct Internal;

typedef class Window_osx : public IWindow
{
public:
    WINDOW_INTERFACE(;,;)
    Window_osx(const char* title, unsigned int width, unsigned int height, WindowType windowType, unsigned int multisamples = 0);
    ~Window_osx();

public:
	void CreateLinuxWindow(const char* title, unsigned int width, unsigned int height);
	
	ScreenState mScreenState;
	unsigned int mMultisamples;
	WindowType mWindowType;
	
    EventQueue mEventQueue;

    Internal* mpI;

} Window;

}
}

#endif
#endif