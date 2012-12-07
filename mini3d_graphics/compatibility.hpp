
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_COMPATIBILITY_H
#define MINI3D_COMPATIBILITY_H

#include "bitmaptexture.hpp"
#include "rendertargettexture.hpp"

namespace mini3d {
namespace graphics {

struct ICompatibility
{

#define ICOMPATIBILITY_INTERFACE(PURE_VIRTUAL)\
\
	virtual unsigned int	TextureStreamCount() const PURE_VIRTUAL;\
	virtual unsigned int	MaxTextureSize() const PURE_VIRTUAL;\
	virtual bool			TextureFormat(IBitmapTexture::Format format) const PURE_VIRTUAL;\
	virtual bool			RenderTargetTextureFormat(IRenderTargetTexture::Format format) const PURE_VIRTUAL;\
\
	virtual const char*		ShaderLanguage() const PURE_VIRTUAL;\
	virtual const char*		PixelShaderVersion() const PURE_VIRTUAL;\
	virtual const char*		VertexShaderVersion() const PURE_VIRTUAL;\
\
	virtual unsigned int	VertexStreamCount() const PURE_VIRTUAL;\
\
	virtual unsigned int	FreeGraphicsMemory() const PURE_VIRTUAL;\
\

public:
	ICOMPATIBILITY_INTERFACE(=0);
	virtual ~ICompatibility() {};

};
} // namespace mini3d
}

#endif