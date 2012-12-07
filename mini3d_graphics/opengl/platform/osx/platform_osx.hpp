
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_GRAPHICS_PLATFORM_OSX_H
#define MINI3D_GRAPHICS_PLATFORM_OSX_H

#if __APPLE__

#include "../iplatform.hpp"

namespace mini3d {
namespace graphics {

struct Internal;
	
typedef class Platform_osx : public IPlatform
{
public:
    IPLATFORM_INTERFACE(;)

    Platform_osx();
    ~Platform_osx();

} Platform;

}
}

#endif
#endif
