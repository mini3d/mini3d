
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#include "../graphicsservice.hpp"

#ifdef MINI3D_GRAPHICSSERVICE_OPENGL

#include <cstdio>

#include "platform/iplatform.hpp"
#include "platform/openglwrapper.hpp"
#include <cstring>
#include <cstdlib>

void mini3d_assert(bool expression, const char* text, ...);

typedef unsigned int uint;

// TODO: Debug information from glGetError


///////// NAMESPACE MINI3D ////////////////////////////////////////////////////

namespace mini3d {
namespace graphics {

class OpenGlWrapper;
struct IPlatform;


///////// FOREWARD DECLARATIONS ///////////////////////////////////////////////

struct WindowRenderTarget_OpenGL;
struct VertexBuffer_OpenGL;
struct IndexBuffer_OpenGL;
struct ShaderProgram_OpenGL;
struct GraphicsService_OpenGL;

struct Size { uint width; uint height; };

///////// CONSTANTS ///////////////////////////////////////////////////////////

const uint LOG_TEXT_MAX_LENGTH = 32768; // Max length of shader log texts
char logText[LOG_TEXT_MAX_LENGTH];

const uint MAX_TEXTURE_SAMPLER_NAME_LENGTH = 127;
// TODO: These constants should be dynamic depending on graphics-card capabilities
// also dynamically create the corresponding arrays in the graphics service class.
const uint MAX_VERTEX_BUFFER_SLOTS = 16;
const uint MAX_ACTIVE_ATTRIBUTE_INDICES = 32; // How many attributes you can use in a single shader


const unsigned int mini3d_IndexBuffer_OpenGL_BytesPerIndex[] = { 2, 4 };

// TODO: move these to where they are used!
struct OpenglBitmapFormat { GLuint internalFormat; GLenum format; GLenum type; };
OpenglBitmapFormat mini3d_BitmapTexture_Formats[] = { {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0} };
unsigned int mini3d_BitmapTexture_BytesPerPixel[] = { 4, 8, 2, 4, 4 };


///////// GRAPHICS SERVICE ////////////////////////////////////////////////////

struct GraphicsService_OpenGL : IGraphicsService
{


///////// COMPATIBILITY ///////////////////////////////////////////////////////

struct Compatibility_OpenGL : ICompatibility
{
    uint GetTextureUnitCount() const                                            { return m_textureUnitCount; }
    uint MaxTextureSize() const                                                 { return m_maxTextureSize; }
    bool TextureFormat(IBitmapTexture::Format format) const                     { return true; } // TODO: Fix this
    bool RenderTargetTextureFormat(IRenderTargetTexture::Format format) const   { return true; } // TODO: Fix this
    const char* ShaderLanguage() const                                          { return "GLSL"; } // TODO: implementation specific, could also be GLSL ES!
    const char* PixelShaderVersion() const                                      { return m_PixelShaderVersion; }
    const char* VertexShaderVersion() const                                     { return m_VertexShaderVersion; }
    uint VertexStreamCount() const                                              { return 1; } // TODO: Fix this
    uint FreeGraphicsMemory() const                                             { return 0; } // TODO: Fix this

    ~Compatibility_OpenGL() {};

    Compatibility_OpenGL(GraphicsService_OpenGL* pGraphicsService)
    {
        m_pGraphicsService = pGraphicsService;

        GLint unit;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &unit);
        m_maxTextureSize = (unsigned int)unit;
    
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &unit);
        m_textureUnitCount = (unsigned int)unit;
    
        m_PixelShaderVersion = m_VertexShaderVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    }

    unsigned int m_maxTextureSize;
    unsigned int m_textureUnitCount;
    
    const char* m_PixelShaderVersion;
    const char* m_VertexShaderVersion;

    GraphicsService_OpenGL* m_pGraphicsService;
};


///////// INDEX BUFFER ////////////////////////////////////////////////////////

struct IndexBuffer_OpenGL : IIndexBuffer
{
    unsigned int GetIndexCount() const              { return m_indexCount; }
    DataType GetDataType() const                    { return m_dataType; }
    GLuint GetGLIndexBuffer() const                 { return m_glIndexBuffer; }
    ~IndexBuffer_OpenGL()                           { glDeleteBuffers(1, &m_glIndexBuffer); if (m_pGraphicsService->GetIndexBuffer() == this) m_pGraphicsService->SetIndexBuffer(0); }

    IndexBuffer_OpenGL(GraphicsService_OpenGL* pGraphics, const void* pIndices, unsigned int count, DataType dataType) : m_pGraphicsService(pGraphics), m_bufferSizeInBytes(0)
    {
        glGenBuffers(1, &m_glIndexBuffer);
        SetIndices(pIndices, count, dataType);
    }

    void SetIndices(const void* pIndices, unsigned int count, DataType dataType)
    {
        if (m_pGraphicsService->GetIndexBuffer() == this)
            m_pGraphicsService->SetIndexBuffer(0);

        unsigned int sizeInBytes = count * mini3d_IndexBuffer_OpenGL_BytesPerIndex[dataType];

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeInBytes, pIndices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        m_bufferSizeInBytes = sizeInBytes;
        m_dataType = dataType;
        m_indexCount = count;
    }

private:
	unsigned int m_indexCount;
    unsigned int m_bufferSizeInBytes;
	IIndexBuffer::DataType m_dataType;
	GLuint m_glIndexBuffer;
	GraphicsService_OpenGL* m_pGraphicsService;
};


///////// VERTEX BUFFER ///////////////////////////////////////////////////////

struct VertexBuffer_OpenGL : IVertexBuffer
{
    uint GetVertexCount() const                                     { return m_vertexCount; }
    uint GetVertexSizeInBytes() const                               { return m_vertexSizeInBytes; }
   GLuint GetGLVertexBuffer()                                       { return m_glVertexBuffer; }
    ~VertexBuffer_OpenGL()                                          { glDeleteBuffers(1, &m_glVertexBuffer); Unbind(); }

    VertexBuffer_OpenGL(GraphicsService_OpenGL* pGraphics, const void* pVertices, unsigned int vertexCount, unsigned int vertexSizeInBytes) :
        m_pGraphicsService(pGraphics), m_vertexCount(0), m_vertexSizeInBytes(vertexSizeInBytes)
    {
        glGenBuffers(1, &m_glVertexBuffer);
        SetVertices(pVertices, vertexCount, vertexSizeInBytes);
    }

