
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#include "opengl.hpp"

#ifdef MINI3D_GRAPHICSSERVICE_OPENGL

#include <cstdlib>
#include <cstdio>
#include <cmath>

#define STB_DXT_IMPLEMENTATION
#include "../common/stb_dxt.h"
#include "../common/mipmap.hpp"

#include "platform/iplatform.hpp"
#include "platform/openglwrapper.hpp"

void mini3d_assert(bool expression, const char* text, ...);

using namespace mini3d::graphics;

typedef unsigned int uint;


////////// INTERNAL HELPER FUNCTIONS //////////////////////////////////////////
	
namespace {
	
	char logText[LOG_TEXT_MAX_LENGTH];

    void PrintShaderLog(GLuint obj, OpenGlWrapper* pWrapper)
	{
        *logText = 0;
		pWrapper->glGetShaderInfoLog(obj, LOG_TEXT_MAX_LENGTH, 0, logText);
		if (*logText) printf("SHADER COMPILATION LOG:\n%s\n", logText);
	}

	void PrintProgramLog(GLuint obj, OpenGlWrapper* pWrapper)
	{
        *logText = 0;
		pWrapper->glGetProgramInfoLog(obj, LOG_TEXT_MAX_LENGTH, 0, logText);
		if (*logText) printf("PROGRAM LINKING LOG:\n%s\n", logText);
	}

	void PrintAttributeInformation(GLuint obj, OpenGlWrapper* pWrapper)
	{
        printf("\nATTRIBUTE INFORMATION: \n");
		GLint count, maxNameLength, size;
		GLenum type;

		pWrapper->glGetProgramiv(obj, GL_ACTIVE_ATTRIBUTES, &count);
		pWrapper->glGetProgramiv(obj, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLength);

		char *name = new char[maxNameLength];
		for (int i = 0; i < count; i++) 
		{
			pWrapper->glGetActiveAttrib(obj, i, maxNameLength, NULL, &size, &type, name);
			printf("Type: %d Name: %s Attribute Location: %d\n", type, name, pWrapper->glGetAttribLocation(obj, name));
		}
		delete[] name;

		pWrapper->glGetProgramiv(obj, GL_ACTIVE_UNIFORMS, &count);
		pWrapper->glGetProgramiv(obj, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);

		name = new char[maxNameLength];
		for (int i = 0; i < count; i++) 
		{
			pWrapper->glGetActiveUniform(obj, i, maxNameLength, NULL, &size, &type, name);
			printf("Type: %d Name: %s Attribute Location: %d\n", type, name, pWrapper->glGetUniformLocation(obj, name));
		}
		delete[] name;
	}
}


///////// INDEX BUFFER ////////////////////////////////////////////////////////

unsigned int mini3d_IndexBuffer_OpenGL_BytesPerIndex[] = { 2, 4 };

unsigned int					IndexBuffer_OpenGL::GetIndexCount() const					{ return m_IndexCount; };
IIndexBuffer::DataType			IndexBuffer_OpenGL::GetDataType() const					    { return m_DataType; };
void*							IndexBuffer_OpenGL::GetIndices(uint& sizeInBytes)			{ return CopyOut(sizeInBytes); }
void*							IndexBuffer_OpenGL::Lock(uint& sizeInBytes, bool readOnly)	{ return Lock(sizeInBytes, readOnly); }
void							IndexBuffer_OpenGL::Unlock()								{ if (LockableResource<char>::Unlock()) LoadResource(); }
								IndexBuffer_OpenGL::~IndexBuffer_OpenGL()					{ UnloadResource(); m_pGraphicsService->RemoveResource(this); }

IndexBuffer_OpenGL::IndexBuffer_OpenGL(GraphicsService_OpenGL* pGraphicsService, const void* pIndices, unsigned int count, DataType dataType)
{
	m_pGraphicsService = pGraphicsService;
	m_BufferSizeInBytes = 0; 
	m_IndexBuffer = 0; 

	SetIndices(pIndices, count, dataType);
	m_pGraphicsService->AddResource(this);
}

void IndexBuffer_OpenGL::SetIndices(const void* pIndices, unsigned int count, DataType dataType)
{
	m_DataType = dataType;
	m_IndexCount = count;
	m_SizeInBytes = count * mini3d_IndexBuffer_OpenGL_BytesPerIndex[dataType];

	CopyIn(pIndices, m_SizeInBytes);	
}

void IndexBuffer_OpenGL::LoadResource(void)
{
    if (m_pResource == 0 || m_SizeInBytes == 0) { UnloadResource(); return; }

	// If the buffer exists but is not the correct size, tear it down and recreate it
	if (m_IndexBuffer != 0 && m_BufferSizeInBytes != m_SizeInBytes) UnloadResource();

    OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();

	// If it does not exist, create a new one
	if (m_IndexBuffer == 0)
	{
		pOgl->glGenBuffers(1, &m_IndexBuffer);
		pOgl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);

		pOgl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_SizeInBytes, m_pResource, GL_STATIC_DRAW);
	}

	// Lock the buffer to gain access to the vertices 
	GLvoid* pBufferIndices = pOgl->glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	memcpy(pBufferIndices, m_pResource, m_SizeInBytes);
	pOgl->glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

	// Clear the bound array
	pOgl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	delete[] m_pResource;

	m_BufferSizeInBytes = m_SizeInBytes;
	isDirty = false;
}

void IndexBuffer_OpenGL::UnloadResource(void)
{
	ExtractResourceFromOpenGL();

	if (m_IndexBuffer != 0)
	{
		// if this is the currently loaded index buffer, release it
		if (m_pGraphicsService->GetIndexBuffer() == this) m_pGraphicsService->SetIndexBuffer(0);
	    OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();
		pOgl->glDeleteBuffers(1, &m_IndexBuffer); 
		m_IndexBuffer = 0;
	}

	isDirty = true;
}

void IndexBuffer_OpenGL::ExtractResourceFromOpenGL()
{
    OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();
	
	m_pResource = new char[m_SizeInBytes];
	pOgl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
	pOgl->glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_SizeInBytes, m_pResource);
	pOgl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


///////// VERTEX BUFFER ///////////////////////////////////////////////////////

uint						VertexBuffer_OpenGL::GetVertexCount() const					{ return m_SizeInBytes / m_VertexSizeInBytes; };
uint						VertexBuffer_OpenGL::GetVertexSizeInBytes() const				{ return m_VertexSizeInBytes; };
IVertexBuffer::StreamMode	VertexBuffer_OpenGL::GetStreamMode() const						{ return m_StreamMode; };
void						VertexBuffer_OpenGL::SetStreamMode(StreamMode streamMode)		{ m_StreamMode = streamMode; };
void*						VertexBuffer_OpenGL::GetVertices(uint& sizeInBytes)			{ return CopyOut(sizeInBytes); }
void*						VertexBuffer_OpenGL::Lock(uint& sizeInBytes, bool readOnly)	{ return Lock(sizeInBytes, readOnly); }
void						VertexBuffer_OpenGL::Unlock()									{ if (LockableResource<char>::Unlock()) LoadResource(); }
							VertexBuffer_OpenGL::~VertexBuffer_OpenGL()					{ UnloadResource(); m_pGraphicsService->RemoveResource(this); }

VertexBuffer_OpenGL::VertexBuffer_OpenGL(GraphicsService_OpenGL* pGraphicsService, const void* pVertices, unsigned int count, unsigned int vertexSizeInBytes, IVertexBuffer::StreamMode streamMode)
{
	m_pGraphicsService = pGraphicsService;
	m_BufferSizeInBytes = 0; 
	m_VertexBuffer = 0; 
	m_ComponentDescriptionCount = 0;
	m_pComponentDescriptions = 0;

	SetVertices(pVertices, count, vertexSizeInBytes, streamMode);
	m_pGraphicsService->AddResource(this);
}

void VertexBuffer_OpenGL::SetVertices(const void* pVertices, unsigned int count, unsigned int vertexSizeInBytes, StreamMode streamMode)
{
	m_StreamMode = streamMode;
	m_VertexSizeInBytes = vertexSizeInBytes;
	m_SizeInBytes = count * vertexSizeInBytes;

	CopyIn(pVertices, m_SizeInBytes);
}

