
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#include "../../graphics.hpp"

#ifdef _WIN32
#ifdef MINI3D_GRAPHICSSERVICE_DIRECT3D_11

#include "../common/mipmap.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3dx11.h>
#include <D3Dcompiler.h>
#include <dxgi.h>
#include <cstdio>

void mini3d_assert(bool expression, const char* text, ...);

typedef unsigned int uint;

// TODO: Remove
const unsigned int MAX_VERTEX_BUFFER_SLOTS = 32;
const unsigned int MAX_TEXTURE_BUFFER_SLOTS = 32;

namespace mini3d {
namespace graphics {

///////// GRAPHCIS SERVICE /////////////////////////////////////////////////////

class GraphicsService_D3D11 : public IGraphicsService
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

    IndexBuffer_D3D11(GraphicsService_D3D11* pGraphicsService, const char* pIndices, unsigned int sizeInBytes, DataType dataType)
    {
        m_pGraphicsService = pGraphicsService; 
        m_pBuffer = 0;

        SetIndices(pIndices, sizeInBytes, dataType);
    }

    void SetIndices(const void* pIndices, unsigned int sizeInBytes, DataType dataType)
    {
        mini3d_assert(pIndices != 0, "Setting an Index Buffer with a NULL data pointer!");

        /// Allocate buffer on the graphics card and add index data.
        ID3D11Device* pDevice = m_pGraphicsService->GetDevice();

        if (m_pBuffer != 0)
            Unload();

        // If it does not exist, create a new one
        static const unsigned int BYTES_PER_INDEX[] = { 2, 4 }; // Maps to IIndexBuffer::DataType enum

        D3D11_BUFFER_DESC desc = { sizeInBytes, D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, 0, 0 };
        D3D11_SUBRESOURCE_DATA pData = {pIndices}; // TODO: set pitch?
        mini3d_assert(S_OK == pDevice->CreateBuffer(&desc, &pData, &m_pBuffer), "Failed to create Direct3D 11 index buffer");
    
        m_dataType = dataType;
        m_indexCount = count / BYTES_PER_INDEX[dataType];
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
    ID3D11Buffer* GetBuffer() const                                         { return m_pBuffer; }
    ~VertexBuffer_D3D11(void)                                               { Unload(); }

    VertexBuffer_D3D11(GraphicsService_D3D11* pGraphicsService, const char* pVertices, unsigned int sizeInBytes, unsigned int vertexSizeInBytes)
    {
        m_pGraphicsService = pGraphicsService; 
        m_pBuffer = 0;

        SetVertices(pVertices, sizeInBytes, vertexSizeInBytes);
    }

    void SetVertices(const char* pVertices, unsigned int sizeInBytes, unsigned int vertexSizeInBytes)
    {
        mini3d_assert(pVertices != 0, "Setting a Vertex Buffer with a NULL data pointer!");

        ID3D11Device* pDevice = m_pGraphicsService->GetDevice();

        if (m_pBuffer != 0)
            Unload();
        
        // If it does not exist, create a new one
        D3D11_BUFFER_DESC desc = { sizeInBytes, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
        D3D11_SUBRESOURCE_DATA pData = {pVertices}; // TODO: set pitch?
        mini3d_assert(S_OK == pDevice->CreateBuffer(&desc, &pData, &m_pBuffer), "Failed to create Direct3D 11 vertex buffer");

        m_vertexCount = sizeInBytes / vertexSizeInBytes;
        m_vertexSizeInBytes = vertexSizeInBytes;
    }

    void Unload(void)
    {
        // if this is the currently loaded vertex buffer, release it
        // TODO: need to check all active slots!
        if (m_pGraphicsService->GetVertexBuffer(0) == this)
            m_pGraphicsService->SetVertexBuffer(0, 0);

        m_pBuffer->Release();
        m_pBuffer = 0;
    }

private:
    unsigned int m_vertexSizeInBytes;
    unsigned int m_vertexCount;
    ID3D11Buffer* m_pBuffer;
    GraphicsService_D3D11* m_pGraphicsService;
};



///////// PIXEL SHADER ////////////////////////////////////////////////////////

class PixelShader_D3D11 : public IPixelShader
{
public:
    ID3D11PixelShader* GetD3D11PixelShader() { return m_pShader; }

    PixelShader_D3D11(GraphicsService_D3D11* pGraphicsService, const char* pShaderBytes, unsigned int sizeInBytes, bool precompiled)
    {
        mini3d_assert(pShaderBytes != 0, "Setting a Pixel Shader with a NULL data pointer!");

        m_pGraphicsService = pGraphicsService; 
        ID3D11Device* pDevice = m_pGraphicsService->GetDevice();

        if (precompiled)
            mini3d_assert(S_OK == pDevice->CreatePixelShader(pShaderBytes, sizeInBytes, NULL, &m_pShader), "Failed to create pixel shader!");
        else
        {
            // Get supported vertex shader profile
            D3D_FEATURE_LEVEL level = pDevice->GetFeatureLevel();

            const char* pProfile;
            if (level >= D3D_FEATURE_LEVEL_11_0)
                pProfile = "ps_5_0";
            else if (level >= D3D_FEATURE_LEVEL_10_0)
                pProfile = "ps_4_0";
            else if (level >= D3D_FEATURE_LEVEL_9_2)
                pProfile = "ps_2_0";
            else
                mini3d_assert(false, "Insufficient shader feature level!");

            // compile the shader source
            ID3D10Blob* pError;
            ID3D10Blob* pShaderBlob;
        
            D3DCompile(pShaderBytes, sizeInBytes, "pixelshader.hlsl", NULL, NULL, "main", pProfile, D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY, 0, &pShaderBlob, &pError);

            // TODO: Fix
            if (pError != 0)
            {
                mini3d_assert(pError->GetBufferSize() < 2, "Error log when compiling Pixel Shader!\n%s", (char*)pError->GetBufferPointer());
                pError->Release();
            }

            mini3d_assert(S_OK == pDevice->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), NULL, &m_pShader), "Failed to create pixel shader!");
            pShaderBlob->Release();
        }
    }

    ~PixelShader_D3D11()
    {
        // if this is the currently loaded pixel shader, release it
        if (m_pGraphicsService->GetShaderProgram()->GetPixelShader() == this)
            m_pGraphicsService->SetShaderProgram(0);

        m_pShader->Release();
    }

private:
    ID3D11PixelShader* m_pShader;
    GraphicsService_D3D11* m_pGraphicsService;
};


///////// VERTEX SHADER ////////////////////////////////////////////////////////

class VertexShader_D3D11 : public IVertexShader
{
public:
    const char* GetShaderBytes()                { return m_pShaderBytes; }
    SIZE_T GetSizeInBytes()                     { return m_sizeInBytes; }
    ID3D11VertexShader* GetD3D11VertexShader()  { return m_pShader; }

