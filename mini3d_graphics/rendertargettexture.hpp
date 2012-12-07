
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_IRENDERTARGETTEXTURE_H
#define MINI3D_IRENDERTARGETTEXTURE_H

#include "itexture.hpp"
#include "irendertarget.hpp"

namespace mini3d {
namespace graphics {

struct IRenderTargetTexture : public IRenderTarget, public ITexture
{

	enum Format { FORMAT_RGBA8UI, FORMAT_RGBA16UI, FORMAT_R16I, FORMAT_R32F };


#define IRENDERTARGETTEXTURE_INTERFACE(PURE_VIRTUAL)\
\
	virtual void SetRenderTargetTexture(unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, bool depthTestEnabled, MipMapMode mipMapMode = MIPMAP_NONE) PURE_VIRTUAL;\
\
	virtual unsigned int GetWidth() const PURE_VIRTUAL;\
	virtual unsigned int GetHeight() const PURE_VIRTUAL;\
	virtual void SetSize(unsigned int width, unsigned int height) PURE_VIRTUAL;\
	virtual Viewport GetViewport() const PURE_VIRTUAL;\
	virtual void SetViewport(Viewport viewport) PURE_VIRTUAL;\
	virtual Format GetFormat() const PURE_VIRTUAL;\
	virtual bool GetDepthTestEnabled() const PURE_VIRTUAL;\
	virtual void SetDepthTestEnabled(bool depthTestEnabled) PURE_VIRTUAL;\
	virtual MipMapMode GetMipMapMode() const PURE_VIRTUAL;\
	virtual void SetMipMapMode(MipMapMode mipMapMode) PURE_VIRTUAL;\
	virtual SamplerSettings GetSamplerSettings() const PURE_VIRTUAL;\


public:

	IRENDERTARGETTEXTURE_INTERFACE(=0);
	virtual ~IRenderTargetTexture() {};

};
}
}

#endif


