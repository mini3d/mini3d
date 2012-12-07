import re

class Platform:
    WIN32 = 0
    LINUX = 1
    MACOSX = 2
    IOS = 3
    ANDROID = 4

class Index:
    RETURN_TYPE = 0
    NAME = 1
    ARGUMENTS = 2
    EXTENSION = 3
    
openGL_1_0 = (
("GLenum", "glGetError",                (), ""),
("const GLubyte*", "glGetString",       [("GLenum", "name")], ""), 

("void", "glBindTexture",               [("GLenum", "target"), ("GLuint", "texture")], ""),  
("void", "glTexParameteri",             [("GLenum", "target"), ("GLenum", "pname"), ("GLint", "param")], ""),  
("void", "glGetTexParameteriv",         [("GLenum", "target"), ("GLenum", "pname"), ("GLint", "*params")], ""),  
("void", "glGenTextures",               [("GLsizei", "n"), ("GLuint", "*textures")], ""),  
("void", "glDeleteTextures",            [("GLsizei", "n"), ("const GLuint", "*textures")], ""),  
                    
("void", "glTexImage2D",                [("GLenum", "target"), ("GLint", "level"), ("GLint", "internalformat"), ("GLsizei", "width"), ("GLsizei", "height"), ("GLint", "border"), ("GLenum", "format"), ("GLenum", "type"), ("const GLvoid", "*pixels")], ""),  
("void", "glTexSubImage2D",             [("GLenum", "target"), ("GLint", "level"), ("GLint", "xoffset"), ("GLint", "yoffset"), ("GLsizei", "width"), ("GLsizei", "height"), ("GLenum", "format"), ("GLenum", "type"), ("const GLvoid", "*pixels")], ""),  
("void", "glGetTexImage",               [("GLenum", "target"), ("GLint", "level"), ("GLenum", "format"), ("GLenum", "type"), ("GLvoid", "*img")], ""),  
                    
("void", "glDrawBuffer",                [("GLenum", "mode")], ""),  
("void", "glReadBuffer",                [("GLenum", "mode")], ""),  
                    
("void", "glEnable",                    [("GLenum", "cap")], ""),  
("void", "glDisable",                   [("GLenum", "cap")], ""),  
                    
("void", "glClear",                     [("GLbitfield", "mask")], ""),  
("void", "glClearColor",                [("GLclampf", "red"), ("GLclampf", "green"), ("GLclampf", "blue"), ("GLclampf", "alpha")], ""),  
("void", "glClearDepth",                [("GLclampd", "depth")], ""),  
                    
("void", "glShadeModel",                [("GLenum", "mode")], ""),  
("void", "glDepthFunc",                 [("GLenum", "func")], ""),  
                    
("void", "glGetIntegerv",               [("GLenum", "pname"), ("GLint", "*params")], ""),  
("void", "glCullFace",                  [("GLenum", "mode")], ""),  
                    
("void", "glDrawElements",              [("GLenum", "mode"), ("GLsizei", "count"), ("GLenum", "type"), ("const GLvoid", "*indices")], ""),  
                    
("void", "glViewport",                  [("GLint", "x"), ("GLint", "y"), ("GLsizei", "width"), ("GLsizei", "height")], ""), 
)       
        
openGL_1_2 = (      
("void", "glDrawRangeElements",         [("GLenum", "mode"), ("GLuint", "start"), ("GLuint", "end"), ("GLsizei", "count"), ("GLenum", "type"), ("const GLvoid", "*indices")], ""), 
)

