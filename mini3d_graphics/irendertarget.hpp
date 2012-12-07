
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_IRENDERTARGET_H
#define MINI3D_IRENDERTARGET_H


namespace mini3d {
namespace graphics {

class IRenderTarget
{
public:

	struct Viewport
	{
		unsigned int x;
		unsigned int y;
		unsigned int width;
		unsigned int height;
	};

	virtual ~IRenderTarget() {};

	virtual unsigned int GetWidth() const = 0;
	virtual unsigned int GetHeight() const = 0;

	virtual Viewport GetViewport() const = 0;
	virtual void SetViewport(Viewport viewport) = 0;

	virtual bool GetDepthTestEnabled() const = 0;
	virtual void SetDepthTestEnabled(bool isDepthTestEnabled) = 0;

};
}
}

#endif
