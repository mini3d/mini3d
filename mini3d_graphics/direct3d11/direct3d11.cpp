
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#include "../graphicsservice.hpp"

#ifdef _WIN32
#ifdef MINI3D_GRAPHICSSERVICE_DIRECT3D_11

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <dxgi.h>
#include <cstdio>

void mini3d_assert(bool expression, const char* text, ...);

typedef unsigned int uint;

namespace mini3d {
namespace graphics {

///////// GRAPHCIS SERVICE /////////////////////////////////////////////////////

typedef class GraphicsService_D3D11 : public IGraphicsService
{
public:


///////// INDEX BUFFER ////////////////////////////////////////////////////////


class IndexBuffer_D3D11 : public IIndexBuffer
{
public:

    unsigned int GetIndexCount() const      { return m_indexCount; }
    DataType GetDataType() const            { return m_dataType; }
    ID3D11Buffer* GetBuffer() const         { return m_pBuffer; }
    ~IndexBuffer_D3D11()                    { Unload(); }

    IndexBuffer_D3D11(GraphicsService_D3D11* pGraphicsService, const void* pIndices, unsigned int count, DataType dataType)
    {
        m_pGraphicsService = pGraphicsService; 
        m_pBuffer = 0;
        m_dataType = INT_32;
        m_indexCount = 0;

        SetIndices(pIndices, count, dataType);
    }

    void SetIndices(const void* pIndices, unsigned int count, DataType dataType)
    {
        mini3d_assert(pIndices != 0, "Setting an Index Buffer with a NULL data pointer!");

        /// Allocate buffer on the graphics card and add index data.
        ID3D11Device* pDevice = m_pGraphicsService->GetDevice();

        if (m_pBuffer != 0)
            Unload();

        // If it does not exist, create a new one
        static const unsigned int BYTES_PER_INDEX[] = { 2, 4 }; // Maps to IIndexBuffer::DataType enum
        unsigned int sizeInBytes = BYTES_PER_INDEX[dataType] * count;

        D3D11_BUFFER_DESC desc = { sizeInBytes, D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, 0, mini3d_IndexBuffer_BytesPerIndex[(unsigned int)dataType] };
        mini3d_assert(S_OK(pDevice->CreateBuffer(&desc, pIndices, &m_pBuffer)), "Failed to create Direct3D 11 index buffer");
    
        m_dataType = dataType;
        m_indexCount = count;
    }

    void Unload()
    {
        // if this is the currently loaded index buffer, release it
        if (m_pGraphicsService->GetIndexBuffer() == this)
            m_pGraphicsService->SetIndexBuffer(0);

        m_pBuffer->Release();
        m_pBuffer = 0;
    }

private:
    ID3D11Buffer* m_pBuffer;
    unsigned int m_indexCount;
    DataType m_dataType;
    GraphicsService_D3D11* m_pGraphicsService;
};



///////// VERTEX BUFFER ///////////////////////////////////////////////////////

class VertexBuffer_D3D11 : public IVertexBuffer
{
public:
    unsigned int GetVertexCount() const                                     { return m_vertexCount; }
    unsigned int GetVertexSizeInBytes() const                               { return m_vertexSizeInBytes; }
    StreamMode GetStreamMode() const                                        { return m_streamMode; }
    void SetStreamMode(StreamMode streamMode)                               { m_streamMode = streamMode; }
    unsigned int GetVertexAttributeCount() const                            { return m_attributeCount; }
    void GetVertexAttributes(VertexAttribute* pAttributes) const            { for (unsigned int i = 0; i < m_attributeCount; ++i) pAttributes[i] = m_pAttributes[i]; }
    const VertexAttribute* GetVertexAttributes(unsigned int &count) const   { count = m_attributeCount; return m_pAttributes; }
    ID3D11Buffer* GetBuffer() const                                         { return m_pBuffer; }
    ~VertexBuffer_D3D11(void)                                               { Unload(); }

    VertexBuffer_D3D11(GraphicsService_D3D11* pGraphicsService, const void* pVertices, unsigned int count, unsigned int vertexSizeInBytes, const VertexAttribute* pAttributes, unsigned int attributeCount, StreamMode streamMode)
    {
        m_pGraphicsService = pGraphicsService; 
        m_pBuffer = 0;
        m_pAttributes = 0;
        m_attributeCount = 0;

        SetVertices(pVertices, count, vertexSizeInBytes, pAttributes, attributeCount, streamMode);
    }

    void SetVertices(const void* pVertices, unsigned int vertexCount, unsigned int vertexSizeInBytes, const VertexAttribute* pAttributes, unsigned int attributeCount, StreamMode streamMode)
    {
        mini3d_assert(pVertices != 0, "Setting a Vertex Buffer with a NULL data pointer!");

        ID3D11Device* pDevice = m_pGraphicsService->GetDevice();

        if (m_pBuffer != 0)
            Unload();
        
        // If it does not exist, create a new one
        unsigned int sizeInBytes = vertexCount * vertexSizeInBytes;

        D3D11_BUFFER_DESC desc = { sizeInBytes, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0, vertexSizeInBytes };
        mini3d_assert(S_OK(pDevice->CreateBuffer(&desc, pVertices, &m_pBuffer)), "Failed to create Direct3D 11 index buffer");

        m_vertexCount = vertexCount;
        m_vertexSizeInBytes = vertexSizeInBytes;
        m_streamMode = streamMode;

        if (m_attributeCount != attributeCount)
        {
            delete m_pAttributes;
            m_pAttributes = new VertexAttribute[attributeCount];
        }

        for (unsigned int i = 0; i < attributeCount; ++i) 
                m_pAttributes[i] = pAttributes[i];

        m_attributeCount = attributeCount;
    }

    void Unload(void)
    {
        // if this is the currently loaded vertex buffer, release it
        if (m_pGraphicsService->GetVertexBuffer(0) == this)
            m_pGraphicsService->SetVertexBuffer(0, 0);

        m_pBuffer->Release();
        m_pBuffer = 0;
    }

private:
    unsigned int m_vertexSizeInBytes;
    unsigned int m_vertexCount;
    StreamMode m_streamMode;
    unsigned int m_attributeCount;
    const VertexAttribute* m_pAttributes;
    ID3D11Buffer* m_pBuffer;
    GraphicsService_D3D11* m_pGraphicsService;
};



///////// PIXEL SHADER ////////////////////////////////////////////////////////


class PixelShader_D3D11 : public IPixelShader
{
public:
    ID3D11PixelShader* GetD3D11PixelShader() { return m_pShader; }


    PixelShader_D3D11(GraphicsService_D3D11* pGraphicsService, const char* pShaderBytes, unsigned int sizeInBytes)
    {
        mini3d_assert(pShaderBytes != 0, "Setting a Pixel Shader with a NULL data pointer!");

        m_pGraphicsService = pGraphicsService; 
        ID3D11Device* pDevice = m_pGraphicsService->GetDevice();

        // Get supported vertex shader profile
        D3D_FEATURE_LEVEL level = pDevice->GetFeatureLevel();

        const char* pProfile;
        if (level >= D3D_FEATURE_LEVEL_11_0)
            pProfile = "cs_5_0";
        else if (level >= D3D_FEATURE_LEVEL_10_0)
            pProfile = "cs_4_0";
        else if (level >= D3D_FEATURE_LEVEL_9_2)
            pProfile = "cs_2_0";
        else
            mini3d_assert(false, "Insufficient shader feature level!");

        // comle the shader source
        ID3D10Blob* pError;
        ID3D10Blob* pShaderBlob;
        
        D3DX11CompileFromMemory(pShaderBytes, sizeInBytes, "shader file", NULL, NULL, "main", pProfile, 0, 0, NULL, &pShaderBlob, &pError, NULL);

        mini3d_assert(pError->GetBufferSize() == 0, "Error log when compiling Pixel Shader!\n%s", (char*)pError->GetBufferPointer());

        pError->Release();

        mini3d_assert(S_OK(pDevice->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), NULL, &m_pShader)), "Failed to compile pixel shader!");
        pShaderBlob->Release();           
    }