    void SetVertices(const void* pVertices, unsigned int vertexCount, unsigned int vertexSizeInBytes)
    {

        Unbind();

        unsigned int sizeInBytes = vertexCount * vertexSizeInBytes;

        glBindBuffer(GL_ARRAY_BUFFER, m_glVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeInBytes, pVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        m_vertexSizeInBytes = vertexSizeInBytes;
        m_vertexCount = vertexCount;
    }

    void Unbind()
    {
        for (unsigned int i = 0; i < MAX_VERTEX_BUFFER_SLOTS; ++i)
            if (m_pGraphicsService->GetVertexBuffer(i) == this) 
                m_pGraphicsService->SetVertexBuffer(0, i);
    }

private:
    // Vertices
    unsigned int m_vertexCount;
	unsigned int m_vertexSizeInBytes;

	GLuint m_glVertexBuffer;

	GraphicsService_OpenGL* m_pGraphicsService;
};


///////// PIXEL SHADER ////////////////////////////////////////////////////////

struct PixelShader_OpenGL : IPixelShader
{
    GLuint GetGLShader() const                  { return m_glShader; }
    ~PixelShader_OpenGL()                       { glDeleteShader(m_glShader); }

    PixelShader_OpenGL(GraphicsService_OpenGL* pGraphics, const char* pShaderBytes, unsigned int sizeInBytes) : m_pGraphicsService(pGraphics)
    {    
        m_glShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(m_glShader, 1, (const GLchar**)&pShaderBytes, (GLint*)&sizeInBytes);
        glCompileShader(m_glShader);

        GLsizei length;
        glGetShaderInfoLog(m_glShader, LOG_TEXT_MAX_LENGTH, &length, logText);
        mini3d_assert(length == 0, "Pixel shader compilation log:\n%s\n", logText);
    }

private:
	GLuint m_glShader;
	GraphicsService_OpenGL* m_pGraphicsService;
};


///////// VERTEX SHADER ////////////////////////////////////////////////////////

struct VertexShader_OpenGL : IVertexShader
{
    GLuint GetGLShader() const                   { return m_glShader; }
    ~VertexShader_OpenGL()                       { glDeleteShader(m_glShader); }

    VertexShader_OpenGL(GraphicsService_OpenGL* pGraphics, const char* pShaderBytes, unsigned int sizeInBytes) : m_pGraphicsService(pGraphics)
    {    
        m_glShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(m_glShader, 1, (const GLchar**)&pShaderBytes, (GLint*)&sizeInBytes);
        glCompileShader(m_glShader);

        GLsizei length;
        glGetShaderInfoLog(m_glShader, LOG_TEXT_MAX_LENGTH, &length, logText);
        mini3d_assert(length == 0, "Vertex shader compilation log:\n%s\n", logText);
    }

private:
	GLuint m_glShader;
	GraphicsService_OpenGL* m_pGraphicsService;
};


///////// SHADER PROGRAM ///////////////////////////////////////////////////////

struct ShaderProgram_OpenGL : IShaderProgram
{
    struct ActiveAttribute { char* name; GLint location; GLint size; GLenum type; }; 
    struct ActiveUniform   { char* name; GLint location; GLint size; GLenum type; }; 

    IPixelShader* GetPixelShader() const                        { return (IPixelShader*)m_pPixelShader; }
    IVertexShader* GetVertexShader() const                      { return (IVertexShader*)m_pVertexShader; }
    GLuint GetGLProgram() const                                 { return m_glProgram; }
    ActiveAttribute* GetActiveAttributes(unsigned int &count)   { count = m_activeAttributeCount; return m_pActiveAttributes; }
    ActiveUniform* GetActiveUniforms(unsigned int &count)       { count = m_activeUniformCount; return m_pActiveUniforms; }

    GLenum GetType(GLenum type)
    {
        switch (type)
        {
            case GL_FLOAT:
            case GL_FLOAT_VEC2:
            case GL_FLOAT_VEC3:
            case GL_FLOAT_VEC4:
                return GL_FLOAT;
            case GL_INT:
            case GL_INT_VEC2:
            case GL_INT_VEC3:
            case GL_INT_VEC4:
                return GL_INT;
        }
        
        mini3d_assert(false, "Unknown active attribute type!");
        return 0;
    }

    GLint GetSize(GLenum type)
    {
        switch (type)
        {
            case GL_FLOAT:
            case GL_INT:
                return 1;
            case GL_FLOAT_VEC2:
            case GL_INT_VEC2:
                return 2;
            case GL_FLOAT_VEC3:
            case GL_INT_VEC3:
                return 3;
            case GL_FLOAT_VEC4:
            case GL_INT_VEC4:
                return 4;
        }
        
        mini3d_assert(false, "Unknown active attribute type!");
        return 0;
    }

    ShaderProgram_OpenGL(GraphicsService_OpenGL* pGraphics, IVertexShader* pVertexShader, IPixelShader* pPixelShader) : 
        m_pGraphicsService(pGraphics), m_pVertexShader(pVertexShader), m_pPixelShader(pPixelShader)
    {
        m_glProgram = glCreateProgram();
        glAttachShader(m_glProgram, ((VertexShader_OpenGL*)m_pVertexShader)->GetGLShader());
        glAttachShader(m_glProgram, ((PixelShader_OpenGL*)m_pPixelShader)->GetGLShader());

        // Link the program
        glLinkProgram(m_glProgram);
        glValidateProgram(m_glProgram);

        // Get a list of all active attributes
        GLint maxNameLength, size;
        GLenum type;

        glGetProgramiv(m_glProgram, GL_ACTIVE_ATTRIBUTES, &m_activeAttributeCount);
        glGetProgramiv(m_glProgram, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLength);

        char *name = new char[maxNameLength];
        m_pActiveAttributes = new ActiveAttribute[m_activeAttributeCount];

        for (int i = 0; i < m_activeAttributeCount; i++)
        {
            glGetActiveAttrib(m_glProgram, i, maxNameLength, NULL, &size, &type, name);
            // TODO: Cleanup, including free of strudp name!
            ActiveAttribute att = { strdup(name), glGetAttribLocation(m_glProgram, name), GetSize(type), GetType(type) };
            m_pActiveAttributes[i] = att;

            printf("Attribute Name: %s, Type: %d, Location: %d, Size:%d\n", name, type, glGetAttribLocation(m_glProgram, name), size);
        }
        delete[] name;

        // Get a list of all active uniforms
        glGetProgramiv(m_glProgram, GL_ACTIVE_UNIFORMS, &m_activeUniformCount);
        glGetProgramiv(m_glProgram, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);

        name = new char[maxNameLength];
        m_pActiveUniforms = new ActiveUniform[m_activeUniformCount];

        for (int i = 0; i < m_activeUniformCount; i++)
        {
            glGetActiveUniform(m_glProgram, i, maxNameLength, NULL, &size, &type, name);
            // TODO: Cleanup, including free of strudp name!
            ActiveUniform uni = { strdup(name), glGetUniformLocation(m_glProgram, name), size, type };
            m_pActiveUniforms[i] = uni;

            printf("Uniform Name: %s, Type: %d, Location: %d, Size:%d\n", name, type, glGetUniformLocation(m_glProgram, name), size);
        }
        delete[] name;


    }

