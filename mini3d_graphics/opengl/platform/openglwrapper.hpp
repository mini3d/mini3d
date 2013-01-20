
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_IOPENGLWRAPPER_H
#define MINI3D_IOPENGLWRAPPER_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include "common/glext.h"
#elif defined(__linux__) && !defined(ANDROID)
#include <GL/gl.h>
#elif defined(ANDROID)
#include <GLES2/gl2.h>
#endif

/*
// include a minimal opengl header with only the stuff we need
#include "win32/opengl_win32.hpp"
#include "linux/opengl_linux.hpp"
#include "osx/opengl_osx.hpp"
#include "ios/opengl_ios.hpp"
#include "android/opengl_android.hpp"
*/

///////// OPENGL WRAPPER //////////////////////////////////////////////////////

namespace mini3d {
namespace graphics {

    void initOpenGL();

    GLenum glGetError();

    void glBindTexture(GLenum target, GLuint texture);
    void glTexParameteri(GLenum target, GLenum pname, GLint param);
    void glGetTexParameteriv (GLenum target, GLenum pname, GLint *params);
    void glGenTextures(GLsizei n, GLuint *textures);
    void glDeleteTextures(GLsizei n, const GLuint *textures);

    void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
    void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid * img);

    void glDrawBuffer(GLenum mode);
    void glReadBuffer(GLenum mode);

    void glEnable(GLenum cap);
    void glDisable(GLenum cap);

    void glClear(GLbitfield mask);
    void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
    void glClearDepth(GLclampf depth);

    void glShadeModel(GLenum mode);
    void glDepthFunc(GLenum func);

    void glGetIntegerv(GLenum pname, GLint *params);
    const GLubyte* glGetString(GLenum name);
    void glCullFace(GLenum mode);

    void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);

    void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

    void glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
    void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
    GLboolean glIsShader(GLuint shader);
    GLuint glCreateShader(GLenum type);

    void glDeleteShader(GLuint shader);
    void glAttachShader(GLuint program, GLuint shader);
    void glShaderSource(GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
    void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
    void glCompileShader(GLuint shader);
    void glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
    void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);

    GLuint glCreateProgram();
    void glDeleteProgram(GLuint program);
    void glLinkProgram(GLuint program);
    void glUseProgram(GLuint program);
    void glGetProgramiv(GLuint program, GLenum pname, GLint *params);
    void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    void glValidateProgram(GLuint program);

    GLint glGetAttribLocation(GLuint program, const GLchar *name);
    void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name);
    void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);

    void glUniform1fv(GLint location, GLsizei count, const GLfloat* value);
    void glUniform2fv(GLint location, GLsizei count, const GLfloat* value);
    void glUniform3fv(GLint location, GLsizei count, const GLfloat* value);
    void glUniform4fv(GLint location, GLsizei count, const GLfloat* value);

    void glUniform1i(GLint location, GLint v0);
    void glUniform2i(GLint location, GLint v0, GLint v1);
    void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2);
    void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    void glUniform4iv(GLint location, GLsizei count, const GLint* value);

    void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

    GLint glGetUniformLocation(GLuint program, const GLchar *name);
    void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);

    void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
    void glDisableVertexAttribArray(GLuint index);
    void glEnableVertexAttribArray(GLuint index);

    void glActiveTexture(GLenum texture);
    void glGenerateMipmap(GLenum target);

    void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers);
    void glBindRenderbuffer(GLenum target, GLuint renderbuffer);
    void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers);

    void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);

    void glGenFramebuffers(GLsizei n, GLuint *framebuffers);
    void glBindFramebuffer(GLenum target, GLuint framebuffer);
    void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers);

    GLenum glCheckFramebufferStatus(GLenum target);

    void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);

    void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

    void glDeleteBuffers(GLsizei n, const GLuint *buffers);
    void glGenBuffers(GLsizei n, GLuint *buffers);
    void glBindBuffer(GLenum target, GLuint buffer);
    GLvoid* glMapBuffer(GLenum target, GLenum access);
    GLboolean glUnmapBuffer(GLenum target);
    void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
    void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);

    void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);

    void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount);
    void glVertexAttribDivisor(GLuint index, GLuint divisor);

}
}

#endif