void VertexBuffer_OpenGL::SetComponentDescriptions(const ComponentDescription* pComponentDescriptions, const unsigned int count) 
{ 
	if (m_pComponentDescriptions != 0) delete[] m_pComponentDescriptions;
	
	m_pComponentDescriptions = (ComponentDescription*)memcpy(new ComponentDescription[count], pComponentDescriptions, count * sizeof(ComponentDescription));
	m_ComponentDescriptionCount = count;
}

const VertexBuffer_OpenGL::ComponentDescription* VertexBuffer_OpenGL::GetComponentDescriptions(unsigned int &count) const 
{ 
	count = m_ComponentDescriptionCount; 
	return m_pComponentDescriptions; 
}

void VertexBuffer_OpenGL::LoadResource(void)
{

	/// Allocate buffer on the graphics card and add index data.
    if (m_pResource == 0 || m_SizeInBytes == 0)
    {
		UnloadResource();
        return;
    }

	// Else if the buffer exists but is not the correct size, tear it down and recreate it
	if (m_VertexBuffer != 0 && m_BufferSizeInBytes != m_SizeInBytes)
	{
		UnloadResource();
	}

    OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();

	// If it does not exist, create a new one
	if (m_VertexBuffer == 0)
	{
		pOgl->glGenBuffers(1, &m_VertexBuffer);
		pOgl->glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);

		pOgl->glBufferData(GL_ARRAY_BUFFER, m_SizeInBytes, m_pResource, GL_STATIC_DRAW);
	}

	// Lock the buffer to gain access to the vertices 
	GLvoid* pBufferVertices = pOgl->glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	memcpy(pBufferVertices, m_pResource, m_SizeInBytes);
	pOgl->glUnmapBuffer(GL_ARRAY_BUFFER);
	
	// clear the bound buffer
	pOgl->glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	delete[] m_pResource;
	
	m_BufferSizeInBytes = m_SizeInBytes;
	isDirty = false;
}

void VertexBuffer_OpenGL::UnloadResource(void)
{
	ExtractResourceFromOpenGL();

	if (m_VertexBuffer != 0)
	{
		// if this is the currently loaded vertex buffer, release it
		if (m_pGraphicsService->GetVertexBuffer(0) == this)
			m_pGraphicsService->SetVertexBuffer(0, 0);

        OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();
		pOgl->glDeleteBuffers(1, &m_VertexBuffer); 
		m_VertexBuffer = 0;
	}

	isDirty = true;
}

void VertexBuffer_OpenGL::ExtractResourceFromOpenGL()
{
    OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();
	
	m_pResource = new char[m_SizeInBytes];
	pOgl->glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
	pOgl->glGetBufferSubData(GL_ARRAY_BUFFER, 0, m_SizeInBytes, m_pResource);
	pOgl->glBindBuffer(GL_ARRAY_BUFFER, 0);
}


///////// PIXEL SHADER ////////////////////////////////////////////////////////

void*	PixelShader_OpenGL::GetPixelShader(uint& sizeInBytes)	{ sizeInBytes = m_SizeInBytes; return m_pResource; };
		PixelShader_OpenGL::~PixelShader_OpenGL()				{ UnloadResource(); m_pGraphicsService->RemoveResource(this); }

PixelShader_OpenGL::PixelShader_OpenGL(GraphicsService_OpenGL* pGraphicsService, const char* pShaderBytes, unsigned int sizeInBytes)
{	
	m_pGraphicsService = pGraphicsService;
	m_ShaderBuffer = 0; 
	CopyIn(pShaderBytes, sizeInBytes);

	m_pGraphicsService->AddResource(this);
}

void PixelShader_OpenGL::LoadResource()
{
	if (m_ShaderBuffer != 0) { UnloadResource(); }
	
    OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();

	m_ShaderBuffer = pOgl->glCreateShader(GL_FRAGMENT_SHADER);
	pOgl->glShaderSource(m_ShaderBuffer, 1, (const GLchar**)&m_pResource, (GLint*)&m_SizeInBytes);
	pOgl->glCompileShader(m_ShaderBuffer);

	PrintShaderLog(m_ShaderBuffer, m_pGraphicsService->GetOpenGlWrapper());
	
	delete[] m_pResource;
	isDirty = false;
} 

void PixelShader_OpenGL::UnloadResource(void)
{
	ExtractResourceFromOpenGL();

	if (m_ShaderBuffer != 0)
	{
		m_pGraphicsService->GetOpenGlWrapper()->glDeleteShader(m_ShaderBuffer);
		m_ShaderBuffer = 0;
	}

	isDirty = true;
}

void PixelShader_OpenGL::ExtractResourceFromOpenGL()
{
    OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();
	
	m_pResource = new char[m_SizeInBytes + 1];
	pOgl->glGetShaderSource(m_ShaderBuffer, m_SizeInBytes + 1, 0, m_pResource);
}


///////// VERTEX SHADER ////////////////////////////////////////////////////////

void*	VertexShader_OpenGL::GetVertexShader(uint& sizeInBytes) const	{ sizeInBytes = m_SizeInBytes; return m_pResource; };
		VertexShader_OpenGL::~VertexShader_OpenGL()						{ UnloadResource(); m_pGraphicsService->RemoveResource(this); }

VertexShader_OpenGL::VertexShader_OpenGL(GraphicsService_OpenGL* pGraphicsService, const char* pShaderBytes, unsigned int sizeInBytes)
{
	m_pGraphicsService = pGraphicsService;
	m_ShaderBuffer = 0; 
	CopyIn(pShaderBytes, sizeInBytes);

	m_pGraphicsService->AddResource(this);
}

void VertexShader_OpenGL::LoadResource()
{
	if (m_ShaderBuffer != 0) { UnloadResource(); }
	
    OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();

	m_ShaderBuffer = pOgl->glCreateShader(GL_VERTEX_SHADER);
	pOgl->glShaderSource(m_ShaderBuffer, 1, (const GLchar**)&m_pResource, (GLint*)&m_SizeInBytes);
	pOgl->glCompileShader(m_ShaderBuffer);

	PrintShaderLog(m_ShaderBuffer, m_pGraphicsService->GetOpenGlWrapper());

	delete[] m_pResource;
	isDirty = false;
} 

void VertexShader_OpenGL::UnloadResource(void)
{
	ExtractResourceFromOpenGL();

	if (m_ShaderBuffer != 0)
	{
		m_pGraphicsService->GetOpenGlWrapper()->glDeleteShader(m_ShaderBuffer);
		m_ShaderBuffer = 0;
	}

	isDirty = true;
}

void VertexShader_OpenGL::ExtractResourceFromOpenGL()
{
    OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();
	
	m_pResource = new char[m_SizeInBytes + 1];
	pOgl->glGetShaderSource(m_ShaderBuffer, m_SizeInBytes + 1, 0, m_pResource);
}


///////// SHADER PROGRAM ///////////////////////////////////////////////////////

IPixelShader*	ShaderProgram_OpenGL::GetPixelShader() const	{ return (IPixelShader*)m_pPixelShader; }
IVertexShader*	ShaderProgram_OpenGL::GetVertexShader() const	{ return (IVertexShader*)m_pVertexShader; }
				ShaderProgram_OpenGL::~ShaderProgram_OpenGL()	{ UnloadResource(); m_pGraphicsService->RemoveResource(this); }

ShaderProgram_OpenGL::ShaderProgram_OpenGL(GraphicsService_OpenGL* pGraphicsService, IVertexShader* pVertexShader, IPixelShader* pPixelShader)
{
	m_pGraphicsService = pGraphicsService;
	m_pVertexShader = dynamic_cast<VertexShader_OpenGL*>(pVertexShader);
	m_pPixelShader = dynamic_cast<PixelShader_OpenGL*>(pPixelShader);
	m_Program = 0;
	LoadResource();
	
	m_pGraphicsService->AddResource(this);
}

void ShaderProgram_OpenGL::LoadResource()
{
	if (m_Program != 0) { UnloadResource(); }

    OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();

	m_Program = pOgl->glCreateProgram();

	// Load and attach the shaders to this program
	if (m_pVertexShader->isDirty) m_pVertexShader->LoadResource();
	if (m_pPixelShader->isDirty) m_pPixelShader->LoadResource();

	if (m_pVertexShader) pOgl->glAttachShader(m_Program, m_pVertexShader->m_ShaderBuffer);
	if (m_pPixelShader) pOgl->glAttachShader(m_Program, m_pPixelShader->m_ShaderBuffer);

	// Link the program
	pOgl->glLinkProgram(m_Program);
	pOgl->glValidateProgram(m_Program);
	
	// Get the log output from linking and information on all attributes and uniforms
    PrintProgramLog(m_Program, m_pGraphicsService->GetOpenGlWrapper());
	PrintAttributeInformation(m_Program, m_pGraphicsService->GetOpenGlWrapper());
	
	isDirty = false;
}