    VertexShader_D3D11(GraphicsService_D3D11* pGraphicsService, const char* pShaderBytes, unsigned int sizeInBytes, bool precompiled)
    {
        mini3d_assert(pShaderBytes != 0, "Setting a Vertex Shader with a NULL data pointer!");

        m_pGraphicsService = pGraphicsService;
        ID3D11Device* pDevice = m_pGraphicsService->GetDevice();

        if (precompiled)
        {
            mini3d_assert(S_OK == pDevice->CreateVertexShader(pShaderBytes, sizeInBytes, NULL, &m_pShader), "Failed to create vertex shader!");
            m_pShaderBytes = new char[sizeInBytes];
            memcpy(m_pShaderBytes, pShaderBytes, sizeInBytes);
            m_sizeInBytes = sizeInBytes;
        }
        else
        {
            // Get supported vertex shader profile
            D3D_FEATURE_LEVEL level = pDevice->GetFeatureLevel();

            const char* pProfile;
            if (level >= D3D_FEATURE_LEVEL_11_0)
                pProfile = "vs_5_0";
            else if (level >= D3D_FEATURE_LEVEL_10_0)
                pProfile = "vs_4_0";
            else if (level >= D3D_FEATURE_LEVEL_9_2)
                pProfile = "vs_2_0";
            else
                mini3d_assert(false, "Insufficient shader feature level!");

            // compile the shader source
            ID3D10Blob* pError;
            ID3D10Blob* pShaderBlob;
        
            D3DCompile(pShaderBytes, sizeInBytes, "vertexshader.hlsl", NULL, NULL, "main", pProfile, D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY, 0, &pShaderBlob, &pError);

            // TODO: Fix
            if (pError != 0)
            {
                mini3d_assert(pError->GetBufferSize() < 2, "Error log when compiling vertex shader!\n%s", (char*)pError->GetBufferPointer());
                pError->Release();
            }

            mini3d_assert(S_OK == pDevice->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), NULL, &m_pShader), "Failed to create vertex shader!");

            m_pShaderBytes = new char[pShaderBlob->GetBufferSize()];
            memcpy(m_pShaderBytes, pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize());
            m_sizeInBytes = pShaderBlob->GetBufferSize();
            pShaderBlob->Release();
        }
    }

    ~VertexShader_D3D11()
    {
        // if this is the currently loaded pixel shader, release it
        if (m_pGraphicsService->GetShaderProgram()->GetVertexShader() == this)
            m_pGraphicsService->SetShaderProgram(0);

        //m_pConstantTable->Release();
        m_pShader->Release();
        delete[] m_pShaderBytes;
    }

private:
    char* m_pShaderBytes;
    ID3D11VertexShader* m_pShader;
    unsigned int m_sizeInBytes;
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

///////// SHADER INPUT LAYOUT //////////////////////////////////////////////////

struct ShaderInputLayout_D3D11 : IShaderInputLayout
{
    unsigned int GetInputElementCount() const               { return m_elementCount; }
    ID3D11InputLayout* GetLayout() const                    { return m_pLayout; }

    void GetInputElements(InputElement* pElements) const    { for(unsigned int i = 0; i < m_elementCount; ++i) pElements[i] = m_pElements[i]; };

    ShaderInputLayout_D3D11(GraphicsService_D3D11* pGraphics, IShaderProgram* pShader, InputElement* pElements, unsigned int count)
    {
        m_pGraphicsService = pGraphics;
        ID3D11Device* pDevice = pGraphics->GetDevice();
        VertexShader_D3D11* pD3DShader = (VertexShader_D3D11*)pShader->GetVertexShader();

        static const DXGI_FORMAT DATA_TYPE_DXGI[] = { DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT };

        m_pElements = new InputElement[count];
        D3D11_INPUT_ELEMENT_DESC* pDesc = new D3D11_INPUT_ELEMENT_DESC[count];
        for (unsigned int i = 0; i < count; ++i)
        {
            InputElement* pE = pElements + i;
            D3D11_INPUT_ELEMENT_DESC desc = { pE->semanticHLSL, pE->semanticIndexHLSL, DATA_TYPE_DXGI[pE->type], pE->vertexBufferIndex, pE->offsetInBytes, (D3D11_INPUT_CLASSIFICATION)pE->rate, pE->rate };
            pDesc[i] = desc;
            m_pElements[i] = pElements[i];
        }

        mini3d_assert(S_OK == pDevice->CreateInputLayout(pDesc, count, pD3DShader->GetShaderBytes(), pD3DShader->GetSizeInBytes(), &m_pLayout), "Failed to create input layout!");
        delete[] pDesc;
        
        m_elementCount = count;
    }

    ~ShaderInputLayout_D3D11()
    {
        delete[] m_pElements;
    }

private:

    ID3D11InputLayout* m_pLayout;
    InputElement* m_pElements;
    unsigned int m_elementCount;

    GraphicsService_D3D11* m_pGraphicsService;
};


struct ConstantBuffer_D3D11 : IConstantBuffer
{
    IShaderProgram* GetVertexShader() const { return m_pShader; }
    ID3D11Buffer* GetBuffer()               { return m_pBuffer; };

    ConstantBuffer_D3D11(GraphicsService_D3D11* pGraphics, unsigned int sizeInBytes, IShaderProgram* pShader, const char** names, unsigned int nameCount)
    {
        m_pGraphicsService = pGraphics;

        ID3D11Device* pDevice = pGraphics->GetDevice();

        unsigned int alignedSize = ((sizeInBytes >> 4) << 4) + 0x10; // size must be in even 16 bytes;

        D3D11_BUFFER_DESC desc = { alignedSize, D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE };
        mini3d_assert(S_OK == pDevice->CreateBuffer(&desc, NULL, &m_pBuffer), "Failed to create constants buffer");

        m_sizeInBytes = sizeInBytes;
        m_pShader = pShader;
    }

    ~ConstantBuffer_D3D11() 
    { 
        m_pBuffer->Release();
    }

