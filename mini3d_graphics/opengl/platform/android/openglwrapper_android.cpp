
// -- THIS FILE IS AUTOGENERATED USING THE SCRIPT GLWRAPPERS.PY --

// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifdef __ANDROID__

#define GL_GLEXT_FUNCTION_POINTERS 1
#include "../openglwrapper.hpp"
#include <GLES2/gl2.h>

void mini3d_assert(bool expression, const char* text, ...);

using namespace mini3d::graphics;

struct OpenGlWrapper::Internal
{
};

OpenGlWrapper::OpenGlWrapper()
{
}

OpenGlWrapper::~OpenGlWrapper()
{
}

////////// OPENGL 1.0 ///////////
GLenum OpenGlWrapper::glGetError() { return ::glGetError(); }
const GLubyte* OpenGlWrapper::glGetString(GLenum name) { return ::glGetString(name); }
void OpenGlWrapper::glBindTexture(GLenum target, GLuint texture) { ::glBindTexture(target, texture); }
void OpenGlWrapper::glTexParameteri(GLenum target, GLenum pname, GLint param) { ::glTexParameteri(target, pname, param); }
void OpenGlWrapper::glGetTexParameteriv(GLenum target, GLenum pname, GLint *params) { ::glGetTexParameteriv(target, pname, params); }
void OpenGlWrapper::glGenTextures(GLsizei n, GLuint *textures) { ::glGenTextures(n, textures); }
void OpenGlWrapper::glDeleteTextures(GLsizei n, const GLuint *textures) { ::glDeleteTextures(n, textures); }
void OpenGlWrapper::glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) { ::glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels); }
void OpenGlWrapper::glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) { ::glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels); }
void OpenGlWrapper::glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *img) {  }
void OpenGlWrapper::glDrawBuffer(GLenum mode) {  }
void OpenGlWrapper::glReadBuffer(GLenum mode) {  }
void OpenGlWrapper::glEnable(GLenum cap) { ::glEnable(cap); }
void OpenGlWrapper::glDisable(GLenum cap) { ::glDisable(cap); }
void OpenGlWrapper::glClear(GLbitfield mask) { ::glClear(mask); }
void OpenGlWrapper::glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) { ::glClearColor(red, green, blue, alpha); }
void OpenGlWrapper::glClearDepth(GLclampd depth) { ::glClearDepthf(depth); }
void OpenGlWrapper::glShadeModel(GLenum mode) { }
void OpenGlWrapper::glDepthFunc(GLenum func) { ::glDepthFunc(func); }
void OpenGlWrapper::glGetIntegerv(GLenum pname, GLint *params) { ::glGetIntegerv(pname, params); }
void OpenGlWrapper::glCullFace(GLenum mode) { ::glCullFace(mode); }
void OpenGlWrapper::glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) { ::glDrawElements(mode, count, type, indices); }
void OpenGlWrapper::glViewport(GLint x, GLint y, GLsizei width, GLsizei height) { ::glViewport(x, y, width, height); }

////////// OPENGL 1.2 ///////////
void OpenGlWrapper::glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices) { ::glDrawElements(mode, count, type, indices); }

////////// OPENGL 1.3 ///////////
void OpenGlWrapper::glActiveTexture(GLenum texture) { ::glActiveTexture(texture); }
void OpenGlWrapper::glGenerateMipmap(GLenum target) { ::glGenerateMipmap(target); }
void OpenGlWrapper::glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) { ::glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data); }
void OpenGlWrapper::glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data) { ::glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data); }

////////// OPENGL 1.5 ///////////
void OpenGlWrapper::glDeleteBuffers(GLsizei n, const GLuint *buffers) { ::glDeleteBuffers(n, buffers); }
void OpenGlWrapper::glGenBuffers(GLsizei n, GLuint *buffers) { ::glGenBuffers(n, buffers); }
void OpenGlWrapper::glBindBuffer(GLenum target, GLuint buffer) { ::glBindBuffer(target, buffer); }
GLvoid* OpenGlWrapper::glMapBuffer(GLenum target, GLenum access) {  }
GLboolean OpenGlWrapper::glUnmapBuffer(GLenum target) { }
void OpenGlWrapper::glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage) { ::glBufferData(target, size, data, usage); }
void OpenGlWrapper::glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data) { }