void ShaderProgram_OpenGL::UnloadResource()
{
	if (m_Program != 0)
	{
		// if this is the currently loaded pixel shader, release it
		if (m_pGraphicsService->GetShaderProgram() == this)
			m_pGraphicsService->SetShaderProgram(0);

        OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();
		pOgl->glDeleteProgram(m_Program);

		m_Program  = 0;
	}

	isDirty = true;
}


///////// BITMAP TEXTURE ///////////////////////////////////////////////////////

struct OpenglBitmapFormat { GLuint internalFormat; GLenum format; GLenum type; };
OpenglBitmapFormat mini3d_BitmapTexture_Formats[] = {	{GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE}, {GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT}, {GL_R16UI, GL_RED, GL_FLOAT}, {GL_R32F, GL_RED, GL_FLOAT}, { GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_RGBA, GL_UNSIGNED_BYTE} };

unsigned int mini3d_BitmapTexture_BytesPerPixel[] = { 4, 8, 2, 4, 4 };

uint						BitmapTexture_OpenGL::GetWidth() const							{ return m_Size.width; };
uint						BitmapTexture_OpenGL::GetHeight() const						{ return m_Size.height; };
ITexture::MipMapMode		BitmapTexture_OpenGL::GetMipMapMode() const					{ return m_mipMapMode; };
void						BitmapTexture_OpenGL::SetMipMapMode(MipMapMode mipMapMode)		{ m_mipMapMode = mipMapMode; };
							BitmapTexture_OpenGL::Format BitmapTexture_OpenGL::GetFormat() const	{ return m_Format; };
ITexture::SamplerSettings	BitmapTexture_OpenGL::GetSamplerSettings() const				{ return m_SamplerSettings; };
void*						BitmapTexture_OpenGL::GetBitmap(uint& sizeInBytes)				{ return CopyOut(sizeInBytes); }
void*						BitmapTexture_OpenGL::Lock(uint& sizeInBytes, bool readOnly)	{ return Lock(sizeInBytes, readOnly); }
void						BitmapTexture_OpenGL::Unlock()									{ if (LockableResource<char>::Unlock()) LoadResource(); }
							BitmapTexture_OpenGL::~BitmapTexture_OpenGL()					{ UnloadResource(); m_pGraphicsService->RemoveResource(this); }

BitmapTexture_OpenGL::BitmapTexture_OpenGL(GraphicsService_OpenGL* pGraphicsService, const void* pBitmap, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, MipMapMode mipMapMode) 
{
	m_pGraphicsService = pGraphicsService;
	m_BufferSize.width = 0;
	m_BufferSize.height = 0; 
	m_Texture = 0;

	SetBitmap(pBitmap, width, height, format, samplerSettings, mipMapMode);
	m_pGraphicsService->AddResource(this);
}

void BitmapTexture_OpenGL::SetBitmap(const void* pBitmap, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, MipMapMode mipMapMode)
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
	m_mipMapMode = mipMapMode;

	CopyIn(pBitmap, width * height * mini3d_BitmapTexture_BytesPerPixel[m_Format]);
}

void BitmapTexture_OpenGL::UpdateTextureSettings()
{
	if (m_Texture == 0) return;

    OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();

	pOgl->glBindTexture(GL_TEXTURE_2D, m_Texture);

	// Set wrap mode
	GLint adressMode;
	switch(m_SamplerSettings.wrapMode)
	{
		case ITexture::SamplerSettings::WRAP_TILE: adressMode = GL_REPEAT; break;
		case ITexture::SamplerSettings::WRAP_CLAMP: adressMode = GL_CLAMP_TO_EDGE; break;
	}

	// set the wrap style
	pOgl->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
	pOgl->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set filter mode
	GLint filterMag;
	GLint filterMin;

	switch(m_SamplerSettings.sampleMode)
	{
	case ITexture::SamplerSettings::SAMPLE_LINEAR:
		filterMin = (m_mipMapMode == ITexture::MIPMAP_NONE) ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR;
		filterMag = GL_LINEAR;
		break;
	case ITexture::SamplerSettings::SAMPLE_NEAREST:
		filterMin = (m_mipMapMode == ITexture::MIPMAP_NONE) ? GL_NEAREST : GL_NEAREST_MIPMAP_NEAREST;
		filterMag = GL_NEAREST;
		break;
	}
	
	pOgl->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, filterMin);
	pOgl->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, filterMag);

	pOgl->glBindTexture(GL_TEXTURE_2D, 0);
}

void BitmapTexture_OpenGL::LoadResource()
{
	if ((m_Texture != 0) && (m_BufferSize.width != m_Size.width || m_BufferSize.height != m_Size.height)) UnloadResource();

    OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();

	// If it does not exist, create a new one
	if (m_Texture == 0)
	{
		pOgl->glGenTextures(1, &m_Texture);
		pOgl->glBindTexture(GL_TEXTURE_2D, m_Texture);

		// Calculate how many mip map levels we need (if any)
		int textureLevels = 1;

		if (m_mipMapMode != MIPMAP_NONE)
		{
			unsigned int largestSize = m_Size.width > m_Size.height ? m_Size.width : m_Size.height;
			while (largestSize >>= 1) ++textureLevels;
		}
		
		pOgl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, textureLevels);

		for (unsigned int level = 0; level < (unsigned int)textureLevels; ++level)
		{
			unsigned int mipMapWidth = (m_Size.width >> level) > 0 ? (m_Size.width >> level) : 1;
			unsigned int mipMapHeight = (m_Size.height >> level) > 0 ? (m_Size.height >> level) : 1;

			if (m_Format == FORMAT_RGBA8UI_COMPRESSED)
			{
				unsigned int compressedSizeInBytes = (4 > mipMapWidth ? 4 : mipMapWidth) * (4 > mipMapHeight ? 4 : mipMapHeight);
				pOgl->glCompressedTexImage2D(GL_TEXTURE_2D, level, mini3d_BitmapTexture_Formats[m_Format].internalFormat, mipMapWidth, mipMapHeight, 0, compressedSizeInBytes, 0);
				GLenum err = pOgl->glGetError();
				uint i = 0;
			}
			else
			{
				pOgl->glTexImage2D(GL_TEXTURE_2D, level, mini3d_BitmapTexture_Formats[m_Format].internalFormat, mipMapWidth, mipMapHeight, 0, mini3d_BitmapTexture_Formats[m_Format].format, mini3d_BitmapTexture_Formats[m_Format].type, 0);
			}
		}
	}

	int textureLevels;
    pOgl->glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, &textureLevels);

	// write bitmap data to texture
	unsigned char* pMipMap = (unsigned char*)m_pResource;
	for (unsigned int level = 0; level < (unsigned int)textureLevels; ++level)
	{
		unsigned int mipMapWidth = (m_Size.width >> level) > 0 ? m_Size.width >> level : 1;
		unsigned int mipMapHeight = (m_Size.height >> level) > 0 ? m_Size.width >> level : 1;
		if (m_Format == FORMAT_RGBA8UI_COMPRESSED)
		{
			// Make sure we can fit at least one full block!
			unsigned int compressedSizeInBytes = (4 > mipMapWidth ? 4 : mipMapWidth) * (4 > mipMapHeight ? 4 : mipMapHeight);
			unsigned char* pCompressedData = new unsigned char[compressedSizeInBytes];
			unsigned char* pCompressedDataPointer = pCompressedData;

			unsigned int pRgbaBlock[16];
			for (unsigned int y = 0; y < mipMapHeight; y += 4)
			{
				for (unsigned int x = 0; x < mipMapWidth; x += 4)
				{
					// We always need to write at least one block. It might have be a good idea to pad the rest of the block if mipmap is smaller than the block, but this is not done!
					// As it is now some random stuff is left over from previous levels.
					unsigned int blockWidth = 4 < mipMapWidth ? 4 : mipMapWidth;
					unsigned int blockHeight = 4 < mipMapHeight ? 4 : mipMapHeight;
					for (unsigned int k = 0; k < blockHeight; ++k) memcpy(pRgbaBlock + 4 * k, (unsigned int*)pMipMap + mipMapWidth * (y + k) + x, blockWidth * 4);

					stb_compress_dxt_block(pCompressedDataPointer, (unsigned char*)pRgbaBlock, true, 0);
					pCompressedDataPointer += 16;
				}
			}
			
			pOgl->glCompressedTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, mipMapWidth, mipMapHeight, mini3d_BitmapTexture_Formats[m_Format].internalFormat, compressedSizeInBytes, pCompressedData);
			GLenum err = pOgl->glGetError();

			delete[] pCompressedData;
		}
		else
		{
			pOgl->glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, mipMapWidth, mipMapHeight, mini3d_BitmapTexture_Formats[m_Format].format, mini3d_BitmapTexture_Formats[m_Format].type, pMipMap);
		}
		
		if (m_mipMapMode == MIPMAP_BOX_FILTER) 
		{	
			unsigned char* pNewMipMap = mini3d_GenerateMipMapBoxFilter(pMipMap, mipMapWidth, mipMapHeight);
			if (pMipMap != (unsigned char*)m_pResource) delete[]pMipMap;
			pMipMap = pNewMipMap;
		}	
	}

	// Clear the current bound texture
	pOgl->glBindTexture(GL_TEXTURE_2D, 0);

	UpdateTextureSettings();

	m_BufferSize.width = m_Size.width;
	m_BufferSize.height = m_Size.height;

	delete[] m_pResource;

	isDirty = false;
}