    ~ShaderProgram_OpenGL() 
    { 
        glDeleteProgram(m_glProgram); 
        
        if (m_pGraphicsService->GetShaderProgram() == this) 
            m_pGraphicsService->SetShaderProgram(0); 
    
        for (GLint i = 0; i < m_activeAttributeCount; ++i)
            free(m_pActiveAttributes[i].name);

        for (GLint i = 0; i < m_activeUniformCount; ++i)
            free(m_pActiveUniforms[i].name);

        delete[] m_pActiveAttributes; 
        delete[] m_pActiveUniforms;
    }


private:
    IPixelShader* m_pPixelShader;
	IVertexShader* m_pVertexShader;
	GLuint m_glProgram;

    ActiveAttribute* m_pActiveAttributes;
    GLint m_activeAttributeCount;

    ActiveUniform* m_pActiveUniforms;
    GLint m_activeUniformCount;

    GraphicsService_OpenGL* m_pGraphicsService;
};


///////// SHADER INPUT LAYOUT //////////////////////////////////////////////////

struct ShaderInputLayout_OpenGL : IShaderInputLayout
{
    struct Attribute { GLuint location; GLint size; GLenum type; unsigned int bufferIndex; GLvoid* offsetInBytes; StreamRate rate; }; 

    unsigned int GetInputElementCount() const { return m_attributeCount; };
    void GetInputElements(InputElement* pElements) const { for(unsigned int i = 0; i < m_attributeCount; ++i) pElements[i]= m_pElements[i]; };

    ShaderInputLayout_OpenGL(GraphicsService_OpenGL* pGraphics, IShaderProgram* pShader, InputElement* pElements, unsigned int count)
    {
        m_pGraphicsService = pGraphics;

        // Get data for all the elements in the pElements array
        unsigned int activeCount;
        ShaderProgram_OpenGL::ActiveAttribute* pActiveAttributes = ((ShaderProgram_OpenGL*)pShader)->GetActiveAttributes(activeCount);

        m_pAttributes = new Attribute[count];
        m_pElements = new InputElement[count];
        for (unsigned int i = 0; i < count; ++i)
        {
            // TODO: move search to shader program code!
            for (unsigned int j = 0; j < activeCount; ++j)
            {
                if (!strcmp(pElements[i].nameGLSL, pActiveAttributes[j].name))
                {
                    Attribute att = { pActiveAttributes[j].location, pActiveAttributes[j].size, pActiveAttributes[j].type, pElements[i].vertexBufferIndex, (GLvoid *)pElements[i].offsetInBytes, pElements[i].rate };
                    m_pAttributes[i] = att;
                    break;
                }
            }
            // TODO: error check here if not found!

            m_pElements[i] = pElements[i];
        }
        m_attributeCount = count;
    }

    ~ShaderInputLayout_OpenGL()
    {
        delete[] m_pAttributes;
        delete[] m_pElements;
    }

