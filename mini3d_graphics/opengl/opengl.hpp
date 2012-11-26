
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_OPENGL_H
#define MINI3D_OPENGL_H

#include "../GraphicsService.hpp"
#ifdef MINI3D_GRAPHICSSERVICE_OPENGL

#include "platform/iplatform.hpp"
#include "platform/iopenglwrapper.hpp"

#include <cstring>

typedef unsigned int uint;

///////// NAMESPACE MINI3D ////////////////////////////////////////////////////

namespace mini3d {

///////// FOREWARD DECLARATIONS ///////////////////////////////////////////////

struct WindowRenderTarget_OpenGL;
struct VertexBuffer_OpenGL;
struct IndexBuffer_OpenGL;
struct ShaderProgram_OpenGL;
struct GraphicsService_OpenGL;

struct Size { uint width; uint height; };


///////// CONSTANTS ///////////////////////////////////////////////////////////

const uint MAX_VERTEX_BUFFER_SLOTS = 16;
const uint MAX_TEXTURE_BUFFER_SLOTS = 16; 
const uint MAX_ACTIVE_ATTRIBUTE_INDICES = 32; // How many attributes you can use in a single shader

const uint LOG_TEXT_MAX_LENGTH = 32768; // Max length of shader log texts


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

	virtual void ExtractResourceFromOpenGL() = 0;

	T* Lock(uint& sizeInBytes, bool readOnly = false)		{ m_IsReadOnly = readOnly; sizeInBytes = m_SizeInBytes; ExtractResourceFromOpenGL(); return m_pResource; }
	bool Unlock()											{ m_IsLocked = false; if (m_IsReadOnly) delete[] m_pResource; return (m_IsReadOnly == false); }
	
	void CopyIn(const void* pResource, uint sizeInBytes)	{ UnloadResource(); Unload(); m_pResource = new T[sizeInBytes / sizeof(T)]; memcpy(m_pResource, pResource, sizeInBytes); m_SizeInBytes = sizeInBytes; isDirty = true; LoadResource(); }
	T* CopyOut(uint& sizeInBytes)							{ ExtractResourceFromOpenGL(); sizeInBytes = m_SizeInBytes; T* outResource = m_pResource; m_pResource = 0; return outResource; }
	void Unload()											{ delete[] m_pResource; m_pResource = 0; }
	
	LockableResource() : m_pResource(0), m_IsLocked(false), m_SizeInBytes(0), m_IsReadOnly(false) {};
	virtual ~LockableResource() { Unload(); };
};


///////// COMPATIBILITY ///////////////////////////////////////////////////////

struct Compatibility_OpenGL : ICompatibility
{
	ICOMPATIBILITY_INTERFACE(;)

	Compatibility_OpenGL(GraphicsService_OpenGL* pGraphicsService);
	~Compatibility_OpenGL();

	unsigned int m_CapabilityMaxTextureSize;
	unsigned int m_CapabilityMaxTextureStreamCount;
	
	const char* m_PixelShaderVersion;
	const char* m_VertexShaderVersion;

	GraphicsService_OpenGL* m_pGraphicsService;
};


///////// GRAPHICS SERVICE ////////////////////////////////////////////////////

typedef struct GraphicsService_OpenGL : IGraphicsService
{
	IGRAPHICSSERVICE_INTERFACE(;)

	GraphicsService_OpenGL();
	~GraphicsService_OpenGL();

	void AddResource(Resource* resource);
	void RemoveResource(Resource* resource);
	void UpdateResources();
	void UnloadResources();

	void RestoreGraphicsState();
	IPlatform* GetPlatform();
    IOpenGlWrapper* GetOpenGlWrapper();
	void SetCullMode(CullMode cullMode, bool force);
	void CreateDevice();
	void CreateInternalWindow();
	void BeginScene();
	void EndScene();
	void UpdateVertexAttributes();

	Resource* m_pResourceList;
	IPlatform* m_pPlatform;
    IOpenGlWrapper* m_pOgl;

	IRenderTarget* m_pCurrentRenderTarget;
	WindowRenderTarget_OpenGL* m_pCurrentWindowRenderTarget;
	VertexBuffer_OpenGL* m_CurrentVertexBufferMap[MAX_VERTEX_BUFFER_SLOTS];
	IndexBuffer_OpenGL* m_pCurrentIndexBuffer;
	ShaderProgram_OpenGL* m_pCurrentShaderProgram;
	ITexture* m_CurrentTextureMap[MAX_TEXTURE_BUFFER_SLOTS];
	const char* m_CurrentTextureSamplerName[MAX_TEXTURE_BUFFER_SLOTS];
	bool m_ActiveAttributeIndices[MAX_ACTIVE_ATTRIBUTE_INDICES];
	bool m_IsUsingInstancedAttributes;
	unsigned int m_InstanceCount;
	CullMode m_CullMode;

	ICompatibility* m_pCompatibility;

} GraphicsService;


///////// INDEX BUFFER ////////////////////////////////////////////////////////

struct IndexBuffer_OpenGL : IIndexBuffer, LockableResource<char>
{
	IINDEXBUFFER_INTERFACE(;)

	IndexBuffer_OpenGL(GraphicsService_OpenGL* pGraphicsService, const void* pIndices, unsigned int count, DataType dataType);
	~IndexBuffer_OpenGL();

	void LoadResource();
	void UnloadResource();
	void ExtractResourceFromOpenGL();

	unsigned int m_IndexCount;
	IIndexBuffer::DataType m_DataType;

	GLuint m_IndexBuffer;
	int m_BufferSizeInBytes;

	GraphicsService_OpenGL* m_pGraphicsService;
};


///////// VERTEX BUFFER ///////////////////////////////////////////////////////