openGL_1_3 = (
("void", "glActiveTexture",             [("GLenum", "texture")], ""), 
("void", "glGenerateMipmap",            [("GLenum", "target")], ""), 
("void", "glCompressedTexImage2D",      [("GLenum", "target"), ("GLint", "level"), ("GLenum", "internalformat"), ("GLsizei", "width"), ("GLsizei", "height"), ("GLint", "border"), ("GLsizei", "imageSize"), ("const GLvoid", "*data")], ""),
("void", "glCompressedTexSubImage2D",   [("GLenum", "target"), ("GLint", "level"), ("GLint", "xoffset"), ("GLint", "yoffset"), ("GLsizei", "width"), ("GLsizei", "height"), ("GLenum", "format"), ("GLsizei", "imageSize"), ("const GLvoid", "*data")], "")
)

openGL_1_5 = (
("void", "glDeleteBuffers",             [("GLsizei", "n"), ("const GLuint", "*buffers")], ""),  
("void", "glGenBuffers",                [("GLsizei", "n"), ("GLuint", "*buffers")], ""),  
("void", "glBindBuffer",                [("GLenum", "target"), ("GLuint", "buffer")], ""),  
("GLvoid*", "glMapBuffer",              [("GLenum", "target"), ("GLenum", "access")], ""),  
("GLboolean", "glUnmapBuffer",          [("GLenum", "target")], ""),  
("void", "glBufferData",                [("GLenum", "target"), ("GLsizeiptr", "size"), ("const GLvoid", "*data"), ("GLenum", "usage")], ""),  
("void", "glGetBufferSubData",          [("GLenum", "target"), ("GLintptr", "offset"), ("GLsizeiptr", "size"), ("GLvoid", "*data")], "")
)

openGL_2_0 = (
("GLboolean", "glIsShader",             [("GLuint", "shader")], ""), 
("GLuint", "glCreateShader",            [("GLenum", "type")], ""), 
("void", "glDeleteShader",              [("GLuint", "shader")], ""), 
("void", "glAttachShader",              [("GLuint", "program"), ("GLuint", "shader")], ""), 
("void", "glShaderSource",              [("GLuint", "shader"), ("GLsizei", "count"), ("const GLchar*", "*string"), ("const GLint", "*length")], ""), 
("void", "glGetShaderSource",           [("GLuint", "shader"), ("GLsizei", "bufSize"), ("GLsizei", "*length"), ("GLchar", "*source")], ""), 
("void", "glCompileShader",             [("GLuint", "shader")], ""), 
("void", "glGetShaderiv",               [("GLuint", "shader"), ("GLenum", "pname"), ("GLint", "*params")], ""), 
("void", "glGetShaderInfoLog",          [("GLuint", "shader"), ("GLsizei", "bufSize"), ("GLsizei", "*length"), ("GLchar", "*infoLog")], ""), 

("GLuint", "glCreateProgram",           (), ""),
("void", "glDeleteProgram",             [("GLuint", "program")], ""),  
("void", "glLinkProgram",               [("GLuint", "program")], ""),  
("void", "glUseProgram",                [("GLuint", "program")], ""),  
("void", "glGetProgramiv",              [("GLuint", "program"), ("GLenum", "pname"), ("GLint", "*params")], ""),  
("void", "glGetProgramInfoLog",         [("GLuint", "program"), ("GLsizei", "bufSize"), ("GLsizei", "*length"), ("GLchar", "*infoLog")], ""),  
("void", "glValidateProgram",           [("GLuint", "program")], ""),  

("GLint", "glGetAttribLocation",        [("GLuint", "program"), ("const GLchar", "*name")], ""), 
("void", "glBindAttribLocation",        [("GLuint", "program"), ("GLuint", "index"), ("const GLchar", "*name")], ""),  
("void", "glGetActiveAttrib",           [("GLuint", "program"), ("GLuint", "index"), ("GLsizei", "bufSize"), ("GLsizei", "*length"), ("GLint", "*size"), ("GLenum", "*type"), ("GLchar", "*name")], ""),  

("void", "glUniform1f",                 [("GLint", "location"), ("GLfloat", "v0")], ""),  
("void", "glUniform2f",                 [("GLint", "location"), ("GLfloat", "v0"), ("GLfloat", "v1")], ""),  
("void", "glUniform3f",                 [("GLint", "location"), ("GLfloat", "v0"), ("GLfloat", "v1"), ("GLfloat", "v2")], ""),  
("void", "glUniform4f",                 [("GLint", "location"), ("GLfloat", "v0"), ("GLfloat", "v1"), ("GLfloat", "v2"), ("GLfloat", "v3")], ""),  
("void", "glUniform4fv",                [("GLint", "location"), ("GLsizei", "count"), ("const GLfloat", "*value")], ""),  

("void", "glUniform1i",                 [("GLint", "location"), ("GLint", "v0")], ""),  
("void", "glUniform2i",                 [("GLint", "location"), ("GLint", "v0"), ("GLint", "v1")], ""),  
("void", "glUniform3i",                 [("GLint", "location"), ("GLint", "v0"), ("GLint", "v1"), ("GLint", "v2")], ""),  
("void", "glUniform4i",                 [("GLint", "location"), ("GLint", "v0"), ("GLint", "v1"), ("GLint", "v2"), ("GLint", "v3")], ""),  
("void", "glUniform4iv",                [("GLint", "location"), ("GLsizei", "count"), ("const GLint", "*value")], ""),  

("void", "glUniformMatrix4fv",          [("GLint", "location"), ("GLsizei", "count"), ("GLboolean", "transpose"), ("const GLfloat", "*value")], ""),  

("GLint", "glGetUniformLocation",       [("GLuint", "program"), ("const GLchar", "*name")], ""), 
("void", "glGetActiveUniform",          [("GLuint", "program"), ("GLuint", "index"), ("GLsizei", "bufSize"), ("GLsizei", "*length"), ("GLint", "*size"), ("GLenum", "*type"), ("GLchar", "*name")], ""),  

("void", "glVertexAttribPointer",       [("GLuint", "index"), ("GLint", "size"), ("GLenum", "type"), ("GLboolean", "normalized"), ("GLsizei", "stride"), ("const GLvoid", "*pointer")], ""),  
("void", "glDisableVertexAttribArray",  [("GLuint", "index")], ""),  
("void", "glEnableVertexAttribArray",   [("GLuint", "index")], "")
)

