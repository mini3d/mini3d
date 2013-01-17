
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_IPIXELSHADER_H
#define MINI3D_IPIXELSHADER_H

namespace mini3d {
namespace graphics {

struct IGraphicsService;

struct IPixelShader
{
    static IPixelShader* New(IGraphicsService* pGraphics, const char* pShaderBytes, unsigned int sizeInBytes, bool precompiled = false);
 	virtual ~IPixelShader() {};
};

}
}


#endif