    void BindAttributes()
    {
        // Bind all shader attributes to vertex buffers
        VertexBuffer_OpenGL* pVertexBuffer;
        for (unsigned int i = 0; i < m_attributeCount; ++i)
        {
            if (i == 0 || m_pAttributes[i].bufferIndex != m_pAttributes[i-1].bufferIndex)
            {
                pVertexBuffer = (VertexBuffer_OpenGL*)m_pGraphicsService->GetVertexBuffer(m_pAttributes[i].bufferIndex);
                mini3d_assert(pVertexBuffer != 0, "Missing vertex buffer for shader input attributes!"); // TODO: add names
                glBindBuffer(GL_ARRAY_BUFFER, pVertexBuffer->GetGLVertexBuffer());
            }
            
            glVertexAttribPointer(m_pAttributes[i].location, m_pAttributes[i].size, m_pAttributes[i].type, GL_FALSE, pVertexBuffer->GetVertexSizeInBytes(), m_pAttributes[i].offsetInBytes);
            glEnableVertexAttribArray(m_pAttributes[i].location);
            
            if (false) // Todo: Vertex attrib devisor (opengl 3.3 and higher)
            {
                glVertexAttribDivisor(m_pAttributes[i].location, (unsigned int)m_pAttributes[i].rate);
            }
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

private:

    Attribute* m_pAttributes;
    InputElement* m_pElements;
    unsigned int m_attributeCount;

    GraphicsService_OpenGL* m_pGraphicsService;
};


struct ConstantBuffer_OpenGL : IConstantBuffer
{
    struct Uniform { GLint location; GLenum type; GLint size; unsigned int offsetInBytes; };

    IShaderProgram* GetVertexShader() const { return m_pShader; }

    ConstantBuffer_OpenGL(GraphicsService_OpenGL* pGraphics, unsigned int sizeInBytes, IShaderProgram* pShader, const char** names, unsigned int nameCount)
    {
        m_pData = new char[sizeInBytes];
        m_sizeInBytes = sizeInBytes;

        // Get data for all the elements in the pElements array
        unsigned int activeCount;
        ShaderProgram_OpenGL::ActiveUniform* pActiveUniforms = ((ShaderProgram_OpenGL*)pShader)->GetActiveUniforms(activeCount);

        unsigned int offset = 0;
        m_pUniforms = new Uniform[nameCount];
        for (unsigned int i = 0; i < nameCount; ++i)
        {
            // TODO: move search to shader program code!
            for (unsigned int j = 0; j < activeCount; ++j)
            {
                if (!strcmp(names[i], pActiveUniforms[j].name))
                {
                    Uniform uni = { pActiveUniforms[j].location, pActiveUniforms[j].type, pActiveUniforms[j].size, offset };
                    m_pUniforms[i] = uni;
                    offset += GetUniformSizeInBytes(pActiveUniforms[j].type) * pActiveUniforms[j].size;
                    break;
                }
            }
            // TODO: error check here if not found!
        }
        m_uniformCount = nameCount;
        m_pShader = pShader;
    }

    ~ConstantBuffer_OpenGL()
    {
        delete[] m_pData;
        delete[] m_pUniforms;
    }

    unsigned int GetUniformSizeInBytes(GLenum type)
    {
        switch(type)
        {
            case GL_INT:
            case GL_FLOAT: return 4;
            case GL_INT_VEC2:
            case GL_FLOAT_VEC2: return 8;
            case GL_INT_VEC3:
            case GL_FLOAT_VEC3: return 12;
            case GL_INT_VEC4:
            case GL_FLOAT_VEC4: return 16;
        }
        mini3d_assert(false, "Unknown Uniform type!");
        return 0;
    }

    void SetData(const char* pData)
    {
        memcpy(m_pData, pData, m_sizeInBytes);
    }
    
    void ApplyUniforms()
    {
        for (unsigned int i = 0; i < m_uniformCount; ++i)
        {
            if (m_pUniforms[i].size != 1)
            {
                glUniform4fv(m_pUniforms[i].location, m_pUniforms[i].size, (const GLfloat*)(m_pData + m_pUniforms[i].offsetInBytes));
                continue;
            }

            switch(m_pUniforms[i].type)
            {
                case GL_FLOAT: glUniform1fv(m_pUniforms[i].location, m_pUniforms[i].size, (GLfloat*)(m_pData + m_pUniforms[i].offsetInBytes)); continue;
                case GL_FLOAT_VEC2: glUniform2fv(m_pUniforms[i].location, m_pUniforms[i].size, (GLfloat*)(m_pData + m_pUniforms[i].offsetInBytes)); continue;
                case GL_FLOAT_VEC3: glUniform3fv(m_pUniforms[i].location, m_pUniforms[i].size, (GLfloat*)(m_pData + m_pUniforms[i].offsetInBytes)); continue;
                case GL_FLOAT_VEC4: glUniform4fv(m_pUniforms[i].location, m_pUniforms[i].size, (GLfloat*)(m_pData + m_pUniforms[i].offsetInBytes)); continue;
                case GL_INT:
                case GL_INT_VEC2:
                case GL_INT_VEC3:
                case GL_INT_VEC4:
                    continue;
            }
        }
    }


private:
    
    Uniform* m_pUniforms;
    unsigned int m_uniformCount;

    IShaderProgram* m_pShader;
    char* m_pData;
    unsigned int m_sizeInBytes;
};

///////// BITMAP TEXTURE ///////////////////////////////////////////////////////

struct Texture_OpenGL { virtual GLuint GetGLTexture() const = 0; } ;

struct BitmapTexture_OpenGL : IBitmapTexture, Texture_OpenGL
{
    uint GetWidth() const                               { return m_size.width; };
    uint GetHeight() const                              { return m_size.height; };
    Format GetFormat() const                            { return m_format; };
    SamplerSettings GetSamplerSettings() const          { return m_samplerSettings; };
    ~BitmapTexture_OpenGL()                             { glDeleteTextures(1, &m_glTexture); m_pGraphicsService->UnbindTexture(this); }
    GLuint GetGLTexture() const                         { return m_glTexture; }
	unsigned int GetMax(unsigned int a, unsigned int b) { return (a > b) ? a : b; }

    BitmapTexture_OpenGL(GraphicsService_OpenGL* pGraphics, const char* pBitmap, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings) :
        m_pGraphicsService(pGraphics)
    {
        glGenTextures(1, &m_glTexture);

        SetBitmap(pBitmap, width, height, format, samplerSettings);
    }

    void SetBitmap(const char* pBitmap, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings)
    {
        mini3d_assert(pBitmap != 0, "Setting a Bitmap Texture with a NULL data pointer!");
        mini3d_assert((width & (width - 1)) == 0, "Setting a Bitmap Texture to a non power of two width!");
        mini3d_assert((height & (height - 1)) == 0, "Setting a Bitmap Texture to a non power of two height!");
        mini3d_assert(width >= 64, "Setting a Bitmap Texture to a height less than 64!");
        mini3d_assert(height >= 64, "Setting a Bitmap Texture to a width less than 64!");

        m_pGraphicsService->UnbindTexture(this);

        glBindTexture(GL_TEXTURE_2D, m_glTexture);

        switch(samplerSettings.wrapMode)
        {
            case SamplerSettings::WRAP_TILE: 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                break;
            case SamplerSettings::WRAP_CLAMP:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                break;
        }

        switch(samplerSettings.sampleMode)
        {
        case SamplerSettings::SAMPLE_LINEAR:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (samplerSettings.mipMapMode == SamplerSettings::MIPMAP_NONE) ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerSettings::SAMPLE_NEAREST:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (samplerSettings.mipMapMode == SamplerSettings::MIPMAP_NONE) ? GL_NEAREST : GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        }

        switch(samplerSettings.mipMapMode)
        {
            case SamplerSettings::MIPMAP_NONE:
                glTexImage2D(GL_TEXTURE_2D, 0, mini3d_BitmapTexture_Formats[format].internalFormat, width, height, 0, mini3d_BitmapTexture_Formats[format].format, mini3d_BitmapTexture_Formats[format].type, pBitmap);
                break;
            case SamplerSettings::MIPMAP_AUTOGENERATE:
                glTexImage2D(GL_TEXTURE_2D, 0, mini3d_BitmapTexture_Formats[format].internalFormat, width, height, 0, mini3d_BitmapTexture_Formats[format].format, mini3d_BitmapTexture_Formats[format].type, pBitmap);
                glGenerateMipmap(GL_TEXTURE_2D);
                break;
            case SamplerSettings::MIPMAP_MANUAL: {

                unsigned int level = 0;
                unsigned int mipMapWidth = width;
                unsigned int mipMapHeight = height;
                
                while (mipMapWidth > 1 || mipMapHeight > 1)
                {
                    unsigned int sizeInBytes = GetMax(width >> 1, 1) * GetMax(height >> 1, 1) * mini3d_BitmapTexture_BytesPerPixel[format];
                    glTexImage2D(GL_TEXTURE_2D, level, mini3d_BitmapTexture_Formats[format].internalFormat, mipMapWidth, mipMapHeight, 0, mini3d_BitmapTexture_Formats[format].format, mini3d_BitmapTexture_Formats[format].type, pBitmap);
                    pBitmap += sizeInBytes;

                    // TODO: Cross platform?
                    mipMapWidth >>= 1;
                    mipMapWidth >>= 1;
                    ++level;
                }

            } break;
        }

        // TODO: Saved here if parital texture update is needed in the future!
        //glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, mipMapWidth, mipMapHeight, mini3d_BitmapTexture_Formats[m_Format].format, mini3d_BitmapTexture_Formats[m_Format].type, pMipMap);

        // Clear the current bound texture
        glBindTexture(GL_TEXTURE_2D, 0);

        m_size.width = width;
        m_size.height = height;

        m_format = format;
        m_samplerSettings = samplerSettings;
        m_size.width = width;
        m_size.height = height;
    }

private:
	Size m_size;
	Format m_format;
	SamplerSettings m_samplerSettings;
	GLuint m_glTexture;
	GraphicsService_OpenGL* m_pGraphicsService;
};


///////// RENDER TARGET TEXTURE ///////////////////////////////////////////////

struct RenderTargetTexture_OpenGL : IRenderTargetTexture, Texture_OpenGL
{

    uint GetWidth() const                       { return size.width; };
    uint GetHeight() const                      { return size.height; }
    Viewport GetViewport() const                { Viewport v = {0,0,0,0}; return v; }; // TODO: Proper values :)
    void SetViewport(Viewport viewport)         { }; // TODO: Do something :)
    inline Format GetFormat() const             { return m_format; };
    SamplerSettings GetSamplerSettings() const  { return m_samplerSettings; };
    bool GetDepthTestEnabled() const            { return m_depthTestEnabled; };
    GLuint GetGLTexture() const                 { return m_glTexture; }
    bool GetIsMipMapDirty() const               { return m_isMipMapDirty; }
    void SetIsMipMapDirty(bool isDirty)         { m_isMipMapDirty = isDirty; }
    GLuint GetGLFramebuffer() const             { return m_glFramebuffer; }
    ~RenderTargetTexture_OpenGL()               { glDeleteFramebuffers(1, &m_glFramebuffer); glDeleteTextures(1, &m_glTexture); glDeleteRenderbuffers(1, &m_glDepthStencil); m_pGraphicsService->UnbindTexture(this); }

    RenderTargetTexture_OpenGL(GraphicsService_OpenGL* pGraphicsService, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, bool depthTestEnabled)
    {
        m_pGraphicsService = pGraphicsService;
        glGenFramebuffers(1, &m_glFramebuffer);
        glGenTextures(1, &m_glTexture);
        glGenRenderbuffers(1, &m_glDepthStencil);

        SetRenderTargetTexture(width, height, format, samplerSettings, depthTestEnabled);
    }

    void SetRenderTargetTexture(unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, bool depthTestEnabled)
    {
        mini3d_assert((width & (width - 1)) == 0, "Setting a Bitmap Texture to a non power of two width!");
        mini3d_assert((height & (height - 1)) == 0, "Setting a Bitmap Texture to a non power of two height!");
        mini3d_assert(width >= 64, "Setting a Bitmap Texture to a height less than 64!");
        mini3d_assert(height >= 64, "Setting a Bitmap Texture to a width less than 64!");

        glBindFramebuffer(GL_FRAMEBUFFER, m_glFramebuffer);

        glBindTexture(GL_TEXTURE_2D, m_glTexture);

        switch(samplerSettings.wrapMode)
        {
            case SamplerSettings::WRAP_TILE: 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                break;
            case SamplerSettings::WRAP_CLAMP:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                break;
        }

        switch(samplerSettings.sampleMode)
        {
        case SamplerSettings::SAMPLE_LINEAR:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (samplerSettings.mipMapMode == SamplerSettings::MIPMAP_NONE) ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerSettings::SAMPLE_NEAREST:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (samplerSettings.mipMapMode == SamplerSettings::MIPMAP_NONE) ? GL_NEAREST : GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, mini3d_BitmapTexture_Formats[format].internalFormat, width, height, 0, mini3d_BitmapTexture_Formats[format].format, mini3d_BitmapTexture_Formats[format].type, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_glTexture, 0);


        if (depthTestEnabled)
        {
            glBindRenderbuffer(GL_RENDERBUFFER, m_glDepthStencil);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_glDepthStencil);
        }

        // Make sure the frame buffer is correctly set up and supported
        GLenum FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        mini3d_assert(FBOstatus == GL_FRAMEBUFFER_COMPLETE, "Failed to create framebuffer for Render Target Texture %u, %u", FBOstatus,  glGetError());

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        printf("Loaded RenderTargetTexture: %d\n",glGetError());

        size.width = width;
        size.height = height;
        m_format = format;
        m_depthTestEnabled = depthTestEnabled;
        m_samplerSettings = samplerSettings;
        m_isMipMapDirty = true;
    }

    void Unbind()
    {
        // if we are removing the current render target, restore the default render target first
        if (m_pGraphicsService->GetRenderTarget() == this)
            m_pGraphicsService->SetRenderTarget(0);

        m_pGraphicsService->UnbindTexture(this);
    }

private:
    Size size;
	IRenderTargetTexture::Format m_format;
	ITexture::SamplerSettings m_samplerSettings;
	bool m_depthTestEnabled;
	bool m_isMipMapDirty;

	GLuint m_glTexture;
	GLuint m_glFramebuffer;
	GLuint m_glDepthStencil;

	GraphicsService_OpenGL* m_pGraphicsService;
};


///////// WINDOW RENDER TARGET ////////////////////////////////////////////////

struct WindowRenderTarget_OpenGL : IWindowRenderTarget
{
    uint GetWidth() const                   { return m_pGraphicsService->GetPlatform()->GetNativeSurfaceWidth(m_pNativeSurface); };
    uint GetHeight() const                  { return m_pGraphicsService->GetPlatform()->GetNativeSurfaceHeight(m_pNativeSurface); };
    Viewport GetViewport() const            { Viewport v = {0,0,0,0}; return v; }; // TODO: Create viewport member variable
    void SetViewport(Viewport viewport)     { }; // TODO: implement!
    bool GetDepthTestEnabled() const        { return m_depthTestEnabled; };
    void* GetNativeWindow() const           { return m_pNativeWindow; }
    void* GetNativeSurface() const          { return m_pNativeSurface; }
    void Display()                          { m_pGraphicsService->GetPlatform()->SwapWindowBuffers(m_pNativeSurface); }
    void Unbind()                           { if (m_pGraphicsService->GetRenderTarget() == this) m_pGraphicsService->SetRenderTarget(0); }
    ~WindowRenderTarget_OpenGL()            { m_pGraphicsService->GetPlatform()->UnPrepareWindow(m_pNativeWindow, m_pNativeSurface); Unbind(); }