openGL_3_1 = [
("void", "glDrawElementsInstanced",     [("GLenum", "mode"), ("GLsizei", "count"), ("GLenum", "type"), ("const GLvoid", "*indices"), ("GLsizei", "primcount")], ""),
]

openGL_3_3 = [
("void", "glVertexAttribDivisor",       [("GLuint", "index"), ("GLuint", "divisor")], "") 
]

openGL_4_2 = (
("void", "glGenRenderbuffers",          [("GLsizei", "n"), ("GLuint", "*renderbuffers")], "EXT_framebuffer_object"), 
("void", "glBindRenderbuffer",          [("GLenum", "target"), ("GLuint", "renderbuffer")], "EXT_framebuffer_object"), 
("void", "glDeleteRenderbuffers",       [("GLsizei", "n"), ("const GLuint", "*renderbuffers")], "EXT_framebuffer_object"), 
("void", "glRenderbufferStorage",       [("GLenum", "target"), ("GLenum", "internalformat"), ("GLsizei", "width"), ("GLsizei", "height")], "EXT_framebuffer_object"), 

("void", "glGenFramebuffers",           [("GLsizei", "n"), ("GLuint", "*framebuffers")], "EXT_framebuffer_object"), 
("void", "glBindFramebuffer",           [("GLenum", "target"), ("GLuint", "framebuffer")], "EXT_framebuffer_object"), 
("void", "glDeleteFramebuffers",        [("GLsizei", "n"), ("const GLuint", "*framebuffers")], "EXT_framebuffer_object"), 

("GLenum", "glCheckFramebufferStatus",  [("GLenum", "target")], "EXT_framebuffer_object"), 
("void", "glFramebufferTexture2D",      [("GLenum", "target"), ("GLenum", "attachment"), ("GLenum", "textarget"), ("GLuint", "texture"), ("GLint", "level")], "EXT_framebuffer_object"), 
("void", "glFramebufferRenderbuffer",   [("GLenum", "target"), ("GLenum", "attachment"), ("GLenum", "renderbuffertarget"), ("GLuint", "renderbuffer")], "EXT_framebuffer_object"), 
)