    ~PixelShader_D3D11()
    {
        // if this is the currently loaded pixel shader, release it
        if (m_pGraphicsService->GetShaderProgram()->GetPixelShader() == this)
            m_pGraphicsService->SetShaderProgram(0);

//        m_pConstantTable->Release();
        m_pShader->Release();
    }

    int GetSamplerIndex(const char* name)
    {
        /*
        D3DXHANDLE handle = m_pConstantTable->GetConstantByName(0, name);
        mini3d_assert(handle != 0, "Trying to query the index for a sampler but the pixel shader does not have any sampler with that name!");

        return m_pConstantTable->GetSamplerIndex(handle);
        */
    }

private:
    ID3D11PixelShader* m_pShader;
    //ID3DConstantTable* m_pConstantTable;
    GraphicsService_D3D11* m_pGraphicsService;
};


///////// VERTEX SHADER ////////////////////////////////////////////////////////

class VertexShader_D3D11 : public IVertexShader
{
public:
    ID3D11VertexShader* GetD3D11VertexShader()  { return m_pShader; }

    VertexShader_D3D11(GraphicsService_D3D11* pGraphicsService, const char* pShaderBytes, unsigned int sizeInBytes, InputAttribute* pAttributes, unsigned int attributeCount)
    {
        mini3d_assert(pShaderBytes != 0, "Setting a Vertex Shader with a NULL data pointer!");

        m_pGraphicsService = pGraphicsService; 
        ID3D11Device* pDevice = m_pGraphicsService->GetDevice();

        mini3d_assert(S_OK(pDevice->CreateVertexShader(pShaderBytes, sizeInBytes, NULL, &m_pShader)), "Failed to compile pixel shader!");
        
        // Create the input layout description
        D3D11_INPUT_ELEMENT_DESC* pDesc = new D3D11_INPUT_ELEMENT_DESC[attributeCount];
        for (unsigned int i = 0; i < attributeCount; ++i)
        {
            
        }

        pDevice->CreateInputLayout(, attributeCount, 
        m_pInputLayout = ID3d11CreateINputLayouyt
        
        delete[] pDesc;
        pShaderBlob->Release();
    }

    ~VertexShader_D3D11()
    {
        // if this is the currently loaded pixel shader, release it
        if (m_pGraphicsService->GetShaderProgram()->GetVertexShader() == this)
            m_pGraphicsService->SetShaderProgram(0);

        //m_pConstantTable->Release();
        m_pShader->Release();
    }

    void SetConstantFloatArray(const char* name, const float* values, const unsigned int count)
    {
        /*
        mini3d_assert(m_pConstantTable != 0, "Trying to set a float array on a vertex shader with no constant table!");
    
        D3DXHANDLE handle = m_pConstantTable->GetConstantByName(0, name);
        mini3d_assert(handle != 0, "Trying to query the handle for a constant but the vertex shader does not have any constant with that name!");

        D3DXCONSTANT_DESC desc;
        UINT cc = 1;
        m_pConstantTable->GetConstantDesc(handle, &desc, &cc);

        HRESULT result = m_pConstantTable->SetFloatArray(m_pGraphicsService->GetDevice(), handle, values, count);
        */
    };

    void SetConstantIntArray(const char* name, const int* values, const unsigned int count)
    {
        /*
        mini3d_assert(m_pConstantTable != 0, "Trying to set an integer array on a vertex shader with no constant table!");
        
        D3DXHANDLE handle = m_pConstantTable->GetConstantByName(0, name);
        mini3d_assert(handle != 0, "Trying to query the handle for a constant but the vertex shader does not have any constant with that name!");

        m_pConstantTable->SetIntArray(m_pGraphicsService->GetDevice(), handle, values, count);
        */
    };

private:
    ID3D11VertexShader* m_pShader;
    ID3D11InputLayout* m_pInputLayout;
    GraphicsService_D3D11* m_pGraphicsService;
};


///////// SHADER PROGRAM ///////////////////////////////////////////////////////

class ShaderProgram_D3D11 : public IShaderProgram
{
public:

