
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#include "../graphicsservice.hpp"

#ifdef _WIN32
#ifdef MINI3D_GRAPHICSSERVICE_DIRECTX_9

#ifndef MINI3D_GRAPHICS_DIRECT3D9_H
#define MINI3D_GRAPHICS_DIRECT3D9_H

typedef unsigned int uint;


///////// FOREWARD DECLARATIONS ///////////////////////////////////////////////

struct IDirect3DDevice9;
struct IDirect3D9;
struct IDirect3DSurface9;
struct _D3DCAPS9; typedef _D3DCAPS9 D3DCAPS9;
struct IDirect3DIndexBuffer9;
struct IDirect3DPixelShader9;
struct ID3DXConstantTable;
struct IDirect3DVertexShader9;
struct IDirect3DVertexBuffer9;
struct IDirect3DTexture9;
struct IDirect3DSwapChain9;

#ifndef _WINDEF_
struct HWND__; typedef HWND__ *HWND; // "Forward declare" windows HWND
#endif


///////// NAMESPACE MINI3D ////////////////////////////////////////////////////

namespace mini3d {
namespace graphics {

///////// FOREWARD DECLARATIONS ///////////////////////////////////////////////

class PixelShader_Direct3d9;
class VertexShader_Direct3d9;
class ShaderProgram_Direct3d9;
class IndexBuffer_Direct3d9;
class VertexBuffer_Direct3d9;
class GraphicsService_Direct3d9;

struct Size { uint width; uint height; };


///////// CONSTANTS ///////////////////////////////////////////////////////////

const uint MAX_VERTEX_BUFFER_SLOTS = 16;
const uint MAX_TEXTURE_BUFFER_SLOTS = 16;


///////// RESOURCE ///////////////////////////////////////////////////////////

struct Resource
{
	Resource() : prev(0), next(0) {};
	virtual ~Resource() {  }

	virtual void LoadResource() = 0;
	virtual void UnloadResource() = 0;
	bool isDirty;

	// Intrusive linked list
	Resource *next, *prev;
};


///////// LOCKABLER ESOURCE ///////////////////////////////////////////////////

template <class T> struct LockableResource : public Resource
{
	bool m_IsReadOnly;
	bool m_IsLocked;
	T* m_pResource;
	uint m_SizeInBytes;

	LockableResource() : m_pResource(0), m_IsLocked(false), m_SizeInBytes(0), m_IsReadOnly(false) {};
	virtual ~LockableResource() { Unload(); };

	T* Lock(uint& sizeInBytes, bool readOnly = false)		{ m_IsReadOnly = readOnly; sizeInBytes = m_SizeInBytes; return m_pResource; }
	bool Unlock()											{ m_IsLocked = false; return (m_IsReadOnly == false); }

	void CopyIn(const void* pResource, uint sizeInBytes)	{ Unload(); m_pResource = new T[sizeInBytes / sizeof(T)]; memcpy(m_pResource, pResource, sizeInBytes); m_SizeInBytes = sizeInBytes; isDirty = true; LoadResource(); }
	T* CopyOut(uint& sizeInBytes) const						{ sizeInBytes = m_SizeInBytes; T* outResource = new T[sizeInBytes / sizeof(T)]; memcpy(outResource, m_pResource, sizeInBytes); return outResource; }
	void Unload()											{ delete[] m_pResource; m_pResource = 0; }
};


///////// COMPATIBILITY ///////////////////////////////////////////////////////

class Compatibility_Direct3d9 : public ICompatibility
{
public:
    ICOMPATIBILITY_INTERFACE(;)

    Compatibility_Direct3d9(GraphicsService_Direct3d9* pGraphicsService);
    ~Compatibility_Direct3d9();

	char m_PixelShaderVersion[16];
	char m_VertexShaderVersion[16];

