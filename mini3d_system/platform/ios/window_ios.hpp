
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifdef __APPLE__
#import "TargetConditionals.h"
#endif

#if TARGET_OS_IPHONE


#ifndef MINI3D_WINDOW_WINDOW_IOS_H
#define MINI3D_WINDOW_WINDOW_IOS_H

#include "../../window.hpp"


namespace mini3d {
namespace window {

typedef class Window_ios : public IWindow
{
public:
    WINDOW_INTERFACE(;,;)
    Window_ios(const char* title, unsigned int width, unsigned int height, WindowType windowType, unsigned int multisamples = 0);
    ~Window_ios();

    static bool processEvents();

private:

    struct Internal;
    Internal* mpI;
    
	ScreenState mScreenState;
	unsigned int mMultisamples;
	WindowType mWindowType;

} Window;

}
}

#endif
#endif