    WindowRenderTarget_OpenGL(GraphicsService_OpenGL* pGraphicsService, void* pNativeWindow, bool depthTestEnabled)
    {
        m_pGraphicsService = pGraphicsService;
        m_pNativeWindow = 0;
        m_screenState = SCREEN_STATE_WINDOWED;

        SetWindowRenderTarget(pNativeWindow, depthTestEnabled);
    }


    void SetWindowRenderTarget(void* nativeWindow, bool depthTestEnabled)
    {
        mini3d_assert(nativeWindow != 0, "Can not set null pointer as nativeWindow!");

        Unbind();

        // set the variables from the call
        m_pNativeWindow = nativeWindow;
        m_depthTestEnabled = depthTestEnabled;

        IPlatform* pPlatform = m_pGraphicsService->GetPlatform();
        m_pNativeSurface = pPlatform->PrepareWindow(m_pNativeWindow);

        printf("Loaded WindowRenderTarget: %d\n", glGetError());
    }

private:
	void* m_pNativeWindow; // Native window pointer (HWND, Window, ...)
	void* m_pNativeSurface; // Native window drawing surface pointer. See platform inplementation classes for details.
    bool m_depthTestEnabled;
	int m_bufferDepthTestEnabled;
	ScreenState m_screenState;

	GraphicsService_OpenGL* m_pGraphicsService;
};


///////// GRAPHICS SERVICE ////////////////////////////////////////////////////

