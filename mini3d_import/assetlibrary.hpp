
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_ASSETIMPORTER_H
#define MINI3D_ASSETIMPORTER_H

#include <cstring>

void mini3d_assert(bool expression, const char* text, ...);

// Vertex Data Structure
namespace mini3d {
namespace import {

const unsigned int NO_TEXTURE = 0xffff;
const unsigned int NO_BONE_PARENT = 0xffff;

////////// HELPER CLASSES ///////////////////////////////////////////////////////

template <typename T> 
struct AutoArray
{
    AutoArray() : array(0), count(0)                                    {}
    AutoArray(T* array, unsigned int size) : array(array), count(size)  {}
    ~AutoArray()                                                        { delete[] array; }

    T* array; 
    unsigned int count; 
};

struct AutoString : AutoArray<char>
{
    // No need to use include cstring for strlen() and strcmp() alone...
    AutoString()                                                        {}
    AutoString(const char* str)                                         { array = strdup(str); count = strlen(str); }
    char* operator = (char* rhs)                                        { array = rhs; count = strlen(rhs); return rhs; }
    bool operator == (const char* rhs)                                  { return !strcmp(array, rhs); }
    bool operator == (AutoString &rhs)                                  { return !strcmp(array, rhs.array); }
};

template <typename T, bool AUTODELETE = false> 
struct AutoObjectArray : AutoArray<T>
{
    ~AutoObjectArray()                                                  { if (AUTODELETE) for (unsigned int i = 0; i < count; ++i) (array + i)->~T(); }
};

struct NamedResource { AutoString name; unsigned int index; };

template <typename T, bool AUTODELETE = false> 
struct AssetArray : AutoArray<T>
{ 
    AssetArray()                                                        {}
    AssetArray(T* array, unsigned int size) : AutoArray(array, size)    {}
    ~AssetArray()                                                       { if (AUTODELETE) for (unsigned int i = 0; i < count; ++i) (array + i)->~T(); }

    T* Find(const char* name)                                           { return (T*) bsearch(name, array, count, sizeof(T), &cmp); }
    static int cmp(const void* a, const void* b)                        { return strcmp((const char*)a, ((NamedResource*)b)->name.array); }
};


////////// ASSET CLASSES ////////////////////////////////////////////////////////

struct Mesh;
struct Material;
struct Texture;

struct Object: public NamedResource
{
  	float position[4];
	float rotation[4];
	float scale[4];

	Mesh* mesh;
    Material* material;
};

struct Camera : public NamedResource
{
	float position[4];
	float rotation[4];

	float horizontalFov;
	float clipPlaneNear;
	float clipPlaneFar;
	float aspectRatio;
};

struct Light : public NamedResource
{
	enum Type { DIRECTIONAL, POINT, SPOT } type;

	float position[4];
	float rotation[4];

	float angleInnerCone;
	float angleOuterCone;
	float color[3];

	float attenuationConstant;
	float attenuationLinear;
	float attenuationQuadratic;

	float clipPlaneNear;
	float clipPlaneFar;
	float aspectRatio;
};


struct Scene : public NamedResource
{
    AssetArray<Camera, true> cameras;
    AssetArray<Light, true> lights;
    AssetArray<Object, true> objects;
};

struct Mesh : public NamedResource
{
    unsigned int vertexSizeInBytes;
    AutoArray<char> vertexData;

    unsigned int indexSizeInBytes;
	AutoArray<char> indexData;
};

struct Material : public NamedResource
{
    AssetArray<Texture*> textures;
};

struct Texture : public NamedResource
{
    AutoString filename;
};

struct Joint : public NamedResource
{
    Joint* parent;
	float offset[4];
    float roll[4];
};

struct Armature : public NamedResource
{
    AssetArray<Joint, true> joints;
};

struct Channel
{
    AutoString boneName;
    enum Type { POSITION, ROTATION, SCALE } type;
    AutoArray<char> animationData;
};

struct Action : public NamedResource
{
    float length;
    AutoObjectArray<Channel, true> channels;
};

struct AssetLibrary
{
    static AssetLibrary* LoadFromFile(const char* filename);
    
    // true means autodelete array contents in array destructor
    AssetArray<Scene, true> scenes;
    AssetArray<Mesh, true> meshes;
    AssetArray<Material, true> materials;
    AssetArray<Armature, true> armatures;
    AssetArray<Texture, true> textures;
    AssetArray<Action, true> actions;
};

} // namespace import
} // namespace mini3d

#endif
