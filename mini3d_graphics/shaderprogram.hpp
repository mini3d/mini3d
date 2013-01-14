
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_ISHADERPROGRAM_H
#define MINI3D_ISHADERPROGRAM_H

namespace mini3d {
namespace graphics {

struct IPixelShader;
struct IVertexShader;
struct IGraphicsService;

struct IShaderProgram
{
    static IShaderProgram* New(IGraphicsService* pGraphics, IVertexShader* pVertexShader, IPixelShader* pPixelShader);
	virtual ~IShaderProgram() {};

    virtual IPixelShader* GetPixelShader() const = 0;
	virtual IVertexShader* GetVertexShader() const = 0;
};
}
}

#endif
