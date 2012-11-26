
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_IBITMAPTEXTURE_H
#define MINI3D_IBITMAPTEXTURE_H

#include "itexture.hpp"

namespace mini3d
{

struct IBitmapTexture : public virtual ITexture
{

	enum Format { FORMAT_RGBA8UI, FORMAT_RGBA16UI, FORMAT_R16I, FORMAT_R32F, FORMAT_RGBA8UI_COMPRESSED };


#define IBITMAPTEXTURE_INTERFACE(PURE_VIRTUAL)\
\
	virtual void* GetBitmap(unsigned int& sizeInBytes) PURE_VIRTUAL; /* TODO: Make Const! */ \
	virtual void SetBitmap(const void* pBitmap, unsigned int width, unsigned int height, Format format, SamplerSettings samplerSettings, MipMapMode mipMapMode = MIPMAP_BOX_FILTER) PURE_VIRTUAL;\
\
	virtual void* Lock(unsigned int& sizeInBytes, bool readOnly = false) PURE_VIRTUAL;\
	virtual void Unlock() PURE_VIRTUAL;\
\
	virtual unsigned int GetWidth() const PURE_VIRTUAL;\
	virtual unsigned int GetHeight() const PURE_VIRTUAL;\
\
	virtual Format GetFormat() const PURE_VIRTUAL;\
	virtual MipMapMode GetMipMapMode() const PURE_VIRTUAL;\
\
	virtual void SetMipMapMode(MipMapMode mipMapMode) PURE_VIRTUAL;\
	virtual SamplerSettings GetSamplerSettings() const PURE_VIRTUAL;\


public:
	IBITMAPTEXTURE_INTERFACE(=0);
	virtual ~IBitmapTexture() {};

};
}


#endif