    void SetData(const char* pData)
    {
        ID3D11DeviceContext* pContext = m_pGraphicsService->GetContext();

        D3D11_MAPPED_SUBRESOURCE resource;
        mini3d_assert(S_OK == pContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource), "Failed to map constant buffer!");
        memcpy(resource.pData, pData, m_sizeInBytes);
        pContext->Unmap(m_pBuffer, 0);
    }

private:
    
    IShaderProgram* m_pShader;
    ID3D11Buffer* m_pBuffer;
    unsigned int m_sizeInBytes;

    GraphicsService_D3D11* m_pGraphicsService;
};


///////// BITMAP TEXTURE ///////////////////////////////////////////////////////

class BitmapTexture_D3D11 : public IBitmapTexture
{
public:

unsigned int GetWidth() const                           { return m_width; }
unsigned int GetHeight() const                          { return m_height; }
Format GetFormat() const                                { return m_format; }
SamplerSettings GetSamplerSettings() const              { return m_samplerSettings; }
ID3D11SamplerState* GetSamplerState() const             { return m_pSamplerState; }
ID3D11ShaderResourceView* GetShaderResourceView() const { return m_pShaderResourceView; }
~BitmapTexture_D3D11()                                  { Unload(); }

BitmapTexture_D3D11(GraphicsService_D3D11* pGraphicsService, const char* pBitmap, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings)
{
    m_pGraphicsService = pGraphicsService; 
    m_pTexture = 0;
    m_pSamplerState = 0;
    
    SetBitmap(pBitmap, width, height, format, samplerSettings);
}

unsigned int GetLevelCount(unsigned int width, unsigned int height)
{
    unsigned int mipMapWidth = width;
    unsigned int mipMapHeight = height;
    unsigned int level = 0;

    while (mipMapWidth > 1 || mipMapHeight > 1)
    {
        ++level;
        mipMapWidth >>= 1;
        mipMapHeight >>= 1;
    }
    
    return level;
}

void SetBitmap(const char* pBitmap, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings)
{
    mini3d_assert(pBitmap != 0, "Setting a Bitmap Texture with a NULL data pointer!");
    mini3d_assert((width & (width - 1)) == 0, "Setting a Bitmap Texture to a non power of two width!");
    mini3d_assert((height & (height - 1)) == 0, "Setting a Bitmap Texture to a non power of two height!");
    mini3d_assert(width >= 64, "Setting a Bitmap Texture to a height less than 64!");
    mini3d_assert(height >= 64, "Setting a Bitmap Texture to a width less than 64!");

    ID3D11Device* pDevice = m_pGraphicsService->GetDevice();
    ID3D11DeviceContext* pContext = m_pGraphicsService->GetContext();

    Unload();

    // Create the texture
    static const DXGI_FORMAT DXGI_FORMATS[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_FLOAT };
    static const UINT BYTES_PER_PIXEL[] = { 4, 8, 2, 4 };

    unsigned int levelCount = GetLevelCount(width, height);

    if (samplerSettings.mipMapMode == SamplerSettings::MIPMAP_MANUAL)
    {
        D3D11_TEXTURE2D_DESC desc = { width, height, levelCount, 1, DXGI_FORMATS[(unsigned int)format], {1}, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0 };
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

        mini3d_assert(S_OK == pDevice->CreateTexture2D(&desc, pResourceData, &m_pTexture), "Creating Direct3D 11 texture failed!");
        delete pResourceData;
    }
    else if (samplerSettings.mipMapMode == SamplerSettings::MIPMAP_NONE)
    {
        levelCount = 1;
        D3D11_TEXTURE2D_DESC desc = { width, height, 1, 1, DXGI_FORMATS[(unsigned int)format], {1}, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0 };
        D3D11_SUBRESOURCE_DATA resourceData = {pBitmap, width * BYTES_PER_PIXEL[(unsigned int)format], 0 };
        mini3d_assert(S_OK == pDevice->CreateTexture2D(&desc, &resourceData, &m_pTexture), "Creating Direct3D 11 texture failed!");
    }
    else // mipMapMode == MIPMAP_AUTOGENERATE
    {
        mini3d_assert(format == FORMAT_RGBA8UI, "Mip auto generation is only available for FORMAT_RGBA8UI");

        D3D11_SUBRESOURCE_DATA* data = new D3D11_SUBRESOURCE_DATA[levelCount];
        D3D11_SUBRESOURCE_DATA full = {pBitmap, width * 4, 0};
        data[0] = full;

        unsigned int mipMapWidth = width;
        unsigned int mipMapHeight = height;
        for (unsigned int i = 1; i < levelCount; ++i)
        {
            unsigned char* pMipMap = mini3d_GenerateMipMapBoxFilter((unsigned char*)data[i - 1].pSysMem, mipMapWidth, mipMapHeight);

            mipMapWidth >>= 1;
	        mipMapHeight >>= 1;

            D3D11_SUBRESOURCE_DATA mip = { pMipMap, mipMapWidth * 4, 0 };
            data[i] = mip;
        }

        D3D11_TEXTURE2D_DESC desc = { width, height, levelCount, 1, DXGI_FORMATS[(unsigned int)format], {1}, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0 };
        mini3d_assert(S_OK == pDevice->CreateTexture2D(&desc, data, &m_pTexture), "Creating Direct3D 11 texture failed!");

        //pContext->UpdateSubresource(m_pTexture, 0, NULL, pBitmap, width * BYTES_PER_PIXEL[(unsigned int)format], 0);

        for (unsigned int i = 1; i < levelCount; ++i)
            delete[] data[i].pSysMem;

        delete[] data;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC rDesc = { DXGI_FORMATS[(unsigned int)format], D3D_SRV_DIMENSION_TEXTURE2D, { 0, levelCount }}; 
    mini3d_assert(S_OK == pDevice->CreateShaderResourceView(m_pTexture, &rDesc, &m_pShaderResourceView), "Creating Direct3D 11 Texture Shader Resource View failed!");

    // Set sampler state
    if (m_pSamplerState != 0)
        m_pSamplerState->Release();
    
    static const D3D11_TEXTURE_ADDRESS_MODE ADRESS_MODE_MAP[] = { D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_WRAP };
    static const D3D11_FILTER FILTER_MODE_MAP[] = { D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_MAG_MIP_LINEAR };

    D3D11_TEXTURE_ADDRESS_MODE adressMode = ADRESS_MODE_MAP[(unsigned int)samplerSettings.wrapMode];
    D3D11_SAMPLER_DESC sampDesc = { FILTER_MODE_MAP[(unsigned int)samplerSettings.sampleMode], adressMode, adressMode, adressMode, 0, 1, D3D11_COMPARISON_LESS, {0}, 0, D3D11_FLOAT32_MAX };
    mini3d_assert(S_OK == pDevice->CreateSamplerState(&sampDesc, &m_pSamplerState), "Error creating sampler state!");

    m_format = format;
    m_samplerSettings = samplerSettings;
    m_width = width;
    m_height = height;
}


void Unload()
{
    // if we are removing one of the current textures, clear that texture slot first
    for(unsigned int i = 0; i < m_pGraphicsService->m_pCompatibility->GetTextureUnitCount(); i++)
        if (m_pGraphicsService->GetTexture(i, "") == this)
            m_pGraphicsService->SetTexture(0, i, 0);

    if (m_pTexture != 0)
    {
        m_pTexture->Release();
        m_pTexture = 0;
    }
}

private:
    unsigned int m_width;
    unsigned int m_height;
	SamplerSettings m_samplerSettings;
	Format m_format;

	ID3D11Texture2D* m_pTexture;
    ID3D11ShaderResourceView* m_pShaderResourceView;
    ID3D11SamplerState* m_pSamplerState;

	GraphicsService_D3D11* m_pGraphicsService;
};


///////// RENDER TARGET TEXTURE ///////////////////////////////////////////////

class RenderTargetTexture_D3D11 : public IRenderTargetTexture
{
public:

    unsigned int GetWidth() const                           { return m_width; }
    unsigned int GetHeight() const                          { return m_height; }
    Viewport GetViewport() const                            { Viewport v = {0,0,0,0}; return v; } // TODO: This should do something
    void SetViewport(Viewport viewport)                     { } // TODO: This should do something

    SamplerSettings GetSamplerSettings() const              { return m_samplerSettings; }
    Format GetFormat() const                                { return m_format; }
    bool GetDepthTestEnabled() const                        { return m_depthTestEnabled; }
    ID3D11ShaderResourceView* GetShaderResourceView() const { return m_pShaderResourceView; }
    ID3D11SamplerState* GetSamplerState() const             { return m_pSamplerState; }

    ID3D11RenderTargetView* GetRenderTargetView() const     { return m_pView; }
    ID3D11DepthStencilView* GetDepthStencilView() const     { return m_pDepthStencilView; }

    D3D11_VIEWPORT GetViewportD3D11()                       { return m_viewportD3D11; }

    ~RenderTargetTexture_D3D11()                            { Unload(); }

    RenderTargetTexture_D3D11(GraphicsService_D3D11* pGraphicsService, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, bool depthTestEnabled)
    {
        m_pGraphicsService = pGraphicsService; 
        m_pTexture = 0;
        m_pView = 0;
        m_pDepthStencilTexture = 0;
        m_pDepthStencilView = 0;
        m_pShaderResourceView = 0;
        m_pSamplerState = 0;

        SetRenderTargetTexture(width, height, format, samplerSettings, depthTestEnabled);
    }

    void SetRenderTargetTexture(unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, bool depthTestEnabled)
    {
        mini3d_assert((width & (width - 1)) == 0, "Setting a Bitmap Texture to a non power of two width!");
        mini3d_assert((height & (height - 1)) == 0, "Setting a Bitmap Texture to a non power of two height!");
        mini3d_assert(width >= 64, "Setting a Bitmap Texture to a height less than 64!");
        mini3d_assert(height >= 64, "Setting a Bitmap Texture to a width less than 64!");
        mini3d_assert(samplerSettings.mipMapMode != SamplerSettings::MIPMAP_MANUAL, "Manual mip-map mode is not available for render target textures!");
        // TODO: Safe release and null!
        if (m_pTexture != 0)
        {
            m_pTexture->Release();
            m_pTexture = 0;
        }

        if (m_pView != 0)
        {
            m_pView->Release();
            m_pView = 0;
        }
        
        if (m_pDepthStencilTexture != 0)
        {
            m_pDepthStencilTexture->Release();
            m_pDepthStencilTexture = 0;
        }

        if (m_pDepthStencilView != 0)
        {
            m_pDepthStencilView->Release();
            m_pDepthStencilView = 0;
        }

        if (m_pShaderResourceView != 0)
        {
            m_pShaderResourceView->Release();
            m_pShaderResourceView = 0;
        }

        ID3D11Device* pDevice = m_pGraphicsService->GetDevice();

        static const DXGI_FORMAT DXGI_FORMATS[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_FLOAT };

        UINT levelCount = (samplerSettings.mipMapMode != SamplerSettings::MIPMAP_NONE) ? 0 : 1;
        D3D11_TEXTURE2D_DESC textureDesc = { width, height, levelCount, 1, DXGI_FORMATS[(unsigned int)format], {1}, D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_GENERATE_MIPS };
        mini3d_assert(S_OK == pDevice->CreateTexture2D(&textureDesc, NULL, &m_pTexture), "Creating Direct3D 11 render target texture failed!");

        D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = { textureDesc.Format, D3D11_RTV_DIMENSION_TEXTURE2D }; // Texture2D.MipSlice will be zero initialized
  	    mini3d_assert(S_OK == pDevice->CreateRenderTargetView(m_pTexture, &renderTargetViewDesc, &m_pView), "Creating Direct3D 11 render target view failed!"); // TODO: desc can be null?

        D3D11_TEXTURE2D_DESC desc;
        m_pTexture->GetDesc(&desc);

        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = { textureDesc.Format, D3D11_SRV_DIMENSION_TEXTURE2D, { 0, desc.MipLevels }};
	    mini3d_assert(S_OK == pDevice->CreateShaderResourceView(m_pTexture, &shaderResourceViewDesc, &m_pShaderResourceView), "Creating Direct3D 11 Render Target Texture Shader Resource View failed!");

        if (depthTestEnabled)
        {
            ID3D11Device* pDevice = m_pGraphicsService->GetDevice();
            D3D11_TEXTURE2D_DESC desc = { width, height, 1, 1, DXGI_FORMAT_D24_UNORM_S8_UINT, {1}, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL };
            mini3d_assert(S_OK == pDevice->CreateTexture2D(&desc, NULL, &m_pDepthStencilTexture), "Unable to create depth stencil texture!");

            D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc = { DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_DSV_DIMENSION_TEXTURE2D }; // Texture2D.MipSlice will be zero initialized
            mini3d_assert(S_OK == pDevice->CreateDepthStencilView(m_pDepthStencilTexture, &viewDesc, &m_pDepthStencilView), "Unable to create depth stencil view!");
        }

        // Set sampler state
        if (m_pSamplerState != 0)
            m_pSamplerState->Release();
    
        static const D3D11_TEXTURE_ADDRESS_MODE ADRESS_MODE_MAP[] = { D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_WRAP };
        static const D3D11_FILTER FILTER_MODE_MAP[] = { D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_MAG_MIP_LINEAR };

        D3D11_TEXTURE_ADDRESS_MODE adressMode = ADRESS_MODE_MAP[(unsigned int)samplerSettings.wrapMode];
        D3D11_SAMPLER_DESC sampDesc = { FILTER_MODE_MAP[(unsigned int)samplerSettings.sampleMode], adressMode, adressMode, adressMode, 0, 1, D3D11_COMPARISON_LESS, {0}, 0, D3D11_FLOAT32_MAX };
        mini3d_assert(S_OK == pDevice->CreateSamplerState(&sampDesc, &m_pSamplerState), "Error creating sampler state!");

        D3D11_VIEWPORT viewport = { 0, 0, (float)width, (float)height, 0 , 1 };
        m_viewportD3D11 = viewport;

        m_width = width;
        m_height = height;
        m_format = format;
        m_samplerSettings = samplerSettings;
        m_depthTestEnabled = depthTestEnabled;
    }

