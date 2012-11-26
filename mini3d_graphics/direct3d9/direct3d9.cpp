
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#include "direct3d9.hpp"

#ifdef _WIN32
#ifdef MINI3D_GRAPHICSSERVICE_DIRECTX_9

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3dx9.h>
#include <cstdio>

#define STB_DXT_IMPLEMENTATION
#include "../common/stb_dxt.h"
#include "../common/mipmap.hpp"


void mini3d_assert(bool expression, const char* text, ...);


////////// INTERNAL HELPER FUNCTIONS //////////////////////////////////////////

namespace mini3d
{

namespace {

	// A map matching window handles to the correct window render target object
	struct WindowInfo { WindowRenderTarget_Direct3d9* windowRenderTarget; WNDPROC pOrigProc; };

	LRESULT CALLBACK HookWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		WindowInfo* windowInfo = (WindowInfo*)GetProp(hWnd, L".mini3d_windowinfo");

		// if we did not find a matching window proc, call default window proc and return result (window is not a mini3d window)
		if (windowInfo == 0) return DefWindowProc(hWnd, msg, wParam, lParam);

		switch(msg)
		{
		// Window has been resized
		case WM_SIZE:

				// Update the window size
				unsigned int width = LOWORD(lParam) | 1;
				unsigned int height = HIWORD(lParam) | 1;

				// update the render target size
				windowInfo->windowRenderTarget->SetSize(width, height);
			break;
		}
		
		return CallWindowProc(windowInfo->pOrigProc, hWnd, msg, wParam, lParam);
	}

	D3DMULTISAMPLE_TYPE FromMultisampleFormat(unsigned int msaa)
	{
		switch(msaa)
		{
			case 0:
			case 1:
				return D3DMULTISAMPLE_NONE;
				break;
			case 2:
				return D3DMULTISAMPLE_2_SAMPLES;
				break;
			case 4:
				return D3DMULTISAMPLE_4_SAMPLES;
				break;
			case 8:
				return D3DMULTISAMPLE_8_SAMPLES;
				break;
			case 16:
				return D3DMULTISAMPLE_16_SAMPLES;
				break;
		}
		// default case, no multisample
		return D3DMULTISAMPLE_NONE;
	}

	void CheckMultisampleFormat(IDirect3D9* pD3D, unsigned int &msaa, bool fullscreen)
	{

		D3DDISPLAYMODE displayMode;
		pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);
	

		DWORD pQualityLevels;
		while (	msaa > 0 && FAILED(pD3D->CheckDeviceMultiSampleType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, displayMode.Format, true, FromMultisampleFormat(msaa), &pQualityLevels)))
		{
			msaa >>= 1;
		}
	}

	D3DFORMAT GetCorrectBackBufferFormat(IDirect3D9* pD3D)
	{
		D3DDISPLAYMODE displayMode;
		pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);

		// Get the color depth of the display
		if (displayMode.Format == D3DFMT_X8R8G8B8)
			return D3DFMT_X8R8G8B8;
		else if (displayMode.Format == D3DFMT_R5G6B5)
			return D3DFMT_R5G6B5;
		
		// TODO: NO BACK BUFFER FORMAT FOUND! display settings?
		// push error INCOMPATIBLE HARDWARE and handle where needed
		return (D3DFORMAT)0;
	}

	D3DFORMAT GetCorrectDepthStencilFormat(IDirect3D9* pD3D)
	{
		D3DDISPLAYMODE displayMode;
		pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);

		// Get the color depth of the display
		D3DFORMAT displayFormat = GetCorrectBackBufferFormat(pD3D);

		// Check if we are running in 32 bit mode and 32 bit depth buffer is supported
		if(displayFormat == D3DFMT_X8R8G8B8)
		{
			if (SUCCEEDED(pD3D->CheckDeviceFormat(0, D3DDEVTYPE_HAL, displayMode.Format,  D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D32 )))
			{
				return D3DFMT_D32;
			}
			else if (SUCCEEDED(pD3D->CheckDeviceFormat(0, D3DDEVTYPE_HAL, displayMode.Format,  D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8 )))
			{
				return D3DFMT_D24S8;
			}
		}
		// TODO: This is ancient! even on mobile platforms! Why not just remove it and say unsupported display format...
		// if we are only running 16 bit color format, then just run a 16 bit depth format...
		else if (displayFormat == D3DFMT_R5G6B5)
		{
			if (SUCCEEDED( pD3D->CheckDeviceFormat(0, D3DDEVTYPE_HAL, displayMode.Format,  D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16)))
			{
				return D3DFMT_D16;
			}
		}
		
		// TODO: NO DEPTH STENCIL FORMAT FOUND!
		// push error INCOMPATIBLE HARSDARE and handle consequences where needed
		return (D3DFORMAT)0;
	}


	D3DPRESENT_PARAMETERS CreatePresentationParameters(IDirect3D9* pD3D, HWND hWnd, unsigned int width, unsigned int height, unsigned int &msaa, bool fullscreen = false)
	{
		CheckMultisampleFormat(pD3D, msaa, fullscreen);

		// get the display mode
		D3DDISPLAYMODE d3ddm;
		pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);

		// set the presentation parameters
		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(d3dpp));

		// Default backbuffer is never used
		// It has small area to not take up memory, but big enough not to cause an issues with the graphcis driver
		d3dpp.BackBufferWidth = width; 
		d3dpp.BackBufferHeight = height;
		d3dpp.Windowed = !fullscreen;
		d3dpp.EnableAutoDepthStencil = false; // we manage swap chains manually
		d3dpp.BackBufferCount = 1;
		d3dpp.BackBufferFormat = GetCorrectBackBufferFormat(pD3D);
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.AutoDepthStencilFormat = GetCorrectDepthStencilFormat(pD3D);
		d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		d3dpp.MultiSampleType = FromMultisampleFormat(msaa);
		d3dpp.MultiSampleQuality = 0;
		d3dpp.hDeviceWindow = hWnd;

		return d3dpp;
	}
}


///////// INDEX BUFFER ////////////////////////////////////////////////////////

_D3DFORMAT mini3d_IndexBuffer_Formats[] = { D3DFMT_INDEX16, D3DFMT_INDEX32 };
unsigned int mini3d_IndexBuffer_BytesPerIndex[] = { 2, 4 };

void* IndexBuffer_Direct3d9::GetIndices(unsigned int& sizeInBytes) { return CopyOut(sizeInBytes); };
void* IndexBuffer_Direct3d9::Lock(unsigned int& sizeInBytes, bool readOnly) { return Lock(sizeInBytes, readOnly); };
void IndexBuffer_Direct3d9::Unlock() { if (LockableResource::Unlock()) LoadResource(); };
unsigned int IndexBuffer_Direct3d9::GetIndexCount() const { return m_IndexCount; };
IndexBuffer_Direct3d9::DataType IndexBuffer_Direct3d9::GetDataType() const { return m_DataType; };
IndexBuffer_Direct3d9::~IndexBuffer_Direct3d9() { UnloadResource(); m_pGraphicsService->RemoveResource(this); }

IndexBuffer_Direct3d9::IndexBuffer_Direct3d9(GraphicsService_Direct3d9* pGraphicsService, const void* pIndices, unsigned int count, DataType dataType)
{
	m_pGraphicsService = pGraphicsService; 
	m_pIndexBuffer = 0;

	SetIndices(pIndices, count, dataType);
	m_pGraphicsService->AddResource(this);
}

void IndexBuffer_Direct3d9::SetIndices(const void* pIndices, unsigned int count, DataType dataType)
{
	mini3d_assert(pIndices != 0, "Setting an Index Buffer with a NULL data pointer!");

	m_DataType = dataType;
	m_IndexCount = count;

	CopyIn(pIndices, count * mini3d_IndexBuffer_BytesPerIndex[m_DataType]);
}

void IndexBuffer_Direct3d9::LoadResource()
{
	/// Allocate buffer on the graphics card and add index data.
	IDirect3DDevice9* pDevice = m_pGraphicsService->GetDevice();

	// If the buffer exists but is not the correct size, tear it down and recreate it
	if (m_pIndexBuffer != 0)
	{
		D3DINDEXBUFFER_DESC desc;
		m_pIndexBuffer->GetDesc(&desc);

		if (m_SizeInBytes != desc.Size) UnloadResource();
	}

	// If it does not exist, create a new one
	if (m_pIndexBuffer == 0)
	{
		if( FAILED( pDevice->CreateIndexBuffer(m_SizeInBytes, D3DUSAGE_WRITEONLY, mini3d_IndexBuffer_Formats[m_DataType], D3DPOOL_DEFAULT, &m_pIndexBuffer, 0 ) ) )
		{
			mini3d_assert(false, "Failed to create Direct3D 9 index buffer");
		}
	}

	// Lock the buffer to gain access to the Indices 
	void* pBufferIndices;

	if(FAILED(m_pIndexBuffer->Lock(0, 0, &pBufferIndices, 0)))
	{
		mini3d_assert(false, "Failed to lock Direct3D 9 index buffer");
	}

	memcpy(pBufferIndices, m_pResource, m_SizeInBytes);
	m_pIndexBuffer->Unlock();
	isDirty = false;
}

void IndexBuffer_Direct3d9::UnloadResource()
{
	if (m_pIndexBuffer != 0)
	{
		// if this is the currently loaded index buffer, release it
		if (m_pGraphicsService->GetIndexBuffer() == this)
			m_pGraphicsService->SetIndexBuffer(0);

		m_pIndexBuffer->Release();
		m_pIndexBuffer = 0;
	}

	isDirty = true;
}



///////// VERTEX BUFFER ///////////////////////////////////////////////////////

void* VertexBuffer_Direct3d9::GetVertices(unsigned int& sizeInBytes) { return CopyOut(sizeInBytes); };
void* VertexBuffer_Direct3d9::Lock(unsigned int& sizeInBytes, bool readOnly) { return Lock(sizeInBytes, readOnly); };
void VertexBuffer_Direct3d9::Unlock() { if (LockableResource::Unlock()) LoadResource(); };
unsigned int VertexBuffer_Direct3d9::GetVertexCount() const { return m_SizeInBytes / m_VertexSizeInBytes; };
unsigned int VertexBuffer_Direct3d9::GetVertexSizeInBytes() const { return m_VertexSizeInBytes; };
VertexBuffer_Direct3d9::StreamMode VertexBuffer_Direct3d9::GetStreamMode() const { return m_StreamMode; };
void VertexBuffer_Direct3d9::SetStreamMode(StreamMode streamMode) { m_StreamMode = streamMode; };
const VertexBuffer_Direct3d9::ComponentDescription* VertexBuffer_Direct3d9::GetComponentDescriptions(unsigned int &count) const { count = m_ComponentDescriptionCount;  return m_pComponentDescriptions; }
VertexBuffer_Direct3d9::~VertexBuffer_Direct3d9(void) { UnloadResource(); m_pGraphicsService->RemoveResource(this);  }