	D3DCAPS9* m_pDeviceCaps;	
	GraphicsService_Direct3d9* m_pGraphicsService;
};


///////// GRAPHICS SERVICE ////////////////////////////////////////////////////

typedef class GraphicsService_Direct3d9 : public IGraphicsService
{
public:
	IGRAPHICSSERVICE_INTERFACE(;)

    ~GraphicsService_Direct3d9();
    GraphicsService_Direct3d9();

    IDirect3DDevice9* GetDevice() { return m_pDevice; };
	IDirect3D9* GetDirect3D() { return m_pD3D; };

	void RecreateDevice();

	void AddResource(Resource* resource);
	void RemoveResource(Resource* resource);

	void BeginScene();
	void EndScene();
	
	bool GetIsDrawingScene() { return m_IsDrawingScene; };

	void SetTexture(ITexture* pTexture, unsigned int index, bool forced);

	void SetPixelShader(PixelShader_Direct3d9* pPixelShader, bool forced = false);
	void SetVertexShader(VertexShader_Direct3d9* pVertexShader, bool forced = false);
	void SetShaderProgram(ShaderProgram_Direct3d9* pShaderProgram, bool forced);
	void SetTexture(ITexture* pTexture, const char* name, bool forced);
	void SetRenderTarget(IRenderTarget* pRenderTarget, bool forced);
	void SetIndexBuffer(IndexBuffer_Direct3d9* pIndexBuffer, bool forced);
	void SetVertexBuffer(VertexBuffer_Direct3d9* pVertexBuffer, unsigned int streamIndex, bool forced);

	void CreateInternalWindow();
	void DisposeInternalWindow();
	void SetRenderStates();
	void UpdateResources();
	void UnloadResources();
	void RestoreGraphicsState();
	void HandleLostDevice();
	void TearDownDevice();
	void CreateDevice();
	void ResetDevice();
	void ReleaseDevice();
	void RestoreDevice();
	void UpdateVertexDeclaration();

	Resource* m_ResourceList;

	IRenderTarget* m_pCurrentRenderTarget;
	VertexBuffer_Direct3d9* m_CurrentIVertexBufferMap[MAX_VERTEX_BUFFER_SLOTS];
	IndexBuffer_Direct3d9* m_pCurrentIndexBuffer;
	PixelShader_Direct3d9* m_pCurrentPixelShader;
	VertexShader_Direct3d9* m_pCurrentVertexShader;
	ShaderProgram_Direct3d9* m_pCurrentShaderProgram;
	ITexture* m_CurrentITextureMap[MAX_TEXTURE_BUFFER_SLOTS];

	bool m_IsDrawingScene;
	IGraphicsService::CullMode m_CurrentCullMode;

	HWND m_Window;
	IDirect3D9* m_pD3D;
	IDirect3DDevice9* m_pDevice;
	bool m_DeviceLost;

	IDirect3DSurface9* m_pDefaultRenderTarget;
	IDirect3DSurface9* m_pDefaultDepthStencilSurface;
	IWindowRenderTarget* m_pDefaultSwapChain;

	ICompatibility* m_pCompatibility;
} GraphicsService;


///////// INDEX BUFFER ////////////////////////////////////////////////////////

class IndexBuffer_Direct3d9 : public IIndexBuffer, public LockableResource<char>
{
public:
    IINDEXBUFFER_INTERFACE(;)

    IndexBuffer_Direct3d9(GraphicsService_Direct3d9* pGraphicsService, const void* pIndices, unsigned int count, DataType dataType);
    ~IndexBuffer_Direct3d9();

	void LoadResource();
	void UnloadResource();

	IDirect3DIndexBuffer9* m_pIndexBuffer;
	unsigned int m_IndexCount;
	IIndexBuffer::DataType m_DataType;

	GraphicsService_Direct3d9* m_pGraphicsService;
};


///////// VERTEX BUFFER ///////////////////////////////////////////////////////

class VertexBuffer_Direct3d9 : public IVertexBuffer, public LockableResource<char>
{
public:
    IVERTEXBUFFER_INTERFACE(;)

    ~VertexBuffer_Direct3d9();
    VertexBuffer_Direct3d9(GraphicsService_Direct3d9* pGraphicsService, const void* pVertices, unsigned int count, unsigned int vertexSizeInBytes, StreamMode streamMode);

	void LoadResource();
	void UnloadResource();

	unsigned int m_VertexSizeInBytes;
	IVertexBuffer::StreamMode m_StreamMode;
	unsigned int m_ComponentDescriptionCount;
	const IVertexBuffer::ComponentDescription* m_pComponentDescriptions;
	IDirect3DVertexBuffer9* m_pVertexBuffer;

	GraphicsService_Direct3d9* m_pGraphicsService;
};


///////// PIXEL SHADER ////////////////////////////////////////////////////////

class PixelShader_Direct3d9 : public IPixelShader, public LockableResource<char>
{
public:
    IPIXELSHADER_INTERFACE(;)

    ~PixelShader_Direct3d9();
    PixelShader_Direct3d9(GraphicsService_Direct3d9* pGraphicsService, const char* pShaderBytes, unsigned int sizeInBytes);

	void LoadResource();
	void UnloadResource();
	int GetSamplerIndex(const char* name);

	IDirect3DPixelShader9* m_pShaderBuffer;
	ID3DXConstantTable* m_pConstantTable;

	GraphicsService_Direct3d9* m_pGraphicsService;
};


///////// VERTEX SHADER ////////////////////////////////////////////////////////

class VertexShader_Direct3d9 : public IVertexShader, public LockableResource<char>
{
public:
    IVERTEXSHADER_INTERFACE(;)