void BitmapTexture_OpenGL::UnloadResource()
{
	if (m_Texture != 0)
	{
        OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();

		ExtractResourceFromOpenGL();

		// if we are removing one of the current textures, clear that texture slot first
		for(unsigned int i = 0; i < m_pGraphicsService->m_pCompatibility->TextureStreamCount(); i++)
		{
			if (m_pGraphicsService->GetTexture(i) == this)
			{
				m_pGraphicsService->SetTexture(0, m_pGraphicsService->m_CurrentTextureSamplerName[i]);
				delete[] m_pGraphicsService->m_CurrentTextureSamplerName[i];
				m_pGraphicsService->m_CurrentTextureSamplerName[i] = 0;
			}
		}

		pOgl->glDeleteTextures(1, &m_Texture);
		m_Texture = 0;
	}

	isDirty = true;
}

void BitmapTexture_OpenGL::ExtractResourceFromOpenGL()
{
    OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();

	m_pResource = new char[m_Size.width * m_Size.height * mini3d_BitmapTexture_BytesPerPixel[m_Format]];
	pOgl->glBindTexture(GL_TEXTURE_2D, m_Texture);
	pOgl->glGetTexImage(GL_TEXTURE_2D, 0, mini3d_BitmapTexture_Formats[m_Format].format, mini3d_BitmapTexture_Formats[m_Format].type, m_pResource);
	pOgl->glBindTexture(GL_TEXTURE_2D, 0);
}


///////// RENDER TARGET TEXTURE ///////////////////////////////////////////////

uint							RenderTargetTexture_OpenGL::GetWidth() const									{ return size.width; };
uint							RenderTargetTexture_OpenGL::GetHeight() const									{ return size.height; }
RenderTargetTexture_OpenGL::Viewport	RenderTargetTexture_OpenGL::GetViewport() const								{ Viewport v = {0,0,0,0}; return v; }; // TODO: Proper values :)
void							RenderTargetTexture_OpenGL::SetViewport(Viewport viewport)						{ }; // TODO: Do something :)
ITexture::MipMapMode			RenderTargetTexture_OpenGL::GetMipMapMode() const								{ return m_MipMapMode; };
void							RenderTargetTexture_OpenGL::SetMipMapMode(MipMapMode mipMapMode)				{ m_MipMapMode = mipMapMode; };
void							RenderTargetTexture_OpenGL::SetSize(unsigned int width, unsigned int height)	{ SetRenderTargetTexture(width, height, m_Format, m_SamplerSettings, m_DepthTestEnabled, m_MipMapMode); };
inline							RenderTargetTexture_OpenGL::Format RenderTargetTexture_OpenGL::GetFormat() const		{ return m_Format; };
ITexture::SamplerSettings		RenderTargetTexture_OpenGL::GetSamplerSettings() const							{ return m_SamplerSettings; };
bool							RenderTargetTexture_OpenGL::GetDepthTestEnabled() const						{ return m_DepthTestEnabled; };
void							RenderTargetTexture_OpenGL::SetDepthTestEnabled(bool depthTestEnabled)			{ m_DepthTestEnabled = depthTestEnabled; LoadResource(); };
								RenderTargetTexture_OpenGL::~RenderTargetTexture_OpenGL()								{ UnloadResource(); m_pGraphicsService->RemoveResource(this); }

RenderTargetTexture_OpenGL::RenderTargetTexture_OpenGL(GraphicsService_OpenGL* pGraphicsService, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, bool depthTestEnabled, MipMapMode mipMapMode)
{
	m_pGraphicsService = pGraphicsService;
	m_pDepthStencil = 0;
	m_pTexture = 0;
	m_pRenderTarget = 0;

	SetRenderTargetTexture(width, height, format, samplerSettings, depthTestEnabled, mipMapMode);
	m_pGraphicsService->AddResource(this);
}

void RenderTargetTexture_OpenGL::SetRenderTargetTexture(unsigned int width, unsigned int height, const Format format, SamplerSettings samplerSettings, bool depthTestEnabled, MipMapMode mipMapMode)
{
	mini3d_assert((width & (width - 1)) == 0, "Setting a Bitmap Texture to a non power of two width!");
	mini3d_assert((height & (height - 1)) == 0, "Setting a Bitmap Texture to a non power of two height!");
	mini3d_assert(width >= 64, "Setting a Bitmap Texture to a height less than 64!");
	mini3d_assert(height >= 64, "Setting a Bitmap Texture to a width less than 64!");

	size.width = width;
	size.height = height;
	m_Format = format;
	m_DepthTestEnabled = depthTestEnabled;
	m_SamplerSettings = samplerSettings;
	m_MipMapMode = mipMapMode;

	isDirty = true;
	isMipMapDirty = true;

	LoadResource();
}

void RenderTargetTexture_OpenGL::LoadResource()
{
	// CREATE FRAME BUFFER OBJECT

	// This method is a little tricky. It has two different flows. One for regular Framebuffer objects
	// and one for depth-only frame buffer objects. Read the OpenGL wiki below to see how this works.
	// http://www.opengl.org/wiki/GL_EXT_framebuffer_object#Color_texture.2C_Depth_texture

    OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();

	// if it is not unloaded, do so now
	UnloadResource();

	pOgl->glGenTextures(1, &m_pTexture);
	pOgl->glBindTexture(GL_TEXTURE_2D, m_pTexture);
	pOgl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	pOgl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	pOgl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	pOgl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//NULL means reserve texture memory, but texels are undefined
	pOgl->glTexImage2D(GL_TEXTURE_2D, 0, mini3d_BitmapTexture_Formats[m_Format].internalFormat, size.width, size.height, 0, mini3d_BitmapTexture_Formats[m_Format].format, mini3d_BitmapTexture_Formats[m_Format].type, NULL);

	if (m_MipMapMode == MIPMAP_BOX_FILTER)
		pOgl->glGenerateMipmap(GL_TEXTURE_2D);

	pOgl->glGenFramebuffers(1, &m_pRenderTarget);
	pOgl->glBindFramebuffer(GL_FRAMEBUFFER, m_pRenderTarget);
	//Attach 2D texture to this FBO
	pOgl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pTexture, 0);
	pOgl->glGenRenderbuffers(1, &m_pDepthStencil);
	pOgl->glBindRenderbuffer(GL_RENDERBUFFER, m_pDepthStencil);
	pOgl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.width, size.height);
	//Attach depth buffer to FBO
	pOgl->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_pDepthStencil);

	//Check so that the GPU supports the FBO configuration
	GLenum FBOstatus = pOgl->glCheckFramebufferStatus(GL_FRAMEBUFFER);
	mini3d_assert(FBOstatus == GL_FRAMEBUFFER_COMPLETE, "Failed to create framebuffer for Render Target Texture");

	// switch back to window-system-provided framebuffer
	pOgl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	pOgl->glBindRenderbuffer(GL_RENDERBUFFER, 0);
	pOgl->glBindTexture(GL_TEXTURE_2D, 0);

	UpdateTextureSettings();

	bufferSize.width = size.width;
	bufferSize.height = size.height;
	isDirty = false;
}

