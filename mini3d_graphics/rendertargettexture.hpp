
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_IRENDERTARGETTEXTURE_H
#define MINI3D_IRENDERTARGETTEXTURE_H

#include "itexture.hpp"
#include "irendertarget.hpp"

namespace mini3d {
namespace graphics {

struct IGraphicsService;

struct IRenderTargetTexture : public IRenderTarget, public ITexture
{
    static const char* TYPE;
    virtual const char* GetType() const { return TYPE; }

    enum Format { FORMAT_RGBA8UI, FORMAT_RGBA16UI, FORMAT_R16I, FORMAT_R32F };

	static IRenderTargetTexture* New(IGraphicsService* pGraphics, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, bool depthTestEnabled, MipMapMode mipMapCount = MIPMAP_AUTOGENERATE);
    virtual ~IRenderTargetTexture() {};
    
	virtual void SetRenderTargetTexture(unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, bool depthTestEnabled, MipMapMode mipMapMode = MIPMAP_AUTOGENERATE) = 0;

	virtual unsigned int GetWidth() const = 0;
	virtual unsigned int GetHeight() const = 0;

	virtual Viewport GetViewport() const = 0;
	virtual void SetViewport(Viewport viewport) = 0;
	
    virtual Format GetFormat() const = 0;
	virtual bool GetDepthTestEnabled() const = 0;
    virtual SamplerSettings GetSamplerSettings() const = 0;
    virtual MipMapMode GetMipMapMode() const = 0;
};

}
}

#endif