    IPixelShader* GetPixelShader() const    { return m_pPixelShader; }
    IVertexShader* GetVertexShader() const  { return m_pVertexShader; }
    ~ShaderProgram_D3D11()                  { }

    ShaderProgram_D3D11(GraphicsService_D3D11* pGraphicsService, IVertexShader* pVertexShader, IPixelShader* pPixelShader)
    {
        m_pGraphicsService = pGraphicsService; 
        m_pVertexShader =  pVertexShader;
        m_pPixelShader = pPixelShader;
    }

private:
    IPixelShader* m_pPixelShader;
    IVertexShader* m_pVertexShader;
    GraphicsService_D3D11* m_pGraphicsService;
};


///////// BITMAP TEXTURE ///////////////////////////////////////////////////////

class BitmapTexture_D3D11 : public IBitmapTexture
{
public:

unsigned int GetWidth() const               { return m_width; };
unsigned int GetHeight() const              { return m_height; };
MipMapMode GetMipMapMode() const            { return m_mipMapMode; };
Format GetFormat() const                    { return m_format; };
SamplerSettings GetSamplerSettings() const  { return m_samplerSettings; };
 ~BitmapTexture_D3D11()                     { Unload();  }

BitmapTexture_D3D11(GraphicsService_D3D11* pGraphicsService, const void* pBitmap, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, MipMapMode mipMapMode)
{
    m_pGraphicsService = pGraphicsService; 
    m_pTexture = 0;
    
    SetBitmap(pBitmap, width, height, format, samplerSettings, mipMapMode);
}

unsigned int GetLevelCount(unsigned int width, unsigned int height)
{
    unsigned int mipMapWidth = width;
    unsigned int mipMapHeight = height;
    unsigned int level = 0;

    while (mipMapWidth > 1 || mipMapHeight > 1)
    {
        ++level;
        mipMapWidth >> 1;
        mipMapHeight >> 1;
    }
    
    return level;
}

void SetBitmap(const void* pBitmap, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, MipMapMode mipMapMode)
{
    mini3d_assert(pBitmap != 0, "Setting a Bitmap Texture with a NULL data pointer!");
    mini3d_assert((width & (width - 1)) == 0, "Setting a Bitmap Texture to a non power of two width!");
    mini3d_assert((height & (height - 1)) == 0, "Setting a Bitmap Texture to a non power of two height!");
    mini3d_assert(width >= 64, "Setting a Bitmap Texture to a height less than 64!");
    mini3d_assert(height >= 64, "Setting a Bitmap Texture to a width less than 64!");

    ID3D11Device* pDevice = m_pGraphicsService->GetDevice();

    Unload();

    static const DXGI_FORMAT DXGI_FORMATS[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_FLOAT };
    static const UINT BYTES_PER_PIXEL[] = { 4, 8, 2, 4 };

    if (mipMapMode == MIPMAP_MANUAL)
    {
        int levelCount = GetLevelCount(width, height);
        D3D11_TEXTURE2D_DESC desc = { width, height, 1, levelCount, DXGI_FORMATS[(unsigned int)format], {1}, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0 };
        D3D11_SUBRESOURCE_DATA* pResourceData = new D3D11_SUBRESOURCE_DATA[levelCount];

        unsigned int mipMapWidth = width;
        unsigned int mipMapHeight = height;
        const char* pSubBitmap = (const char*)pBitmap;

        for (unsigned int i = 0; i < levelCount; ++i)
        {
            width = mipMapWidth;
            height = mipMapHeight;

            pResourceData[i].pSysMem = pSubBitmap;
            pResourceData[i].SysMemPitch = max(mipMapWidth, 1) * BYTES_PER_PIXEL[(unsigned int)format];

            mipMapWidth >>= 1;
            mipMapHeight >>= 1;
        }

        mini3d_assert(S_OK(pDevice->CreateTexture2D(&desc, pResourceData, &m_pTexture)), "Creating Direct3D 11 texture failed!");
        delete pResourceData;
    }
    else
    {
        int levelCount = (mipMapMode != MIPMAP_NONE) ? 0 : 1;
        D3D11_TEXTURE2D_DESC desc = { width, height, levelCount, 1, DXGI_FORMATS[(unsigned int)format], {1}, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0 };
        D3D11_SUBRESOURCE_DATA resourceData = {pBitmap, width * BYTES_PER_PIXEL[(unsigned int)format], 0 };
        mini3d_assert(S_OK(pDevice->CreateTexture2D(&desc, &resourceData, &m_pTexture)), "Creating Direct3D 11 texture failed!");
    }

    D3D11_TEXTURE2D_DESC desc;
    m_pTexture->GetDesc(&desc);

    D3D11_SHADER_RESOURCE_VIEW_DESC rDesc = { desc.Format, D3D_SRV_DIMENSION_TEXTURE2D, { desc.MipLevels, desc.MipLevels -1 }};
    mini3d_assert(S_OK(pDevice->CreateShaderResourceView(m_pTexture, &rDesc, &m_pShaderResourceView)), "Creating Direct3D 11 Texture Shader Resource View failed!");

    m_format = format;
    m_samplerSettings = samplerSettings;
    m_width = width;
    m_height = height;
    m_mipMapMode = mipMapMode;
}


void Unload()
{
    // if we are removing one of the current textures, clear that texture slot first
    for(unsigned int i = 0; i < m_pGraphicsService->m_pCompatibility->TextureStreamCount(); i++)
        if (m_pGraphicsService->GetTexture(i) == this)
            m_pGraphicsService->SetTexture(0, i, false);

    m_pTexture->Release();
    m_pTexture = 0;
}

private:
    unsigned int m_width;
    unsigned int m_height;
	MipMapMode m_mipMapMode;
	SamplerSettings m_samplerSettings;
	Format m_format;