void RenderTargetTexture_OpenGL::UnloadResource()
{
	if (m_pRenderTarget != 0)
	{
		// if we are removing the current render target, restore the default render target first
		if (m_pGraphicsService->GetRenderTarget() == this)
			m_pGraphicsService->SetRenderTarget(0);

		// if we are removing one of the current textures, clear that texture slot first
		for(unsigned int i = 0; i < m_pGraphicsService->m_pCompatibility->TextureStreamCount(); i++)
		{
			if (m_pGraphicsService->GetTexture(i) == this)
			{
				m_pGraphicsService->SetTexture(0, m_pGraphicsService->m_CurrentTextureSamplerName[i]);
				delete[] m_pGraphicsService->m_CurrentTextureSamplerName[i];
				m_pGraphicsService->m_CurrentTextureSamplerName[i] = 0;
			}
		}

        OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();
		pOgl->glDeleteRenderbuffers(1, &m_pDepthStencil);
		m_pDepthStencil = 0;

		pOgl->glDeleteTextures(1, &m_pTexture);
		m_pTexture = 0;

		pOgl->glDeleteFramebuffers(1, &m_pRenderTarget);
		m_pRenderTarget = 0;
	}

	isDirty = true;
}

void RenderTargetTexture_OpenGL::UpdateTextureSettings()
{
	if (m_pTexture == 0)	return;

    OpenGlWrapper* pOgl = m_pGraphicsService->GetOpenGlWrapper();

	pOgl->glBindTexture(GL_TEXTURE_2D, m_pTexture);
	
	// Set wrap mode 
	GLint adressMode;
	switch(m_SamplerSettings.wrapMode)
	{
		case SamplerSettings::WRAP_TILE: adressMode = GL_REPEAT; break;
		case SamplerSettings::WRAP_CLAMP: adressMode = GL_CLAMP_TO_EDGE; break;
	}

	// set the wrap style
	pOgl->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, adressMode);
	pOgl->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, adressMode);

	// Set filter mode
	GLint filterMag;
	GLint filterMin;

	switch(m_SamplerSettings.sampleMode)
	{
	case SamplerSettings::SAMPLE_LINEAR:
		filterMin = (m_MipMapMode == ITexture::MIPMAP_NONE) ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR;
		filterMag = GL_LINEAR;
		break;
	case SamplerSettings::SAMPLE_NEAREST:
		filterMin = (m_MipMapMode == ITexture::MIPMAP_NONE) ? GL_NEAREST : GL_NEAREST_MIPMAP_NEAREST;
		filterMag = GL_NEAREST;
		break;
	}

	// set the wrap style
	pOgl->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, filterMin);
	pOgl->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, filterMag);

	pOgl->glBindTexture(GL_TEXTURE_2D, 0);
}


///////// WINDOW RENDER TARGET ////////////////////////////////////////////////

uint							WindowRenderTarget_OpenGL::GetWidth() const								{ return size.width; };
uint							WindowRenderTarget_OpenGL::GetHeight() const							{ return size.height; };
IWindowRenderTarget::Viewport	WindowRenderTarget_OpenGL::GetViewport() const							{ Viewport v = {0,0,0,0}; return v; };
void							WindowRenderTarget_OpenGL::SetViewport(Viewport viewport)				{ };
bool							WindowRenderTarget_OpenGL::GetDepthTestEnabled() const					{ return m_DepthTestEnabled; };
void							WindowRenderTarget_OpenGL::SetDepthTestEnabled(bool depthTestEnabled)	{ m_DepthTestEnabled = depthTestEnabled; LoadResource();};
MINI3D_WINDOW					WindowRenderTarget_OpenGL::GetWindowHandle() const						{ return m_Window; }
void							WindowRenderTarget_OpenGL::Display()									{ m_pGraphicsService->GetPlatform()->SwapWindowBuffers(this); }

WindowRenderTarget_OpenGL::WindowRenderTarget_OpenGL(GraphicsService_OpenGL* pGraphicsService, MINI3D_WINDOW window, bool depthTestEnabled)
{
	m_pGraphicsService = pGraphicsService;
	m_pScreenRenderTarget = 0;
	m_Window = 0;
	m_pDepthStencil = 0;
	m_ScreenState = SCREEN_STATE_WINDOWED;

	SetWindowRenderTarget(window, depthTestEnabled);
	m_pGraphicsService->AddResource(this);
}

WindowRenderTarget_OpenGL::~WindowRenderTarget_OpenGL()						
{ 
	UnloadResource(); 
	m_pGraphicsService->RemoveResource(this);
}

void WindowRenderTarget_OpenGL::SetWindowRenderTarget(MINI3D_WINDOW window, bool depthTestEnabled)
{
	IPlatform* pPlatform = m_pGraphicsService->GetPlatform();

	// set the variables from the call
	m_Window = window;
	m_DepthTestEnabled = depthTestEnabled;
	
	// load the buffer
	isDirty = true;
	LoadResource();
}

void WindowRenderTarget_OpenGL::LoadResource()
{
	// In MS windows, update the pixel format of the internal window if needed
	// as all windows for the rendercontext must have the same pixelformat
    /*
    IPlatform* pPlatform = m_pGraphicsService->GetPlatform();
	if (pPlatform->SetInternalWindowPixelFormat(m_Window))
	{
		// If the internal pixel format was changed, we need to recreate the device context and reload all resources
		m_pGraphicsService->UnloadResources();
		pPlatform->RecreateDeviceContext();
		m_pGraphicsService->UpdateResources();
		m_pGraphicsService->RestoreGraphicsState();
	}
    */
	isDirty = false;
}

void WindowRenderTarget_OpenGL::UnloadResource()
{
	if (m_pScreenRenderTarget != 0)
	{
		// if we are removing the current render target, restore the default render target first
		if (m_pGraphicsService->GetRenderTarget() == this)
			m_pGraphicsService->SetRenderTarget(0);
	}

	isDirty = true;
}

void WindowRenderTarget_OpenGL::UpdateSize()
{
	unsigned int width, height;
    IPlatform* pPlatform = m_pGraphicsService->GetPlatform();
	pPlatform->GetWindowContentSize(m_Window, width, height);

	if (size.width != width || size.height != height)
	{
		size.width = width;
		size.height = height;
	}
}


///////// COMPATIBILITY ///////////////////////////////////////////////////////

uint		Compatibility_OpenGL::TextureStreamCount() const											{ return m_CapabilityMaxTextureStreamCount; }
uint		Compatibility_OpenGL::MaxTextureSize() const												{ return m_CapabilityMaxTextureSize; }
bool		Compatibility_OpenGL::TextureFormat(IBitmapTexture::Format format) const					{ return true; } // TODO: Fix this
bool		Compatibility_OpenGL::RenderTargetTextureFormat(IRenderTargetTexture::Format format) const	{ return true; } // TODO: Fix this
const char*	Compatibility_OpenGL::ShaderLanguage() const												{ return "GLSL"; }
const char*	Compatibility_OpenGL::PixelShaderVersion() const											{ return m_PixelShaderVersion; }
const char*	Compatibility_OpenGL::VertexShaderVersion() const											{ return m_VertexShaderVersion; }
uint		Compatibility_OpenGL::VertexStreamCount() const												{ return 1; } // TODO: Fix this
uint		Compatibility_OpenGL::FreeGraphicsMemory() const											{ return 0; } // TODO: Fix this

Compatibility_OpenGL::~Compatibility_OpenGL() {};


Compatibility_OpenGL::Compatibility_OpenGL(GraphicsService_OpenGL* pGraphicsService)
{
	m_pGraphicsService = pGraphicsService;

    OpenGlWrapper* pOgl = pGraphicsService->GetOpenGlWrapper();

	GLint unit;
	pOgl->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &unit);
	m_CapabilityMaxTextureSize = (unsigned int)unit;
	
	pOgl->glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &unit);
	m_CapabilityMaxTextureStreamCount = (unsigned int)unit;
	
	m_PixelShaderVersion = m_VertexShaderVersion = (const char*)pOgl->glGetString(GL_SHADING_LANGUAGE_VERSION);
}