    void Unload()
    {
        // if we are removing the current render target, restore the default render target first
        if (m_pGraphicsService->GetRenderTarget() == this)
            m_pGraphicsService->SetRenderTarget(0);

        // if we are removing one of the current textures, clear that texture slot first
        for(unsigned int i = 0; i < m_pGraphicsService->m_pCompatibility->GetTextureUnitCount(); i++)
            if (m_pGraphicsService->GetTexture(i, "") == this)
                m_pGraphicsService->SetTexture(0, i, "");

        m_pTexture->Release();
        m_pView->Release();
        m_pShaderResourceView->Release();
    }

private:
	unsigned int m_width;
    unsigned int m_height;
	Format m_format;
	SamplerSettings m_samplerSettings;
	bool m_depthTestEnabled;

    ID3D11Texture2D* m_pTexture;
    ID3D11RenderTargetView* m_pView;
    ID3D11SamplerState* m_pSamplerState;
    ID3D11ShaderResourceView* m_pShaderResourceView;
    ID3D11Texture2D* m_pDepthStencilTexture;
    ID3D11DepthStencilView* m_pDepthStencilView;
	GraphicsService_D3D11* m_pGraphicsService;
    D3D11_VIEWPORT m_viewportD3D11;
};


///////// WINDOW RENDER TARGET ////////////////////////////////////////////////

class WindowRenderTarget_D3D11 : public IWindowRenderTarget
{
public:

    unsigned int GetWidth() const                       { return m_width; } 
    unsigned int GetHeight() const                      { return m_height; }
    Viewport GetViewport() const                        { Viewport v = {0,0,0,0}; return v; } // TODO: Should Do Something
    void SetViewport(Viewport viewport)                 { } // TODO: Should Do Something
    bool GetDepthTestEnabled() const                    { return m_depthTestEnabled; };
    void* GetNativeWindow() const                       { return m_pNativeWindow; }
    ID3D11RenderTargetView* GetRenderTargetView() const { return m_pRenderTargetView; }
    ID3D11DepthStencilView* GetDepthStencilView() const { return m_pDepthStencilView; }
    D3D11_VIEWPORT GetViewportD3D11()                   { return m_viewportD3D11; }
    ~WindowRenderTarget_D3D11()                         { Unload(); }

    WindowRenderTarget_D3D11(GraphicsService_D3D11* pGraphicsService, void* pNativeWindow, bool depthTestEnabled)
    {
        m_pGraphicsService = pGraphicsService; 
        m_pSwapChain = 0;
        m_screenState = SCREEN_STATE_WINDOWED; 
        m_pNativeWindow = 0;
        m_pDepthStencilTexture = 0;
        m_pRenderTargetView = 0;
        m_width = 0;
        m_height = 0;

        // Create the window render target
        SetWindowRenderTarget(pNativeWindow, depthTestEnabled);
    }

    void SetWindowRenderTarget(void* pNativeWindow, bool depthTestEnabled)
    {
        ID3D11Device* pDevice = m_pGraphicsService->GetDevice();

        Unload();

        m_pNativeWindow = pNativeWindow;

        DXGI_SWAP_CHAIN_DESC desc = { {m_width, m_height, { 0, 1 }, DXGI_FORMAT_R8G8B8A8_UNORM }, {1}, DXGI_USAGE_RENDER_TARGET_OUTPUT, 2, (HWND)pNativeWindow, TRUE };
        
        IDXGIFactory * pFactory = m_pGraphicsService->GetFactory();
        mini3d_assert(S_OK == pFactory->CreateSwapChain(pDevice, &desc, &m_pSwapChain), "Unable to create swap chain!");

        UpdateSize();

        // Get the size of the client area of the window 
        m_depthTestEnabled = depthTestEnabled;
    }

    void UpdateSize()
    {
        RECT rect;
        GetClientRect((HWND)m_pNativeWindow, &rect);

        unsigned int width = rect.right - rect.left;
        unsigned int height = rect.bottom - rect.top;

        if (width != m_width || height != m_height)
        {
            D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
            m_viewportD3D11 = viewport;

            ID3D11Device* pDevice = m_pGraphicsService->GetDevice();

            if (m_pRenderTargetView)
                m_pRenderTargetView->Release();

            if (m_pSwapChain != 0)
            {
                m_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

                ID3D11Texture2D* pBackBuffer;
                mini3d_assert(S_OK == m_pSwapChain->GetBuffer(0, __uuidof( ID3D11Texture2D ), (LPVOID*)&pBackBuffer), "Unable to get back buffer for swap chain!");
                mini3d_assert(S_OK == pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTargetView), "Unable to create render target view for swap chain back buffer!");
                pBackBuffer->Release();
            }

            if (m_pDepthStencilTexture != 0)
            {
                m_pDepthStencilTexture->Release();
                m_pDepthStencilView->Release();

                m_pDepthStencilTexture = 0;
                m_pDepthStencilView = 0;
            }

            if (m_depthTestEnabled)
            {
                D3D11_TEXTURE2D_DESC desc = { width, height, 1, 1, DXGI_FORMAT_D24_UNORM_S8_UINT, {1}, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL };
                mini3d_assert(S_OK == pDevice->CreateTexture2D(&desc, NULL, &m_pDepthStencilTexture), "Unable to create depth stencil texture!");
            
                D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc = { DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_DSV_DIMENSION_TEXTURE2D }; // Texture2D.MipSlice will be zero initialized
                mini3d_assert(S_OK == pDevice->CreateDepthStencilView(m_pDepthStencilTexture, &viewDesc, &m_pDepthStencilView), "Unable to create depth stencil view!");
            }

            m_width = width;
            m_height = height;

            // If this was the currently bound render target it will have been automatically
            // unbound by direct x. Rebind it!
            if (m_pGraphicsService->GetRenderTarget() == this)
            {
                m_pGraphicsService->SetRenderTarget(0);
                m_pGraphicsService->SetRenderTarget(this);
            }
        }
    }