	ID3D11Texture2D* m_pTexture;
    ID3D11ShaderResourceView* m_pShaderResourceView;

	GraphicsService_D3D11* m_pGraphicsService;
};


///////// RENDER TARGET TEXTURE ///////////////////////////////////////////////

class RenderTargetTexture_D3D11 : public IRenderTargetTexture
{
public:

    unsigned int GetWidth() const               { return m_width; }
    unsigned int GetHeight() const              { return m_height; }
    Viewport GetViewport() const                { Viewport v = {0,0,0,0}; return v; } // TODO: This should do something
    void SetViewport(Viewport viewport)         { } // TODO: This should do something

    MipMapMode GetMipMapMode() const            { return m_mipMapMode; }
    Format GetFormat() const                    { return m_format; }
    SamplerSettings GetSamplerSettings() const  { return m_samplerSettings; }
    bool GetDepthTestEnabled() const            { return m_depthTestEnabled; }
    ~RenderTargetTexture_D3D11()                { Unload(); }

    RenderTargetTexture_D3D11(GraphicsService_D3D11* pGraphicsService, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, bool depthTestEnabled, MipMapMode mipMapMode)
    {
        m_pGraphicsService = pGraphicsService; 
        m_pTexture = 0;
        m_pView = 0;
        m_pShaderResourceView = 0;

        SetRenderTargetTexture(width, height, format, samplerSettings, depthTestEnabled, mipMapMode);
    }

    void SetRenderTargetTexture(unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, bool depthTestEnabled, MipMapMode mipMapMode)
    {
        mini3d_assert((width & (width - 1)) == 0, "Setting a Bitmap Texture to a non power of two width!");
        mini3d_assert((height & (height - 1)) == 0, "Setting a Bitmap Texture to a non power of two height!");
        mini3d_assert(width >= 64, "Setting a Bitmap Texture to a height less than 64!");
        mini3d_assert(height >= 64, "Setting a Bitmap Texture to a width less than 64!");
        mini3d_assert(mipMapMode != MIPMAP_MANUAL, "Manual mip-map mode is not available for render target textures!");

        ID3D11Device* pDevice = m_pGraphicsService->GetDevice();

        static const DXGI_FORMAT DXGI_FORMATS[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_FLOAT };

        UINT levelCount = (mipMapMode != MIPMAP_NONE) ? 0 : 1;
        D3D11_TEXTURE2D_DESC textureDesc = { width, height, levelCount, 1, DXGI_FORMATS[(unsigned int)format], {1}, D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_GENERATE_MIPS };
        mini3d_assert(S_OK(pDevice->CreateTexture2D(&textureDesc, NULL, &m_pTexture)), "Creating Direct3D 11 render target texture failed!");

        D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = { textureDesc.Format, D3D11_RTV_DIMENSION_TEXTURE2D }; // Texture2D.MipSlice will be zero initialized
  	    mini3d_assert(S_OK(pDevice->CreateRenderTargetView(m_pTexture, &renderTargetViewDesc, &m_pView)), "Creating Direct3D 11 render target view failed!"); // TODO: desc can be null?

        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = { textureDesc.Format, D3D11_SRV_DIMENSION_TEXTURE2D, { 0, 1 }};
	    mini3d_assert(S_OK(pDevice->CreateShaderResourceView(m_pTexture, &shaderResourceViewDesc, &m_pShaderResourceView)), "Creating Direct3D 11 Render Target Texture Shader Resource View failed!");

        m_width = width;
        m_height = height;
        m_format = format;
        m_samplerSettings = samplerSettings;
        m_depthTestEnabled = depthTestEnabled;
        m_mipMapMode = mipMapMode;
    }


    void Unload()
    {
        // if we are removing the current render target, restore the default render target first
        if (m_pGraphicsService->GetRenderTarget() == this)
            m_pGraphicsService->SetRenderTarget(0);

        // if we are removing one of the current textures, clear that texture slot first
        for(unsigned int i = 0; i < m_pGraphicsService->m_pCompatibility->TextureStreamCount(); i++)
            if (m_pGraphicsService->GetTexture(i) == this)
                m_pGraphicsService->SetTexture(0, i, false);

        m_pTexture->Release();
        m_pView->Release();
        m_pShaderResourceView->Release();
    }

private:
	unsigned int m_width;
    unsigned int m_height;
	MipMapMode m_mipMapMode;
	Format m_format;
	SamplerSettings m_samplerSettings;
	bool m_depthTestEnabled;

