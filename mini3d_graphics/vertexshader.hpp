
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_IVERTEXSHADER_H
#define MINI3D_IVERTEXSHADER_H

namespace mini3d {
namespace graphics {

struct IGraphicsService;

struct IVertexShader
{
    enum DataType { R32_FLOAT, R32G32_FLOAT, R32G32B32_FLOAT, R32G32B32A32_FLOAT };
    enum StreamRate { PER_VERTEX = 0, PER_INSTANCE = 1 };

    // HLSL shaders bind vertex buffer input on semantic and semantic index.
    // GLSL shaders bind vertex buffer input on variable name.
    struct InputAttribute { char nameGLSL[32]; char semanticHLSL[32]; unsigned int semanticIndexHLSL; unsigned int slot; unsigned int offset; DataType type; StreamRate rate; };

    struct ShaderParameter { char nameGLSL[32]; DataType type; };

    static IVertexShader* New(IGraphicsService* pGraphics, const char* pShaderBytes, unsigned int sizeInBytes, InputAttribute* pAttributes, unsigned int attributeCount);
	~IVertexShader() {};

    unsigned int GetInputAttributeCount() const;
    void GetInputAttributes(InputAttribute* pAttributes) const;
};



}
}

#endif