def writeHeader(platform):
    ow('\n')
    ow('// -- THIS FILE IS AUTOGENERATED USING THE SCRIPT GLWRAPPERS.PY --\n')
    ow('\n')
    ow('// Copyright (c) <2012> Daniel Peterson\n')
    ow('// This file is part of Mini3D <www.mini3d.org>\n')
    ow('// It is distributed under the MIT Software License <www.mini3d.org/license.php>\n')
    ow('\n')
    ow('\n')
    
    if platform == Platform.WIN32:
        ow('#ifdef _WIN32\n')
        ow('\n')    
        ow('#define WIN32_LEAN_AND_MEAN\n')
        ow('#include <windows.h>\n');
        ow('#include <GL/gl.h>\n');
        ow('#include "../common/glext.h"\n')
    elif platform == Platform.LINUX:
        ow('#if defined(__linux__) && !defined(ANDROID)\n')
        ow('\n')    
        ow('#include <X11/Xlib.h>\n')
        ow('#include <GL/gl.h>\n')
        ow('#include <GL/glx.h>\n')
        ow('#include "../common/glext.h"\n')
        ow('#include <GL/glxext.h>\n')
    elif platform == Platform.MACOSX:
        ow('#ifdef __APPLE__\n')
        ow('\n')
        ow('#define GL_GLEXT_FUNCTION_POINTERS 1\n')
        ow('#include <OpenGL/gl.h>\n')
        ow('#include "nsglgetprocaddress.hpp"\n')

    ow('#include \"../openglwrapper.hpp\"\n')
    ow('\n')
    ow('\n')
    ow('void mini3d_assert(bool expression, const char* text, ...);\n')
    ow('\n')
    ow('using namespace mini3d::graphics;\n')
    ow('\n')
    
def writeFooter(platform):
    ow('\n')
    ow('#endif')
    ow('\n')
    
def procDataType(procName, platform):
    if platform == Platform.WIN32 or platform == Platform.LINUX:
        return 'PFN' + procName.upper() + 'PROC'
    elif platform == Platform.MACOSX:
        return procName + 'ProcPtr'
    
    
def procExtDataType(procName, platform):
    if platform == Platform.WIN32 or platform == Platform.LINUX:
        return 'PFN' + procName.upper() + 'EXTPROC'
    elif platform == Platform.MACOSX:
        return procName + 'EXTProcPtr'
    
def writeDeclaration(procDesc, platform):
    name = procDesc[Index.NAME]
    hasExtension = (procDesc[Index.EXTENSION] != '')
    
    ow('    ' +  procDataType(name, platform) + ' _' + name + ';\n')

    if procDesc[Index.EXTENSION] != '':
        ow('    ' +  procExtDataType(name, platform) + ' _' + name + 'EXT;\n')
            
            
