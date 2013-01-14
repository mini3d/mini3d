
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef __ANDROID__

#ifndef MINI3D_GRAPHICS_PLATFORM_ANDROID_H
#define MINI3D_GRAPHICS_PLATFORM_ANDROID_H

#include "../iplatform.hpp"
#include "../../../IRenderTarget.hpp"

namespace mini3d {
namespace graphics {

struct OpenGlWrapper;

typedef class Platform_android : public IPlatform
{
public:
    IPLATFORM_INTERFACE(;)

    Platform_android();
    ~Platform_android();

private:
    OpenGlWrapper* pOgl;

	struct Internal;
	Internal* mpI;

} Platform;

}
}

#endif
#endif