    void Display()
    {
        HRESULT res = m_pSwapChain->Present(1, 0);
        int j = 0;
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
    D3D11_VIEWPORT m_viewportD3D11;
};


///////// COMPATIBILITY ///////////////////////////////////////////////////////

class Compatibility_D3D11 : public ICompatibility
{
public:

    uint GetTextureUnitCount() const                                            { return 16; } // TODO: Determined by pixel shader version!
    uint MaxTextureSize() const                                                 { return 4096; }
    bool TextureFormat(IBitmapTexture::Format format) const                     { return true; } // TODO: Fix this
    bool RenderTargetTextureFormat(IRenderTargetTexture::Format format) const   { return true; } // TODO: Fix this
    ShaderLanguage GetShaderLanguage() const                                    { return HLSL; }
    const char* PixelShaderVersion() const                                      { return 0; }
    const char* VertexShaderVersion() const                                     { return 0; }
    uint VertexStreamCount() const                                              { return 32; }
    uint  FreeGraphicsMemory() const                                            { return 0; }

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
    ID3D11Device* GetDevice() const                                                 { return m_pDevice; }
    ID3D11DeviceContext* GetContext() const                                         { return m_pContext; }
    IDXGIFactory* GetFactory() const                                                { return m_pFactory; }

    ~GraphicsService_D3D11() { m_pDevice->Release(); m_pContext->Release(); m_pFactory->Release(); }

    GraphicsService_D3D11()
    {
        m_pDevice = 0;
        m_pCurrentRenderTarget = 0;
        m_pCurrentVertexShader = 0;

        mini3d_assert(S_OK == CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&m_pFactory)), "Unable to create DXGI factory!");

        IDXGIAdapter* pAdapter;
        mini3d_assert(S_OK == m_pFactory->EnumAdapters(0, &pAdapter), "Unable to get default graphics adapter");
    