    IRenderTarget* GetRenderTarget() const          { return m_pCurrentRenderTarget; }
    const ICompatibility* GetCompatibility() const  { return m_pCompatibility; }
    inline IPlatform* GetPlatform()                 { return m_pPlatform; };
    void BeginScene()                               { glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); }
    void EndScene()                                 { }

    GraphicsService_OpenGL()
    {
        m_IsUsingInstancedAttributes = false;

        m_pCurrentShaderProgram = 0;
        m_pCurrentWindowRenderTarget = 0;
        m_pCurrentRenderTarget = 0;
        m_pCurrentShaderInputLayout = 0;
        m_pCurrentConstantBuffer = 0;

        m_pPlatform = IPlatform::New();
        initOpenGL();

        m_pCompatibility = new Compatibility_OpenGL(this);

        for (unsigned int i = 0; i < MAX_VERTEX_BUFFER_SLOTS; ++i)
            m_CurrentVertexBufferMap[i] = 0;

        unsigned int textureImageUnitCount = m_pCompatibility->GetTextureUnitCount();
        m_pTextureBindings = new TextureBinding[textureImageUnitCount];
        m_boundTextureCount = 0;

        // Set initial mappings for the texture image unit bindings
        for (unsigned int i = 0; i < textureImageUnitCount; ++i)
        {
            m_pTextureBindings[i].index = i;
            *m_pTextureBindings[i].name = 0;
        }

        for (unsigned int i = 0; i < MAX_ACTIVE_ATTRIBUTE_INDICES; ++i)
            m_ActiveAttributeIndices[i] = false;

        SetCullMode(CULL_CLOCKWIZE);
    }

    ~GraphicsService_OpenGL() 
    { 
        delete m_pPlatform; 
        delete m_pCompatibility; 
        delete[] m_pTextureBindings;
    }


    IShaderProgram* GetShaderProgram(void) const { return m_pCurrentShaderProgram; }
    void SetShaderProgram(IShaderProgram* pShaderProgram)
    {
        ShaderProgram_OpenGL* pShaderProgram_OpenGL = (ShaderProgram_OpenGL*)pShaderProgram;

        if (m_pCurrentShaderProgram == pShaderProgram_OpenGL) 
            return;

        if (pShaderProgram_OpenGL == 0)
            glUseProgram(0);
        else
            glUseProgram(pShaderProgram_OpenGL->GetGLProgram());

        // Clear the bound textures
        m_boundTextureCount = 0;

        m_pCurrentShaderProgram = pShaderProgram_OpenGL;
    }

    // Texture
    // TODO: Should be based on name instead
    ITexture* GetTexture(unsigned int idD3D, const char* nameOGL) const
    {
        //mini3d_assert(index <= m_pCompatibility->GetTextureUnitCount(), "Trying to access texture outside valid range!");
        return 0; 
    }

    void UnbindTexture(ITexture* pTexture)
    {
        for (unsigned int i = 0; i < m_boundTextureCount; ++i)
        {
            if (m_pTextureBindings[i].pTexture == pTexture)
            {
                GLint freedIndex = m_pTextureBindings[i].index;
                m_pTextureBindings[i] = m_pTextureBindings[--m_boundTextureCount];
                
                m_pTextureBindings[m_boundTextureCount].index = freedIndex;
            }
        }
    }

