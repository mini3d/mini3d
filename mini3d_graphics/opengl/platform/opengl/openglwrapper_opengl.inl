
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#include "../iopenglwrapper.hpp"

#ifdef _WIN32
#define getProcAddress(PROC_NAME) wglGetProcAddress(PROC_NAME);
#elif defined (__APPLE__)
// TODO: add
#elif defined (__linux__)
#define getProcAddress(PROC_NAME) glXGetProcAddress((GLubyte*)PROC_NAME);
#endif

namespace mini3d
{

struct OpenGlWrapper_opengl : IOpenGlWrapper
{
    OpenGlWrapper_opengl()
    {
        // assign a platform specific value to function pointer getProcAddress

        _glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)getProcAddress("glCompressedTexImage2D");
        _glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)getProcAddress("glCompressedTexSubImage2D");

        _glIsShader = (PFNGLISSHADERPROC)getProcAddress("glIsShader");
	
        _glCreateShader = (PFNGLCREATESHADERPROC)getProcAddress("glCreateShader");
        _glShaderSource = (PFNGLSHADERSOURCEPROC)getProcAddress("glShaderSource");
	
        _glCompileShader = (PFNGLCOMPILESHADERPROC)getProcAddress("glCompileShader");
        _glDeleteShader = (PFNGLDELETESHADERPROC)getProcAddress("glDeleteShader");
        _glGetShaderiv = (PFNGLGETSHADERIVPROC)getProcAddress("glGetShaderiv");
        _glGetShaderSource = (PFNGLGETSHADERSOURCEPROC)getProcAddress("glGetShaderSource");
        _glGetProgramiv = (PFNGLGETPROGRAMIVNVPROC)getProcAddress("glGetProgramiv");
        _glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)getProcAddress("glGetShaderInfoLog");
        _glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)getProcAddress("glGetProgramInfoLog");

        _glDeleteProgram = (PFNGLDELETEPROGRAMPROC)getProcAddress("glDeleteProgram");
        _glCreateProgram = (PFNGLCREATEPROGRAMPROC)getProcAddress("glCreateProgram");
        _glAttachShader = (PFNGLATTACHSHADERPROC)getProcAddress("glAttachShader");
        _glLinkProgram = (PFNGLLINKPROGRAMPROC)getProcAddress("glLinkProgram");

        _glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)getProcAddress("glBindAttribLocation");

        _glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC)getProcAddress("glGetActiveAttrib");
        _glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)getProcAddress("glGetAttribLocation");
        _glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)getProcAddress("glBindAttribLocation");

        _glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC)getProcAddress("glGetActiveUniform");
        _glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)getProcAddress("glGetUniformLocation");

        _glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)getProcAddress("glValidateProgram");
	
        _glUseProgram = (PFNGLUSEPROGRAMPROC)getProcAddress("glUseProgram");
        _glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)getProcAddress("glValidateProgram");
        _glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)getProcAddress("glEnableVertexAttribArray");
        _glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)getProcAddress("glDisableVertexAttribArray");
        _glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)getProcAddress("glVertexAttribPointer");
        _glActiveTexture = (PFNGLACTIVETEXTUREPROC)getProcAddress("glActiveTexture");
        _glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)getProcAddress("glGenerateMipmap");

        _glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)getProcAddress("glGenRenderbuffersEXT");
        _glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)getProcAddress("glGenRenderbuffers");

        _glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)getProcAddress("glBindRenderbufferEXT");
        _glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)getProcAddress("glBindRenderbuffer");

        _glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)getProcAddress("glCheckFramebufferStatusEXT");
        _glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)getProcAddress("glCheckFramebufferStatus");

        _glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)getProcAddress("glRenderbufferStorageEXT");
        _glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)getProcAddress("glRenderbufferStorage");

        _glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)getProcAddress("glGenFramebuffersEXT");
        _glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)getProcAddress("glGenFramebuffers");

        _glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)getProcAddress("glBindFramebufferEXT");
        _glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)getProcAddress("glBindFramebuffer");

        _glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)getProcAddress("glDeleteRenderbuffers");
        _glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)getProcAddress("glDeleteRenderbuffersEXT");

        _glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)getProcAddress("glDeleteFramebuffers");
        _glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)getProcAddress("glDeleteFramebuffersEXT");

        _glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)getProcAddress("glFramebufferTexture2DEXT");
        _glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)getProcAddress("glFramebufferTexture2D");

        _glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)getProcAddress("glFramebufferRenderbufferEXT");
        _glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)getProcAddress("glFramebufferRenderbuffer");

        _glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)getProcAddress("glDeleteBuffers");

        _glBindBuffer = (PFNGLBINDBUFFERPROC)getProcAddress("glBindBuffer");
        _glMapBuffer = (PFNGLMAPBUFFERPROC)getProcAddress("glMapBuffer");
        _glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)getProcAddress("glUnmapBuffer");
        _glBufferData = (PFNGLBUFFERDATAPROC)getProcAddress("glBufferData");
        _glGenBuffers = (PFNGLGENBUFFERSPROC)getProcAddress("glGenBuffers");
        _glGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC)getProcAddress("glGetBufferSubData");

        _glUniform1f = (PFNGLUNIFORM1FPROC)getProcAddress("glUniform1f");
        _glUniform2f = (PFNGLUNIFORM2FPROC)getProcAddress("glUniform2f");
        _glUniform3f = (PFNGLUNIFORM3FPROC)getProcAddress("glUniform3f");
        _glUniform4f = (PFNGLUNIFORM4FPROC)getProcAddress("glUniform4f");
        _glUniform4fv = (PFNGLUNIFORM4FVPROC)getProcAddress("glUniform4fv");

        _glUniform1i = (PFNGLUNIFORM1IPROC)getProcAddress("glUniform1i");
        _glUniform2i = (PFNGLUNIFORM2IPROC)getProcAddress("glUniform2i");
        _glUniform3i = (PFNGLUNIFORM3IPROC)getProcAddress("glUniform3i");
        _glUniform4i = (PFNGLUNIFORM4IPROC)getProcAddress("glUniform4i");
        _glUniform4iv = (PFNGLUNIFORM4IVPROC)getProcAddress("glUniform4iv");

        _glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)getProcAddress("glUniformMatrix4fv");

        _glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)getProcAddress("glDrawRangeElements");
	
        _glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)getProcAddress("glVertexAttribDivisor");
        _glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC)getProcAddress("glDrawElementsInstanced");
    }

    GLenum glGetError() { return ::glGetError(); }

    void glBindTexture(GLenum target, GLuint texture) { ::glBindTexture(target, texture); }
    void glTexParameteri(GLenum target, GLenum pname, GLint param) { ::glTexParameteri(target, pname, param); }
    void glGetTexParameteriv (GLenum target, GLenum pname, GLint *params) { ::glGetTexParameteriv(target, pname, params); }
    void glGenTextures(GLsizei n, GLuint *textures) { ::glGenTextures(n, textures); }
    void glDeleteTextures(GLsizei n, const GLuint *textures) { ::glDeleteTextures(n, textures); }

    void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) { ::glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels); }
    void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) { ::glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels); }
    void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid * img) { ::glGetTexImage(target, level, format, type, img); }

    void glDrawBuffer(GLenum mode) { ::glDrawBuffer(mode); }
    void glReadBuffer(GLenum mode) { ::glReadBuffer(mode); }

    void glEnable(GLenum cap) { ::glEnable(cap); }
    void glDisable(GLenum cap) { ::glDisable(cap); }

    void glClear(GLbitfield mask) { ::glClear(mask); }
    void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) { ::glClearColor(red, green, blue, alpha); }
    void glClearDepth(GLclampd depth) { ::glClearDepth(depth); }
	
    void glShadeModel(GLenum mode) { ::glShadeModel(mode); }
    void glDepthFunc(GLenum func) { ::glDepthFunc(func); }
	
    void glGetIntegerv(GLenum pname, GLint *params) { ::glGetIntegerv(pname, params); }
    const GLubyte* glGetString(GLenum name) { return ::glGetString(name); }
    void glCullFace(GLenum mode) { ::glCullFace(mode); }

    void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) { ::glDrawElements(mode, count, type, indices); }

    void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) { ::glViewport(x, y, width, height); }

    // Shader

    void glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) { _glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data); }
    void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data) { _glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data); }

    GLboolean glIsShader(GLuint shader) { return _glIsShader(shader); }
    GLuint glCreateShader(GLenum type) { return _glCreateShader(type); }
    void glDeleteShader(GLuint shader) { _glDeleteShader(shader); }
    void glAttachShader(GLuint program, GLuint shader) { _glAttachShader(program, shader); }
    void glShaderSource(GLuint shader, GLsizei count, const GLchar* *string, const GLint *length) { _glShaderSource(shader, count, string, length); }
    void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) { _glGetShaderSource(shader, bufSize, length, source); }
    void glCompileShader(GLuint shader) { _glCompileShader(shader); }
    void glGetShaderiv(GLuint shader, GLenum pname, GLint *params) { _glGetShaderiv(shader, pname, params); }
    void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) { _glGetShaderInfoLog(shader, bufSize, length, infoLog); }

    // Program
    GLuint glCreateProgram() { return _glCreateProgram(); } 
    void glDeleteProgram(GLuint program) { _glDeleteProgram(program); } 
    void glLinkProgram(GLuint program) { _glLinkProgram(program); } 
    void glUseProgram(GLuint program) { _glUseProgram(program); } 
    void glGetProgramiv(GLuint program, GLenum pname, GLint *params) { _glGetProgramiv(program, pname, params); } 
    void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) { _glGetProgramInfoLog(program, bufSize, length, infoLog); } 
    void glValidateProgram(GLuint program) { _glValidateProgram(program); } 

    // Attribs
    GLint glGetAttribLocation(GLuint program, const GLchar *name) { return _glGetAttribLocation(program, name); } 
    void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name) { _glBindAttribLocation(program, index, name); } 
    void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) { _glGetActiveAttrib(program, index, bufSize, length, size, type, name); } 

    // Uniform
    void glUniform1f(GLint location, GLfloat v0) { _glUniform1f(location, v0); } 
    void glUniform2f(GLint location, GLfloat v0, GLfloat v1) { _glUniform2f(location, v0, v1); } 
    void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) { _glUniform3f(location, v0, v1, v2); } 
    void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) { _glUniform4f(location, v0, v1, v2, v3); } 
    void glUniform4fv(GLint location, GLsizei count, const GLfloat* value ) { _glUniform4fv(location, count, value); } 
	
    void glUniform1i(GLint location, GLint v0) { _glUniform1i(location, v0); } 
    void glUniform2i(GLint location, GLint v0, GLint v1) { _glUniform2i(location, v0, v1); } 
    void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) { _glUniform3i(location, v0, v1, v2); } 
    void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) { _glUniform4i(location, v0, v1, v2, v3); } 
    void glUniform4iv(GLint location, GLsizei count, const GLint* value ) { _glUniform4iv(location, count, value); } 

    void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { _glUniformMatrix4fv(location, count, transpose, value); } 

    GLint glGetUniformLocation(GLuint program, const GLchar *name) { return _glGetUniformLocation(program, name); } 
    void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) { _glGetActiveUniform(program, index, bufSize, length, size, type, name); } 
	
    // VertexAttributes
    void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer) { _glVertexAttribPointer(index, size, type, normalized, stride, pointer); } 
    void glDisableVertexAttribArray(GLuint index) { _glDisableVertexAttribArray(index); } 
    void glEnableVertexAttribArray(GLuint index) { _glEnableVertexAttribArray(index); } 

    // Texture
    void glActiveTexture(GLenum texture) { _glActiveTexture(texture); } 
    void glGenerateMipmap(GLenum target) { _glGenerateMipmap(target); } // TODO: Remove? (Also from IOpenGlWrapper)

    // Render Buffers
    void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers) 
    {	
	    if (_glGenRenderbuffers) _glGenRenderbuffers(n, renderbuffers);
	    else if (_glGenRenderbuffersEXT) _glGenRenderbuffersEXT(n, renderbuffers);
	    else mini3d_assert(false, "OpenGL driver does not support EXTENSION"); // TODO: Find out extension name
    } 
    void glBindRenderbuffer(GLenum target, GLuint renderbuffer) 
    { 	
	    if (_glBindRenderbuffer) _glBindRenderbuffer(target, renderbuffer);
	    else if (_glBindRenderbufferEXT) _glBindRenderbufferEXT(target, renderbuffer);
	    else mini3d_assert(false, "OpenGL driver does not support EXTENSION"); // TODO: Find out extension name
    }
    void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers) 
    { 
	    if (_glDeleteRenderbuffers) _glDeleteRenderbuffers(n, renderbuffers);
	    else if (_glDeleteRenderbuffersEXT) _glDeleteRenderbuffersEXT(n, renderbuffers);
	    else mini3d_assert(false, "OpenGL driver does not support EXTENSION"); // TODO: Find out extension name
    } 

    void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) 
    { 
	    if (_glRenderbufferStorage) _glRenderbufferStorage(target, internalformat, width, height);
	    else if (_glRenderbufferStorageEXT) _glRenderbufferStorageEXT(target, internalformat, width, height);
	    else mini3d_assert(false, "OpenGL driver does not support EXTENSION"); // TODO: Find out extension name
    } 

    // Frame Buffers
    void glGenFramebuffers(GLsizei n, GLuint *framebuffers) 
    { 
	    if (_glGenFramebuffers) _glGenFramebuffers(n, framebuffers);
	    else if (_glGenFramebuffersEXT) _glGenFramebuffersEXT(n, framebuffers);
	    else mini3d_assert(false, "OpenGL driver does not support EXTENSION"); // TODO: Find out extension name
    } 
    void glBindFramebuffer(GLenum target, GLuint framebuffer) 
    { 
	    if (_glBindFramebuffer) _glBindFramebuffer(target, framebuffer);
	    else if (_glBindFramebufferEXT) _glBindFramebufferEXT(target, framebuffer);
	    else mini3d_assert(false, "OpenGL driver does not support EXTENSION"); // TODO: Find out extension name
    } 
    void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers) 
    { 
	    if (_glDeleteFramebuffers) _glDeleteFramebuffers(n, framebuffers);
	    else if (_glDeleteFramebuffersEXT) _glDeleteFramebuffersEXT(n, framebuffers);
	    else mini3d_assert(false, "OpenGL driver does not support EXTENSION"); // TODO: Find out extension name
    } 

    GLenum glCheckFramebufferStatus(GLenum target) 
    { 
	    if (_glCheckFramebufferStatus) return _glCheckFramebufferStatus(target);
	    else if (_glCheckFramebufferStatusEXT) return _glCheckFramebufferStatusEXT(target);
	    else mini3d_assert(false, "OpenGL driver does not support EXTENSION"); // TODO: Find out extension name

	    return 0; // To avoid compiler warnings 
    } 

    void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) 
    { 
	    if (_glFramebufferTexture2D) _glFramebufferTexture2D(target, attachment, textarget, texture, level);
	    else if (_glFramebufferTexture2DEXT) _glFramebufferTexture2DEXT(target, attachment, textarget, texture, level);
	    else mini3d_assert(false, "OpenGL driver does not support EXTENSION"); // TODO: Find out extension name
    }

    void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) 
    { 
	    if (_glFramebufferRenderbuffer) _glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
	    else if (_glFramebufferRenderbufferEXT) _glFramebufferRenderbufferEXT(target, attachment, renderbuffertarget, renderbuffer);
	    else mini3d_assert(false, "OpenGL driver does not support EXTENSION"); // TODO: Find out extension name
    } 

    // Buffers(vertices and indices)
    void glDeleteBuffers(GLsizei n, const GLuint *buffers) { _glDeleteBuffers(n, buffers); } 
    void glGenBuffers(GLsizei n, GLuint *buffers) { _glGenBuffers(n, buffers); } 
    void glBindBuffer(GLenum target, GLuint buffer) { _glBindBuffer(target, buffer); } 
    GLvoid* glMapBuffer(GLenum target, GLenum access) { return _glMapBuffer(target, access); } 
    GLboolean glUnmapBuffer(GLenum target) { return _glUnmapBuffer(target); } 
    void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage) { _glBufferData(target, size, data, usage); } 
    void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data) { _glGetBufferSubData(target, offset, size, data); }

    // Draw Calls
    void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices) { _glDrawRangeElements(mode, start, end, count, type, indices); } 

    // Geometry Instancing(OpenGL 3.1)
    void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount) { _glDrawElementsInstanced(mode, count, type, indices, primcount); } 
	
    // Geometry Instancing(OpenGL 3.3)
    void glVertexAttribDivisor(GLuint index, GLuint divisor) { _glVertexAttribDivisor(index, divisor); } 


