
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_IVERTEXBUFFER_H
#define MINI3D_IVERTEXBUFFER_H

namespace mini3d {
namespace graphics {

struct IGraphicsService;

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
	enum StreamMode { STREAM_MODE_VERTEX_DATA, STREAM_MODE_INSTANCE_DATA };

	struct VertexAttribute { const char* name; DataUsage usage; unsigned int usageIndex; DataType type; unsigned int count; };

    static IVertexBuffer* New(IGraphicsService* pGraphics, const void* pVertices, unsigned int vertexCount, unsigned int vertexSizeInBytes, const VertexAttribute* pAttributes, unsigned int attributeCount, StreamMode streamMode = STREAM_MODE_VERTEX_DATA);
    virtual ~IVertexBuffer() {};

	virtual void SetVertices(const void* pVertices, unsigned int vertexCount, unsigned int vertexSizeInBytes, const VertexAttribute* pAttributes, unsigned int attributeCount, StreamMode streamMode = STREAM_MODE_VERTEX_DATA) = 0;

    virtual unsigned int GetVertexAttributeCount() const = 0;
	virtual void GetVertexAttributes(VertexAttribute* pAttributes) const = 0;
	
    virtual unsigned int GetVertexCount() const = 0;
	virtual unsigned int GetVertexSizeInBytes() const = 0;

    virtual StreamMode GetStreamMode() const = 0;
	virtual void SetStreamMode(StreamMode streamMode) = 0;

};
}
}

#endif
