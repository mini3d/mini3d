
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_IVERTEXSHADER_H
#define MINI3D_IVERTEXSHADER_H

namespace mini3d {
namespace graphics {


struct IVertexShader
{
	
#define IVERTEXSHADER_INTERFACE(PURE_VIRTUAL)\
\
	virtual void* GetVertexShader(unsigned int& sizeInBytes) const PURE_VIRTUAL;\


public:

	IVERTEXSHADER_INTERFACE(=0);
	~IVertexShader() {};

};
}
}

#endif
