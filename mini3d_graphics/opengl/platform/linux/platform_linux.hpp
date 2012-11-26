
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_GRAPHICS_PLATFORM_LINUX_H
#define MINI3D_GRAPHICS_PLATFORM_LINUX_H

#if defined(__linux__) && !defined(ANDROID) && !defined(__APPLE__)

#include "../iplatform.hpp"
#include "../iopenglwrapper.hpp"

namespace mini3d
{

struct Internal;
	
typedef class Platform_linux : public IPlatform
{
public:
    IPLATFORM_INTERFACE(;)

    Platform_linux();
    ~Platform_linux();

private:
	void CreateInternalWindow();
	void Init();

    IOpenGlWrapper* mpOpenGlWrapper;

	float mOpenGLVersion;
	bool mVERSION_3_3;

	Internal* mpInternal;

} Platform;

}

#endif
#endif
