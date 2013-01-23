
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_IBITMAPTEXTURE_H
#define MINI3D_IBITMAPTEXTURE_H

#include "itexture.hpp"

namespace mini3d {
namespace graphics {

struct IGraphicsService;

struct IBitmapTexture : public ITexture
{
    static const char* TYPE;
    virtual const char* GetType() const { return TYPE; }

    // TODO: Compressed texture formats BC1, ETC1, PVRTC_RGB_2BPP
	enum Format { FORMAT_RGBA8UI = 0 };

    static IBitmapTexture* New(IGraphicsService* pGraphics, const char* pBitmap, unsigned int width, unsigned int height, Format format = FORMAT_RGBA8UI, SamplerSettings samplerSettings = SAMPLER_SETTINGS_DEFAULT);
 	virtual ~IBitmapTexture() {};

    // pBitmap is a pointer to a raw pixel buffer with the same format as the requested texture format.
    // When using MipMapMode MIPMAP_MANUAL, all additional mip-map levels are expected to be appended in ascending order after the base level texture in pBitmap.
    virtual void SetBitmap(const char* pBitmap, unsigned int width, unsigned int height, Format format = FORMAT_RGBA8UI, SamplerSettings samplerSettings = SAMPLER_SETTINGS_DEFAULT) = 0;

    virtual unsigned int GetWidth() const = 0;
	virtual unsigned int GetHeight() const = 0;

    virtual Format GetFormat() const = 0;
	virtual SamplerSettings GetSamplerSettings() const = 0;
};

}
}

#endif
