
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_IOPENGLWRAPPER_H
#define MINI3D_IOPENGLWRAPPER_H

// Determine if we should use opengl or opengl es
#if defined(_WIN32) || defined(__APPLE__) || defined(__linux) && !defined(ANDROID) && !defined(IOS)
#define MINI3D_USE_OPENGL
#else
#define MINI3D_USE_OPENGL_ES
#endif

#include "opengl/opengldefs_opengl.hpp"
#include "opengl_es/opengldefs_opengl_es.hpp"


///////// OPENGL WRAPPER //////////////////////////////////////////////////////

struct IOpenGlWrapper
{

#define IOPENGLWRAPPER_INTERFACE(PURE_VIRTUAL)\
    virtual GLenum glGetError() PURE_VIRTUAL;\
\
    virtual void glBindTexture(GLenum target, GLuint texture) PURE_VIRTUAL;\
    virtual void glTexParameteri(GLenum target, GLenum pname, GLint param) PURE_VIRTUAL;\
    virtual void glGetTexParameteriv (GLenum target, GLenum pname, GLint *params) PURE_VIRTUAL;\
    virtual void glGenTextures(GLsizei n, GLuint *textures) PURE_VIRTUAL;\
    virtual void glDeleteTextures(GLsizei n, const GLuint *textures) PURE_VIRTUAL;\
\
    virtual void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) PURE_VIRTUAL;\
    virtual void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) PURE_VIRTUAL;\
    virtual void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid * img) PURE_VIRTUAL;\
\
    virtual void glDrawBuffer(GLenum mode) PURE_VIRTUAL;\
    virtual void glReadBuffer(GLenum mode) PURE_VIRTUAL;\
\
    virtual void glEnable(GLenum cap) PURE_VIRTUAL;\
    virtual void glDisable(GLenum cap) PURE_VIRTUAL;\
\
    virtual void glClear(GLbitfield mask) PURE_VIRTUAL;\
    virtual void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) PURE_VIRTUAL;\
    virtual void glClearDepth(GLclampd depth) PURE_VIRTUAL;\
\
    virtual void glShadeModel(GLenum mode) PURE_VIRTUAL;\
    virtual void glDepthFunc(GLenum func) PURE_VIRTUAL;\
\
    virtual void glGetIntegerv(GLenum pname, GLint *params) PURE_VIRTUAL;\
    virtual const GLubyte* glGetString(GLenum name) PURE_VIRTUAL;\
    virtual void glCullFace(GLenum mode) PURE_VIRTUAL;\
\
    virtual void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) PURE_VIRTUAL;\
\
    virtual void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) PURE_VIRTUAL;\
\
    virtual void glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) PURE_VIRTUAL;\
    virtual void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data) PURE_VIRTUAL;\
    virtual GLboolean glIsShader(GLuint shader) PURE_VIRTUAL;\
    virtual GLuint glCreateShader(GLenum type) PURE_VIRTUAL;\
\
    virtual void glDeleteShader(GLuint shader) PURE_VIRTUAL;\
    virtual void glAttachShader(GLuint program, GLuint shader) PURE_VIRTUAL;\
    virtual void glShaderSource(GLuint shader, GLsizei count, const GLchar* *string, const GLint *length) PURE_VIRTUAL;\
    virtual void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) PURE_VIRTUAL;\
    virtual void glCompileShader(GLuint shader) PURE_VIRTUAL;\
    virtual void glGetShaderiv(GLuint shader, GLenum pname, GLint *params) PURE_VIRTUAL;\
    virtual void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) PURE_VIRTUAL;\
\
    virtual GLuint glCreateProgram() PURE_VIRTUAL;\
    virtual void glDeleteProgram(GLuint program) PURE_VIRTUAL;\
    virtual void glLinkProgram(GLuint program) PURE_VIRTUAL;\
    virtual void glUseProgram(GLuint program) PURE_VIRTUAL;\
    virtual void glGetProgramiv(GLuint program, GLenum pname, GLint *params) PURE_VIRTUAL;\
    virtual void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) PURE_VIRTUAL;\
    virtual void glValidateProgram(GLuint program) PURE_VIRTUAL;\