VertexBuffer_Direct3d9::VertexBuffer_Direct3d9(GraphicsService_Direct3d9* pGraphicsService, const void* pVertices, unsigned int count, unsigned int vertexSizeInBytes, StreamMode streamMode)
{
	m_pGraphicsService = pGraphicsService; 
	m_pVertexBuffer = 0;
	m_pComponentDescriptions = 0;

	SetVertices(pVertices, count, vertexSizeInBytes, streamMode);
	m_pGraphicsService->AddResource(this);
}

void VertexBuffer_Direct3d9::SetVertices(const void* pVertices, unsigned int count, unsigned int vertexSizeInBytes, StreamMode streamMode)
{
	mini3d_assert(pVertices != 0, "Setting a Vertex Buffer with a NULL data pointer!");

	m_VertexSizeInBytes = vertexSizeInBytes;
	m_StreamMode = streamMode;

	CopyIn(pVertices, count * vertexSizeInBytes);
}

void VertexBuffer_Direct3d9::SetComponentDescriptions(const ComponentDescription* pComponentDescriptions, const unsigned int count) 
{ 
	if (m_pComponentDescriptions != 0) delete[] m_pComponentDescriptions;
	
	m_pComponentDescriptions = (ComponentDescription*)memcpy(new ComponentDescription[count], pComponentDescriptions, count * sizeof(ComponentDescription));
	m_ComponentDescriptionCount = count;
}

void VertexBuffer_Direct3d9::LoadResource(void)
{
	/// Allocate buffer on the graphics card and add index data.
	IDirect3DDevice9* pDevice = m_pGraphicsService->GetDevice();

	// If the buffer exists but is not the correct size, tear it down and recreate it
	if (m_pVertexBuffer != 0)
	{
		D3DVERTEXBUFFER_DESC desc;
		m_pVertexBuffer->GetDesc(&desc);

		if (desc.Size != m_SizeInBytes) UnloadResource();
	}

	// If it does not exist, create a new one
	if (m_pVertexBuffer == 0)
	{
		if( FAILED( pDevice->CreateVertexBuffer(m_SizeInBytes, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVertexBuffer, 0 ) ) )
		{
			mini3d_assert(false, "Failed to create Direct3D 9 index buffer");
		}
	}

	// Lock the buffer to gain access to the vertices 
	void* pBufferVertices;

	if(FAILED(m_pVertexBuffer->Lock(0, 0, &pBufferVertices, 0)))
	{
		mini3d_assert(false, "Failed to lock Direct3D 9 index buffer");
	}

	memcpy(pBufferVertices, m_pResource, m_SizeInBytes);
	m_pVertexBuffer->Unlock();

	isDirty = false;
}

void VertexBuffer_Direct3d9::UnloadResource(void)
{
	if (m_pVertexBuffer != 0)
	{
		// if this is the currently loaded vertex buffer, release it
		if (m_pGraphicsService->GetVertexBuffer(0) == this)
			m_pGraphicsService->SetVertexBuffer(0, 0);

		m_pVertexBuffer->Release();
		m_pVertexBuffer = 0;
	}

	isDirty = true;
}


///////// PIXEL SHADER ////////////////////////////////////////////////////////

void* PixelShader_Direct3d9::GetPixelShader(unsigned int& sizeInBytes) { sizeInBytes = m_SizeInBytes; return m_pResource; };
PixelShader_Direct3d9::~PixelShader_Direct3d9() { UnloadResource(); m_pGraphicsService->RemoveResource(this);  }

PixelShader_Direct3d9::PixelShader_Direct3d9(GraphicsService_Direct3d9* pGraphicsService, const char* pShaderBytes, unsigned int sizeInBytes)
{
	mini3d_assert(pShaderBytes != 0, "Setting a Pixel Shader with a NULL data pointer!");

	m_pGraphicsService = pGraphicsService; 
	m_pShaderBuffer = 0;
	m_pConstantTable = 0;

	CopyIn((const char*)pShaderBytes, sizeInBytes);
	m_pGraphicsService->AddResource(this);
}

void PixelShader_Direct3d9::LoadResource()
{
	IDirect3DDevice9* pDevice = m_pGraphicsService->GetDevice();

	// Get supported vertex shader profile
	LPCSTR profile = D3DXGetPixelShaderProfile(pDevice);
	
	// comle the shader source
	ID3DXBuffer* buffer;
	LPD3DXBUFFER ppErroMessage;
	D3DXCompileShader((LPCSTR)m_pResource, m_SizeInBytes, 0, 0, "main", profile, 0, &buffer, &ppErroMessage, &m_pConstantTable);

	if (ppErroMessage != 0)
	{
		printf((char*)(ppErroMessage->GetBufferPointer()));
		mini3d_assert(false, "Error log when compiling Pixel Shader!");
	}

	if( FAILED( pDevice->CreatePixelShader((DWORD*)buffer->GetBufferPointer(), &m_pShaderBuffer)))
	{
		mini3d_assert(false, "Failed to compile pixel shader!");	
	}

	buffer->Release();
	isDirty = false;
}

void PixelShader_Direct3d9::UnloadResource()
{
	if (m_pShaderBuffer != 0)
	{
		// if this is the currently loaded pixel shader, release it
		if (m_pGraphicsService->m_pCurrentPixelShader == this)
			m_pGraphicsService->SetShaderProgram(0);

		m_pConstantTable->Release();
		m_pConstantTable = 0;

		m_pShaderBuffer->Release();
		m_pShaderBuffer = 0;
	}

	isDirty = true;
}

int PixelShader_Direct3d9::GetSamplerIndex(const char* name)
{
	mini3d_assert(m_pConstantTable != 0, "Trying to query a sampler index on a pixel shader with no constant table!");
	
	D3DXHANDLE handle = m_pConstantTable->GetConstantByName(0, name);
	mini3d_assert(handle != 0, "Trying to query the index for a sampler but the pixel shader does not have any sampler with that name!");

	return m_pConstantTable->GetSamplerIndex(handle);
}


///////// VERTEX SHADER ////////////////////////////////////////////////////////

void* VertexShader_Direct3d9::GetVertexShader(unsigned int& sizeInBytes) const { sizeInBytes = m_SizeInBytes; return m_pResource; };
VertexShader_Direct3d9::~VertexShader_Direct3d9() { UnloadResource(); m_pGraphicsService->RemoveResource(this);  }

VertexShader_Direct3d9::VertexShader_Direct3d9(GraphicsService_Direct3d9* pGraphicsService, const char* pShaderBytes, unsigned int sizeInBytes)
{
	mini3d_assert(pShaderBytes != 0, "Setting a Pixel Shader with a NULL data pointer!");

	m_pGraphicsService = pGraphicsService; 
	m_pShaderBuffer = 0;
	m_pConstantTable = 0;

	CopyIn((const char*)pShaderBytes, sizeInBytes);
	m_pGraphicsService->AddResource(this);
}

void VertexShader_Direct3d9::LoadResource()
{
	IDirect3DDevice9* pDevice = m_pGraphicsService->GetDevice();

	// Get supported vertex shader profile
	LPCSTR profile = D3DXGetVertexShaderProfile(pDevice);
	
	// comle the shader source
	ID3DXBuffer* buffer;
	LPD3DXBUFFER ppErroMessage;
	D3DXCompileShader((LPCSTR)m_pResource, m_SizeInBytes, 0, 0, "main", profile, 0, &buffer, &ppErroMessage, &m_pConstantTable);

	if (ppErroMessage != 0)
	{
		printf((char*)(ppErroMessage->GetBufferPointer()));
		mini3d_assert(false, "Error log when compiling Vertex Shader!");
	}

	if( FAILED( pDevice->CreateVertexShader((DWORD*)buffer->GetBufferPointer(), &m_pShaderBuffer)))
	{
		mini3d_assert(false, "Failed to compile Vertex Shader!");	
	}

	buffer->Release();
	isDirty = false;
}

void VertexShader_Direct3d9::UnloadResource()
{
	if (m_pShaderBuffer != 0)
	{
		// if this is the currently loaded pixel shader, release it
		if (m_pGraphicsService->m_pCurrentVertexShader == this)
			m_pGraphicsService->SetShaderProgram(0);

		m_pConstantTable->Release();
		m_pConstantTable = 0;

		m_pShaderBuffer->Release();
		m_pShaderBuffer = 0;
	}

	isDirty = true;
}

void VertexShader_Direct3d9::SetConstantFloatArray(const char* name, const float* values, const unsigned int count)
{
	mini3d_assert(m_pConstantTable != 0, "Trying to set a float array on a vertex shader with no constant table!");
	
	D3DXHANDLE handle = m_pConstantTable->GetConstantByName(0, name);
	mini3d_assert(handle != 0, "Trying to query the handle for a constant but the vertex shader does not have any constant with that name!");

    D3DXCONSTANT_DESC desc;
    UINT cc = 1;
    m_pConstantTable->GetConstantDesc(handle, &desc, &cc);

	HRESULT result = m_pConstantTable->SetFloatArray(m_pGraphicsService->GetDevice(), handle, values, count);
};

void VertexShader_Direct3d9::SetConstantIntArray(const char* name, const int* values, const unsigned int count)
{
	mini3d_assert(m_pConstantTable != 0, "Trying to set an integer array on a vertex shader with no constant table!");
		
	D3DXHANDLE handle = m_pConstantTable->GetConstantByName(0, name);
	mini3d_assert(handle != 0, "Trying to query the handle for a constant but the vertex shader does not have any constant with that name!");

	m_pConstantTable->SetIntArray(m_pGraphicsService->GetDevice(), handle, values, count);
};


///////// SHADER PROGRAM ///////////////////////////////////////////////////////

IPixelShader* ShaderProgram_Direct3d9::GetPixelShader() const { return m_pPixelShader; };
IVertexShader* ShaderProgram_Direct3d9::GetVertexShader() const { return m_pVertexShader; };

