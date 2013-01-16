
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_ICONSTANTBUFFER_H
#define MINI3D_ICONSTANTBUFFER_H

namespace mini3d {
namespace graphics {

struct IGraphicsService;
struct IShaderProgram;

struct IConstantBuffer
{
    // For OpenGL version 2.1 and OpenGL ES 2.0 the names parameter shall contain an array with the names of the uniforms to be included in the buffer.
    // For OpenGL with uniform buffers names parameter is unused
    // For DirectX 11 names parameter is unused
    // The order of the names determines the order in the buffer. 
    // If you are developing cross platform code, make sure the layouts match so that the same constant buffer data can be used for all targets.
    static IConstantBuffer* New(IGraphicsService* pGraphics, unsigned int sizeInBytes, IShaderProgram* pShader, const char** names, unsigned int nameCount);
    virtual ~IConstantBuffer() {};

    virtual void SetData(const char* pData) = 0;
    virtual IShaderProgram* GetVertexShader() const = 0;
};
}
}

#endif
