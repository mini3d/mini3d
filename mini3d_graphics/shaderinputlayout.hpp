
// Copyright (c) <2013> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_ISHADERINPUTLAYOUT_H
#define MINI3D_ISHADERINPUTLAYOUT_H

namespace mini3d {
namespace graphics {

struct IShaderProgram;
struct IGraphicsService;

struct IShaderInputLayout
{

    enum DataType { R32_FLOAT, R32G32_FLOAT, R32G32B32_FLOAT, R32G32B32A32_FLOAT };
    enum StreamRate { PER_VERTEX = 0, PER_INSTANCE = 1 };

    // HLSL shaders bind vertex buffer input on semantic and semantic index.
    // GLSL shaders bind vertex buffer input on variable name.
    struct InputElement { const char* nameGLSL; const char* semanticHLSL; unsigned int semanticIndexHLSL; unsigned int vertexBufferIndex; unsigned int offsetInBytes; DataType type; StreamRate rate; };

    static IShaderInputLayout* New(IGraphicsService* pGraphics, IShaderProgram* pShader, InputElement* pElements, unsigned int count);
	virtual ~IShaderInputLayout() {};

    virtual unsigned int GetInputElementCount() const = 0;
    virtual void GetInputAttributes(InputElement* pElements) const = 0;
};
}
}

#endif