    void SetTexture(ITexture* pTexture, unsigned int idD3D, const char* nameOGL)
    {
        GLenum err = glGetError();
        mini3d_assert(m_pCurrentShaderProgram != 0, "Trying to assign a texture to a sampler without having set a shader program!");

        // check if this sampler already has a bound texture
        unsigned int i = 0;
        while (i < m_boundTextureCount && strcmp(m_pTextureBindings[i].name, nameOGL) != 0)
            ++i;

        if (i < m_boundTextureCount && m_pTextureBindings[i].pTexture == pTexture)
            return;

        GLint index = glGetUniformLocation(m_pCurrentShaderProgram->GetGLProgram(), nameOGL);
        mini3d_assert(index != -1, "Trying to assign a texture to a sampler that can not be found in the current shader program!");

        if (pTexture == 0)
        {
            if (i == m_boundTextureCount)
                return;

            GLint freedIndex = m_pTextureBindings[i].index;
            m_pTextureBindings[i] = m_pTextureBindings[--m_boundTextureCount];
            m_pTextureBindings[m_boundTextureCount].index = freedIndex;
            
            glActiveTexture(GL_TEXTURE0 + freedIndex);
            glBindTexture(GL_TEXTURE_2D, 0);
            glUniform1i(index, freedIndex);
        }
        else
        {
            // Assign to the next spot
            unsigned int textureUnitCount = m_pCompatibility->GetTextureUnitCount();
            mini3d_assert(i < textureUnitCount, "Assigning to many simultanous textures to samplers! Max amount is: %u!", textureUnitCount);

            glActiveTexture(GL_TEXTURE0 + m_pTextureBindings[i].index);

            if (pTexture->GetType() == IBitmapTexture::TYPE)
            { 
                BitmapTexture_OpenGL* pBitmapTexture = (BitmapTexture_OpenGL*)pTexture;
                glBindTexture(GL_TEXTURE_2D, pBitmapTexture->GetGLTexture());
            }
            else if (pTexture->GetType() == IRenderTargetTexture::TYPE) 
            { 
                RenderTargetTexture_OpenGL* pRenderTargetTexture = (RenderTargetTexture_OpenGL*)pTexture;
                glBindTexture(GL_TEXTURE_2D, pRenderTargetTexture->GetGLTexture());

                if (pRenderTargetTexture->GetSamplerSettings().mipMapMode == ITexture::SamplerSettings::MIPMAP_AUTOGENERATE && pRenderTargetTexture->GetIsMipMapDirty())
                {
                    glGenerateMipmap(GL_TEXTURE_2D);
                    pRenderTargetTexture->SetIsMipMapDirty(false);
                }
            }

            // Set the sampler to use the texture we just bound
            glUniform1i(index, m_pTextureBindings[i].index);

            // Set the current texture
            m_pTextureBindings[i].pTexture = pTexture;
            mini3d_assert(strlen(nameOGL) <= MAX_TEXTURE_SAMPLER_NAME_LENGTH, "Using a texture sampler name longer than %u characters (MAX_TEXTURE_SAMPLER_NAME_LENGTH)!", MAX_TEXTURE_SAMPLER_NAME_LENGTH);
            strcpy(m_pTextureBindings[i].name, nameOGL);
            ++m_boundTextureCount;
        }

        return;
    }

    void SetRenderTarget(IRenderTarget* pRenderTarget)
    { 
        if (pRenderTarget == m_pCurrentRenderTarget)
            return;

        if (pRenderTarget == 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            m_pPlatform->MakeCurrent(0);
            return;
        }

        if (pRenderTarget->GetType() == IRenderTargetTexture::TYPE)
        {
            RenderTargetTexture_OpenGL* pRenderTargetTexture = (RenderTargetTexture_OpenGL*)pRenderTarget;
            pRenderTargetTexture->SetIsMipMapDirty(true);
            glBindFramebuffer(GL_FRAMEBUFFER, pRenderTargetTexture->GetGLFramebuffer());

            unsigned int width = pRenderTargetTexture->GetWidth();
            unsigned int height = pRenderTargetTexture->GetHeight();

            glViewport(0,0, pRenderTargetTexture->GetWidth(), pRenderTargetTexture->GetHeight());

            return;
        }

        else if (pRenderTarget->GetType() == IWindowRenderTarget::TYPE)
	    {
            WindowRenderTarget_OpenGL* pWindowRenderTarget = (WindowRenderTarget_OpenGL*)pRenderTarget;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            m_pPlatform->MakeCurrent(pWindowRenderTarget->GetNativeSurface());
            glViewport(0,0, pWindowRenderTarget->GetWidth(), pWindowRenderTarget->GetHeight());
            return;
	    }

        m_pCurrentRenderTarget = pRenderTarget; 
    }

    // Index Buffer
    IIndexBuffer* GetIndexBuffer() const { return (IIndexBuffer*)m_pCurrentIndexBuffer; }
    void SetIndexBuffer(IIndexBuffer* pIndexBuffer)
    {
        IndexBuffer_OpenGL* pIndexBuffer_OpenGL = (IndexBuffer_OpenGL*)pIndexBuffer;

        if (m_pCurrentIndexBuffer == pIndexBuffer_OpenGL)
            return;

        if (pIndexBuffer == 0)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        else
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIndexBuffer_OpenGL->GetGLIndexBuffer());

        m_pCurrentIndexBuffer = pIndexBuffer_OpenGL;
    }

    // Vertex Buffer
    IVertexBuffer* GetVertexBuffer(unsigned int streamIndex) const
    {
        mini3d_assert(streamIndex <= m_pCompatibility->VertexStreamCount() || streamIndex <= MAX_VERTEX_BUFFER_SLOTS, "Trying to get a Vertex Buffer with a stream index outside the valid interval");
        return m_CurrentVertexBufferMap[streamIndex];
    }

    void SetVertexBuffer(IVertexBuffer* pVertexBuffer, unsigned int streamIndex)
    {
        mini3d_assert(streamIndex <= m_pCompatibility->VertexStreamCount() || streamIndex <= MAX_VERTEX_BUFFER_SLOTS, "Trying to set a Vertex Buffer with a stream index outside the valid interval");

        VertexBuffer_OpenGL* pVertexBuffer_OpenGL = (VertexBuffer_OpenGL*)pVertexBuffer;

        if (m_CurrentVertexBufferMap[streamIndex] == pVertexBuffer) 
            return;

        // Only set this here. The actual binding (opengl call) will be done in the Draw() function.
        m_CurrentVertexBufferMap[streamIndex] = pVertexBuffer_OpenGL;
    }

    IConstantBuffer* GetConstantBuffer(unsigned int id, const char* nameOGL)
    {
        // TODO: fix!
        return m_pCurrentConstantBuffer;
    }
    
    void SetConstantBuffer(IConstantBuffer* pBuffer, unsigned int id, const char* nameOGL)
    {
        // TODO: Make this some sort of map!
        m_pCurrentConstantBuffer = (ConstantBuffer_OpenGL*)pBuffer;
    }
    
    IShaderInputLayout* GetShaderInputLayout()
    {
        return m_pCurrentShaderInputLayout;
    }
    
    void SetShaderInputLayout(IShaderInputLayout* pLayout)
    {
        m_pCurrentShaderInputLayout = (ShaderInputLayout_OpenGL*)pLayout;
    }