ShaderProgram_Direct3d9::~ShaderProgram_Direct3d9(void) { UnloadResource(); m_pGraphicsService->RemoveResource(this);  }

ShaderProgram_Direct3d9::ShaderProgram_Direct3d9(GraphicsService_Direct3d9* pGraphicsService, IVertexShader* pVertexShader, IPixelShader* pPixelShader)
{
	m_pGraphicsService = pGraphicsService; 
	m_pVertexShader =  pVertexShader;
	m_pPixelShader = pPixelShader;

	LoadResource();
	m_pGraphicsService->AddResource(this);
}

void ShaderProgram_Direct3d9::LoadResource(void)
{
	isDirty = false;
}

void ShaderProgram_Direct3d9::UnloadResource(void)
{
	isDirty = true;
}


///////// BITMAP TEXTURE ///////////////////////////////////////////////////////

_D3DFORMAT mini3d_BitmapTexture_Formats[] = { D3DFMT_X8R8G8B8, D3DFMT_A16B16G16R16, D3DFMT_L16, D3DFMT_R32F, D3DFMT_DXT5 };

unsigned int mini3d_BitmapTexture_BytesPerPixel[] = { 4, 8, 2, 4, 4 };

void*							BitmapTexture_Direct3d9::GetBitmap(unsigned int& sizeInBytes)   				{ return CopyOut(sizeInBytes); };
unsigned int					BitmapTexture_Direct3d9::GetWidth() const										{ return m_Size.width; };
unsigned int					BitmapTexture_Direct3d9::GetHeight() const										{ return m_Size.height; };
ITexture::MipMapMode	        BitmapTexture_Direct3d9::GetMipMapMode() const									{ return m_MipMapMode; };
void							BitmapTexture_Direct3d9::SetMipMapMode(MipMapMode mipMapMode)					{ m_MipMapMode = mipMapMode; };
IBitmapTexture::Format	        BitmapTexture_Direct3d9::GetFormat() const										{ return m_Format; };
IBitmapTexture::SamplerSettings	BitmapTexture_Direct3d9::GetSamplerSettings() const								{ return m_SamplerSettings; };
void*							BitmapTexture_Direct3d9::Lock(unsigned int& sizeInBytes, bool readOnly)			{ return Lock(sizeInBytes, readOnly); };
void							BitmapTexture_Direct3d9::Unlock()												{ if (LockableResource::Unlock()) LoadResource(); };
								BitmapTexture_Direct3d9::~BitmapTexture_Direct3d9()								{ UnloadResource(); m_pGraphicsService->RemoveResource(this);  }

BitmapTexture_Direct3d9::BitmapTexture_Direct3d9(GraphicsService_Direct3d9* pGraphicsService, const void* pBitmap, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, MipMapMode mipMapMode)
{
	m_pGraphicsService = pGraphicsService; 
	m_pTexture = 0;

	SetBitmap(pBitmap, width, height, format, samplerSettings, mipMapMode);
	m_pGraphicsService->AddResource(this);
}

void BitmapTexture_Direct3d9::SetBitmap(const void* pBitmap, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, MipMapMode mipMapMode)
{
	mini3d_assert(pBitmap != 0, "Setting a Bitmap Texture with a NULL data pointer!");
	mini3d_assert((width & (width - 1)) == 0, "Setting a Bitmap Texture to a non power of two width!");
	mini3d_assert((height & (height - 1)) == 0, "Setting a Bitmap Texture to a non power of two height!");
	mini3d_assert(width >= 64, "Setting a Bitmap Texture to a height less than 64!");
	mini3d_assert(height >= 64, "Setting a Bitmap Texture to a width less than 64!");

	m_Format = format;
	m_SamplerSettings = samplerSettings;
	m_Size.width = width;
	m_Size.height = height;
	m_MipMapMode = mipMapMode;

	CopyIn(pBitmap, width * height * mini3d_BitmapTexture_BytesPerPixel[m_Format]);
}

void BitmapTexture_Direct3d9::LoadResource()
{
	IDirect3DDevice9* pDevice = m_pGraphicsService->GetDevice();

	// If the buffer exists but is not the correct size, tear it down and recreate it
	if ((m_pTexture != 0))
	{
		D3DSURFACE_DESC desc;
		m_pTexture->GetLevelDesc(0, &desc);
		
		if (m_Size.width != desc.Width || m_Size.height != desc.Height) UnloadResource();
	}

	// If it does not exist, create a new one
	if (m_pTexture == 0)
	{
		int numLevels = (m_MipMapMode != MIPMAP_NONE) ? 0 : 1;

		if( FAILED( pDevice->CreateTexture(m_Size.width, m_Size.height, 0, 0, mini3d_BitmapTexture_Formats[m_Format], D3DPOOL_DEFAULT, &m_pTexture, 0) ) )
		{
			mini3d_assert(false, "Creating Direct3D 9 texture failed!");
		}
	}

	IDirect3DTexture9* pStagingTexture;
	if( FAILED( pDevice->CreateTexture(m_Size.width, m_Size.height, 0, 0, mini3d_BitmapTexture_Formats[m_Format], D3DPOOL_SYSTEMMEM, &pStagingTexture, 0) ) )
	{
		mini3d_assert(false, "Creating Direct3D 9 texture failed!");
	}

	unsigned char* pMipMap = (unsigned char*)m_pResource;

	unsigned int levelCount = m_pTexture->GetLevelCount();
	for (unsigned int level = 0; level < levelCount; ++level)
	{
		// Lock the buffer to gain access to the Bitmap 
		IDirect3DSurface9* pSurface;
		pStagingTexture->GetSurfaceLevel(level, &pSurface);
	
		D3DLOCKED_RECT textureDataRectangle;

		if(FAILED(pSurface->LockRect(&textureDataRectangle, 0, 0)))
		{
			mini3d_assert(false, "Locking Direct3D 9 texture failed!");
		}

		// Copy the data from pBitmap to d3dbuffer.
		// if the width (pitch) of the bitmap is small, it might be smaller than the d3dbuffer pitch so we have to take this into account when copying.

		unsigned int mipMapWidth = (m_Size.width >> level) > 0 ? m_Size.width >> level : 1;
		unsigned int mipMapHeight = (m_Size.height >> level) > 0 ? m_Size.width >> level : 1;

		unsigned int bytesPerPixel = mini3d_BitmapTexture_BytesPerPixel[m_Format];
		unsigned int rowSizeInBytes = mipMapWidth * bytesPerPixel;

		// When copying color bitmap data we need to shift the bits from RBGA to ARGB which is used in DirectX9
		for (unsigned int i = 0; i < mipMapHeight; ++i)
		{
			unsigned char* pBitmapData = (unsigned char*)pMipMap + mipMapWidth * i * bytesPerPixel;

			if (m_Format == FORMAT_RGBA8UI_COMPRESSED)
			{
				unsigned char* pD3DData = (unsigned char*)textureDataRectangle.pBits + i * textureDataRectangle.Pitch / 4;
					
				// We always need to write at least one block. It might have be a good idea to pad the rest of the block if mipmap is smaller than the block, but this is not done!
				unsigned int blockWidth = 4 < mipMapWidth ? 4 : mipMapWidth;
				unsigned int blockHeight = 4 < mipMapHeight ? 4 : mipMapHeight;

				unsigned int pRgbaBlock[16];
				for (unsigned int j = 0; j < mipMapWidth; j += 4)
				{
					for (unsigned int k = 0; k < blockHeight; ++k) memcpy(pRgbaBlock + 4 * k, (unsigned int*)pMipMap + mipMapWidth * (i + k) + j, blockWidth * 4);
					stb_compress_dxt_block(pD3DData, (unsigned char*)pRgbaBlock, true, 0);
					pD3DData += 16;
				}
				i+= 3;
			}
			else if (m_Format == FORMAT_RGBA8UI)
			{
				unsigned char* pD3DData = (unsigned char*)textureDataRectangle.pBits + i * textureDataRectangle.Pitch;

				// RGBA > ARGB
				for (unsigned int j = 0; j < rowSizeInBytes; j += bytesPerPixel)
					((unsigned int*)pD3DData)[j / 4] = D3DCOLOR_ARGB(pBitmapData[j + 3], pBitmapData[j], pBitmapData[j + 1], pBitmapData[j + 2]);
			}
			else if (m_Format == FORMAT_RGBA16UI)
			{
				unsigned char* pD3DData = (unsigned char*)textureDataRectangle.pBits + i * textureDataRectangle.Pitch;

				for (unsigned int j = 0; j < rowSizeInBytes; j += bytesPerPixel)
				{
					// RGBA > ARGB
					memcpy(&pD3DData[j], &pBitmapData[j + 2], 6);
					memcpy(&pD3DData[j + 6], &pBitmapData[j], 2);
				}
			}
			else // Any other format
			{
				unsigned char* pD3DData = (unsigned char*)textureDataRectangle.pBits + i * textureDataRectangle.Pitch;
				memcpy(pD3DData, pBitmapData, rowSizeInBytes);
			}
		}

		pSurface->UnlockRect();
		pSurface->Release();

		if (m_MipMapMode == MIPMAP_BOX_FILTER) 
		{	
			unsigned char* pNewMipMap = mini3d_GenerateMipMapBoxFilter(pMipMap, mipMapWidth, mipMapHeight);
			if (level != 0) delete[]pMipMap;
			pMipMap = pNewMipMap;
		}		
	}

	pDevice->UpdateTexture(pStagingTexture, m_pTexture);
	pStagingTexture->Release();

	isDirty = false;
}

void BitmapTexture_Direct3d9::UnloadResource()
{
	if (m_pTexture != 0)
	{
		// if we are removing one of the current textures, clear that texture slot first
		for(unsigned int i = 0; i < m_pGraphicsService->m_pCompatibility->TextureStreamCount(); i++)
			if (m_pGraphicsService->GetTexture(i) == this)
				m_pGraphicsService->SetTexture(0, i, false);

		m_pTexture->Release();
		m_pTexture = 0;
	}

	isDirty = true;
}


///////// RENDER TARGET TEXTURE ///////////////////////////////////////////////

unsigned int RenderTargetTexture_Direct3d9::GetWidth() const { return m_Size.width; };
unsigned int RenderTargetTexture_Direct3d9::GetHeight() const { return m_Size.height; }


IRenderTarget::Viewport             RenderTargetTexture_Direct3d9::GetViewport() const { Viewport v = {0,0,0,0}; return v; }; // TODO: This should do something
void RenderTargetTexture_Direct3d9::SetViewport(Viewport viewport) { }; // TODO: This should do something

