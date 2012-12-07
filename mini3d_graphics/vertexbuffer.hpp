
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_IVERTEXBUFFER_H
#define MINI3D_IVERTEXBUFFER_H

namespace mini3d {
namespace graphics {


struct IVertexBuffer
{
	enum DataUsage {
	  USAGE_POSITION       = 0,
	  USAGE_BLENDWEIGHT    = 1,
	  USAGE_BLENDINDICES   = 2,
	  USAGE_NORMAL         = 3,
	  USAGE_PSIZE          = 4,
	  USAGE_TEXCOORD       = 5,
	  USAGE_TANGENT        = 6,
	  USAGE_BINORMAL       = 7,
	  USAGE_TESSFACTOR     = 8,
	  USAGE_POSITIONT      = 9,
	  USAGE_COLOR          = 10,
	  USAGE_FOG            = 11,
	  USAGE_DEPTH          = 12,
	  USAGE_SAMPLE         = 13 
	};

	enum DataType { DATA_TYPE_FLOAT };
	enum StreamMode { STREAM_PER_VERTEX, STREAM_PER_INSTANCE };

	struct ComponentDescription { const char* name; DataUsage usage; unsigned int usageIndex; DataType type; unsigned int count; };


#define IVERTEXBUFFER_INTERFACE(PURE_VIRTUAL)\
\
	virtual void* GetVertices(unsigned int& sizeInBytes) PURE_VIRTUAL; /* TODO: Make Const! */ \
	virtual void SetVertices(const void* pVertices, unsigned int count, unsigned int vertexSizeInBytes, StreamMode streamMode = STREAM_PER_VERTEX) PURE_VIRTUAL;\
\
	virtual void* Lock(unsigned int& sizeInBytes, bool readOnly = false) PURE_VIRTUAL;\
	virtual void Unlock() PURE_VIRTUAL;\
\
	virtual void SetComponentDescriptions(const ComponentDescription* pComponentDescriptions, const unsigned int count) PURE_VIRTUAL;\
	virtual const ComponentDescription* GetComponentDescriptions(unsigned int &count) const PURE_VIRTUAL;\
\
	virtual unsigned int GetVertexCount() const PURE_VIRTUAL;\
	virtual unsigned int GetVertexSizeInBytes() const PURE_VIRTUAL;\
	virtual StreamMode GetStreamMode() const PURE_VIRTUAL;\
	virtual void SetStreamMode(StreamMode streamMode) PURE_VIRTUAL;\


public:
	
	IVERTEXBUFFER_INTERFACE(=0);
	virtual ~IVertexBuffer() {};
};
}
}

#endif