struct VertexBuffer_OpenGL : IVertexBuffer, LockableResource<char>
{
	IVERTEXBUFFER_INTERFACE(;)

	VertexBuffer_OpenGL(GraphicsService_OpenGL* pGraphicsService, const void* pVertices, unsigned int count, unsigned int vertexSizeInBytes, IVertexBuffer::StreamMode streamMode);
	~VertexBuffer_OpenGL();

	void LoadResource();
	void UnloadResource();
	void ExtractResourceFromOpenGL();

	// Vertices
	unsigned int m_VertexSizeInBytes;
	IVertexBuffer::StreamMode m_StreamMode;
	unsigned int m_ComponentDescriptionCount;
	const IVertexBuffer::ComponentDescription* m_pComponentDescriptions;

	// Buffer
	GLuint m_VertexBuffer;
	int m_BufferSizeInBytes;

	GraphicsService_OpenGL* m_pGraphicsService;
};


///////// PIXEL SHADER ////////////////////////////////////////////////////////

struct PixelShader_OpenGL : IPixelShader, LockableResource<char>
{
	IPIXELSHADER_INTERFACE(;)

	PixelShader_OpenGL(GraphicsService_OpenGL* pGraphicsService, const char* pShaderBytes, unsigned int sizeInBytes);
	~PixelShader_OpenGL();

	void LoadResource();
	void UnloadResource();
	void ExtractResourceFromOpenGL();

	void printLog(GLuint obj);

	GLuint m_ShaderBuffer;
	
	GraphicsService_OpenGL* m_pGraphicsService;
};


///////// VERTEX SHADER ////////////////////////////////////////////////////////

struct VertexShader_OpenGL : IVertexShader, LockableResource<char>
{
	IVERTEXSHADER_INTERFACE(;)

	VertexShader_OpenGL(GraphicsService_OpenGL* pGraphicsService, const char* pShaderBytes, unsigned int sizeInBytes);
	~VertexShader_OpenGL();

	void LoadResource();
	void UnloadResource();
	void ExtractResourceFromOpenGL();

	void printLog(GLuint obj);

	GLuint m_ShaderBuffer;
	
	GraphicsService_OpenGL* m_pGraphicsService;
};


///////// SHADER PROGRAM ///////////////////////////////////////////////////////

struct ShaderProgram_OpenGL : IShaderProgram, Resource
{
	ISHADERPROGRAM_INTERFACE(;)

	ShaderProgram_OpenGL(GraphicsService_OpenGL* pGraphicsService, IVertexShader* pVertexShader, IPixelShader* pPixelShader);
	~ShaderProgram_OpenGL();

	void LoadResource();
	void UnloadResource();

	void printLog(GLuint obj);

	PixelShader_OpenGL* m_pPixelShader;
	VertexShader_OpenGL* m_pVertexShader;
	GLuint m_Program;

	GraphicsService_OpenGL* m_pGraphicsService;
};


///////// BITMAP TEXTURE ///////////////////////////////////////////////////////

struct BitmapTexture_OpenGL : IBitmapTexture, LockableResource<char>
{
	IBITMAPTEXTURE_INTERFACE(;)

	BitmapTexture_OpenGL(GraphicsService_OpenGL* pGraphicsService, const void* pBitmap, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, MipMapMode mipMapMode);
	~BitmapTexture_OpenGL();

	void LoadResource();
	void UnloadResource();
	void ExtractResourceFromOpenGL();

	void UpdateTextureSettings();

	Size m_Size;
	ITexture::MipMapMode m_mipMapMode;
	IBitmapTexture::Format m_Format;
	ITexture::SamplerSettings m_SamplerSettings;

	GLuint m_Texture;
	Size m_BufferSize;

	GraphicsService_OpenGL* m_pGraphicsService;
};


///////// RENDER TARGET TEXTURE ///////////////////////////////////////////////

struct RenderTargetTexture_OpenGL : IRenderTargetTexture, Resource
{
	IRENDERTARGETTEXTURE_INTERFACE(;)

	RenderTargetTexture_OpenGL(GraphicsService_OpenGL* pGraphicsService, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, bool depthTestEnabled, MipMapMode mipMapMode);
	~RenderTargetTexture_OpenGL();

	void LoadResource();
	void UnloadResource();

	void UpdateTextureSettings();

	Size size;
	ITexture::MipMapMode m_MipMapMode;
	IRenderTargetTexture::Format m_Format;
	ITexture::SamplerSettings m_SamplerSettings;
	bool m_DepthTestEnabled;
	bool isMipMapDirty;

	Size bufferSize;
	GLuint m_pTexture;
	GLuint m_pRenderTarget;
	GLuint m_pDepthStencil;

	GraphicsService_OpenGL* m_pGraphicsService;
};


///////// WINDOW RENDER TARGET ////////////////////////////////////////////////

struct WindowRenderTarget_OpenGL : IWindowRenderTarget, Resource
{
	IWINDOWRENDERTARGET_INTERFACE(;)

	WindowRenderTarget_OpenGL(GraphicsService_OpenGL* pGraphicsService, MINI3D_WINDOW window, bool depthTestEnabled);
	~WindowRenderTarget_OpenGL();

	void LoadResource();
	void UnloadResource();
	
	void UpdateSize();

	MINI3D_WINDOW m_Window;
	Size size;
	bool m_DepthTestEnabled;
	unsigned int m_Msaa;
	int m_BufferDepthTestEnabled;
	IWindowRenderTarget::ScreenState m_ScreenState;

	GLuint m_pScreenRenderTarget;
	GLuint m_pDepthStencil;

	GraphicsService_OpenGL* m_pGraphicsService;
};

}

#endif //MINI3D_GRAPHICSSERVICE_OPENGL
#endif //MINI3D_OPENGL_H