    ID3D11Texture2D* m_pTexture;
    ID3D11RenderTargetView* m_pView;
    ID3D11ShaderResourceView* m_pShaderResourceView;

	GraphicsService_D3D11* m_pGraphicsService;
};


///////// WINDOW RENDER TARGET ////////////////////////////////////////////////

class WindowRenderTarget_D3D11 : public IWindowRenderTarget
{
public:

    unsigned int GetWidth() const           { return m_width; } 
    unsigned int GetHeight() const          { return m_height; }
    Viewport GetViewport() const            { Viewport v = {0,0,0,0}; return v; } // TODO: Should Do Something
    void SetViewport(Viewport viewport)     { } // TODO: Should Do Something
    bool GetDepthTestEnabled() const        { return m_depthTestEnabled; };
    void* GetNativeWindow() const           { return m_pNativeWindow; }
    ~WindowRenderTarget_D3D11()             { Unload(); }

    WindowRenderTarget_D3D11(GraphicsService_D3D11* pGraphicsService, void* pNativeWindow, bool depthTestEnabled)
    {
        m_pGraphicsService = pGraphicsService; 
        m_pSwapChain = 0;
        m_screenState = SCREEN_STATE_WINDOWED; 
        m_pNativeWindow = 0;
        m_pDepthStencilTexture = 0;
        m_width = 0;
        m_height = 0;

        // Create the window render target
        SetWindowRenderTarget(pNativeWindow, depthTestEnabled);
    }

    void SetWindowRenderTarget(void* pNativeWindow, bool depthTestEnabled)
    {
        ID3D11Device* pDevice = m_pGraphicsService->GetDevice();

        Unload();

        DXGI_SWAP_CHAIN_DESC desc = { {0, 0, { 60, 1 }, DXGI_FORMAT_R8G8B8A8_UNORM }, {1}, DXGI_USAGE_RENDER_TARGET_OUTPUT, 2, (HWND)pNativeWindow };
        
        IDXGIFactory * pFactory;
        mini3d_assert(S_OK(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory))), "Unable to create DXGI factory!");
        mini3d_assert(S_OK(pFactory->CreateSwapChain(pDevice, &desc, &m_pSwapChain)), "Unable to create swap chain!");

        ID3D11Texture2D* pBackBuffer;
        mini3d_assert(S_OK(m_pSwapChain->GetBuffer(0, __uuidof( ID3D11Texture2D ), (LPVOID*)&pBackBuffer)), "Unable to get back buffer for swap chain!");
        mini3d_assert(S_OK(pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTargetView)), "Unable to create render target view for swap chain back buffer!");

        UpdateSize();

        // Get the size of the client area of the window 
        m_depthTestEnabled = depthTestEnabled;
        m_pNativeWindow = pNativeWindow;
        
        pFactory->Release();
    }

    void UpdateSize()
    {
        unsigned int width, height;
        RECT rect;
        GetClientRect((HWND)m_pNativeWindow, &rect);

        width = rect.right - rect.left;
        height = rect.bottom - rect.top;

        if (width != m_width || height != m_height)
        {
            m_pSwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

            if (m_pDepthStencilTexture != 0)
            {
                m_pDepthStencilTexture->Release();
                m_pDepthStencilView->Release();

                m_pDepthStencilTexture = 0;
                m_pDepthStencilView = 0;
            }

            if (m_depthTestEnabled)
            {
                ID3D11Device* pDevice = m_pGraphicsService->GetDevice();
                D3D11_TEXTURE2D_DESC desc = { width, height, 1, 1, DXGI_FORMAT_D24_UNORM_S8_UINT, {1}, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL };
                mini3d_assert(S_OK(pDevice->CreateTexture2D(&desc, NULL, &m_pDepthStencilTexture)), "Unable to create depth stencil texture!");
            
                D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc = { DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_DSV_DIMENSION_TEXTURE2D }; // Texture2D.MipSlice will be zero initialized
                mini3d_assert(S_OK(pDevice->CreateDepthStencilView(m_pDepthStencilTexture, &viewDesc, &m_pDepthStencilView)), "Unable to create depth stencil view!");
            }

            m_width = width;
            m_height = height;
        }
    }

    void Display()
    {
        /// Make sure we do an endScene before we present (DirectX9 specific).
        if (m_pGraphicsService->GetIsDrawingScene() == true)
            m_pGraphicsService->EndScene();

        m_pSwapChain->Present(1, 0);
    }

    void Unload()
    {
        if (m_pGraphicsService->GetRenderTarget() == this)
            m_pGraphicsService->SetRenderTarget(0);

        if (m_pSwapChain != 0)
        {
            m_pSwapChain->Release();
            m_pRenderTargetView->Release();

            m_pSwapChain = 0;
            m_pRenderTargetView = 0;
        }

        if (m_pDepthStencilTexture != 0)
        {
            m_pDepthStencilTexture->Release();
            m_pDepthStencilView->Release();

            m_pDepthStencilTexture = 0;
            m_pDepthStencilView = 0;        
        }
    }

private:
    unsigned int m_width;
    unsigned int m_height;
    void* m_pNativeWindow;
    bool m_depthTestEnabled;
    ScreenState m_screenState;
    IDXGISwapChain* m_pSwapChain;
    ID3D11RenderTargetView* m_pRenderTargetView;
    ID3D11Texture2D* m_pDepthStencilTexture;
    ID3D11DepthStencilView* m_pDepthStencilView;
    GraphicsService_D3D11* m_pGraphicsService;
};