def writeDeclarations(platform):
    ow('struct OpenGlWrapper::Internal\n')
    ow('{\n')

    ow('    ////////// OPENGL 1.0 ///////////\n')
    if platform == Platform.MACOSX:
        for procDesc in openGL_1_0: writeDeclaration(procDesc, platform)
    ow('\n')
    ow('    ////////// OPENGL 1.2 ///////////\n')
    for procDesc in openGL_1_2: writeDeclaration(procDesc, platform)
    ow('\n')    
    ow('    ////////// OPENGL 1.3 ///////////\n')
    for procDesc in openGL_1_3: writeDeclaration(procDesc, platform)
    ow('\n')    
    ow('    ////////// OPENGL 1.5 ///////////\n')
    for procDesc in openGL_1_5: writeDeclaration(procDesc, platform)
    ow('\n')    
    ow('    ////////// OPENGL 2.0 ///////////\n')
    for procDesc in openGL_2_0: writeDeclaration(procDesc, platform)
    ow('\n')
    ow('    ////////// OPENGL 3.1 ///////////\n')
    for procDesc in openGL_3_1: writeDeclaration(procDesc, platform)
    ow('\n')    
    ow('    ////////// OPENGL 3.3 ///////////\n')
    for procDesc in openGL_3_3: writeDeclaration(procDesc, platform)
    ow('\n')    

    ow('    ////////// OPENGL 4.2 ///////////\n')
    for procDesc in openGL_4_2: writeDeclaration(procDesc, platform)

    ow("};\n\n");

def procFunction(platform):
    if platform == Platform.WIN32:
        return 'wglGetProcAddress'
    elif platform == Platform.LINUX:
        return 'glXGetProcAddress'
    elif platform == Platform.MACOSX:
        return 'NSglGetProcAddress'

    
def writeAssignment(procDesc, platform):
    name = procDesc[Index.NAME]
    hasExtension = (procDesc[Index.EXTENSION] != '')
    
    typeCast = ""
    if (platform == Platform.LINUX) : typeCast = '(const GLubyte*)'

    ow('    mpI->_' + name + ' = (' + procDataType(name, platform) + ')' + procFunction(platform) + '('+ typeCast + '"' + name + '");\n')
    
    if hasExtension:
        ow('    mpI->_' + name + 'EXT = (' + procExtDataType(name, platform) + ')' + procFunction(platform) + '('+ typeCast + '"' + name + '");\n')
    
    
def writeAssignments(platform):
    ow("OpenGlWrapper::OpenGlWrapper()\n")
    ow("{\n")
    ow('\n')
    ow('    mpI = new Internal();\n')
    ow('\n')
    
    ow('    ////////// OPENGL 1.0 ///////////\n')
    if platform == Platform.MACOSX:
        for procDesc in openGL_1_0: writeAssignment(procDesc, platform)
    ow('\n')
    ow('    ////////// OPENGL 1.2 ///////////\n')
    for procDesc in openGL_1_2: writeAssignment(procDesc, platform)
    ow('\n')    
    ow('    ////////// OPENGL 1.3 ///////////\n')
    for procDesc in openGL_1_3: writeAssignment(procDesc, platform)
    ow('\n')    
    ow('    ////////// OPENGL 1.5 ///////////\n')
    for procDesc in openGL_1_5: writeAssignment(procDesc, platform)
    ow('\n')    
    ow('    ////////// OPENGL 2.0 ///////////\n')
    for procDesc in openGL_2_0: writeAssignment(procDesc, platform)
    ow('\n')    
    ow('    ////////// OPENGL 3.1 ///////////\n')
    for procDesc in openGL_3_1: writeAssignment(procDesc, platform)
    ow('\n')    
    ow('    ////////// OPENGL 3.3 ///////////\n')
    for procDesc in openGL_3_3: writeAssignment(procDesc, platform)
    ow('\n')    

    ow('    ////////// OPENGL 4.2 ///////////\n')
    for procDesc in openGL_4_2: writeAssignment(procDesc, platform) 
    ow("}\n\n");

    
def writeWrapper(procDesc, platform, passThrough):
    returnType = procDesc[Index.RETURN_TYPE];
    name = procDesc[Index.NAME]
    arguments = procDesc[Index.ARGUMENTS]
    hasExtension = (procDesc[Index.EXTENSION] != '')
    
    argString = ""
    argString2 = ""
    for argSet in arguments: 
        argString += argSet[0] + " " + argSet[1] + ", "
        argString2 += argSet[1].lstrip('*') + ", "
    
    ow(returnType + " OpenGlWrapper::" + name + "("  + argString[0:-2] + ") { ")
    
    if returnType != "void": ow("return ");
    
    if passThrough: ow("::")
    else: ow("mpI->_")
    
    ow(name + "(" + argString2[0:-2] + "); }\n")
    
    