ITexture::MipMapMode		        RenderTargetTexture_Direct3d9::GetMipMapMode() const								{ return m_MipMapMode; };
void								RenderTargetTexture_Direct3d9::SetMipMapMode(MipMapMode mipMapMode)				    { m_MipMapMode = mipMapMode; };
void								RenderTargetTexture_Direct3d9::SetSize(unsigned int width, unsigned int height)	    { m_Size.width = width; m_Size.height = height; LoadResource(); }; 
inline IRenderTargetTexture::Format	RenderTargetTexture_Direct3d9::GetFormat() const									{ return m_Format; };
ITexture::SamplerSettings			RenderTargetTexture_Direct3d9::GetSamplerSettings() const							{ return m_SamplerSettings; };
bool								RenderTargetTexture_Direct3d9::GetDepthTestEnabled() const						    { return m_DepthTestEnabled; };
void								RenderTargetTexture_Direct3d9::SetDepthTestEnabled(bool depthTestEnabled)			{ m_DepthTestEnabled = depthTestEnabled; LoadResource(); };
									RenderTargetTexture_Direct3d9::~RenderTargetTexture_Direct3d9()						{ UnloadResource(); m_pGraphicsService->RemoveResource(this);}

RenderTargetTexture_Direct3d9::RenderTargetTexture_Direct3d9(GraphicsService_Direct3d9* pGraphicsService, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, bool depthTestEnabled, MipMapMode mipMapMode)
{
	m_pGraphicsService = pGraphicsService; 
	m_pDepthStencil = 0;
	m_pRenderTarget = 0;

	SetRenderTargetTexture(width, height, format, samplerSettings, depthTestEnabled, mipMapMode);
	m_pGraphicsService->AddResource(this);
}


void RenderTargetTexture_Direct3d9::SetRenderTargetTexture(unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, bool depthTestEnabled, MipMapMode mipMapMode)
{
	mini3d_assert((width & (width - 1)) == 0, "Setting a Bitmap Texture to a non power of two width!");
	mini3d_assert((height & (height - 1)) == 0, "Setting a Bitmap Texture to a non power of two height!");
	mini3d_assert(width >= 64, "Setting a Bitmap Texture to a height less than 64!");
	mini3d_assert(height >= 64, "Setting a Bitmap Texture to a width less than 64!");

	m_Size.width = width;
	m_Size.height = height;
	m_Format = format;
	m_SamplerSettings = samplerSettings;
	m_DepthTestEnabled = depthTestEnabled;
	m_MipMapMode = mipMapMode;
	isDirty = true;

	LoadResource();	
}

void RenderTargetTexture_Direct3d9::LoadResource(void)
{
	// Get handle to device
	IDirect3DDevice9* pDevice = m_pGraphicsService->GetDevice();

	bool setRenderTargetToThis = (m_pGraphicsService->GetRenderTarget() == this);

	// ---------- Load the render target --------------------------------------

	// If the buffer exists but is not the correct size, tear it down and recreate it
	if (m_pRenderTarget != 0)
	{
		D3DSURFACE_DESC desc;
		m_pRenderTarget->GetLevelDesc(0, &desc);
		
		if (desc.Width != m_Size.width || desc.Height != m_Size.height) UnloadRenderTarget(); 
	}

	// If it does not exist, create a new one
	if (m_pRenderTarget == 0)
	{
		int numLevels = (m_MipMapMode != MIPMAP_NONE) ? 0 : 1;
		DWORD usage = (m_MipMapMode != MIPMAP_NONE) ? D3DUSAGE_AUTOGENMIPMAP : 0;

		if( FAILED( pDevice->CreateTexture(m_Size.width, m_Size.height, 1, usage | D3DUSAGE_RENDERTARGET, mini3d_BitmapTexture_Formats[m_Format], D3DPOOL_DEFAULT, &m_pRenderTarget, 0 ) ) ) 
		{
			mini3d_assert(false, "Failed to create Render Target for Render Target Texture!");
		}
		
		// Capture the render target surfrace to avoid reference counting in directx
		m_pRenderTarget->GetSurfaceLevel(0, &m_pRenderTargetSurface);
	}
	
	// ---------- Load depth stencil (if needed)-------------------------------

	// If depth test is disabled, unload the depth stencil and return ok!
	if (m_DepthTestEnabled == false)
	{
		UnloadDepthStencil();
	}
	else
	{

		// If the buffer exists but is not the correct size, tear it down and recreate it
		if (m_pDepthStencil != 0)
		{
			D3DSURFACE_DESC desc;
			m_pDepthStencil->GetDesc(&desc);
		
			if (desc.Width != m_Size.width || desc.Height != m_Size.height) UnloadDepthStencil();
		}

		// If it does not exist, create a new one
		if (m_pDepthStencil == 0)
		{
			D3DSURFACE_DESC desc;
			m_pRenderTargetSurface->GetDesc(&desc);
			D3DFORMAT depthFormat = GetCorrectDepthStencilFormat(m_pGraphicsService->GetDirect3D());
			if( FAILED( pDevice->CreateDepthStencilSurface(m_Size.width, m_Size.height, depthFormat, desc.MultiSampleType, desc.MultiSampleQuality, true, &m_pDepthStencil, 0 ))) 
			{
				mini3d_assert(false, "Failed to set up Depth Target for Render Target Texture!");
			}
		}
	}

	// restore rendertarget if neccessary
	if (setRenderTargetToThis == true && m_pGraphicsService->GetRenderTarget() != this) m_pGraphicsService->SetRenderTarget(this);

	isDirty = false;
}

void RenderTargetTexture_Direct3d9::UnloadResource()
{
	UnloadRenderTarget();
	UnloadDepthStencil();

	isDirty = true;
}


void RenderTargetTexture_Direct3d9::UnloadRenderTarget()
{
	if (m_pRenderTarget != 0)
	{
		// if we are removing the current render target, restore the default render target first
		if (m_pGraphicsService->GetRenderTarget() == this)
			m_pGraphicsService->SetRenderTarget(0);

		// if we are removing one of the current textures, clear that texture slot first
		for(unsigned int i = 0; i < m_pGraphicsService->m_pCompatibility->TextureStreamCount(); i++)
			if (m_pGraphicsService->GetTexture(i) == this)
				m_pGraphicsService->SetTexture(0, i, false);

		m_pRenderTargetSurface->Release();
		m_pRenderTargetSurface = 0;

		m_pRenderTarget->Release();
		m_pRenderTarget = 0;
	}
}

void RenderTargetTexture_Direct3d9::UnloadDepthStencil()
{
	if (m_pDepthStencil != 0)
	{
		m_pDepthStencil->Release();
		m_pDepthStencil = 0;
	}
}


///////// WINDOW RENDER TARGET ////////////////////////////////////////////////

unsigned int			WindowRenderTarget_Direct3d9::GetWidth() const									    { return m_Size.width; } 
unsigned int			WindowRenderTarget_Direct3d9::GetHeight() const									    { return m_Size.height; }
IRenderTarget::Viewport WindowRenderTarget_Direct3d9::GetViewport() const									{ Viewport v = {0,0,0,0}; return v; } // TODO: Should Do Something
void					WindowRenderTarget_Direct3d9::SetViewport(Viewport viewport)						{ } // TODO: Should Do Something
bool					WindowRenderTarget_Direct3d9::GetDepthTestEnabled() const							{ return m_DepthTestEnabled; };
void					WindowRenderTarget_Direct3d9::SetDepthTestEnabled(bool depthTestEnabled)			{ m_DepthTestEnabled = depthTestEnabled; LoadResource();}
MINI3D_WINDOW			WindowRenderTarget_Direct3d9::GetWindowHandle() const								{ return m_Window; }
void					WindowRenderTarget_Direct3d9::SetSize(unsigned int width, unsigned int height)	    { m_Size.width = width; m_Size.height = height; LoadResource(); }

WindowRenderTarget_Direct3d9::WindowRenderTarget_Direct3d9(GraphicsService_Direct3d9* pGraphicsService, MINI3D_WINDOW windowHandle, bool depthTestEnabled)
{
	m_pGraphicsService = pGraphicsService; 
	m_pScreenRenderTarget = 0;
	m_pRenderTargetSurface = 0;
	m_pDepthStencil = 0;
	m_DepthTestEnabled = depthTestEnabled; 
	m_ScreenState = SCREEN_STATE_WINDOWED; 
	m_Window = 0;

	// Get the requested msaa from the window properties
	HANDLE msaaHandle = GetProp((HWND)windowHandle, L"mini3d.msaa");
	m_Msaa = (msaaHandle) ? *(int*)msaaHandle : 0;

	// Create the window render target
	SetWindowRenderTarget(windowHandle, depthTestEnabled);
	
	// Update actual msaa (if one was requested)
	if (msaaHandle) *(int*)msaaHandle = m_Msaa;

	m_pGraphicsService->AddResource(this);
}

WindowRenderTarget_Direct3d9::~WindowRenderTarget_Direct3d9()
{
	UnloadResource();
	m_pGraphicsService->RemoveResource(this);
	
	FreeMini3dWindowToWindowRenderTargetAssociation();
}

void WindowRenderTarget_Direct3d9::FreeMini3dWindowToWindowRenderTargetAssociation()
{
	// TODO: Create Atom for the ".mini3d_windowinfo" string
	WindowInfo* windowInfo = (WindowInfo*)GetProp(m_Window, L".mini3d_windowinfo");
	RemoveProp(m_Window, L".mini3d_windowinfo");

	(WNDPROC)SetWindowLongPtr(m_Window, GWLP_WNDPROC, (LONG_PTR)windowInfo->pOrigProc);
	delete windowInfo;
}

void WindowRenderTarget_Direct3d9::SetMini3dWindowToWindowRenderTargetAssociation()
{
	WindowInfo* windowInfo = (WindowInfo*)GetProp(m_Window, L".mini3d_windowinfo");
	mini3d_assert(windowInfo == 0, "Trying to create a window render target with a window handle that is already associated with a different window render target!");
		
	// overwrite the window process for the window (our override window process will call the original window process saved in m_pOrigProc)
	WNDPROC pOrigProc = (WNDPROC)SetWindowLongPtr(m_Window, GWLP_WNDPROC, (LONG_PTR)HookWndProc);
	
	// add new window
	windowInfo = new WindowInfo(); 
	windowInfo->windowRenderTarget = this; 
	windowInfo->pOrigProc = pOrigProc;
	
	SetProp(m_Window, L".mini3d_windowinfo", windowInfo);
}

