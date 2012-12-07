
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_GRAPHICS_OPENGLDEFS_OPENGL_H
#define MINI3D_GRAPHICS_OPENGLDEFS_OPENGL_H

#include <stdint.h>
#include <cstddef>

/* Base GL types */
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef char GLchar;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef unsigned short GLhalf;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
typedef ptrdiff_t GLintptr;
typedef intptr_t GLsizeiptr;
typedef uint32_t GLbitfield;


/*************************************************************/

#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3

#define GL_R16UI                          0x8234

#define GL_R32F                           0x822E
#define GL_R32UI                          0x8236

#define GL_RED                            0x1903
	
#define GL_FALSE                          0
#define GL_TRUE                           1

#define GL_NONE                           0
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405

#define GL_CULL_FACE                      0x0B44

#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_COLOR_BUFFER_BIT               0x00004000

#define GL_DEPTH_TEST                     0x0B71

#define GL_LEQUAL                         0x0203
#define GL_TRIANGLES                      0x0004

#define GL_ACTIVE_ATTRIBUTES              0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH    0x8B8A
#define GL_ACTIVE_UNIFORMS                0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH      0x8B87

#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893

#define GL_STATIC_DRAW                    0x88E4
#define GL_WRITE_ONLY                     0x88B9

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31

#define GL_RGBA                           0x1908
#define GL_BGRA                           0x80E1

#define GL_RGBA8                          0x8058
#define GL_RGBA16                         0x805B
#define GL_UNSIGNED_INT_8_8_8_8           0x8035

#define GL_UNSIGNED_BYTE                  0x1401
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406

#define GL_TEXTURE_2D                     0x0DE1
#define GL_TEXTURE0                       0x84C0

#define GL_REPEAT                         0x2901
#define GL_CLAMP_TO_EDGE                  0x812F

#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803

#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_LINEAR           0x2703

#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801

#define GL_TEXTURE_MAX_LEVEL              0x813D

#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601

#define GL_TEXTURE_COMPARE_MODE           0x884C
#define GL_TEXTURE_COMPARE_FUNC           0x884D

#define GL_DEPTH_COMPONENT                0x1902
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_COLOR_ATTACHMENT0              0x8CE0

#define GL_FRAMEBUFFER                    0x8D40
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5

#define GL_RENDERBUFFER                   0x8D41

#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_TEXTURE_IMAGE_UNITS        0x8872

#define GL_LUMINANCE16                    0x8042
#define GL_LUMINANCE                      0x1909

#define GL_LUMINANCE32F_ARB               0x8818

#define GL_GENERATE_MIPMAP                0x8191
#define GL_COMPARE_R_TO_TEXTURE           0x884E

#define GL_SHADING_LANGUAGE_VERSION       0x8B8C

#endif // MINI3D_GRAPHICS_OPENGLDEFS_OPENGL_H

