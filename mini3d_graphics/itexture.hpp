
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_ITEXTURE_H
#define MINI3D_ITEXTURE_H

namespace mini3d {
namespace graphics {

class ITexture
{
public:

	struct SamplerSettings
	{
		enum WrapMode { WRAP_CLAMP, WRAP_TILE };
		enum SampleMode { SAMPLE_NEAREST, SAMPLE_LINEAR };

		WrapMode wrapMode;
		SampleMode sampleMode;
	};

	enum MipMapMode {MIPMAP_NONE, MIPMAP_BOX_FILTER};

	virtual ~ITexture(void) {};

	virtual unsigned int GetWidth() const = 0;
	virtual unsigned int GetHeight() const = 0;

	virtual MipMapMode GetMipMapMode() const = 0;
	virtual void SetMipMapMode(MipMapMode mipMapMode) = 0;

//	virtual Format GetFormat() const = 0;
	virtual SamplerSettings GetSamplerSettings() const = 0;
};
}
}

#endif