void WindowRenderTarget_Direct3d9::SetWindowRenderTarget(MINI3D_WINDOW window, bool depthTestEnabled)
{
	if (window != m_Window)
	{
		if (m_Window)	FreeMini3dWindowToWindowRenderTargetAssociation();
		m_Window = (HWND)window;
		SetMini3dWindowToWindowRenderTargetAssociation();
	}

	// Get the size of the client area of the window 
	GetWindowContentSize(window, m_Size.width, m_Size.height);
	m_DepthTestEnabled = depthTestEnabled;

	// load the buffer
	LoadResource();
}

void WindowRenderTarget_Direct3d9::Display()
{
	if (m_pScreenRenderTarget == 0)
		return;

	/// Make sure we do an endScene before we present (DirectX9 specific).
	if (m_pGraphicsService->GetIsDrawingScene() == true)
		m_pGraphicsService->EndScene();

	m_pScreenRenderTarget->Present(0,0,0,0,0);
}

void WindowRenderTarget_Direct3d9::LoadResource(void)
{
	// Get handle to device
	IDirect3DDevice9* pDevice = m_pGraphicsService->GetDevice();

	// Check if this is the currently bound render target
	bool setRenderTargetToThis = (m_pGraphicsService->GetRenderTarget() == this);

	// ---------- Load the render target --------------------------------------

	// If the buffer exists but is not the correct size, tear it down and recreate it
	if (m_pScreenRenderTarget != 0)
	{
		D3DPRESENT_PARAMETERS pp;
		m_pScreenRenderTarget->GetPresentParameters(&pp);

		if (pp.BackBufferWidth != m_Size.width || pp.BackBufferHeight != m_Size.height)
		{
			UnloadRenderTarget();
		}
	}

	// If it does not exist, create a new one
	if (m_pScreenRenderTarget == 0)
	{
		D3DPRESENT_PARAMETERS pp = CreatePresentationParameters(m_pGraphicsService->GetDirect3D(), m_Window, m_Size.width, m_Size.height, m_Msaa);
			
		if( FAILED( pDevice->CreateAdditionalSwapChain(&pp, &m_pScreenRenderTarget))) 
		{
			mini3d_assert(false, "Failed to create Swap Chain for Window Render Target!");
		}

		// store the rendertargetsurface to avoid reference counting
		m_pScreenRenderTarget->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_pRenderTargetSurface);
	}

	// ---------- Load the depth target ---------------------------------------

	// If depth test is disabled, unload the depth stencil and return ok!
	if (m_DepthTestEnabled == false)
	{
		UnloadDepthStencil();
	}
	else
	{
		D3DPRESENT_PARAMETERS pp;
		m_pScreenRenderTarget->GetPresentParameters(&pp);

		// If the buffer exists but is not the correct size an multisampleformat, tear it down and recreate it
		if (m_pDepthStencil != 0)
		{
			D3DSURFACE_DESC desc;
			m_pDepthStencil->GetDesc(&desc);
		
			if (desc.Width != m_Size.width || desc.Height != m_Size.height || desc.MultiSampleType != pp.MultiSampleType || desc.MultiSampleQuality != pp.MultiSampleQuality) UnloadDepthStencil();
		}

		// If it does not exist, create a new one
		if (m_pDepthStencil == 0)
		{
			if( FAILED( pDevice->CreateDepthStencilSurface(m_Size.width, m_Size.height, pp.AutoDepthStencilFormat, pp.MultiSampleType, pp.MultiSampleQuality, true, &m_pDepthStencil, 0 ))) 
			{
				mini3d_assert(false, "Failed to create Depth Surface for Window Render Target!");
			}
		}
	}

	// restore rendertarget if neccessary
	if (setRenderTargetToThis == true)	m_pGraphicsService->SetRenderTarget(this);

	isDirty = false;
}

void WindowRenderTarget_Direct3d9::UnloadResource()
{
	UnloadRenderTarget();
	UnloadDepthStencil();

	isDirty = true;
}

void WindowRenderTarget_Direct3d9::UnloadRenderTarget()
{
	// if we are removing the current render target, restore the default render target first
	if (m_pGraphicsService->GetRenderTarget() == this)
		m_pGraphicsService->SetRenderTarget(0);

	if (m_pScreenRenderTarget != 0)
	{
		m_pRenderTargetSurface->Release();
		m_pRenderTargetSurface = 0;

		m_pScreenRenderTarget->Release();
		m_pScreenRenderTarget = 0;
	}
}

void WindowRenderTarget_Direct3d9::UnloadDepthStencil()
{
	// if we are removing the current render target, restore the default render target first
	if (m_pGraphicsService->GetRenderTarget() == this)
		m_pGraphicsService->SetRenderTarget(0);

	if (m_pDepthStencil != 0)
	{
		m_pDepthStencil->Release();
		m_pDepthStencil = 0;
	}
}

// Window Functions
void WindowRenderTarget_Direct3d9::GetWindowContentSize(const MINI3D_WINDOW window, unsigned int &width, unsigned int &height) const
{
	RECT rect;
	GetClientRect((HWND)window, &rect);

	// get the width and height (must be bigger than 0)
	width = (rect.right - rect.left) | 1;
	height = (rect.bottom - rect.top) | 1;
}


///////// COMPATIBILITY ///////////////////////////////////////////////////////

uint			Compatibility_Direct3d9::TextureStreamCount() const											            { return 4; } // TODO: Determined by pixel shader version!
uint			Compatibility_Direct3d9::MaxTextureSize() const												            { return (m_pDeviceCaps->MaxTextureHeight < m_pDeviceCaps->MaxTextureWidth) ? m_pDeviceCaps->MaxTextureHeight : m_pDeviceCaps->MaxTextureWidth; }
bool			Compatibility_Direct3d9::TextureFormat(BitmapTexture_Direct3d9::Format format) const					{ return true; } // TODO: Fix this
bool			Compatibility_Direct3d9::RenderTargetTextureFormat(RenderTargetTexture_Direct3d9::Format format) const	{ return true; } // TODO: Fix this
const char*		Compatibility_Direct3d9::ShaderLanguage() const												            { return "HLSL"; }
const char*		Compatibility_Direct3d9::PixelShaderVersion() const											            { return m_PixelShaderVersion; }
const char*		Compatibility_Direct3d9::VertexShaderVersion() const										           	{ return m_VertexShaderVersion; }
uint			Compatibility_Direct3d9::VertexStreamCount() const											            { return m_pDeviceCaps->MaxStreams; }
uint			Compatibility_Direct3d9::FreeGraphicsMemory() const											            { return m_pGraphicsService->m_pDevice->GetAvailableTextureMem(); }

Compatibility_Direct3d9::~Compatibility_Direct3d9() { delete m_pDeviceCaps; }

Compatibility_Direct3d9::Compatibility_Direct3d9(GraphicsService_Direct3d9* pGraphicsService)
{
    m_pDeviceCaps = new D3DCAPS9();
	m_pGraphicsService = (GraphicsService_Direct3d9*)pGraphicsService;
	m_pGraphicsService->m_pD3D->GetDeviceCaps(0, D3DDEVTYPE_HAL, m_pDeviceCaps);
	
	UINT ps_major = D3DSHADER_VERSION_MAJOR(m_pDeviceCaps->PixelShaderVersion);
	UINT ps_minor = D3DSHADER_VERSION_MINOR(m_pDeviceCaps->PixelShaderVersion);
	sprintf(m_PixelShaderVersion, "%d.d%", ps_major, ps_minor);

	UINT vs_major = D3DSHADER_VERSION_MAJOR(m_pDeviceCaps->VertexShaderVersion);
	UINT vs_minor = D3DSHADER_VERSION_MINOR(m_pDeviceCaps->VertexShaderVersion);
	sprintf(m_VertexShaderVersion, "%d.d%", vs_major, vs_minor);
}


///////// GRAPHICS SERVICE ////////////////////////////////////////////////////

const ICompatibility* GraphicsService_Direct3d9::GetCompatibility() const { return m_pCompatibility; }

// Pipeline States
void GraphicsService_Direct3d9::SetShaderProgram(IShaderProgram* pShaderProgram) { SetShaderProgram((ShaderProgram_Direct3d9*)pShaderProgram, false); };
void GraphicsService_Direct3d9::SetTexture(ITexture* pTexture, const char* name) { SetTexture(pTexture, name, false); };
void GraphicsService_Direct3d9::SetRenderTarget(IRenderTarget* pRenderTarget) { SetRenderTarget(pRenderTarget, false); };
void GraphicsService_Direct3d9::SetIndexBuffer(IIndexBuffer* pIndexBuffer) { SetIndexBuffer((IndexBuffer_Direct3d9*) pIndexBuffer, false); };
void GraphicsService_Direct3d9::SetVertexBuffer(IVertexBuffer* pVertexBuffer, unsigned int streamIndex) { SetVertexBuffer((VertexBuffer_Direct3d9*)pVertexBuffer, streamIndex, false); };

// Resource Creation (Just a simple pass-through of the arguments to the resource constructors)
IBitmapTexture*	GraphicsService_Direct3d9::CreateBitmapTexture(const void* pBitmap, uint width, uint height, IBitmapTexture::Format format, ITexture::SamplerSettings samplerSettings, ITexture::MipMapMode mipMapMode) { return new BitmapTexture_Direct3d9(this, pBitmap, width, height, format, samplerSettings, mipMapMode); }
IRenderTargetTexture* GraphicsService_Direct3d9::CreateRenderTargetTexture(uint width, uint height, IRenderTargetTexture::Format format, ITexture::SamplerSettings samplerSettings, bool depthTestEnabled, ITexture::MipMapMode mipMapMode) { return new RenderTargetTexture_Direct3d9(this, width, height, format, samplerSettings, depthTestEnabled, mipMapMode); }
IVertexBuffer* GraphicsService_Direct3d9::CreateVertexBuffer(const void* pVertices, uint count, uint vertexSizeInBytes, IVertexBuffer::StreamMode streamMode) { return new VertexBuffer_Direct3d9(this, pVertices, count, vertexSizeInBytes, streamMode); }
IIndexBuffer* GraphicsService_Direct3d9::CreateIndexBuffer(const void* pIndices, uint count, IIndexBuffer::DataType dataType) { return new IndexBuffer_Direct3d9(this, pIndices, count, dataType); }
IPixelShader* GraphicsService_Direct3d9::CreatePixelShader(const char* shaderBytes, unsigned int sizeInBytes) { return new PixelShader_Direct3d9(this, shaderBytes, sizeInBytes); }
IVertexShader* GraphicsService_Direct3d9::CreateVertexShader(const char* shaderBytes, unsigned int sizeInBytes) { return new VertexShader_Direct3d9(this, shaderBytes, sizeInBytes); }
IShaderProgram* GraphicsService_Direct3d9::CreateShaderProgram(IVertexShader* pVertexShader, IPixelShader* pPixelShader) {	return new ShaderProgram_Direct3d9(this, pVertexShader, pPixelShader); }
IWindowRenderTarget* GraphicsService_Direct3d9::CreateWindowRenderTarget(MINI3D_WINDOW window, bool depthTestEnabled) { return new WindowRenderTarget_Direct3d9(this, window, depthTestEnabled); }