///////// GRAPHICS SERVICE ////////////////////////////////////////////////////

void							GraphicsService_OpenGL::SetCullMode(CullMode cullMode)	{ SetCullMode(cullMode, false); }
IRenderTarget*					GraphicsService_OpenGL::GetRenderTarget() const			{ return m_pCurrentRenderTarget == 0 ? m_pCurrentWindowRenderTarget : m_pCurrentRenderTarget; }
const ICompatibility*			GraphicsService_OpenGL::GetCompatibility() const		{ return m_pCompatibility; }
inline IPlatform*	            GraphicsService_OpenGL::GetPlatform()		            { return m_pPlatform; };
inline OpenGlWrapper*	        GraphicsService_OpenGL::GetOpenGlWrapper()		        { return m_pOgl; };
								GraphicsService_OpenGL::~GraphicsService_OpenGL()		{ delete m_pPlatform; }

// Resource Creation (Just a simple pass-through of the arguments to the resource constructors)
IBitmapTexture*	GraphicsService_OpenGL::CreateBitmapTexture(const void* pBitmap, uint width, uint height, IBitmapTexture::Format format, ITexture::SamplerSettings samplerSettings, ITexture::MipMapMode mipMapMode) { return new BitmapTexture_OpenGL(this, pBitmap, width, height, format, samplerSettings, mipMapMode); }
IRenderTargetTexture* GraphicsService_OpenGL::CreateRenderTargetTexture(uint width, uint height, IRenderTargetTexture::Format format, ITexture::SamplerSettings samplerSettings, bool depthTestEnabled, ITexture::MipMapMode mipMapMode) { return new RenderTargetTexture_OpenGL(this, width, height, format, samplerSettings, depthTestEnabled, mipMapMode); }
IVertexBuffer* GraphicsService_OpenGL::CreateVertexBuffer(const void* pVertices, uint count, uint vertexSizeInBytes, IVertexBuffer::StreamMode streamMode) { return new VertexBuffer_OpenGL(this, pVertices, count, vertexSizeInBytes, streamMode); }
IIndexBuffer* GraphicsService_OpenGL::CreateIndexBuffer(const void* pIndices, uint count, IIndexBuffer::DataType dataType) { return new IndexBuffer_OpenGL(this, pIndices, count, dataType); }
IPixelShader* GraphicsService_OpenGL::CreatePixelShader(const char* shaderBytes, unsigned int sizeInBytes) { return new PixelShader_OpenGL(this, shaderBytes, sizeInBytes); }
IVertexShader* GraphicsService_OpenGL::CreateVertexShader(const char* shaderBytes, unsigned int sizeInBytes) { return new VertexShader_OpenGL(this, shaderBytes, sizeInBytes); }
IShaderProgram* GraphicsService_OpenGL::CreateShaderProgram(IVertexShader* pVertexShader, IPixelShader* pPixelShader) {	return new ShaderProgram_OpenGL(this, pVertexShader, pPixelShader); }
IWindowRenderTarget* GraphicsService_OpenGL::CreateWindowRenderTarget(MINI3D_WINDOW window, bool depthTestEnabled) { return new WindowRenderTarget_OpenGL(this, window, depthTestEnabled); }

GraphicsService_OpenGL::GraphicsService_OpenGL()
{
	m_pCurrentShaderProgram = 0; 
	m_pCurrentWindowRenderTarget = 0; 
	m_pCurrentRenderTarget = 0;
	m_pResourceList = 0;
	m_CullMode = CULL_COUNTERCLOCKWIZE;

	m_pPlatform = new Platform();
    m_pOgl = new OpenGlWrapper();
	m_pCompatibility = new Compatibility_OpenGL(this);

	for (unsigned int i = 0; i < MAX_VERTEX_BUFFER_SLOTS; ++i)
		m_CurrentVertexBufferMap[i] = 0;

	for (unsigned int i = 0; i < MAX_TEXTURE_BUFFER_SLOTS; ++i)
	{
		m_CurrentTextureMap[i] = 0;
		m_CurrentTextureSamplerName[i] = 0;
	}

	for (unsigned int i = 0; i < MAX_ACTIVE_ATTRIBUTE_INDICES; ++i)
		m_ActiveAttributeIndices[i] = false;


	// Set counter clockwize culling
	m_pOgl->glEnable(GL_CULL_FACE);
	m_pOgl->glCullFace(GL_FRONT);
}



// Locking resources
void GraphicsService_OpenGL::BeginScene()
{
	m_pOgl->glEnable(GL_DEPTH_TEST); // Enables Depth Testing
	m_pOgl->glDepthFunc(GL_LEQUAL); // The Type Of Depth Test To Do
}

void GraphicsService_OpenGL::EndScene()
{

}

// setting render states
void GraphicsService_OpenGL::RestoreGraphicsState()
{
	// Todo: add render states here?
	SetCullMode(m_CullMode, true);
}


IShaderProgram* GraphicsService_OpenGL::GetShaderProgram(void) const { return m_pCurrentShaderProgram; }
void GraphicsService_OpenGL::SetShaderProgram(IShaderProgram* pShaderProgram)
{
	ShaderProgram_OpenGL* pShaderProgram_OpenGL = dynamic_cast<ShaderProgram_OpenGL*>(pShaderProgram);

	if (m_pCurrentShaderProgram == pShaderProgram_OpenGL) return;

	// Set the new shader program
	if (pShaderProgram_OpenGL == 0)
	{
		m_pOgl->glUseProgram(0);
	}
	else
	{
		m_pOgl->glUseProgram(pShaderProgram_OpenGL->m_Program);
	}

	m_pCurrentShaderProgram = pShaderProgram_OpenGL;

	// Clear the bound textures
	for (unsigned int i = 0; i < 16; ++i)
	{
		if (m_CurrentTextureSamplerName[i])
		{
			m_pOgl->glActiveTexture(GL_TEXTURE0 + i);
			m_pOgl->glBindTexture(GL_TEXTURE_2D, 0);
			m_CurrentTextureMap[i] = 0;
			m_CurrentTextureSamplerName[i] = 0;
		}
	}
}

// Texture
ITexture* GraphicsService_OpenGL::GetTexture(unsigned int index) const
{
	mini3d_assert(index <= m_pCompatibility->TextureStreamCount() || index <= MAX_TEXTURE_BUFFER_SLOTS, "Trying to access texture outside valid range!");

	return m_CurrentTextureMap[index];
}

void GraphicsService_OpenGL::SetTexture(ITexture* pTexture, const char* name)
{	
	// NOTE: Also look at SetIShaderProgram which clears all bound textures and zeroes
	// m_CurrentITextureMap and m_CurrentITextureSamplerName

	mini3d_assert(m_pCurrentShaderProgram != 0, "Trying to assign a texture to a sampler without having set a shader program!");

	int index = m_pOgl->glGetUniformLocation(m_pCurrentShaderProgram->m_Program, name);
	mini3d_assert(index != -1, "Trying to assign a texture to a sampler that can not be found in the current shader program!");

	// check if this sampler already has a bound texture
	for (unsigned int i = 0; i < MAX_TEXTURE_BUFFER_SLOTS; ++i)
	{
		if (!m_CurrentTextureSamplerName[i] || strcmp(m_CurrentTextureSamplerName[i], name) == 0)
		{
			// Dont set the texture if it is already set!
			if (m_CurrentTextureMap[i] == pTexture)
				return;

			m_pOgl->glActiveTexture(GL_TEXTURE0 + i);

			if (pTexture == 0)
			{
				m_pOgl->glBindTexture(GL_TEXTURE_2D, 0);
				m_CurrentTextureMap[i] = 0;
				
				delete[] m_CurrentTextureSamplerName[i];
				m_CurrentTextureSamplerName[i] = 0;
			}
			else
			{
				// this cast is "unfailable" (not exception caught). Whoever inherits from ITexture must also inherit from ITexture

				BitmapTexture_OpenGL* pBitmapTexture = dynamic_cast<BitmapTexture_OpenGL*>(pTexture);
				RenderTargetTexture_OpenGL* pRenderTargetTexture = dynamic_cast<RenderTargetTexture_OpenGL*>(pTexture);

				if (pBitmapTexture) 
				{ 
					if (pBitmapTexture->isDirty == true) pBitmapTexture->LoadResource(); 

					// Bind the texture
					m_pOgl->glBindTexture(GL_TEXTURE_2D, pBitmapTexture->m_Texture);
				}
				else if (pRenderTargetTexture) 
				{ 
					if (pRenderTargetTexture->isDirty == true) pRenderTargetTexture->LoadResource(); 

					// Bind the texture
					m_pOgl->glBindTexture(GL_TEXTURE_2D, pRenderTargetTexture->m_pTexture);

					// update mim-maps as needed
					if (pRenderTargetTexture->GetMipMapMode() == ITexture::MIPMAP_BOX_FILTER && pRenderTargetTexture->isMipMapDirty)
						m_pOgl->glGenerateMipmap(GL_TEXTURE_2D);
				}

				// Set the sampler to use the texture we just bound
				m_pOgl->glUniform1i(index, i);

				// Set the current texture
				m_CurrentTextureMap[i] = pTexture;
				m_CurrentTextureSamplerName[i] = strcpy(new char[strlen(name) + sizeof(char)], name);
			}
			
			// Texture is bound, we are done!
			return;
		}
	}

	mini3d_assert(false, "Trying to assign a texture to a sampler, but no free texture slots are available!");
}

