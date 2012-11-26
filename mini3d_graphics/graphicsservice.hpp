
#ifndef MINI3D_IGRAPHICSSERVICE_H
#define MINI3D_IGRAPHICSSERVICE_H

// Decides what implementation to use for graphcis
//#define MINI3D_GRAPHICSSERVICE_DIRECTX_9
#define MINI3D_GRAPHICSSERVICE_OPENGL

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
#include "windowrendertarget.hpp"
#include "shaderprogram.hpp"


namespace mini3d {

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

#define IGRAPHICSSERVICE_INTERFACE(PURE_VIRTUAL)\
\
	virtual IBitmapTexture* CreateBitmapTexture(const void* pBitmap, unsigned int width, unsigned int height, IBitmapTexture::Format format, ITexture::SamplerSettings samplerSettings, ITexture::MipMapMode mipMapMode = ITexture::MIPMAP_BOX_FILTER) PURE_VIRTUAL;\
	virtual IRenderTargetTexture* CreateRenderTargetTexture(unsigned int width, unsigned int height, IRenderTargetTexture::Format format, ITexture::SamplerSettings samplerSettings, bool depthTestEnabled, ITexture::MipMapMode mipMapMode = ITexture::MIPMAP_NONE) PURE_VIRTUAL;\
	virtual IVertexBuffer* CreateVertexBuffer(const void* pVertices, unsigned int count, unsigned int vertexSizeInBytes, IVertexBuffer::StreamMode streamMode = IVertexBuffer::STREAM_PER_VERTEX) PURE_VIRTUAL;\
	virtual IIndexBuffer* CreateIndexBuffer(const void* pIndices, unsigned int count, IIndexBuffer::DataType dataType = IIndexBuffer::INT_32) PURE_VIRTUAL;\
	virtual IPixelShader* CreatePixelShader(const char* shaderBytes, unsigned int sizeInBytes) PURE_VIRTUAL;\
	virtual IVertexShader* CreateVertexShader(const char* shaderBytes, unsigned int sizeInBytes) PURE_VIRTUAL;\
	virtual IShaderProgram* CreateShaderProgram(IVertexShader* pVertexShader, IPixelShader* pPixelShader) PURE_VIRTUAL;\
	virtual IWindowRenderTarget* CreateWindowRenderTarget(MINI3D_WINDOW window, bool depthTestEnabled) PURE_VIRTUAL;\
\
	virtual IShaderProgram* GetShaderProgram() const PURE_VIRTUAL;\
	virtual void SetShaderProgram(IShaderProgram* pShaderProgram) PURE_VIRTUAL;\
	virtual ITexture* GetTexture(unsigned int index) const PURE_VIRTUAL;\
	virtual void SetTexture(ITexture* pTexture, const char* name) PURE_VIRTUAL;\
	virtual IRenderTarget* GetRenderTarget() const PURE_VIRTUAL;\
	virtual void SetRenderTarget(IRenderTarget* pRenderTarget) PURE_VIRTUAL;\
	virtual IIndexBuffer* GetIndexBuffer() const PURE_VIRTUAL;\
	virtual void SetIndexBuffer(IIndexBuffer* indexBuffer) PURE_VIRTUAL;\
	virtual IVertexBuffer* GetVertexBuffer(unsigned int streamIndex) const PURE_VIRTUAL;\
	virtual void SetVertexBuffer(IVertexBuffer* vertexBuffer, unsigned int streamIndex = 0) PURE_VIRTUAL;\
\
	virtual void SetShaderParameterFloat(const char* name, const float* pData, unsigned int count) PURE_VIRTUAL;\
	virtual void SetShaderParameterInt(const char* name, const int* pData, unsigned int count) PURE_VIRTUAL;\
	virtual void SetShaderParameterMatrix4x4(const char* name, const float* pData) PURE_VIRTUAL;\
\
	virtual void SetCullMode(CullMode cullMode) PURE_VIRTUAL;\
\
	virtual void Clear(float depth) PURE_VIRTUAL;\
	virtual void Clear(float r, float g, float b, float a) PURE_VIRTUAL;\
	virtual void Clear(float r, float g, float b, float a, float depth) PURE_VIRTUAL;\
\
	virtual void Draw() PURE_VIRTUAL;\
	virtual void DrawIndices(unsigned int startIndex, unsigned int numIndices) PURE_VIRTUAL;\
\
	virtual const ICompatibility* GetCompatibility() const PURE_VIRTUAL;\


public:

	IGRAPHICSSERVICE_INTERFACE(=0)
	virtual ~IGraphicsService() {};

};
}

// Include platform specific inmplementations
#include "direct3d9/direct3d9.hpp"
#include "opengl/opengl.hpp"

#endif