public:
    ///////////////////////////////////////////////////////////////////////////////
    // OpenGL version 1.2                                                        //
    ///////////////////////////////////////////////////////////////////////////////

    PFNGLDRAWRANGEELEMENTSPROC _glDrawRangeElements;


    ///////////////////////////////////////////////////////////////////////////////
    // OpenGL version 1.3                                                        //
    ///////////////////////////////////////////////////////////////////////////////

    PFNGLACTIVETEXTUREPROC _glActiveTexture;
    PFNGLGENERATEMIPMAPPROC _glGenerateMipmap;
    PFNGLCOMPRESSEDTEXIMAGE2DPROC _glCompressedTexImage2D;
    PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC _glCompressedTexSubImage2D;


    ///////////////////////////////////////////////////////////////////////////////
    // OpenGL version 1.5                                                        //
    ///////////////////////////////////////////////////////////////////////////////

    PFNGLBINDBUFFERPROC _glBindBuffer;
    PFNGLDELETEBUFFERSPROC _glDeleteBuffers;
    PFNGLGENBUFFERSPROC _glGenBuffers;
    PFNGLBUFFERDATAPROC _glBufferData;
    PFNGLGETBUFFERSUBDATAPROC _glGetBufferSubData;
    PFNGLMAPBUFFERPROC _glMapBuffer;
    PFNGLUNMAPBUFFERPROC _glUnmapBuffer;


    ///////////////////////////////////////////////////////////////////////////////
    // OpenGL version 2.0                                                        //
    ///////////////////////////////////////////////////////////////////////////////

    PFNGLATTACHSHADERPROC				_glAttachShader;
    PFNGLBINDATTRIBLOCATIONPROC			_glBindAttribLocation;
    PFNGLCOMPILESHADERPROC				_glCompileShader;

    PFNGLCREATEPROGRAMPROC				_glCreateProgram;
    PFNGLCREATESHADERPROC				_glCreateShader;
    PFNGLDELETEPROGRAMPROC				_glDeleteProgram;
    PFNGLDELETESHADERPROC				_glDeleteShader;
    PFNGLDISABLEVERTEXATTRIBARRAYPROC	_glDisableVertexAttribArray;
    PFNGLENABLEVERTEXATTRIBARRAYPROC	_glEnableVertexAttribArray;
    PFNGLGETACTIVEATTRIBPROC			_glGetActiveAttrib;
    PFNGLGETACTIVEUNIFORMPROC			_glGetActiveUniform;
    PFNGLGETATTRIBLOCATIONPROC			_glGetAttribLocation;
    PFNGLGETPROGRAMIVNVPROC				_glGetProgramiv;
    PFNGLGETPROGRAMINFOLOGPROC			_glGetProgramInfoLog;
    PFNGLGETSHADERIVPROC				_glGetShaderiv;
    PFNGLGETSHADERINFOLOGPROC			_glGetShaderInfoLog;
    PFNGLGETSHADERSOURCEPROC			_glGetShaderSource;
    PFNGLGETUNIFORMLOCATIONPROC			_glGetUniformLocation;
    PFNGLISSHADERPROC					_glIsShader;
    PFNGLLINKPROGRAMPROC				_glLinkProgram;
    PFNGLSHADERSOURCEPROC				_glShaderSource;
    PFNGLUSEPROGRAMPROC					_glUseProgram;
    PFNGLUNIFORM1IPROC					_glUniform1i;
    PFNGLUNIFORM2IPROC					_glUniform2i;
    PFNGLUNIFORM3IPROC					_glUniform3i;
    PFNGLUNIFORM4IPROC					_glUniform4i;
    PFNGLUNIFORM4IVPROC					_glUniform4iv;
    PFNGLUNIFORM1FPROC					_glUniform1f;
    PFNGLUNIFORM2FPROC					_glUniform2f;
    PFNGLUNIFORM3FPROC					_glUniform3f;
    PFNGLUNIFORM4FPROC					_glUniform4f;
    PFNGLUNIFORM4FVPROC					_glUniform4fv;
    PFNGLUNIFORMMATRIX4FVPROC			_glUniformMatrix4fv;
    PFNGLVALIDATEPROGRAMPROC			_glValidateProgram;
    PFNGLVERTEXATTRIBPOINTERPROC		_glVertexAttribPointer;


    ///////////////////////////////////////////////////////////////////////////////
    // Framebuffer Object                                                        //
    // http://www.opengl.org/wiki/Framebuffer_Object							 //
    //																			 //
    // EXT extension:				GL_EXT_framebuffer_object					 //
    // Core ARB extension:			GL_ARB_framebuffer_object					 //
    // Adopted into core version	3.0											 //
    // Core in version:				4.2											 //
    ///////////////////////////////////////////////////////////////////////////////

    //GL_ARB_framebuffer_object
    //GL_EXT_framebuffer_object
    PFNGLBINDRENDERBUFFERPROC			_glBindRenderbuffer;
    PFNGLBINDRENDERBUFFEREXTPROC		_glBindRenderbufferEXT;
    PFNGLDELETERENDERBUFFERSPROC		_glDeleteRenderbuffers;
    PFNGLDELETERENDERBUFFERSEXTPROC		_glDeleteRenderbuffersEXT;
    PFNGLGENRENDERBUFFERSPROC			_glGenRenderbuffers;
    PFNGLGENRENDERBUFFERSEXTPROC		_glGenRenderbuffersEXT;
    PFNGLRENDERBUFFERSTORAGEPROC		_glRenderbufferStorage;
    PFNGLRENDERBUFFERSTORAGEEXTPROC		_glRenderbufferStorageEXT;

    PFNGLBINDFRAMEBUFFERPROC			_glBindFramebuffer;
    PFNGLBINDFRAMEBUFFEREXTPROC			_glBindFramebufferEXT;
    PFNGLDELETEFRAMEBUFFERSPROC			_glDeleteFramebuffers;
    PFNGLDELETEFRAMEBUFFERSPROC			_glDeleteFramebuffersEXT;
    PFNGLGENFRAMEBUFFERSPROC			_glGenFramebuffers;
    PFNGLGENFRAMEBUFFERSEXTPROC			_glGenFramebuffersEXT;

    PFNGLCHECKFRAMEBUFFERSTATUSPROC		_glCheckFramebufferStatus;
    PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC	_glCheckFramebufferStatusEXT;

    PFNGLFRAMEBUFFERTEXTURE2DPROC		_glFramebufferTexture2D;
    PFNGLFRAMEBUFFERTEXTURE2DEXTPROC	_glFramebufferTexture2DEXT;

    PFNGLFRAMEBUFFERRENDERBUFFERPROC	_glFramebufferRenderbuffer;
    PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC _glFramebufferRenderbufferEXT;


    ///////////////////////////////////////////////////////////////////////////////
    // OpenGL version 3.1                                                        //
    ///////////////////////////////////////////////////////////////////////////////

    PFNGLDRAWELEMENTSINSTANCEDPROC _glDrawElementsInstanced;


    ///////////////////////////////////////////////////////////////////////////////
    // OpenGL version 3.3                                                        //
    ///////////////////////////////////////////////////////////////////////////////

    PFNGLVERTEXATTRIBDIVISORPROC _glVertexAttribDivisor;
};

}