GraphicsService_Direct3d9::~GraphicsService_Direct3d9(){ TearDownDevice(); DisposeInternalWindow(); m_pD3D->Release(); }

GraphicsService_Direct3d9::GraphicsService_Direct3d9() 
{
	m_pD3D = 0;
	m_pDevice = 0;
	m_IsDrawingScene = false;
	m_DeviceLost = true;
	m_pCurrentRenderTarget = 0;
	m_pDefaultSwapChain = 0;
	m_pCurrentVertexShader = 0;
	m_ResourceList = 0;

	m_pD3D=Direct3DCreate9(D3D_SDK_VERSION);
	mini3d_assert(m_pD3D != 0, "Failed to create a Direct 3D 9 Device!");

	// Set the texture arrays to correct size
    m_pCompatibility = new Compatibility_Direct3d9(this);

	// Clear the maps for currently bound resources
	for (unsigned int i = 0; i < MAX_VERTEX_BUFFER_SLOTS; ++i)
	{
		m_CurrentIVertexBufferMap[i] = 0;
		m_CurrentITextureMap[i] = 0;
	}

	CreateInternalWindow();
	
	CreateDevice();
	SetCullMode(CULL_COUNTERCLOCKWIZE); // TODO: Move somewhere proper!
}

LRESULT CALLBACK InternalWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_DISPLAYCHANGE:

			GraphicsService_Direct3d9* pGraphicsService = (GraphicsService_Direct3d9*)GetProp(hWnd, L".mini3d_graphicsservice");
			// if we did not find a matching window proc, call default window proc and return result (window is not an internal graphicsservice window)
			if (pGraphicsService == 0) return DefWindowProc(hWnd, msg, wParam, lParam);

			pGraphicsService->RecreateDevice();
			break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void GraphicsService_Direct3d9::CreateInternalWindow()
{
	if (m_Window != 0) DisposeInternalWindow();

	HINSTANCE hInstance = GetModuleHandle(NULL);
	WNDCLASSEX wc = {};
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.lpfnWndProc   = InternalWindowProc;
	wc.hInstance     = hInstance;
	wc.lpszClassName = L"Mini3DInternalWindowClass";
	RegisterClassEx(&wc);
	m_Window = CreateWindowEx(WS_EX_CLIENTEDGE, L"Mini3DInternalWindowClass", L"Mini3DHiddenWindow", 0, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, HWND_MESSAGE, 0, hInstance, 0);

	mini3d_assert(m_Window != 0, "Failed to create an internal window for the Graphics Service!");
	SetProp(m_Window, L".mini3d_graphicsservice", this);
}

void GraphicsService_Direct3d9::DisposeInternalWindow()
{
	RemoveProp(m_Window, L".mini3d_graphicsservice");
	DestroyWindow(m_Window); m_Window = 0;

	HINSTANCE hInstance = GetModuleHandle(0);
	UnregisterClass(L"Mini3DInternalWindowClass", hInstance);
}


// Resource Management
void GraphicsService_Direct3d9::UpdateResources() { for (Resource* resource = m_ResourceList; resource != 0; resource = resource->next) if (resource->isDirty) resource->LoadResource(); }
void GraphicsService_Direct3d9::UnloadResources() { for (Resource* resource = m_ResourceList; resource != 0; resource = resource->next) resource->UnloadResource(); }

void GraphicsService_Direct3d9::AddResource(Resource* resource)
{
	// Intrusive linked list
	resource->next = m_ResourceList;
	if (m_ResourceList) m_ResourceList->prev = resource;
	
	m_ResourceList = resource;
}

void GraphicsService_Direct3d9::RemoveResource(Resource* resource)
{
	if (m_ResourceList == resource) m_ResourceList = resource->next;

	// Intrusive linked list
	if (resource->prev) resource->prev->next = resource->next ? resource->next : 0; 
	if (resource->next) resource->next->prev = resource->prev ? resource->prev : 0; 
	resource->prev = resource->next = 0;
}

void GraphicsService_Direct3d9::RestoreGraphicsState()
{
	// Force set all current resources again
	SetRenderTarget(m_pCurrentRenderTarget, true);
	SetIndexBuffer(m_pCurrentIndexBuffer, true);
	SetPixelShader(m_pCurrentPixelShader, true);
	SetVertexShader(m_pCurrentVertexShader, true);
	SetShaderProgram(m_pCurrentShaderProgram, true);

	uint textureBufferCount = ( m_pCompatibility->TextureStreamCount() < MAX_TEXTURE_BUFFER_SLOTS) ?  m_pCompatibility->TextureStreamCount() : MAX_TEXTURE_BUFFER_SLOTS;
	for (unsigned int i = 0; i < textureBufferCount; ++i) SetTexture(m_CurrentITextureMap[i], i, true);

	uint vertexBufferCount = ( m_pCompatibility->VertexStreamCount() < MAX_VERTEX_BUFFER_SLOTS) ?  m_pCompatibility->VertexStreamCount() : MAX_TEXTURE_BUFFER_SLOTS;
	for (unsigned int i = 0; i < vertexBufferCount; ++i) SetVertexBuffer(m_CurrentIVertexBufferMap[i], i, true);
}

void GraphicsService_Direct3d9::RecreateDevice()
{
	// Tear down the old device
	TearDownDevice();

	// try to recreate the device
	// this will throw an error if the driver is broken so there is no need to do that explicitly
	CreateDevice();

	// Reload the resources
	UpdateResources();

	// put back the stored graphics pipeline states
	RestoreGraphicsState();
}

void GraphicsService_Direct3d9::CreateDevice()
{
	
	// if we are not in fullscreen mode then m_pDefaultSwapChain == 0
	unsigned int msaa = 0;
	D3DPRESENT_PARAMETERS d3dpp = CreatePresentationParameters(m_pD3D, m_Window, 64, 64, msaa);

	if (FAILED(m_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, m_Window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &m_pDevice)))
	{
		if (FAILED(m_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, m_Window, D3DCREATE_MIXED_VERTEXPROCESSING, &d3dpp, &m_pDevice)))
		{
			if (FAILED(m_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, m_Window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pDevice)))
			{				
				mini3d_assert(false,  "Could not find any matching DirectX9 device type. Is hardware missing vertex/pixel shader support?");
			}
		}
	}

	// m_pDefaultSwapChain is 0 if we are not in fullscreen mode
	m_pCurrentRenderTarget = m_pDefaultSwapChain;
	
	// Device created correctly, set device lost to false
	m_DeviceLost = false;

	// store the default back buffer so we can put it back when we reset the device
	m_pDevice->GetRenderTarget(0, &m_pDefaultRenderTarget);

	// get the depthstencil even if it might be 0
	m_pDevice->GetDepthStencilSurface(&m_pDefaultDepthStencilSurface);
}

void GraphicsService_Direct3d9::TearDownDevice() { ReleaseDevice(); m_pDevice->Release(); m_pDevice = 0; }
void GraphicsService_Direct3d9::ResetDevice() { ReleaseDevice(); RestoreDevice(); }

void GraphicsService_Direct3d9::RestoreDevice()
{
	unsigned int msaa = 0;
	D3DPRESENT_PARAMETERS pp = CreatePresentationParameters(m_pD3D, m_Window, 64, 64, msaa);
	int newDeviceState = m_pDevice->Reset(&pp);
		
	// if it was not successful, just return and try agian later
	if (newDeviceState != D3D_OK)
		return;

	// capture the new default render target
	m_pDevice->GetRenderTarget(0, &m_pDefaultRenderTarget);
	m_pDevice->GetDepthStencilSurface(&m_pDefaultDepthStencilSurface);

	//if it was succesful, reload all resources and set device lost to false
	UpdateResources();

	// put back the current graphics pipeline states
	RestoreGraphicsState();

	m_DeviceLost = false;
}

void GraphicsService_Direct3d9::ReleaseDevice()
{
	m_DeviceLost = true;

	// unload all resources
	UnloadResources();

	// Release the default render target 
	if (m_pDefaultRenderTarget != 0)
	{
		// This must be done after UnloadResources since UnloadResources restores
		// the default render target
		m_pDefaultRenderTarget->Release();
		m_pDefaultRenderTarget = 0;
	}

	if (m_pDefaultDepthStencilSurface != 0)
	{
		m_pDefaultDepthStencilSurface->Release();
		m_pDefaultDepthStencilSurface = 0;
	}
}

void GraphicsService_Direct3d9::HandleLostDevice()
{
	// find lost device type
	int deviceState = m_pDevice->TestCooperativeLevel();

	// if it is a driver internal error, just recreate the whole thing and return
	if (deviceState == D3DERR_DRIVERINTERNALERROR)
	{
		RecreateDevice();
		return;
	}

	// check if this is the first time we get the lost device
	if (m_DeviceLost == false)
	{
		ReleaseDevice();
	}
	
	// se if we can recreate the device
	if (deviceState == D3DERR_DEVICENOTRESET)
	{
		RestoreDevice();
	}

	// device is lost but can not yet be recreated.
	if (deviceState == D3DERR_DEVICELOST)
	{
		// do nothing
	}
}

// Locking resources
void GraphicsService_Direct3d9::BeginScene(void)
{
	if (m_IsDrawingScene == true)
		return;

	if (m_pDevice == 0)
		return;

	// check for lost device
	if (m_DeviceLost || m_pDevice->TestCooperativeLevel() != D3D_OK)
	{
		HandleLostDevice();

		//if device is still lost, just return
		if (m_DeviceLost == true)
			return;
	}

	SetRenderStates();
	m_pDevice->BeginScene();
	m_IsDrawingScene = true;
}
void GraphicsService_Direct3d9::EndScene(void)
{
	m_pDevice->EndScene();
	m_IsDrawingScene = false;
}

