
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_IGRAPHICSSERVICE_H
#define MINI3D_IGRAPHICSSERVICE_H

// Decides what implementation to use for graphcis
#define MINI3D_GRAPHICSSERVICE_DIRECT3D_11
//#define MINI3D_GRAPHICSSERVICE_OPENGL

// TODO: When a resource is set. it should be tested if it needs to be reloaded. if an out of memory error occurs then the resource might still be loadable at a later time, without altering any parameters.

typedef void* MINI3D_WINDOW;

#include "compatibility.hpp"
#include "pixelshader.hpp"
#include "vertexshader.hpp"
#include "itexture.hpp"
#include "bitmaptexture.hpp"
#include "irendertarget.hpp"
#include "rendertargettexture.hpp"
#include "vertexbuffer.hpp"
#include "indexbuffer.hpp"
#include "constantbuffer.hpp"
#include "windowrendertarget.hpp"
#include "shaderprogram.hpp"
#include "shaderinputlayout.hpp"

namespace mini3d {
namespace graphics {

struct IRenderTargetTexture;
struct IBitmapTexture;
struct IVertexBuffer;
struct IIndexBuffer;
struct IPixelShader;
struct IVertexShader;
struct IShaderProgram;

struct IGraphicsService
{
	enum CullMode { CULL_CLOCKWIZE = 0, CULL_COUNTERCLOCKWIZE = 1, CULL_NONE = 2 };

    static IGraphicsService* New();
	virtual ~IGraphicsService() {};

	virtual IShaderProgram* GetShaderProgram() const = 0;
	virtual void SetShaderProgram(IShaderProgram* pShaderProgram) = 0;
	
    virtual ITexture* GetTexture(unsigned int idD3D, const char* nameOGL) const = 0;
	virtual void SetTexture(ITexture* pTexture, unsigned int idD3D, const char* nameOGL) = 0;
	
    virtual IRenderTarget* GetRenderTarget() const = 0;
	virtual void SetRenderTarget(IRenderTarget* pRenderTarget) = 0;
	
    virtual IIndexBuffer* GetIndexBuffer() const = 0;
	virtual void SetIndexBuffer(IIndexBuffer* indexBuffer) = 0;

	virtual IVertexBuffer* GetVertexBuffer(unsigned int streamIndex) const = 0;
	virtual void SetVertexBuffer(IVertexBuffer* vertexBuffer, unsigned int streamIndex = 0) = 0;
    
    // Constant buffer name for OpenGL is neede if multiple simultanous buffers are bound even if you are not using
    // OpenGL Uniform Buffer Objects (OpenGL 3.1)
    virtual IConstantBuffer* GetConstantBuffer(unsigned int idD3D, const char* nameOGL) = 0;
    virtual void SetConstantBuffer(IConstantBuffer* pBuffer, unsigned int idD3D, const char* nameOGL) = 0;
    
    virtual IShaderInputLayout* GetShaderInputLayout() = 0;
    virtual void SetShaderInputLayout(IShaderInputLayout* pLayout) = 0;

	virtual void SetCullMode(CullMode cullMode) = 0;

    // Todo: move to render targets
	virtual void Clear(float depth) = 0;
	virtual void Clear(float r, float g, float b, float a) = 0;
	virtual void Clear(float r, float g, float b, float a, float depth) = 0;

	virtual void Draw() = 0;

	virtual const ICompatibility* GetCompatibility() const = 0;

};
}
}

#endif