    void SetCullMode(CullMode cullMode)
    {
        if (m_cullMode == cullMode)
            return;
    
        m_cullMode = cullMode;

        if (cullMode == CULL_CLOCKWIZE)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }
        else if (cullMode == CULL_COUNTERCLOCKWIZE)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }
    }

    // Drawing
    void Draw()
    {
        BeginScene();

        if(m_pCurrentIndexBuffer == 0)
            return;

        if (m_pCurrentShaderProgram == 0)
            return;

        if (m_pCurrentShaderInputLayout == 0)
            return;

        GLenum err;


        m_pCurrentShaderInputLayout->BindAttributes();
        err = glGetError();

        if (m_pCurrentConstantBuffer != 0)
            m_pCurrentConstantBuffer->ApplyUniforms();
        err = glGetError();

        // Draw the scene
        if (m_IsUsingInstancedAttributes) // && m_pPlatform->VERSION_3_3()) TODO: Move somewhere other than platform
        {
            // Draw using instancing
            glDrawElementsInstanced(GL_TRIANGLES, m_pCurrentIndexBuffer->GetIndexCount(), GL_UNSIGNED_INT, 0, m_InstanceCount);
        }
        else
        {
            // TODO: glDrawRangeElements is faster than glDrawElements according to: http://www.spec.org/gwpg/gpc.static/vbo_whitepaper.html
            glDrawElements(GL_TRIANGLES, m_pCurrentIndexBuffer->GetIndexCount(), GL_UNSIGNED_INT, 0);
            err = glGetError();
        }

    }

    // Clear
    void Clear(float depth)
    {
        glClearDepth(depth);
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void Clear(float r, float g, float b, float a)
    {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Clear(float r, float g, float b, float a, float depth)
    {
        glClearColor(r, g, b, a);
        glClearDepth(depth);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

private:
    struct TextureBinding { GLint index; ITexture* pTexture; char name[MAX_TEXTURE_SAMPLER_NAME_LENGTH + 1]; };

	IPlatform* m_pPlatform;
    OpenGlWrapper* m_pOgl;

	IRenderTarget* m_pCurrentRenderTarget;
	WindowRenderTarget_OpenGL* m_pCurrentWindowRenderTarget;
	VertexBuffer_OpenGL* m_CurrentVertexBufferMap[MAX_VERTEX_BUFFER_SLOTS];
	IndexBuffer_OpenGL* m_pCurrentIndexBuffer;
	ShaderProgram_OpenGL* m_pCurrentShaderProgram;
    ShaderInputLayout_OpenGL* m_pCurrentShaderInputLayout;
    ConstantBuffer_OpenGL* m_pCurrentConstantBuffer;

    TextureBinding* m_pTextureBindings;
    unsigned int m_boundTextureCount;

    bool m_ActiveAttributeIndices[MAX_ACTIVE_ATTRIBUTE_INDICES];
	bool m_IsUsingInstancedAttributes;
	unsigned int m_InstanceCount;
	CullMode m_cullMode;
	ICompatibility* m_pCompatibility;
};

IGraphicsService* IGraphicsService::New() { return new GraphicsService_OpenGL(); }

IIndexBuffer* IIndexBuffer::New(IGraphicsService* pGraphics, const void* pIndices, unsigned int count, DataType dataType) { return new GraphicsService_OpenGL::IndexBuffer_OpenGL((GraphicsService_OpenGL*)pGraphics, pIndices, count, dataType); }
IVertexBuffer* IVertexBuffer::New(IGraphicsService* pGraphics, const void* pVertices, unsigned int vertexCount, unsigned int vertexSizeInBytes) { return new GraphicsService_OpenGL::VertexBuffer_OpenGL((GraphicsService_OpenGL*)pGraphics, pVertices, vertexCount, vertexSizeInBytes); }
IPixelShader* IPixelShader::New(IGraphicsService* pGraphics, const char* pShaderBytes, unsigned int sizeInBytes, bool precompiled) { return new GraphicsService_OpenGL::PixelShader_OpenGL((GraphicsService_OpenGL*)pGraphics, pShaderBytes, sizeInBytes); }
IVertexShader* IVertexShader::New(IGraphicsService* pGraphics, const char* pShaderBytes, unsigned int sizeInBytes, bool precompiled) { return new GraphicsService_OpenGL::VertexShader_OpenGL((GraphicsService_OpenGL*)pGraphics, pShaderBytes, sizeInBytes); }
IShaderProgram* IShaderProgram::New(IGraphicsService* pGraphics, IVertexShader* pVertexShader, IPixelShader* pPixelShader) { return new GraphicsService_OpenGL::ShaderProgram_OpenGL((GraphicsService_OpenGL*)pGraphics, pVertexShader, pPixelShader); }

IRenderTargetTexture* IRenderTargetTexture::New(IGraphicsService* pGraphics, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, bool depthTestEnabled) { return new GraphicsService_OpenGL::RenderTargetTexture_OpenGL((GraphicsService_OpenGL*)pGraphics, width, height, format, samplerSettings, depthTestEnabled); }
IWindowRenderTarget* IWindowRenderTarget::New(IGraphicsService* pGraphics, void* pNativeWindow, bool depthTestEnabled) { return new GraphicsService_OpenGL::WindowRenderTarget_OpenGL((GraphicsService_OpenGL*)pGraphics, pNativeWindow, depthTestEnabled); }
IBitmapTexture* IBitmapTexture::New(IGraphicsService* pGraphics, const char* pBitmap, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings) { return new GraphicsService_OpenGL::BitmapTexture_OpenGL((GraphicsService_OpenGL*) pGraphics, pBitmap, width, height, format, samplerSettings); }

IConstantBuffer* IConstantBuffer::New(IGraphicsService* pGraphics, unsigned int sizeInBytes, IShaderProgram* pShader, const char** names, unsigned int nameCount) { return new GraphicsService_OpenGL::ConstantBuffer_OpenGL((GraphicsService_OpenGL*)pGraphics, sizeInBytes, pShader, names, nameCount); }
IShaderInputLayout* IShaderInputLayout::New(IGraphicsService* pGraphics, IShaderProgram* pShader, InputElement* pElements, unsigned int count) { return new GraphicsService_OpenGL::ShaderInputLayout_OpenGL((GraphicsService_OpenGL*)pGraphics, pShader, pElements, count); }
}
}

#endif // MINI3D_GRAPHICSSERVICE_OPENGL
