
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_IINDEXBUFFER_H
#define MINI3D_IINDEXBUFFER_H

namespace mini3d {
namespace graphics {

struct IGraphicsService;

struct IIndexBuffer
{
	enum DataType { INT_16 = 0, INT_32 = 1 }; // Don't change the numbers, they are used as array indices in the implementations

    static IIndexBuffer* New(IGraphicsService* pGraphics, const void* pIndices, unsigned int count, DataType dataType = INT_32);
    virtual ~IIndexBuffer() {};

	virtual void SetIndices(const void* pIndices, unsigned int count, DataType dataType = INT_32) = 0;

	virtual unsigned int GetIndexCount() const = 0;
	virtual DataType GetDataType() const = 0;
};

}
}

#endif