        D3D_FEATURE_LEVEL m_featureLevel;
        mini3d_assert(S_OK == D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG, NULL, 0, D3D11_SDK_VERSION, &m_pDevice, &m_featureLevel, &m_pContext), "Failed to create device!");

        pAdapter->Release();

        // Set the texture arrays to correct size
        m_pCompatibility = new Compatibility_D3D11(this);

        // Clear the maps for currently bound resources
        for (unsigned int i = 0; i < MAX_VERTEX_BUFFER_SLOTS; ++i)
        {
            m_CurrentIVertexBufferMap[i] = 0;
            m_CurrentITextureMap[i] = 0;
        }

        D3D11_RASTERIZER_DESC desc = { D3D11_FILL_SOLID, D3D11_CULL_FRONT, FALSE, 0, 0.0f, 0.0f, TRUE, FALSE, FALSE, FALSE };
        m_pDevice->CreateRasterizerState(&desc, &m_pRasterizerStateCullClockwize);
        D3D11_RASTERIZER_DESC desc2 = { D3D11_FILL_SOLID, D3D11_CULL_BACK, FALSE, 0, 0.0f, 0.0f, TRUE, FALSE, FALSE, FALSE };
        m_pDevice->CreateRasterizerState(&desc2, &m_pRasterizerStateCullCounterClockwize);
        D3D11_RASTERIZER_DESC desc3 = { D3D11_FILL_SOLID, D3D11_CULL_NONE, FALSE, 0, 0.0f, 0.0f, TRUE, FALSE, FALSE, FALSE };
        m_pDevice->CreateRasterizerState(&desc3, &m_pRasterizerStateCullNone);

        SetCullMode(CULL_CLOCKWIZE); // TODO: Move somewhere proper!
    }

    // Graphics Pipeline States ---------------------------------------------------

    // Shader Program
    IShaderProgram* GetShaderProgram() const { return m_pCurrentShaderProgram; }

    void SetShaderProgram(IShaderProgram* pShaderProgram)
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

        m_pCurrentShaderProgram = (ShaderProgram_D3D11*)pShaderProgram;
    }

    // Texture
    ITexture* GetTexture(unsigned int idD3D, const char* nameOGL) const 
    {
        mini3d_assert((idD3D < m_pCompatibility->GetTextureUnitCount() && idD3D < MAX_TEXTURE_BUFFER_SLOTS), "Trying to access texture outside valid range!");

        return m_CurrentITextureMap[idD3D];
    }

    void SetTexture(ITexture* pTexture, unsigned int idD3D, const char* nameOGL)
    {    
        mini3d_assert(m_pCurrentShaderProgram != 0, "Trying to assign a texture to a sampler without having set a shader program!");
        mini3d_assert(idD3D != -1, "Trying to assign a texture to a sampler that can not be found in the current shader program!");

        mini3d_assert(idD3D < m_pCompatibility->GetTextureUnitCount(), "Trying to assign a texture to a sampler slot outside the valid range!");

        // if texture already assigned, then there is no need to re-assign it
        if (m_CurrentITextureMap[idD3D] == pTexture) 
            return;

        if (pTexture == 0)
        {
            // TODO: Does the NULL trick below work or does it have to be a list?
            ID3D11ShaderResourceView* const NULL_LIST[] = { NULL };
            ID3D11SamplerState* const NULL_LIST2[] = { NULL };
            m_pContext->PSSetShaderResources(idD3D, 1, &NULL_LIST[0]);
            m_pContext->PSSetSamplers(idD3D, 1, &NULL_LIST2[0]);

            m_CurrentITextureMap[idD3D] = 0;
            return;
        }

        ID3D11ShaderResourceView* pShaderResourceView;
        ID3D11SamplerState* pSamplerState;

        if (pTexture->GetType() == IBitmapTexture::TYPE)
        {
            pShaderResourceView = ((BitmapTexture_D3D11*)pTexture)->GetShaderResourceView();
            pSamplerState = ((BitmapTexture_D3D11*)pTexture)->GetSamplerState();
        }
        else // IRenderTargetTexture::TYPE
        {
            pShaderResourceView = ((RenderTargetTexture_D3D11*)pTexture)->GetShaderResourceView();

            if (((RenderTargetTexture_D3D11*)pTexture)->GetSamplerSettings().mipMapMode == ITexture::SamplerSettings::MIPMAP_AUTOGENERATE)
                m_pContext->GenerateMips(pShaderResourceView);

            pSamplerState = ((RenderTargetTexture_D3D11*)pTexture)->GetSamplerState();
        }

        m_pContext->PSSetShaderResources(idD3D, 1, &pShaderResourceView);
        m_pContext->PSSetSamplers(idD3D, 1, &pSamplerState);

        // Set the current texture
        m_CurrentITextureMap[idD3D] = pTexture;
    }

    // Render Target
    IRenderTarget* GetRenderTarget() const { return m_pCurrentRenderTarget; }

    void SetRenderTarget(IRenderTarget* pRenderTarget)
    {
        // If we are setting a render target window, make sure it has correct resolutoin
        if (pRenderTarget && pRenderTarget->GetType() == IWindowRenderTarget::TYPE)
            ((WindowRenderTarget_D3D11*)pRenderTarget)->UpdateSize();

        // Dont set the rendertarget if it is already set
        if (pRenderTarget == m_pCurrentRenderTarget) 
            return;

        if (pRenderTarget == 0)
        {
            m_pContext->OMSetRenderTargets(0, 0, 0);
            m_pCurrentRenderTarget = 0;
            return;
        }
        
        ID3D11RenderTargetView* pRTView;
        ID3D11DepthStencilView* pDSView;
        D3D11_VIEWPORT viewport;

        if (pRenderTarget->GetType() == IRenderTargetTexture::TYPE)
        {
            pRTView = ((RenderTargetTexture_D3D11*)pRenderTarget)->GetRenderTargetView();
            pDSView = ((RenderTargetTexture_D3D11*)pRenderTarget)->GetDepthStencilView();
            viewport = ((RenderTargetTexture_D3D11*)pRenderTarget)->GetViewportD3D11();
        }
        else // IWindowRenderTarget::TYPE
        {
            pRTView = ((WindowRenderTarget_D3D11*)pRenderTarget)->GetRenderTargetView();
            pDSView = ((WindowRenderTarget_D3D11*)pRenderTarget)->GetDepthStencilView();
            viewport = ((WindowRenderTarget_D3D11*)pRenderTarget)->GetViewportD3D11();
        }
        
        m_pContext->OMSetRenderTargets(1, &pRTView, pDSView);
        m_pContext->RSSetViewports(1, &viewport);
        m_pCurrentRenderTarget = pRenderTarget;
    }

    // Index Buffer
    IIndexBuffer* GetIndexBuffer() const { return m_pCurrentIndexBuffer; }

    void SetIndexBuffer(IIndexBuffer* pIndexBuffer)
    {
        if (m_pCurrentIndexBuffer == pIndexBuffer) 
            return;

        const DXGI_FORMAT FORMATS[] = { DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R32_UINT }; // Maps to IIndexBuffer::DataType

        if (pIndexBuffer == 0)
            m_pContext->IASetIndexBuffer(0, DXGI_FORMAT_R16_UINT, 0);
        else
            m_pContext->IASetIndexBuffer(((IndexBuffer_D3D11*)pIndexBuffer)->GetBuffer(), FORMATS[pIndexBuffer->GetDataType()], 0);

        m_pCurrentIndexBuffer = (IndexBuffer_D3D11*)pIndexBuffer;
    }

    // Vertex Buffer
    IVertexBuffer* GetVertexBuffer(unsigned int streamIndex) const
    {
        mini3d_assert(streamIndex < m_pCompatibility->VertexStreamCount() && streamIndex < MAX_VERTEX_BUFFER_SLOTS, "Trying to get a Vertex Buffer with a stream index outside the valid interval");

        return m_CurrentIVertexBufferMap[streamIndex];
    }

    void SetVertexBuffer(IVertexBuffer* pVertexBuffer, unsigned int streamIndex)
    {
        mini3d_assert(streamIndex < m_pCompatibility->VertexStreamCount() && streamIndex < MAX_VERTEX_BUFFER_SLOTS, "Trying to set a Vertex Buffer with a stream index outside the valid interval");

        if (m_CurrentIVertexBufferMap[streamIndex] == pVertexBuffer)
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
            ID3D11Buffer* pBuffer = ((VertexBuffer_D3D11*)pVertexBuffer)->GetBuffer();
            m_pContext->IASetVertexBuffers(0, 1, &pBuffer, &stride, ZERO_ARRAY);
            m_CurrentIVertexBufferMap[streamIndex] = (VertexBuffer_D3D11*)pVertexBuffer;
        }
    }

    IConstantBuffer* GetConstantBuffer(unsigned int id, const char* nameOGL)
    {
        // TODO: fix!
        return m_pCurrentConstantBuffer;
    }
    
    void SetConstantBuffer(IConstantBuffer* pBuffer, unsigned int id, const char* nameOGL)
    {
        // TODO: Make this some sort of map!
        ID3D11Buffer* pBufferD3D11 = ((ConstantBuffer_D3D11*)pBuffer)->GetBuffer();
        m_pContext->VSSetConstantBuffers(id, 1, &pBufferD3D11);
        
        m_pCurrentConstantBuffer = (ConstantBuffer_D3D11*)pBuffer;
    }
    
    IShaderInputLayout* GetShaderInputLayout()
    {
        return m_pCurrentShaderInputLayout;
    }
    
    void SetShaderInputLayout(IShaderInputLayout* pLayout)
    {
        m_pContext->IASetInputLayout(((ShaderInputLayout_D3D11*)pLayout)->GetLayout());
        m_pCurrentShaderInputLayout = (ShaderInputLayout_D3D11*)pLayout;
    }

    void SetCullMode(CullMode cullMode)
    {
        if (m_cullMode == cullMode)
            return;
    
        m_cullMode = cullMode;

        if (cullMode == CULL_CLOCKWIZE)
            m_pContext->RSSetState(m_pRasterizerStateCullClockwize);
        else if (cullMode == CULL_COUNTERCLOCKWIZE)
            m_pContext->RSSetState(m_pRasterizerStateCullCounterClockwize);
        else // cullmode == CULL_NONE
            m_pContext->RSSetState(m_pRasterizerStateCullNone);
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

        if (m_pCurrentShaderInputLayout == 0)
            return;

        // Update vertex delcaration as needed
        //UpdateVertexDeclaration();

        // TODO: Where does this go?
        m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Draw the stuff in the buffers
        m_pContext->DrawIndexed(m_pCurrentIndexBuffer->GetIndexCount(), 0, 0);
    }

    // Clear
    void Clear(float depth)
    {
        if (m_pCurrentRenderTarget == 0)
            return;

        if (m_pCurrentRenderTarget->GetDepthTestEnabled() == false)
            return;

        ID3D11DepthStencilView* pDSView;

        if (m_pCurrentRenderTarget->GetType() == IRenderTargetTexture::TYPE)
            pDSView = ((RenderTargetTexture_D3D11*)m_pCurrentRenderTarget)->GetDepthStencilView();
        else // IWindowRenderTarget::TYPE
            pDSView = ((WindowRenderTarget_D3D11*)m_pCurrentRenderTarget)->GetDepthStencilView();

        m_pContext->ClearDepthStencilView(pDSView, D3D11_CLEAR_DEPTH, depth, 0);
    }

    void Clear(float r, float g, float b, float a)
    {
        FLOAT color[] = { r, g, b, a };

        ID3D11RenderTargetView* pRTView;

        if (m_pCurrentRenderTarget->GetType() == IRenderTargetTexture::TYPE)
            pRTView = ((RenderTargetTexture_D3D11*)m_pCurrentRenderTarget)->GetRenderTargetView();
        else // IWindowRenderTarget::TYPE
            pRTView = ((WindowRenderTarget_D3D11*)m_pCurrentRenderTarget)->GetRenderTargetView();

        m_pContext->ClearRenderTargetView(pRTView, color);
    }

    void Clear(float r, float g, float b, float a, float depth)
    {
        Clear(depth);
        Clear(r,g,b,a);
    }