////////// OPENGL 2.0 ///////////
GLboolean OpenGlWrapper::glIsShader(GLuint shader) { return ::glIsShader(shader); }
GLuint OpenGlWrapper::glCreateShader(GLenum type) { return ::glCreateShader(type); }
void OpenGlWrapper::glDeleteShader(GLuint shader) { ::glDeleteShader(shader); }
void OpenGlWrapper::glAttachShader(GLuint program, GLuint shader) { ::glAttachShader(program, shader); }
void OpenGlWrapper::glShaderSource(GLuint shader, GLsizei count, const GLchar* *string, const GLint *length) { ::glShaderSource(shader, count, string, length); }
void OpenGlWrapper::glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) { ::glGetShaderSource(shader, bufSize, length, source); }
void OpenGlWrapper::glCompileShader(GLuint shader) { ::glCompileShader(shader); }
void OpenGlWrapper::glGetShaderiv(GLuint shader, GLenum pname, GLint *params) { ::glGetShaderiv(shader, pname, params); }
void OpenGlWrapper::glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) { ::glGetShaderInfoLog(shader, bufSize, length, infoLog); }
GLuint OpenGlWrapper::glCreateProgram() { return ::glCreateProgram(); }
void OpenGlWrapper::glDeleteProgram(GLuint program) { ::glDeleteProgram(program); }
void OpenGlWrapper::glLinkProgram(GLuint program) { ::glLinkProgram(program); }
void OpenGlWrapper::glUseProgram(GLuint program) { ::glUseProgram(program); }
void OpenGlWrapper::glGetProgramiv(GLuint program, GLenum pname, GLint *params) { ::glGetProgramiv(program, pname, params); }
void OpenGlWrapper::glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) { ::glGetProgramInfoLog(program, bufSize, length, infoLog); }
void OpenGlWrapper::glValidateProgram(GLuint program) { ::glValidateProgram(program); }
GLint OpenGlWrapper::glGetAttribLocation(GLuint program, const GLchar *name) { return ::glGetAttribLocation(program, name); }
void OpenGlWrapper::glBindAttribLocation(GLuint program, GLuint index, const GLchar *name) { ::glBindAttribLocation(program, index, name); }
void OpenGlWrapper::glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) { ::glGetActiveAttrib(program, index, bufSize, length, size, type, name); }
void OpenGlWrapper::glUniform1f(GLint location, GLfloat v0) { ::glUniform1f(location, v0); }
void OpenGlWrapper::glUniform2f(GLint location, GLfloat v0, GLfloat v1) { ::glUniform2f(location, v0, v1); }
void OpenGlWrapper::glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) { ::glUniform3f(location, v0, v1, v2); }
void OpenGlWrapper::glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) { ::glUniform4f(location, v0, v1, v2, v3); }
void OpenGlWrapper::glUniform4fv(GLint location, GLsizei count, const GLfloat *value) { ::glUniform4fv(location, count, value); }
void OpenGlWrapper::glUniform1i(GLint location, GLint v0) { ::glUniform1i(location, v0); }
void OpenGlWrapper::glUniform2i(GLint location, GLint v0, GLint v1) { ::glUniform2i(location, v0, v1); }
void OpenGlWrapper::glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) { ::glUniform3i(location, v0, v1, v2); }
void OpenGlWrapper::glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) { ::glUniform4i(location, v0, v1, v2, v3); }
void OpenGlWrapper::glUniform4iv(GLint location, GLsizei count, const GLint *value) { ::glUniform4iv(location, count, value); }
void OpenGlWrapper::glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { ::glUniformMatrix4fv(location, count, transpose, value); }
GLint OpenGlWrapper::glGetUniformLocation(GLuint program, const GLchar *name) { return ::glGetUniformLocation(program, name); }
void OpenGlWrapper::glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) { ::glGetActiveUniform(program, index, bufSize, length, size, type, name); }
void OpenGlWrapper::glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer) { ::glVertexAttribPointer(index, size, type, normalized, stride, pointer); }
void OpenGlWrapper::glDisableVertexAttribArray(GLuint index) { ::glDisableVertexAttribArray(index); }
void OpenGlWrapper::glEnableVertexAttribArray(GLuint index) { ::glEnableVertexAttribArray(index); }

////////// OPENGL 3.1 ///////////
void OpenGlWrapper::glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount) { }

////////// OPENGL 3.3 ///////////
void OpenGlWrapper::glVertexAttribDivisor(GLuint index, GLuint divisor) { }

////////// OPENGL 4.2 ///////////
void OpenGlWrapper::glGenRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
::glGenRenderbuffers(n, renderbuffers);
}

void OpenGlWrapper::glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
::glBindRenderbuffer(target, renderbuffer);
}

void OpenGlWrapper::glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers)
{
::glDeleteRenderbuffers(n, renderbuffers);
}

void OpenGlWrapper::glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
::glRenderbufferStorage(target, internalformat, width, height);
}

void OpenGlWrapper::glGenFramebuffers(GLsizei n, GLuint *framebuffers)
{
::glGenFramebuffers(n, framebuffers);
}

void OpenGlWrapper::glBindFramebuffer(GLenum target, GLuint framebuffer)
{
::glBindFramebuffer(target, framebuffer);
}

void OpenGlWrapper::glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers)
{
::glDeleteFramebuffers(n, framebuffers);
}

GLenum OpenGlWrapper::glCheckFramebufferStatus(GLenum target)
{
return ::glCheckFramebufferStatus(target);
}

void OpenGlWrapper::glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
::glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

void OpenGlWrapper::glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
::glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}


#endif