\
    virtual GLint glGetAttribLocation(GLuint program, const GLchar *name) PURE_VIRTUAL;\
    virtual void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name) PURE_VIRTUAL;\
    virtual void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) PURE_VIRTUAL;\
\
    virtual void glUniform1f(GLint location, GLfloat v0) PURE_VIRTUAL;\
    virtual void glUniform2f(GLint location, GLfloat v0, GLfloat v1) PURE_VIRTUAL;\
    virtual void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) PURE_VIRTUAL;\
    virtual void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) PURE_VIRTUAL;\
    virtual void glUniform4fv(GLint location, GLsizei count, const GLfloat* value) PURE_VIRTUAL;\
\
    virtual void glUniform1i(GLint location, GLint v0) PURE_VIRTUAL;\
    virtual void glUniform2i(GLint location, GLint v0, GLint v1) PURE_VIRTUAL;\
    virtual void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) PURE_VIRTUAL;\
    virtual void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) PURE_VIRTUAL;\
    virtual void glUniform4iv(GLint location, GLsizei count, const GLint* value) PURE_VIRTUAL;\
\
    virtual void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) PURE_VIRTUAL;\
\
    virtual GLint glGetUniformLocation(GLuint program, const GLchar *name) PURE_VIRTUAL;\
    virtual void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) PURE_VIRTUAL;\
\
    virtual void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer) PURE_VIRTUAL;\
    virtual void glDisableVertexAttribArray(GLuint index) PURE_VIRTUAL;\
    virtual void glEnableVertexAttribArray(GLuint index) PURE_VIRTUAL;\
\
    virtual void glActiveTexture(GLenum texture) PURE_VIRTUAL;\
    virtual void glGenerateMipmap(GLenum target) PURE_VIRTUAL;\
\
    virtual void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers) PURE_VIRTUAL;\
    virtual void glBindRenderbuffer(GLenum target, GLuint renderbuffer) PURE_VIRTUAL;\
    virtual void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers) PURE_VIRTUAL;\
\
    virtual void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) PURE_VIRTUAL;\
\
    virtual void glGenFramebuffers(GLsizei n, GLuint *framebuffers) PURE_VIRTUAL;\
    virtual void glBindFramebuffer(GLenum target, GLuint framebuffer) PURE_VIRTUAL;\
    virtual void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers) PURE_VIRTUAL;\
\
    virtual GLenum glCheckFramebufferStatus(GLenum target) PURE_VIRTUAL;\
\
    virtual void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) PURE_VIRTUAL;\
\
    virtual void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) PURE_VIRTUAL;\
\
    virtual void glDeleteBuffers(GLsizei n, const GLuint *buffers) PURE_VIRTUAL;\
    virtual void glGenBuffers(GLsizei n, GLuint *buffers) PURE_VIRTUAL;\
    virtual void glBindBuffer(GLenum target, GLuint buffer) PURE_VIRTUAL;\
    virtual GLvoid* glMapBuffer(GLenum target, GLenum access) PURE_VIRTUAL;\
    virtual GLboolean glUnmapBuffer(GLenum target) PURE_VIRTUAL;\
    virtual void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage) PURE_VIRTUAL;\
    virtual void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data) PURE_VIRTUAL;\
\
    virtual void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices) PURE_VIRTUAL;\
\
    virtual void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount) PURE_VIRTUAL;\
    virtual void glVertexAttribDivisor(GLuint index, GLuint divisor) PURE_VIRTUAL;\


public:
    IOPENGLWRAPPER_INTERFACE(=0);
    virtual ~IOpenGlWrapper() {};
};

#endif // MINI3D_IOPENGLWRAPPER_H
