
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifdef __APPLE__
#import "TargetConditionals.h"
#endif

#if TARGET_OS_IPHONE

#ifndef MINI3D_GRAPHICS_PLATFORM_IOS_H
#define MINI3D_GRAPHICS_PLATFORM_IOS_H

#include "../iplatform.hpp"

namespace mini3d {
namespace graphics {
    
typedef class Platform_ios : public IPlatform
{
public:
    IPLATFORM_INTERFACE(;)

    Platform_ios();
    ~Platform_ios();
    
} Platform;

}
}

#endif
#endif
