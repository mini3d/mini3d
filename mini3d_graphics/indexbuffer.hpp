
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_IINDEXBUFFER_H
#define MINI3D_IINDEXBUFFER_H

namespace mini3d {
namespace graphics {

struct IIndexBuffer
{
	enum DataType { INT_16 = 0, INT_32 = 1 };


#define IINDEXBUFFER_INTERFACE(PURE_VIRTUAL)\
\
	virtual void* GetIndices(unsigned int& sizeInBytes) PURE_VIRTUAL; /* TODO: Make Const! */ \
	virtual void SetIndices(const void* pIndices, unsigned int count, DataType dataType = INT_32) PURE_VIRTUAL;\
\
	virtual void* Lock(unsigned int& sizeInBytes, bool readOnly = false) PURE_VIRTUAL;\
	virtual void Unlock() PURE_VIRTUAL;\
\
	virtual unsigned int GetIndexCount() const PURE_VIRTUAL;\
	virtual DataType GetDataType() const PURE_VIRTUAL;\


public:

	IINDEXBUFFER_INTERFACE(=0);
	virtual ~IIndexBuffer() {};
};
}
}

#endif