// setting render states
void GraphicsService_Direct3d9::SetRenderStates()
{
	// Set Render States
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}



// Graphics Pipeline States ---------------------------------------------------

// Shader Program
IShaderProgram* GraphicsService_Direct3d9::GetShaderProgram() const { return m_pCurrentShaderProgram; }

void GraphicsService_Direct3d9::SetShaderProgram(ShaderProgram_Direct3d9* pShaderProgram, bool forced)
{
    if (pShaderProgram != 0)
    {
	    SetVertexShader((VertexShader_Direct3d9*)pShaderProgram->GetVertexShader(), forced);
	    SetPixelShader((PixelShader_Direct3d9*)pShaderProgram->GetPixelShader(), forced);
    }
    else
    {
	    SetVertexShader(0, forced);
	    SetPixelShader(0, forced);
    }

	m_pCurrentShaderProgram = pShaderProgram;
}

void GraphicsService_Direct3d9::SetPixelShader(PixelShader_Direct3d9* pPixelShader, bool forced)
{
	if (m_pCurrentPixelShader == pPixelShader && forced == false) return;

	if (pPixelShader == 0)
	{
		m_pDevice->SetPixelShader(0);
	}
	else
	{
		if (pPixelShader->isDirty) pPixelShader->LoadResource();
		m_pDevice->SetPixelShader((IDirect3DPixelShader9*)pPixelShader->m_pShaderBuffer);
	}

	m_pCurrentPixelShader = pPixelShader;
}

void GraphicsService_Direct3d9::SetVertexShader(VertexShader_Direct3d9* pVertexShader, bool forced)
{
	if (m_pCurrentVertexShader == pVertexShader && forced == false) return;

	if (pVertexShader == 0)
	{
		m_pDevice->SetVertexShader(0);
		m_pDevice->SetVertexDeclaration(0);
	}
	else
	{
		if (pVertexShader->isDirty) pVertexShader->LoadResource();
		m_pDevice->SetVertexShader((IDirect3DVertexShader9*)pVertexShader->m_pShaderBuffer);
	}

	m_pCurrentVertexShader = pVertexShader;
}

// Texture
ITexture* GraphicsService_Direct3d9::GetTexture(unsigned int index) const 
{
	mini3d_assert((index < m_pCompatibility->TextureStreamCount() && index < MAX_TEXTURE_BUFFER_SLOTS), "Trying to access texture outside valid range!");

	return m_CurrentITextureMap[index];
}

void GraphicsService_Direct3d9::SetTexture(ITexture* pTexture, const char* name, bool forced)
{	
	mini3d_assert(m_pCurrentShaderProgram != 0, "Trying to assign a texture to a sampler without having set a shader program!");

	unsigned int index = m_pCurrentPixelShader->GetSamplerIndex(name);
	mini3d_assert(index != -1, "Trying to assign a texture to a sampler that can not be found in the current shader program!");

	SetTexture(pTexture, index, false);
}

void GraphicsService_Direct3d9::SetTexture(ITexture* pTexture, unsigned int index, bool forced)
{	
	mini3d_assert(index < m_pCompatibility->TextureStreamCount() && index < MAX_TEXTURE_BUFFER_SLOTS, "Trying to assign a texture to a sampler slot outside the valid range!");

	// if texture already assigned, then there is no need to re-assign it
	if (m_CurrentITextureMap[index] == pTexture && forced == false) return;

	if (pTexture == 0)
	{
		m_pDevice->SetTexture(index, 0);
		m_CurrentITextureMap[index] = 0;
	}
	else
	{

		BitmapTexture_Direct3d9* pBitmapTexture = dynamic_cast<BitmapTexture_Direct3d9*>(pTexture);
		RenderTargetTexture_Direct3d9* pRenderTargetTexture = dynamic_cast<RenderTargetTexture_Direct3d9*>(pTexture);

		if (pBitmapTexture) { if (pBitmapTexture->isDirty) pBitmapTexture->LoadResource(); }
		else if (pRenderTargetTexture) { if (pRenderTargetTexture->isDirty) pRenderTargetTexture->LoadResource(); }

		// Get the sampler settings from the texture
		ITexture::SamplerSettings samplerSettings = pTexture->GetSamplerSettings();

		// TODO: These should only be set if needed, not every time.

		// Set wrap mode 
		D3DTEXTUREADDRESS adressMode;
		switch(samplerSettings.wrapMode)
		{
			case ITexture::SamplerSettings::WRAP_TILE: adressMode = D3DTADDRESS_WRAP; break;
			case ITexture::SamplerSettings::WRAP_CLAMP: adressMode = D3DTADDRESS_CLAMP; break;
		}

		// set the wrap style
		m_pDevice->SetSamplerState(index, D3DSAMP_ADDRESSU, adressMode);
		m_pDevice->SetSamplerState(index, D3DSAMP_ADDRESSV, adressMode);

		// Set filter mode
		D3DTEXTUREFILTERTYPE filter;

		switch(samplerSettings.sampleMode)
		{
			case ITexture::SamplerSettings::SAMPLE_LINEAR: filter = D3DTEXF_LINEAR; break;
			case ITexture::SamplerSettings::SAMPLE_NEAREST: filter = D3DTEXF_POINT; break;
		}

		// set the wrap style
		m_pDevice->SetSamplerState(index, D3DSAMP_MAGFILTER, filter);
		m_pDevice->SetSamplerState(index, D3DSAMP_MINFILTER, filter);
		m_pDevice->SetSamplerState(index, D3DSAMP_MIPFILTER, filter);

		// set the texture
		if (pBitmapTexture) { m_pDevice->SetTexture(index, pBitmapTexture->m_pTexture); }
		else if (pRenderTargetTexture) { m_pDevice->SetTexture(index, pRenderTargetTexture->m_pRenderTarget); }
		
		// Set the current texture
		m_CurrentITextureMap[index] = pTexture;
	}
}

// Render Target
IRenderTarget* GraphicsService_Direct3d9::GetRenderTarget() const { return m_pCurrentRenderTarget; }

void GraphicsService_Direct3d9::SetRenderTarget(IRenderTarget* pRenderTarget, bool forced)
{
	// Dont set the rendertarget if it is already set
	if (pRenderTarget == m_pCurrentRenderTarget && forced == false) return;

	// ---------- SETTING DEFAULT RENDER TARGET -------------------------------
	if (pRenderTarget == 0)
	{
		// Dont set the rendertarget if it is already set
		if (pRenderTarget == m_pCurrentRenderTarget)
			return;

		m_pDevice->SetRenderTarget(0, m_pDefaultRenderTarget);
		m_pDevice->SetDepthStencilSurface(m_pDefaultDepthStencilSurface);
		
		m_pCurrentRenderTarget = pRenderTarget;
		return;	
	}


	// ---------- SET THE RENDER TARGET ---------------------------------------
	
	WindowRenderTarget_Direct3d9* pWindowRenderTarget = dynamic_cast<WindowRenderTarget_Direct3d9*>(pRenderTarget);
	RenderTargetTexture_Direct3d9* pRenderTargetTexture = dynamic_cast<RenderTargetTexture_Direct3d9*>(pRenderTarget);

	IDirect3DSurface9* pRenderTargetSurface;
	IDirect3DSurface9* pDepthStencilSurface;

	if (pWindowRenderTarget) 
	{ 
		if (pWindowRenderTarget->isDirty) pWindowRenderTarget->LoadResource(); 
		pRenderTargetSurface = pWindowRenderTarget->m_pRenderTargetSurface;
		pDepthStencilSurface = pWindowRenderTarget->m_pDepthStencil;
	}
	else if (pRenderTargetTexture) 
	{ 
		if (pRenderTargetTexture->isDirty == true) pRenderTargetTexture->LoadResource(); 
		pRenderTargetSurface = pRenderTargetTexture->m_pRenderTargetSurface;
		pDepthStencilSurface = pRenderTargetTexture->m_pDepthStencil;
	}
	
	// set the render target
	m_pDevice->SetRenderTarget(0, pRenderTargetSurface);
	m_pCurrentRenderTarget = pRenderTarget;

	// Set the depth stencil if we have one
	if (pRenderTarget->GetDepthTestEnabled() == true)
	{
		m_pDevice->SetDepthStencilSurface(pDepthStencilSurface);

		m_pDevice->SetRenderState(D3DRS_ZENABLE, true);
		m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	}
	else
	{
		m_pDevice->SetDepthStencilSurface(0);

		m_pDevice->SetRenderState(D3DRS_ZENABLE, false);
		m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);
	}
}

// Index Buffer
IIndexBuffer* GraphicsService_Direct3d9::GetIndexBuffer() const {	return m_pCurrentIndexBuffer; }

void GraphicsService_Direct3d9::SetIndexBuffer(IndexBuffer_Direct3d9* pIndexBuffer, bool forced)
{
	if (m_pCurrentIndexBuffer == pIndexBuffer && forced == false) return;

	if (pIndexBuffer == 0)
	{
		m_pDevice->SetIndices(0);
	}
	else
	{
		if (pIndexBuffer->isDirty) pIndexBuffer->LoadResource();

		m_pDevice->SetIndices(pIndexBuffer->m_pIndexBuffer);
	}

	m_pCurrentIndexBuffer = pIndexBuffer;
}

// Vertex Buffer
IVertexBuffer* GraphicsService_Direct3d9::GetVertexBuffer(unsigned int streamIndex) const
{
	mini3d_assert(streamIndex < m_pCompatibility->VertexStreamCount() && streamIndex < MAX_VERTEX_BUFFER_SLOTS, "Trying to get a Vertex Buffer with a stream index outside the valid interval");

	return m_CurrentIVertexBufferMap[streamIndex];
}