def writeExtWrapper(procDesc, platform):
    returnType = procDesc[Index.RETURN_TYPE];
    name = procDesc[Index.NAME]
    arguments = procDesc[Index.ARGUMENTS]
    extension = procDesc[Index.EXTENSION]
    
    returnString = ""
    if returnType != "void": returnString = "return"
    
    argString = ""
    argString2 = ""
    for argSet in arguments:
        argString += argSet[0] + " " + argSet[1] + ", "
        argString2 += argSet[1].lstrip('*') + ", "  
    
    ow(returnType + " OpenGlWrapper::" + name + "(" + argString[0:-2] + ")\n")
    ow("{\n")
    
    ow("    if (mpI->_" + name + ") " + returnString + " mpI->_" + name + "(" + argString2[0:-2] + ");\n")
    ow("    else if (mpI->_" + name + "EXT) " + returnString + " mpI->_" + name + "EXT(" + argString2[0:-2] + ");\n")
        
    ow("    else mini3d_assert(false, \"OpenGL driver does not support " + extension + "\");\n")
    
    if returnType != "void": 
        ow("    return 0;\n")
    
    ow("}\n\n")

    
def writeWrappers(platform):

    ow('    ////////// OPENGL 1.0 ///////////\n')
    for procDesc in openGL_1_0: writeWrapper(procDesc, platform, platform != Platform.MACOSX)
    ow('\n')
    ow('    ////////// OPENGL 1.2 ///////////\n')
    for procDesc in openGL_1_2: writeWrapper(procDesc, platform, False)
    ow('\n')
    ow('    ////////// OPENGL 1.3 ///////////\n')
    for procDesc in openGL_1_3: writeWrapper(procDesc, platform, False)
    ow('\n')
    ow('    ////////// OPENGL 1.5 ///////////\n')
    for procDesc in openGL_1_5: writeWrapper(procDesc, platform, False)
    ow('\n')
    ow('    ////////// OPENGL 2.0 ///////////\n')
    for procDesc in openGL_2_0: writeWrapper(procDesc, platform, False)
    ow('\n')
    ow('    ////////// OPENGL 3.1 ///////////\n')
    for procDesc in openGL_3_1: writeWrapper(procDesc, platform, False)
    ow('\n')
    ow('    ////////// OPENGL 3.3 ///////////\n')
    for procDesc in openGL_3_3: writeWrapper(procDesc, platform, False)
    ow('\n')
    ow('    ////////// OPENGL 4.2 ///////////\n')
    for procDesc in openGL_4_2: writeExtWrapper(procDesc, platform)


# Parse opengl function definitions from the input file

#input = open('openglwrapper.txt', 'r').readlines()
#functions = [];
#for line in input:
#   functions.append(OpenGLFunction(line))

# Write Win32 version
ow = open('../win32/openglwrapper_win32.cpp', 'w').write
writeHeader(Platform.WIN32)
writeDeclarations(Platform.WIN32)
writeAssignments(Platform.WIN32)
writeWrappers(Platform.WIN32)
writeFooter(Platform.WIN32)

ow = open('../linux/openglwrapper_linux.cpp', 'w').write
writeHeader(Platform.LINUX)
writeDeclarations(Platform.LINUX)
writeAssignments(Platform.LINUX)
writeWrappers(Platform.LINUX)
writeFooter(Platform.LINUX)

ow = open('../osx/openglwrapper_osx.cpp', 'w').write
writeHeader(Platform.MACOSX)
writeDeclarations(Platform.MACOSX)
writeAssignments(Platform.MACOSX)
writeWrappers(Platform.MACOSX)
writeFooter(Platform.MACOSX)