    ~VertexShader_Direct3d9();
    VertexShader_Direct3d9(GraphicsService_Direct3d9* pGraphicsService, const char* pShaderBytes, unsigned int sizeInBytes);

	void LoadResource();
	void UnloadResource();

	void SetConstantFloatArray(const char* name, const float* matrix, const unsigned int count);
	void SetConstantIntArray(const char* name, const int* values, const unsigned int count);

	IDirect3DVertexShader9* m_pShaderBuffer;
	ID3DXConstantTable* m_pConstantTable;

	GraphicsService_Direct3d9* m_pGraphicsService;
};


///////// SHADER PROGRAM ///////////////////////////////////////////////////////

class ShaderProgram_Direct3d9 : public IShaderProgram, public LockableResource<char>
{
public:
    ISHADERPROGRAM_INTERFACE(;)

    ~ShaderProgram_Direct3d9();
    ShaderProgram_Direct3d9(GraphicsService_Direct3d9* pGraphicsService, IVertexShader* pVertexShader, IPixelShader* pPixelShader);

	void LoadResource();
	void UnloadResource();

	IPixelShader* m_pPixelShader;
	IVertexShader* m_pVertexShader;

	GraphicsService_Direct3d9* m_pGraphicsService;
};


///////// BITMAP TEXTURE ///////////////////////////////////////////////////////

class BitmapTexture_Direct3d9 : public IBitmapTexture, public LockableResource<char>
{
public:
    IBITMAPTEXTURE_INTERFACE(;)

	~BitmapTexture_Direct3d9();
    BitmapTexture_Direct3d9(GraphicsService_Direct3d9* pGraphicsService, const void* pBitmap, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, MipMapMode mipMapMode);

	void LoadResource();
	void UnloadResource();
	
	Size m_Size;
	ITexture::MipMapMode m_MipMapMode;
	ITexture::SamplerSettings m_SamplerSettings;
	IBitmapTexture::Format m_Format;

	IDirect3DTexture9* m_pTexture;

	GraphicsService_Direct3d9* m_pGraphicsService;
};


///////// RENDER TARGET TEXTURE ///////////////////////////////////////////////

class RenderTargetTexture_Direct3d9 : public IRenderTargetTexture, public Resource
{
public:
    IRENDERTARGETTEXTURE_INTERFACE(;)

    ~RenderTargetTexture_Direct3d9();
    RenderTargetTexture_Direct3d9(GraphicsService_Direct3d9* pGraphicsService, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, bool depthTestEnabled, MipMapMode mipMapMode);

    void LoadResource();
	void UnloadResource();

	void UnloadRenderTarget();
	void UnloadDepthStencil();
		
	Size m_Size;
	ITexture::MipMapMode m_MipMapMode;
	IRenderTargetTexture::Format m_Format;
	ITexture::SamplerSettings m_SamplerSettings;
	bool m_DepthTestEnabled;

	IDirect3DTexture9* m_pRenderTarget;
	IDirect3DSurface9* m_pDepthStencil;
	IDirect3DSurface9* m_pRenderTargetSurface;
	
	GraphicsService_Direct3d9* m_pGraphicsService;
};


///////// WINDOW RENDER TARGET ////////////////////////////////////////////////

class WindowRenderTarget_Direct3d9 : public IWindowRenderTarget, public Resource
{
public:
    IWINDOWRENDERTARGET_INTERFACE(;)

    WindowRenderTarget_Direct3d9(GraphicsService_Direct3d9* pGraphicsService, MINI3D_WINDOW windowHandle, bool depthTestEnabled);
    ~WindowRenderTarget_Direct3d9();

	void LoadResource();
	void UnloadResource();

	bool LoadRenderTarget(IDirect3DDevice9* pDevice);
	bool LoadDepthStencil(IDirect3DDevice9* pDevice);

	void UnloadRenderTarget();
	void UnloadDepthStencil();

	void GetWindowContentSize(const MINI3D_WINDOW window, unsigned int &width, unsigned int &height) const;
	void SetSize(unsigned int width, unsigned int height);

	void FreeMini3dWindowToWindowRenderTargetAssociation();
	void SetMini3dWindowToWindowRenderTargetAssociation();

	Size m_Size;
	HWND m_Window;
	bool m_DepthTestEnabled;
	unsigned int m_Msaa;
	IWindowRenderTarget::ScreenState m_ScreenState;

	IDirect3DSwapChain9* m_pScreenRenderTarget;
	IDirect3DSurface9* m_pDepthStencil;
	IDirect3DSurface9* m_pRenderTargetSurface;

	GraphicsService_Direct3d9* m_pGraphicsService;
};

}   // namespace mini3d
}

#endif // MINI3D_GRAPHICS_DIRECT3D9_H
#endif // MINI3D_GRAPHICSSERVICE_DIRECTX_9
#endif // _WIN32