void GraphicsService_Direct3d9::SetVertexBuffer(VertexBuffer_Direct3d9* pVertexBuffer, unsigned int streamIndex, bool forced)
{
	mini3d_assert(streamIndex < m_pCompatibility->VertexStreamCount() && streamIndex < MAX_VERTEX_BUFFER_SLOTS, "Trying to set a Vertex Buffer with a stream index outside the valid interval");

	if (m_CurrentIVertexBufferMap[streamIndex] == pVertexBuffer && forced == false)
		return;

	if (pVertexBuffer == 0)
	{
		m_pDevice->SetStreamSource(streamIndex, 0, 0, 0);
		m_pDevice->SetStreamSourceFreq(streamIndex, 0);
		m_CurrentIVertexBufferMap[streamIndex] = 0;
	}
	else
	{
		if (pVertexBuffer->isDirty) pVertexBuffer->LoadResource();

		m_pDevice->SetStreamSource(streamIndex, pVertexBuffer->m_pVertexBuffer, 0, pVertexBuffer->GetVertexSizeInBytes());
		m_CurrentIVertexBufferMap[streamIndex] = pVertexBuffer;
	}
}

// Shader Parameters
void GraphicsService_Direct3d9::SetShaderParameterFloat(const char* name, const float* pData, unsigned int count)
{
	mini3d_assert(m_pCurrentShaderProgram != 0, "Trying to set a Float Shader Parameter without having set a Shader Program!");

	m_pCurrentVertexShader->SetConstantFloatArray(name, pData, count);
}
void GraphicsService_Direct3d9::SetShaderParameterInt(const char* name, const int* pData, unsigned int count)
{
	mini3d_assert(m_pCurrentShaderProgram != 0, "Trying to set an Integer Shader Parameter without having set a Shader Program!");

	m_pCurrentVertexShader->SetConstantIntArray(name, pData, count);
}
void GraphicsService_Direct3d9::SetShaderParameterMatrix4x4(const char* name, const float* pData)
{
	mini3d_assert(m_pCurrentShaderProgram != 0, "Trying to set a Shader Parameter Matrix without having set a Shader Program!");
	
    m_pCurrentVertexShader->SetConstantFloatArray(name, pData, 16);
}

// Set all vertex attributes for all streams with vertex buffers
void GraphicsService_Direct3d9::UpdateVertexDeclaration()
{
	bool activeAttributeIndices[32] = {};
	bool usesInstancing = false;
	unsigned int instanceCount = 0;

	// Find out how many component descriptions there are for all streams
	unsigned int vertexElementCount = 0;
	for (unsigned int streamIndex = 0; streamIndex < 16; ++streamIndex)
	{
		if (m_CurrentIVertexBufferMap[streamIndex] != 0) 
		{
			unsigned int componentDescriptionCount;
			m_CurrentIVertexBufferMap[0]->GetComponentDescriptions(componentDescriptionCount);
			vertexElementCount += componentDescriptionCount;
		}
	}

	D3DVERTEXELEMENT9* vertexElements = new D3DVERTEXELEMENT9[vertexElementCount + 1];

	// Loop over all streams
	for (unsigned int streamIndex = 0; streamIndex < 16; ++streamIndex)
	{
		// Skip empty streams
		if (m_CurrentIVertexBufferMap[streamIndex] == 0)
			continue;

		VertexBuffer_Direct3d9* pVertexBuffer = dynamic_cast<VertexBuffer_Direct3d9*>(m_CurrentIVertexBufferMap[streamIndex]);
		ShaderProgram_Direct3d9* pShaderProgram = dynamic_cast<ShaderProgram_Direct3d9*>(m_pCurrentShaderProgram);

		unsigned int componentDescriptionCount;
		const VertexBuffer_Direct3d9::ComponentDescription* pComponentDescriptions = m_CurrentIVertexBufferMap[0]->GetComponentDescriptions(componentDescriptionCount); 

		unsigned int stride = 0;
		for(unsigned int i = 0; i < componentDescriptionCount; ++i)
			stride += pComponentDescriptions[i].count * 4;
	
		unsigned int offset = 0;
		for(unsigned int i = 0; i < componentDescriptionCount; ++i)
		{
			D3DVERTEXELEMENT9 element;

			VertexShader_Direct3d9* pVertexShader = dynamic_cast<VertexShader_Direct3d9*>(pShaderProgram->GetVertexShader());

			unsigned int shaderSize;
			DWORD* shaderData = (DWORD*)pVertexShader->GetVertexShader(shaderSize);

			unsigned int semanticsCount;
			D3DXGetShaderInputSemantics(shaderData, 0, &semanticsCount);

			D3DXSEMANTIC* semantics = new D3DXSEMANTIC[semanticsCount];
			D3DXGetShaderInputSemantics(shaderData, semantics, &semanticsCount);

			// Find the semantic in the semantics list to get the input index
			int index = -1;
			for (unsigned int j = 0; j < semanticsCount; ++j) 
				if (semantics[j].Usage == pComponentDescriptions[i].usage && semantics[j].UsageIndex == pComponentDescriptions[i].usageIndex) 
					{ index = j; break; }
			// TODO: Check that we actually found an index!
	
			int type;
			switch (pComponentDescriptions[i].type) {
				case VertexBuffer_Direct3d9::DATA_TYPE_FLOAT:
				default:
					type = D3DDECLTYPE_FLOAT1 + (pComponentDescriptions[i].count - 1);
			}

			// Get value for geometry instancing
			unsigned int frequency = 0;
			switch(pVertexBuffer->GetStreamMode()) 
			{
				case VertexBuffer_Direct3d9::STREAM_PER_INSTANCE:
					frequency = 1;
					instanceCount = pVertexBuffer->GetVertexCount(); // TODO: Error if this is set and then reset to something else for the same draw call!
					m_pDevice->SetStreamSourceFreq(streamIndex, (D3DSTREAMSOURCE_INSTANCEDATA | 1));
					usesInstancing = true;
					break;
				case VertexBuffer_Direct3d9::STREAM_PER_VERTEX:
				default:
					m_pDevice->SetStreamSourceFreq(streamIndex, (D3DSTREAMSOURCE_INDEXEDDATA | 1));
			}

			element.Stream = streamIndex;
			element.Offset = offset;
			element.Type = type;
			element.Method = D3DDECLMETHOD_DEFAULT;
			element.Usage = pComponentDescriptions[i].usage;
			element.UsageIndex = pComponentDescriptions[i].usageIndex;

			offset += pComponentDescriptions[i].count * 4;
		
			vertexElements[i] = element;
		}
	}

	// Turn on geometry instancing as needed
	if (usesInstancing) m_pDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | instanceCount); // TODO: This should only be done once... (but most of the time there will only be one instance stream!)

	D3DVERTEXELEMENT9 end = D3DDECL_END();
	vertexElements[vertexElementCount] = end;

	// Create a new vertex declaration from the elements above and set it
	IDirect3DVertexDeclaration9* vertexDeclaration;
	m_pDevice->CreateVertexDeclaration(vertexElements, &vertexDeclaration);
	m_pDevice->SetVertexDeclaration(vertexDeclaration);

	// Release the vertex declaration (if it is not null (was created correctly))
	if (vertexDeclaration) vertexDeclaration->Release();
}

_D3DCULL mini3d_Cullmodes[] = { D3DCULL_CW, D3DCULL_CCW, D3DCULL_NONE };

void GraphicsService_Direct3d9::SetCullMode(CullMode cullMode)
{
	if (cullMode == m_CurrentCullMode) return;
	
	m_pDevice->SetRenderState(D3DRS_CULLMODE, mini3d_Cullmodes[cullMode]);
	m_CurrentCullMode = cullMode;
}

// Drawing
void GraphicsService_Direct3d9::Draw()
{
	BeginScene();

	// Dont draw if in lost state
	if (m_DeviceLost == true)
		return;

	// Vertex buffer in stream 0 must be set
	if (m_CurrentIVertexBufferMap[0] == 0)
		return;

	// there must be an index buffer
	if (m_pCurrentIndexBuffer == 0)
		return;

	// There must be a shader program
	if (m_pCurrentShaderProgram == 0)
		return;

	// Update vertex delcaration as needed
	UpdateVertexDeclaration();

	// Draw the stuff in the buffers
	m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_CurrentIVertexBufferMap[0]->GetVertexCount(), 0, m_pCurrentIndexBuffer->GetIndexCount() / 3);
}

void GraphicsService_Direct3d9::DrawIndices(unsigned int startIndex, unsigned int numIndices)
{
	BeginScene();

	// Dont draw if in lost state
	if (m_DeviceLost == true)
		return;	

	// Vertex buffer in stream 0 must be set
	if (m_CurrentIVertexBufferMap[0] == 0)
		return;

	// there must be an index buffer
	if (m_pCurrentIndexBuffer == 0)
		return;

	// There must be a shader program
	if (m_pCurrentShaderProgram == 0)
		return;

	// Update vertex delcaration as needed
	UpdateVertexDeclaration();

	// Draw the selected set of indices
	m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_CurrentIVertexBufferMap[0]->GetVertexCount(), startIndex, numIndices);
}

// Clear
void GraphicsService_Direct3d9::Clear(float depth)
{
	if (m_pCurrentRenderTarget->GetDepthTestEnabled() == false)
		return;

	m_pDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, depth, 0);
}

void GraphicsService_Direct3d9::Clear(float r, float g, float b, float a)
{
	if (m_DeviceLost == true)
		return;

	// Pack the color into bits.
	unsigned int red = static_cast<unsigned int>(r * 0xff) << 16;
	unsigned int green = static_cast<unsigned int>(g * 0xff) << 8;
	unsigned int blue = static_cast<unsigned int>(b * 0xff);
	unsigned int alpha = static_cast<unsigned int>(a * 0xff) << 24;
	DWORD color = alpha | red | green | blue;

	m_pDevice->Clear(0, 0, D3DCLEAR_TARGET, color, 1.0f, 0);
}

void GraphicsService_Direct3d9::Clear(float r, float g, float b, float a, float depth)
{
	if (m_DeviceLost == true)
		return;

	DWORD flags = D3DCLEAR_TARGET;
	
	// if we have a depthstencil we need to clear that too
	if (m_pCurrentRenderTarget->GetDepthTestEnabled() == true)
		flags |= D3DCLEAR_ZBUFFER;

	// Pack the color into bits.
	unsigned int red = static_cast<unsigned int>(r * 0xff) << 16;
	unsigned int green = static_cast<unsigned int>(g * 0xff) << 8;
	unsigned int blue = static_cast<unsigned int>(b * 0xff);
	unsigned int alpha = static_cast<unsigned int>(a * 0xff) << 24;
	DWORD color = alpha | red | green | blue;

	m_pDevice->Clear(0, 0, flags, color, depth, 0);
}

}

#endif //MINI3D_GRAPHICSSERVICE_DIRECTX_9
#endif