// Render Target
void GraphicsService_OpenGL::SetRenderTarget(IRenderTarget* pRenderTarget)
{
	// This is a dynamic cast used as a typecheck, code police says this should be solved with virtual function calls instead
	RenderTargetTexture_OpenGL* pCurrentRenderTarget = dynamic_cast<RenderTargetTexture_OpenGL*>(m_pCurrentRenderTarget);

	// If render target is 0, clear the framebuffer and set the default render window.
	if (pRenderTarget == 0)
	{
		// early out if there is no change
		if (pRenderTarget == m_pCurrentRenderTarget)
			return;

		// If there is a bound render target texture, clear it!
		if (pCurrentRenderTarget != 0)
		{
			m_pOgl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// set the current render target to 0
			m_pCurrentRenderTarget = NULL;
		}

        m_pPlatform->SetRenderWindow(0);
		m_pCurrentWindowRenderTarget = 0;
		return;
	}

	// This is a dynamic cast used as a type check, code police says this should be solved with virtual function calls instead
	RenderTargetTexture_OpenGL* pRenderTargetTexture = dynamic_cast<RenderTargetTexture_OpenGL*>(pRenderTarget);

	if (pRenderTargetTexture != 0)
	{
		pRenderTargetTexture->isMipMapDirty;

		if (pRenderTargetTexture->isDirty == true)
			pRenderTargetTexture->LoadResource();

		// If the framebuffer is already set, just return;
		if (m_pCurrentRenderTarget == pRenderTarget)
			return;

		// Bind the new framebuffer
		m_pOgl->glBindFramebuffer(GL_FRAMEBUFFER, pRenderTargetTexture->m_pRenderTarget);
		m_pOgl->glViewport(0,0,pRenderTargetTexture->GetWidth(), pRenderTargetTexture->GetHeight());

		m_pCurrentRenderTarget = pRenderTarget;
		return;
	}

	// This is a dynamic cast used as a type check, code police says this should be solved with virtual function calls instead
	WindowRenderTarget_OpenGL* pWindowRenderTarget = dynamic_cast<WindowRenderTarget_OpenGL*>(pRenderTarget);

	if (pWindowRenderTarget != 0)
	{
		if (pWindowRenderTarget->isDirty == true)
			pWindowRenderTarget->LoadResource();
		
		// Make sure the viewport is correct! 
		// TODO: This hogs performance and could probably be solved in a nicer way.
		pWindowRenderTarget->UpdateSize();
		m_pOgl->glViewport(0,0,pWindowRenderTarget->GetWidth(), pWindowRenderTarget->GetHeight());

		// If there is a bound render target texture, clear it!
		if (pCurrentRenderTarget != NULL)
		{
			m_pOgl->glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// set the current render target to 0
			m_pCurrentRenderTarget = NULL;
		}

		// if this rendertarget is already the current and the correct screen state is set, return
		if (m_pCurrentWindowRenderTarget == pWindowRenderTarget)
			return;
		
		m_pPlatform->SetRenderWindow(pWindowRenderTarget);

		// Because render target textures overrides window render targets we need to store this one also.
		m_pCurrentWindowRenderTarget = pWindowRenderTarget;

		return;
	}
}

// Index Buffer
IIndexBuffer* GraphicsService_OpenGL::GetIndexBuffer() const { return (IIndexBuffer*)m_pCurrentIndexBuffer; }
void GraphicsService_OpenGL::SetIndexBuffer(IIndexBuffer* pIndexBuffer)
{
	IndexBuffer_OpenGL* pIndexBuffer_OpenGL = dynamic_cast<IndexBuffer_OpenGL*>(pIndexBuffer);

	if (m_pCurrentIndexBuffer == pIndexBuffer_OpenGL)
		return;

	if (pIndexBuffer == 0)
	{
		m_pOgl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	else
	{
		if (pIndexBuffer_OpenGL->isDirty == true)
			pIndexBuffer_OpenGL->LoadResource();

		m_pOgl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIndexBuffer_OpenGL->m_IndexBuffer);
	}

	m_pCurrentIndexBuffer = pIndexBuffer_OpenGL;
}

// Vertex Buffer
IVertexBuffer* GraphicsService_OpenGL::GetVertexBuffer(unsigned int streamIndex) const
{
	mini3d_assert(streamIndex <= m_pCompatibility->VertexStreamCount() || streamIndex <= MAX_VERTEX_BUFFER_SLOTS, "Trying to get a Vertex Buffer with a stream index outside the valid interval");
	return m_CurrentVertexBufferMap[streamIndex];
}

void GraphicsService_OpenGL::SetVertexBuffer(IVertexBuffer* pVertexBuffer, unsigned int streamIndex)
{
	mini3d_assert(streamIndex <= m_pCompatibility->VertexStreamCount() || streamIndex <= MAX_VERTEX_BUFFER_SLOTS, "Trying to set a Vertex Buffer with a stream index outside the valid interval");

	VertexBuffer_OpenGL* pVertexBuffer_OpenGL = dynamic_cast<VertexBuffer_OpenGL*>(pVertexBuffer);

	if (m_CurrentVertexBufferMap[streamIndex] == pVertexBuffer) return;

	if (pVertexBuffer_OpenGL && pVertexBuffer_OpenGL->isDirty == true) pVertexBuffer_OpenGL->LoadResource();

	// Just set this for now. The actual binding will be done when draw method is called
	m_CurrentVertexBufferMap[streamIndex] = pVertexBuffer_OpenGL;
}

// Shader Parameters
void GraphicsService_OpenGL::SetShaderParameterMatrix4x4(const char* name, const float* pData)
{
	mini3d_assert(m_pCurrentShaderProgram != 0, "Trying to set a Shader Parameter Matrix without having set a Shader Program!");

	int location = m_pOgl->glGetUniformLocation(m_pCurrentShaderProgram->m_Program, name);
	mini3d_assert(location != -1, "Trying to ste a Shader Parameter Matrix but the name could not be found in the currently set Shader Program!");

	// TODO: Fix all of them! Or should the API be different?
	m_pOgl->glUniformMatrix4fv(location, 1, GL_FALSE, pData);
}

void GraphicsService_OpenGL::SetShaderParameterFloat(const char* name, const float* pData, unsigned int count)
{
	mini3d_assert(m_pCurrentShaderProgram != 0, "Trying to set a float Shader Parameter without having set a Shader Program!");

	int location = m_pOgl->glGetUniformLocation(m_pCurrentShaderProgram->m_Program, name);
	mini3d_assert(location != -1, "Trying to set a Shader Parameter but the name could not be found in the currently set Shader Program!");

    if (count == 1)
		m_pOgl->glUniform1f(location, *pData);
	else if (count == 2)
		m_pOgl->glUniform2f(location, *pData, *(pData + 1));
	else if (count == 3)
		m_pOgl->glUniform3f(location, *pData, *(pData + 1), *(pData + 2));
	else if (count == 4)
		m_pOgl->glUniform4f(location, *pData, *(pData + 1), *(pData + 2), *(pData + 3));
    else
		m_pOgl->glUniform4fv(location, count, pData);
}

