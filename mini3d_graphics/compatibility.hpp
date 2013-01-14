
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
	virtual ~ICompatibility() {};

	virtual unsigned int	GetTextureUnitCount() const = 0;
    virtual unsigned int	MaxTextureSize() const = 0;
	virtual bool			TextureFormat(IBitmapTexture::Format format) const = 0;
	virtual bool			RenderTargetTextureFormat(IRenderTargetTexture::Format format) const = 0;

	virtual const char*		ShaderLanguage() const = 0;
	virtual const char*		PixelShaderVersion() const = 0;
	virtual const char*		VertexShaderVersion() const = 0;

	virtual unsigned int	VertexStreamCount() const = 0;

	virtual unsigned int	FreeGraphicsMemory() const = 0;
};
}
}

#endif