private:

	IRenderTarget* m_pCurrentRenderTarget;
	VertexBuffer_D3D11* m_CurrentIVertexBufferMap[MAX_VERTEX_BUFFER_SLOTS];
	IndexBuffer_D3D11* m_pCurrentIndexBuffer;
	PixelShader_D3D11* m_pCurrentPixelShader;
	VertexShader_D3D11* m_pCurrentVertexShader;
	ShaderProgram_D3D11* m_pCurrentShaderProgram;
    ConstantBuffer_D3D11* m_pCurrentConstantBuffer;
    ShaderInputLayout_D3D11* m_pCurrentShaderInputLayout;

	ITexture* m_CurrentITextureMap[MAX_TEXTURE_BUFFER_SLOTS];
    CullMode m_CurrentCullMode;
	
	ICompatibility* m_pCompatibility;
    D3D_FEATURE_LEVEL m_featureLevel;
    IDXGIFactory* m_pFactory;
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pContext;

    CullMode m_cullMode;
    ID3D11RasterizerState* m_pRasterizerStateCullClockwize;
    ID3D11RasterizerState* m_pRasterizerStateCullCounterClockwize;
    ID3D11RasterizerState* m_pRasterizerStateCullNone;
};

IGraphicsService* IGraphicsService::New() { return new GraphicsService_D3D11(); }

IIndexBuffer* IIndexBuffer::New(IGraphicsService* pGraphics, const char* pIndices, unsigned int sizeInBytes, DataType dataType) { return new GraphicsService_D3D11::IndexBuffer_D3D11((GraphicsService_D3D11*)pGraphics, pIndices, sizeInBytes, dataType); }
IVertexBuffer* IVertexBuffer::New(IGraphicsService* pGraphics, const char* pVertices, unsigned int sizeInBytes, unsigned int vertexSizeInBytes) { return new GraphicsService_D3D11::VertexBuffer_D3D11((GraphicsService_D3D11*)pGraphics, pVertices, sizeInBytes, vertexSizeInBytes); }
IPixelShader* IPixelShader::New(IGraphicsService* pGraphics, const char* pShaderBytes, unsigned int sizeInBytes, bool precompiled) { return new GraphicsService_D3D11::PixelShader_D3D11((GraphicsService_D3D11*)pGraphics, pShaderBytes, sizeInBytes, precompiled); }
IVertexShader* IVertexShader::New(IGraphicsService* pGraphics, const char* pShaderBytes, unsigned int sizeInBytes, bool precompiled) { return new GraphicsService_D3D11::VertexShader_D3D11((GraphicsService_D3D11*)pGraphics, pShaderBytes, sizeInBytes, precompiled); }
IShaderProgram* IShaderProgram::New(IGraphicsService* pGraphics, IVertexShader* pVertexShader, IPixelShader* pPixelShader) { return new GraphicsService_D3D11::ShaderProgram_D3D11((GraphicsService_D3D11*)pGraphics, pVertexShader, pPixelShader); }

IRenderTargetTexture* IRenderTargetTexture::New(IGraphicsService* pGraphics, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, bool depthTestEnabled) { return new GraphicsService_D3D11::RenderTargetTexture_D3D11((GraphicsService_D3D11*)pGraphics, width, height, format, samplerSettings, depthTestEnabled); }
IWindowRenderTarget* IWindowRenderTarget::New(IGraphicsService* pGraphics, void* pNativeWindow, bool depthTestEnabled) { return new GraphicsService_D3D11::WindowRenderTarget_D3D11((GraphicsService_D3D11*)pGraphics, pNativeWindow, depthTestEnabled); }
IBitmapTexture* IBitmapTexture::New(IGraphicsService* pGraphics, const char* pBitmap, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings) { return new GraphicsService_D3D11::BitmapTexture_D3D11((GraphicsService_D3D11*) pGraphics, pBitmap, width, height, format, samplerSettings); }

IConstantBuffer* IConstantBuffer::New(IGraphicsService* pGraphics, unsigned int sizeInBytes, IShaderProgram* pShader, const char** names, unsigned int nameCount) { return new GraphicsService_D3D11::ConstantBuffer_D3D11((GraphicsService_D3D11*)pGraphics, sizeInBytes, pShader, names, nameCount); }
IShaderInputLayout* IShaderInputLayout::New(IGraphicsService* pGraphics, IShaderProgram* pShader, InputElement* pElements, unsigned int count) { return new GraphicsService_D3D11::ShaderInputLayout_D3D11((GraphicsService_D3D11*)pGraphics, pShader, pElements, count); }


}
}

#endif //MINI3D_GRAPHICSSERVICE_DIRECTX_11
#endif