void GraphicsService_OpenGL::SetShaderParameterInt(const char* name, const int* pData, unsigned int count)
{
	mini3d_assert(m_pCurrentShaderProgram != 0, "Trying to set an int Shader Parameter without having set a Shader Program!");

	int location = m_pOgl->glGetUniformLocation(m_pCurrentShaderProgram->m_Program, name);
	mini3d_assert(location != -1, "Trying to set an int Shader Parameter but the name could not be found in the currently set Shader Program!");

	if (count == 1)
		m_pOgl->glUniform1i(location, *pData);
	else if (count == 2)
		m_pOgl->glUniform2i(location, *pData, *(pData + 1));
	else if (count == 3)
		m_pOgl->glUniform3i(location, *pData, *(pData + 1), *(pData + 2));
	else if (count == 4)
		m_pOgl->glUniform4i(location, *pData, *(pData + 1), *(pData + 2), *(pData + 3));
    else
		m_pOgl->glUniform4iv(location, count, pData);
}

// Set all vertex attributes for all streams with vertex buffers
void GraphicsService_OpenGL::UpdateVertexAttributes()
{
	bool activeAttributeIndices[MAX_ACTIVE_ATTRIBUTE_INDICES] = {};
	bool attributeInstancing = false;
	m_InstanceCount = 1;

	// Loop over all streams
	for (unsigned int streamIndex = 0; streamIndex < MAX_VERTEX_BUFFER_SLOTS; ++streamIndex)
	{
		// Skip empty streams
		if (m_CurrentVertexBufferMap[streamIndex] == 0)
			continue;
		
		m_pOgl->glBindBuffer(GL_ARRAY_BUFFER, m_CurrentVertexBufferMap[streamIndex]->m_VertexBuffer);

		GLuint program = m_pCurrentShaderProgram->m_Program;

		unsigned int componentDescriptionCount;
		const IVertexBuffer::ComponentDescription* pComponentDescriptions = m_CurrentVertexBufferMap[0]->GetComponentDescriptions(componentDescriptionCount); 

		unsigned int stride = 0;
		for(unsigned int i = 0; i < componentDescriptionCount; ++i)
			stride += pComponentDescriptions[i].count * 4;

		unsigned int offset = 0;
		for(unsigned int i = 0; i < componentDescriptionCount; ++i)
		{
			GLint index = m_pOgl->glGetAttribLocation(program, pComponentDescriptions[i].name);
			
			if (index == -1)
			{
				offset += pComponentDescriptions[i].count * 4;
				continue; // TODO: Display debug information
			}
			
			GLenum bindingType;
			switch (pComponentDescriptions[i].type) {
				case IVertexBuffer::DATA_TYPE_FLOAT:
				default:
					bindingType = GL_FLOAT;
			}
			
			GLenum size = pComponentDescriptions[i].count;
			m_pOgl->glVertexAttribPointer(index, size, bindingType, false, stride, (GLvoid*)offset);
			m_pOgl->glEnableVertexAttribArray(index);

			// Set geometry instancing if requested and supported
			// TODO: Move somewhere other than platform!
			//if (m_pPlatform->VERSION_3_3())
            if (false)
			{
				unsigned int divisor = 0;
				switch(m_CurrentVertexBufferMap[streamIndex]->GetStreamMode()) {
					case IVertexBuffer::STREAM_PER_INSTANCE:
						divisor = 1;
						attributeInstancing = true;
						m_InstanceCount = m_CurrentVertexBufferMap[streamIndex]->GetVertexCount(); // TODO: Error if this is set and then reset to something else!
						break;
					case IVertexBuffer::STREAM_PER_VERTEX:
					default:
						divisor = 0;
				}
				m_pOgl->glVertexAttribDivisor(index, divisor);
			}
			activeAttributeIndices[index] = 1;

			offset += pComponentDescriptions[i].count * 4;
		}
	}

	m_IsUsingInstancedAttributes = attributeInstancing;

	// Turn off inactive attribute indices
	for (unsigned int i = 0; i < MAX_ACTIVE_ATTRIBUTE_INDICES; ++i)
	{
		if (activeAttributeIndices[i] == false && m_ActiveAttributeIndices[i] == true)
			m_pOgl->glDisableVertexAttribArray(i);
		
		m_ActiveAttributeIndices[i] = activeAttributeIndices[i];
	}
}

void GraphicsService_OpenGL::SetCullMode(CullMode cullMode, bool force)
{
	if (m_CullMode == cullMode && !force)
		return;
	
	m_CullMode = cullMode;

	if (cullMode == CULL_CLOCKWIZE)
	{
		m_pOgl->glEnable(GL_CULL_FACE);
		m_pOgl->glCullFace(GL_BACK);
	}
	else if (cullMode == CULL_COUNTERCLOCKWIZE)
	{
		m_pOgl->glEnable(GL_CULL_FACE);
		m_pOgl->glCullFace(GL_FRONT);
	}
	else
	{
		m_pOgl->glDisable(GL_CULL_FACE);
	}
}

// Drawing
void GraphicsService_OpenGL::Draw()
{
	BeginScene();

	// Only draw if we have an index buffer set
	if(m_pCurrentIndexBuffer == 0)
		return;

	// Only draw if we have a shader program set
	if (m_pCurrentShaderProgram == 0)
		return;

	// Make sure buffers are correctly bound and vertex attributes are correctly set
	UpdateVertexAttributes();

	// Draw the scene
	if (m_IsUsingInstancedAttributes) // && m_pPlatform->VERSION_3_3()) TODO: Move somewhere other than platform
	{
		// Draw using instancing
		m_pOgl->glDrawElementsInstanced(GL_TRIANGLES, m_pCurrentIndexBuffer->GetIndexCount(), GL_UNSIGNED_INT, 0, m_InstanceCount);
	}
	else
	{
		// TODO: glDrawRangeElements is faster than glDrawElements according to: http://www.spec.org/gwpg/gpc.static/vbo_whitepaper.html
		m_pOgl->glDrawElements(GL_TRIANGLES, m_pCurrentIndexBuffer->GetIndexCount(), GL_UNSIGNED_INT, 0);
	}
}

void GraphicsService_OpenGL::DrawIndices(unsigned int startIndex, unsigned int numIndices)
{
	BeginScene();

	// Only draw if we have an index buffer set
	if(m_pCurrentIndexBuffer == 0)
		return;

	// Only draw if we have a shader program set
	if (m_pCurrentShaderProgram == 0)
		return;

	// Make sure buffers are correctly bound and vertex attributes are correctly set
	UpdateVertexAttributes();

	m_pOgl->glDrawRangeElements(GL_TRIANGLES, startIndex, numIndices + startIndex, numIndices / 3, GL_UNSIGNED_INT, 0);
}

// Clear
void GraphicsService_OpenGL::Clear(float depth)
{
	m_pOgl->glClearDepth(depth);
	m_pOgl->glClear(GL_DEPTH_BUFFER_BIT);
}

void GraphicsService_OpenGL::Clear(float r, float g, float b, float a)
{
	m_pOgl->glClearColor(r, g, b, a);
	m_pOgl->glClear(GL_COLOR_BUFFER_BIT);
}

void GraphicsService_OpenGL::Clear(float r, float g, float b, float a, float depth)
{
	m_pOgl->glClearColor(r, g, b, a);
	m_pOgl->glClearDepth(depth);
	m_pOgl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GraphicsService_OpenGL::AddResource(Resource* resource)
{
	// Intrusive linked list
	resource->next = m_pResourceList;
	if (m_pResourceList) m_pResourceList->prev = resource;
	
	m_pResourceList = resource;
}
void GraphicsService_OpenGL::RemoveResource(Resource* resource)
{
	if (m_pResourceList == resource) m_pResourceList = resource->next;

	// Intrusive linked list
	if (resource->prev) resource->prev->next = resource->next ? resource->next : 0; 
	if (resource->next) resource->next->prev = resource->prev ? resource->prev : 0; 
	resource->prev = resource->next = 0;
}

void GraphicsService_OpenGL::UpdateResources()
{
	for (Resource* resource = m_pResourceList; resource != 0; resource = resource->next)
		if (resource->isDirty == true)
			resource->LoadResource();
}
void GraphicsService_OpenGL::UnloadResources()
{
	for (Resource* resource = m_pResourceList; resource != 0; resource = resource->next)
		resource->UnloadResource();
}


#endif // MINI3D_GRAPHICSSERVICE_OPENGL