
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_ISHADERPROGRAM_H
#define MINI3D_ISHADERPROGRAM_H

namespace mini3d {
namespace graphics {

struct IPixelShader;
struct IVertexShader;

struct IShaderProgram
{

#define ISHADERPROGRAM_INTERFACE(PURE_VIRTUAL)\
\
	virtual IPixelShader* GetPixelShader() const PURE_VIRTUAL;\
	virtual IVertexShader* GetVertexShader() const PURE_VIRTUAL;\


public:
	
	ISHADERPROGRAM_INTERFACE(=0);
	virtual ~IShaderProgram() {};

};
}
}

#endif
