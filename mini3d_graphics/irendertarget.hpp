
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_IRENDERTARGET_H
#define MINI3D_IRENDERTARGET_H

namespace mini3d {
namespace graphics {

struct IGraphicsService;

struct IRenderTarget
{
    virtual const char* GetType() const = 0;
    
    struct Viewport { unsigned int x, y, width, height;	};

	virtual ~IRenderTarget() {};

    // bool GetContents(char* pixelBuffer) // Not implemented yet, will read back buffer data (buffer is buffer of size GetWidth() * GetHeight() * 4 bytes per pixel)

	virtual unsigned int GetWidth() const = 0;
	virtual unsigned int GetHeight() const = 0;

	virtual Viewport GetViewport() const = 0;
	virtual void SetViewport(Viewport viewport) = 0;

	virtual bool GetDepthTestEnabled() const = 0;
};

}
}

#endif