///////// COMPATIBILITY ///////////////////////////////////////////////////////

class Compatibility_D3D11 : public ICompatibility
{
public:

    uint GetTextureUnitCount() const                                            { return 16; } // TODO: Determined by pixel shader version!
    uint MaxTextureSize() const                                                 { return 4096; }
    bool TextureFormat(IBitmapTexture::Format format) const                     { return true; } // TODO: Fix this
    bool RenderTargetTextureFormat(IRenderTargetTexture::Format format) const   { return true; } // TODO: Fix this
    const char* ShaderLanguage() const                                          { return "HLSL"; }
    const char* PixelShaderVersion() const                                      { return 0; }
    const char* VertexShaderVersion() const                                     { return 0; }
    uint VertexStreamCount() const                                              { return 32; }
    uint  FreeGraphicsMemory() const                                            { return m_pGraphicsService->m_pDevice->GetAvailableTextureMem(); }

    ~Compatibility_D3D11()                                                      { }

    Compatibility_D3D11(GraphicsService_D3D11* pGraphicsService)
    {
    }

	char m_PixelShaderVersion[16];
	char m_VertexShaderVersion[16];

	GraphicsService_D3D11* m_pGraphicsService;
};


///////// GRAPHICS SERVICE ////////////////////////////////////////////////////

    const ICompatibility* GetCompatibility() const                                  { return m_pCompatibility; }

    // Pipeline States
    void SetShaderProgram(IShaderProgram* pShaderProgram)                           { SetShaderProgram((ShaderProgram_D3D11*)pShaderProgram, false); };
    void SetTexture(ITexture* pTexture, const char* name)                           { SetTexture(pTexture, name, false); };
    void SetRenderTarget(IRenderTarget* pRenderTarget)                              { SetRenderTarget(pRenderTarget, false); };
    void SetIndexBuffer(IIndexBuffer* pIndexBuffer)                                 { SetIndexBuffer((IndexBuffer_D3D11*)pIndexBuffer, false); };
    void SetVertexBuffer(IVertexBuffer* pVertexBuffer, unsigned int streamIndex)    { SetVertexBuffer((VertexBuffer_D3D11*)pVertexBuffer, streamIndex, false); };

    ~GraphicsService_D3D11()                                                        { m_pDevice->Release(); m_pContext->Release(); m_pFactory->Release(); }

    GraphicsService_D3D11()
    {
        m_pD3D = 0;
        m_pDevice = 0;
        m_IsDrawingScene = false;
        m_DeviceLost = true;
        m_pCurrentRenderTarget = 0;
        m_pDefaultSwapChain = 0;
        m_pCurrentVertexShader = 0;

        mini3d_assert(S_OK(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&m_pFactory))), "Unable to create DXGI factory!");

        IDXGIAdapter* pAdapter;
        mini3d_assert(S_OK(m_pFactory->EnumAdapters(0, &pAdapter)), "Unable to get default graphics adapter");
    
        D3D_FEATURE_LEVEL m_featureLevel;
        D3D11CreateDevice(pAdapter, D3D_DRIVER_TYPE_HARDWARE, GetModuleHandle(NULL), D3D11_CREATE_DEVICE_SINGLETHREADED, NULL, 0, D3D11_SDK_VERSION, &m_pDevice, &m_featureLevel, &m_pContext); 


        pAdapter->Release();

        // Set the texture arrays to correct size
        m_pCompatibility = new Compatibility_D3D11(this);

        // Clear the maps for currently bound resources
        for (unsigned int i = 0; i < MAX_VERTEX_BUFFER_SLOTS; ++i)
        {
            m_CurrentIVertexBufferMap[i] = 0;
            m_CurrentITextureMap[i] = 0;
        }

        SetCullMode(CULL_COUNTERCLOCKWIZE); // TODO: Move somewhere proper!
    }

    // Locking resources
    void BeginScene(void)
    {
        if (m_IsDrawingScene == true)
            return;

        if (m_pDevice == 0)
            return;

        m_IsDrawingScene = true;
    }
    void EndScene(void)
    {
        m_pDevice->EndScene();
        m_IsDrawingScene = false;
    }



    // Graphics Pipeline States ---------------------------------------------------

    // Shader Program
    IShaderProgram* GetShaderProgram() const { return m_pCurrentShaderProgram; }

    void SetShaderProgram(ShaderProgram_D3D11* pShaderProgram, bool forced)
    {
        if (pShaderProgram != 0)
        {
            PixelShader_D3D11* pPixel = (PixelShader_D3D11*)pShaderProgram->GetPixelShader();
            VertexShader_D3D11* pVertex = (VertexShader_D3D11*)pShaderProgram->GetVertexShader();

            if (m_pCurrentPixelShader != pPixel)
            {
                m_pContext->PSSetShader(pPixel->GetD3D11PixelShader(), NULL, 0);
                m_pCurrentPixelShader = pPixel;
            }

            if (m_pCurrentVertexShader != pVertex)
            {
                m_pContext->VSSetShader(pVertex->GetD3D11VertexShader(), NULL, 0);
                m_pCurrentVertexShader = pVertex;
            }
        }
        else
        {
            if (m_pCurrentPixelShader != 0)
            {
                m_pContext->PSSetShader(0, NULL, 0);
                m_pCurrentPixelShader = 0;
            }

            if (m_pCurrentVertexShader != 0)
            {
                m_pContext->VSSetShader(0, NULL, 0);
                m_pCurrentVertexShader = 0;
            }
        }

        m_pCurrentShaderProgram = pShaderProgram;
    }

    // Texture
    ITexture* GetTexture(unsigned int index) const 
    {
        mini3d_assert((index < m_pCompatibility->GetTextureUnitCount() && index < MAX_TEXTURE_BUFFER_SLOTS), "Trying to access texture outside valid range!");

        return m_CurrentITextureMap[index];
    }

    void SetTexture(ITexture* pTexture, const char* name)
    {    
        mini3d_assert(m_pCurrentShaderProgram != 0, "Trying to assign a texture to a sampler without having set a shader program!");

        unsigned int index = m_pCurrentPixelShader->GetSamplerIndex(name);
        mini3d_assert(index != -1, "Trying to assign a texture to a sampler that can not be found in the current shader program!");

        SetTexture(pTexture, index);
    }

    void SetTexture(ITexture* pTexture, unsigned int index)
    {    
        mini3d_assert(index < m_pCompatibility->GetTextureUnitCount() && index < MAX_TEXTURE_BUFFER_SLOTS, "Trying to assign a texture to a sampler slot outside the valid range!");

        // if texture already assigned, then there is no need to re-assign it
        if (m_CurrentITextureMap[index] == pTexture) 
            return;

        if (pTexture == 0)
        {
            // TODO: Does the NULL trick below work or does it have to be a list?
            //static const ID3D11ShaderResourceView* NULL_LIST[] = { NULL };
            m_pContext->PSSetShaderResources(index, 1, NULL);
            m_pContext->PSSetSamplers(index, 1, NULL);

            m_CurrentITextureMap[index] = 0;
        }
        else
        {
            BitmapTexture_D3D11* pBitmapTexture = dynamic_cast<BitmapTexture_D3D11*>(pTexture);
            RenderTargetTexture_D3D11* pRenderTargetTexture = dynamic_cast<RenderTargetTexture_D3D11*>(pTexture);

            /*
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
            */

            // set the texture
            if (pBitmapTexture) 
            { 
                m_pContext->PSSetShaderResources(index, 1, &pBitmapTexture->GetTextureResourceView());
                // TODO: Set proper sampler settings
                m_pContext->PSSetSamplers(index, 1, 0);
            }
            else if (pRenderTargetTexture) 
            { 
                m_pDevice->SetTexture(index, pRenderTargetTexture->GetTextureResourceView()); 
                // TODO: Set proper sampler settings
                m_pContext->PSSetSamplers(index, 1, 0);
            }
        
            // Set the current texture
            m_CurrentITextureMap[index] = pTexture;
        }
    }

    // Render Target
    IRenderTarget* GetRenderTarget() const { return m_pCurrentRenderTarget; }

    void SetRenderTarget(IRenderTarget* pRenderTarget)
    {
        // Dont set the rendertarget if it is already set
        if (pRenderTarget == m_pCurrentRenderTarget) 
            return;

        if (pRenderTarget == 0)
        {
            m_pContext->OMSetRenderTargets(1, 0, 0);
            m_pCurrentRenderTarget = 0;
            return;
        }

        IRenderTarget_D3D11* pRenderTargetD3D11 = (IRenderTarget_D3D11*)pRenderTarget;
        m_pContext->OMSetRenderTargets(1, &pRenderTargetD3D11->GetRenderTargetView(), &pRenderTargetD3D11->GetDepthStencilView());
        
        m_pCurrentRenderTarget = pRenderTarget;
    }

    // Index Buffer
    IIndexBuffer* GetIndexBuffer() const { return m_pCurrentIndexBuffer; }

    void SetIndexBuffer(IndexBuffer_D3D11* pIndexBuffer)
    {
        if (m_pCurrentIndexBuffer == pIndexBuffer) 
            return;

        const DXGI_FORMAT FORMATS[] = { DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R32_UINT }; // Maps to IIndexBuffer::DataType

        if (pIndexBuffer == 0)
            m_pContext->IASetIndexBuffer(0, DXGI_FORMAT_R16_UINT, 0);
        else
            m_pContext->IASetIndexBuffer(pIndexBuffer->GetBuffer(), FORMATS[pIndexBuffer->GetDataType()], 0);

        m_pCurrentIndexBuffer = pIndexBuffer;
    }

    // Vertex Buffer
    IVertexBuffer* GetVertexBuffer(unsigned int streamIndex) const
    {
        mini3d_assert(streamIndex < m_pCompatibility->VertexStreamCount() && streamIndex < MAX_VERTEX_BUFFER_SLOTS, "Trying to get a Vertex Buffer with a stream index outside the valid interval");

        return m_CurrentIVertexBufferMap[streamIndex];
    }

    void SetVertexBuffer(VertexBuffer_D3D11* pVertexBuffer, unsigned int streamIndex, bool forced)
    {
        mini3d_assert(streamIndex < m_pCompatibility->VertexStreamCount() && streamIndex < MAX_VERTEX_BUFFER_SLOTS, "Trying to set a Vertex Buffer with a stream index outside the valid interval");

        if (m_CurrentIVertexBufferMap[streamIndex] == pVertexBuffer && forced == false)
            return;

        const UINT ZERO_ARRAY[] = {0};

        if (pVertexBuffer == 0)
        {
            ID3D11Buffer* pBuffer = 0;
            m_pContext->IASetVertexBuffers(0, 1, &pBuffer, ZERO_ARRAY, ZERO_ARRAY);
            m_CurrentIVertexBufferMap[streamIndex] = 0;
        }
        else
        {
            UINT stride = pVertexBuffer->GetVertexSizeInBytes();
            ID3D11Buffer* pBuffer = pVertexBuffer->GetBuffer();
            m_pContext->IASetVertexBuffers(0, 1, &pBuffer, &stride, ZERO_ARRAY);
            m_CurrentIVertexBufferMap[streamIndex] = pVertexBuffer;
        }
    }

    // Shader Parameters
    void SetShaderParameterFloat(const char* name, const float* pData, unsigned int count)
    {
        mini3d_assert(m_pCurrentShaderProgram != 0, "Trying to set a Float Shader Parameter without having set a Shader Program!");

        m_pCurrentVertexShader->SetConstantFloatArray(name, pData, count);
    }
    void SetShaderParameterInt(const char* name, const int* pData, unsigned int count)
    {
        mini3d_assert(m_pCurrentShaderProgram != 0, "Trying to set an Integer Shader Parameter without having set a Shader Program!");

        m_pCurrentVertexShader->SetConstantIntArray(name, pData, count);
    }
    void SetShaderParameterMatrix4x4(const char* name, const float* pData)
    {
        mini3d_assert(m_pCurrentShaderProgram != 0, "Trying to set a Shader Parameter Matrix without having set a Shader Program!");
    
        m_pCurrentVertexShader->SetConstantFloatArray(name, pData, 16);
    }

    // Set all vertex attributes for all streams with vertex buffers
    void UpdateVertexDeclaration()
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

            VertexBuffer_D3D11* pVertexBuffer = dynamic_cast<VertexBuffer_D3D11*>(m_CurrentIVertexBufferMap[streamIndex]);
            ShaderProgram_D3D11* pShaderProgram = dynamic_cast<ShaderProgram_D3D11*>(m_pCurrentShaderProgram);

            unsigned int componentDescriptionCount;
            const VertexBuffer_D3D11::ComponentDescription* pComponentDescriptions = m_CurrentIVertexBufferMap[0]->GetComponentDescriptions(componentDescriptionCount); 

            unsigned int stride = 0;
            for(unsigned int i = 0; i < componentDescriptionCount; ++i)
                stride += pComponentDescriptions[i].count * 4;
    
            unsigned int offset = 0;
            for(unsigned int i = 0; i < componentDescriptionCount; ++i)
            {
                D3DVERTEXELEMENT9 element;

                VertexShader_D3D11* pVertexShader = dynamic_cast<VertexShader_D3D11*>(pShaderProgram->GetVertexShader());

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
                    case VertexBuffer_D3D11::DATA_TYPE_FLOAT:
                    default:
                        type = D3DDECLTYPE_FLOAT1 + (pComponentDescriptions[i].count - 1);
                }

                // Get value for geometry instancing
                unsigned int frequency = 0;
                switch(pVertexBuffer->GetStreamMode()) 
                {
                    case VertexBuffer_D3D11::STREAM_PER_INSTANCE:
                        frequency = 1;
                        instanceCount = pVertexBuffer->GetVertexCount(); // TODO: Error if this is set and then reset to something else for the same draw call!
                        m_pDevice->SetStreamSourceFreq(streamIndex, (D3DSTREAMSOURCE_INSTANCEDATA | 1));
                        usesInstancing = true;
                        break;
                    case VertexBuffer_D3D11::STREAM_PER_VERTEX:
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

    void SetCullMode(CullMode cullMode)
    {
        if (cullMode == m_CurrentCullMode) return;
    
        m_pDevice->SetRenderState(D3DRS_CULLMODE, mini3d_Cullmodes[cullMode]);
        m_CurrentCullMode = cullMode;
    }

    // Drawing
    void Draw()
    {
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
        m_pContext->DrawAuto();
    }

    // Clear
    void Clear(float depth)
    {
        if (m_pCurrentRenderTarget->GetDepthTestEnabled() == false)
            return;

        m_pContext->ClearDepthStencilView(m_pCurrentRenderTarget->GetDepthStencilView(), D3D11_CLEAR_DEPTH, depth, 0);
    }

    void Clear(float r, float g, float b, float a)
    {
        FLOAT color[] = { r, g, b, a };
        m_pContext->ClearRenderTargetView(m_pCurrentRenderTarget->GetRenderTargetView(), color);
    }

    void Clear(float r, float g, float b, float a, float depth)
    {
        // if we have a depthstencil we need to clear that too
        if (m_pCurrentRenderTarget->GetDepthTestEnabled() == true)
            m_pContext->ClearDepthStencilView(m_pCurrentRenderTarget->GetDepthStencilView(), D3D11_CLEAR_DEPTH, depth, 0);

        FLOAT color[] = { r, g, b, a };
        m_pContext->ClearRenderTargetView(m_pCurrentRenderTarget->GetRenderTargetView(), color);
    }

private:
	IRenderTarget* m_pCurrentRenderTarget;
	VertexBuffer_D3D11* m_CurrentIVertexBufferMap[MAX_VERTEX_BUFFER_SLOTS];
	IndexBuffer_D3D11* m_pCurrentIndexBuffer;
	PixelShader_D3D11* m_pCurrentPixelShader;
	VertexShader_D3D11* m_pCurrentVertexShader;
	ShaderProgram_D3D11* m_pCurrentShaderProgram;
	ITexture* m_CurrentITextureMap[MAX_TEXTURE_BUFFER_SLOTS];
    CullMode m_CurrentCullMode;
	
	ICompatibility* m_pCompatibility;
    D3D_FEATURE_LEVEL m_featureLevel;
    IDXGIFactory* m_pFactory;
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pContext;

};

}
}

#endif //MINI3D_GRAPHICSSERVICE_DIRECTX_9
#endif