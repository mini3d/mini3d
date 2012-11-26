
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_IPIXELSHADER_H
#define MINI3D_IPIXELSHADER_H

namespace mini3d
{

struct IPixelShader
{


#define IPIXELSHADER_INTERFACE(PURE_VIRTUAL)\
\
	virtual void* GetPixelShader(unsigned int& sizeInBytes) PURE_VIRTUAL;\


public:

	IPIXELSHADER_INTERFACE(=0);
	virtual ~IPixelShader() {};

};
}

	


#endif
