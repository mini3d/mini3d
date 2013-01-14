
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef __ANDROID__

#ifndef MINI3D_WINDOW_WINDOW_ANDROID_H
#define MINI3D_WINDOW_WINDOW_ANDROID_H

#include "../../window.hpp"

namespace mini3d {
namespace window {

typedef class Window_android : public IWindow
{
public:
    WINDOW_INTERFACE(;,;)
    Window_android(const char* title, unsigned int width, unsigned int height, WindowType windowType, unsigned int multisamples = 0);
    ~Window_android();

public:
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
