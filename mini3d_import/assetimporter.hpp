
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_ASSETIMPORTER_H
#define MINI3D_ASSETIMPORTER_H

#include <stdint.h>
#include <cstring>
#include <cstdlib>

// Vertex Data Structure
namespace mini3d {
namespace import {

const unsigned int NO_TEXTURE = 0xffff;
const unsigned int NO_BONE_PARENT = 0xffff;

struct NameIndexMap { const char* name; unsigned int index; };

struct Scene;
struct Object;
struct Mesh;
struct Material;
struct Texture;
struct Bone;
struct Armature;
struct Action;
struct Light;
struct Camera;

struct AssetImporter
{
    static AssetImporter* LoadFromFile(const char* filename);
    AssetImporter();
    ~AssetImporter();
    
    inline static int stringComparer(const void* a, const void* b) { return strcmp((const char*)a, ((NameIndexMap*)b)->name); }

    unsigned int GetSceneByName(const char* name)       { return ((NameIndexMap*) bsearch(name, sceneNameIndexMap, sceneCount,          sizeof(NameIndexMap), &AssetImporter::stringComparer))->index; }
    unsigned int GetMeshByName(const char* name)        { return ((NameIndexMap*) bsearch(name, meshNameIndexMap, meshCount,            sizeof(NameIndexMap), &AssetImporter::stringComparer))->index; }
    unsigned int GetMaterialByName(const char* name)    { return ((NameIndexMap*) bsearch(name, materialNameIndexMap, materialCount,    sizeof(NameIndexMap), &AssetImporter::stringComparer))->index; }
    unsigned int GetArmatureByName(const char* name)    { return ((NameIndexMap*) bsearch(name, armatureNameIndexMap, armatureCount,    sizeof(NameIndexMap), &AssetImporter::stringComparer))->index; }
    unsigned int GetTextureByName(const char* name)     { return ((NameIndexMap*) bsearch(name, textureNameIndexMap, textureCount,      sizeof(NameIndexMap), &AssetImporter::stringComparer))->index; }
    unsigned int GetActionByName(const char* name)      { return ((NameIndexMap*) bsearch(name, actionNameIndexMap, actionCount,        sizeof(NameIndexMap), &AssetImporter::stringComparer))->index; }

    unsigned int sceneCount;
    NameIndexMap* sceneNameIndexMap;
    Scene* scenes;

	unsigned int meshCount;
    NameIndexMap* meshNameIndexMap;
	Mesh* meshes;

	unsigned int materialCount;
    NameIndexMap* materialNameIndexMap;
	Material* materials;

	unsigned int armatureCount;
    NameIndexMap* armatureNameIndexMap;
    Armature* armatures;

	unsigned int textureCount;
    NameIndexMap* textureNameIndexMap;
    Texture* textures;

	unsigned int actionCount;
    NameIndexMap* actionNameIndexMap;
    Action* actions;
};

struct Scene
{
    Scene();
    ~Scene();

    unsigned int GetCameraByName(const char* name)      { return ((NameIndexMap*)bsearch(name, cameraNameIndexMap, cameraCount,         sizeof(NameIndexMap), &AssetImporter::stringComparer))->index; }
    unsigned int GetLightByName(const char* name)       { return ((NameIndexMap*)bsearch(name, lightNameIndexMap, lightCount,           sizeof(NameIndexMap), &AssetImporter::stringComparer))->index; }
    unsigned int GetObjectByName(const char* name)      { return ((NameIndexMap*)bsearch(name, objectNameIndexMap, objectCount,         sizeof(NameIndexMap), &AssetImporter::stringComparer))->index; }

	unsigned int cameraCount;
    NameIndexMap* cameraNameIndexMap;
	Camera* cameras;

	unsigned int lightCount;
    NameIndexMap* lightNameIndexMap;
	Light* lights;

	unsigned int objectCount;
    NameIndexMap* objectNameIndexMap;
	Object* objects;
};

struct Object
{
    Object();
    ~Object();

  	float position[4];
	float rotation[4];
	float scale[4];

	unsigned int meshIndex;
    unsigned int materialIndex;
};

struct BoneData
{
	float boneIndex[4];
	float weight[4];
};

struct Mesh
{
    Mesh();
    ~Mesh();

	unsigned int vertexCount;
    unsigned int vertexSizeInBytes;
    const char* vertexData;
	
	unsigned int indexCount;
	unsigned int* indices;
};

struct Material
{
    Material();
    ~Material();

	int shaderIndex;
	
	unsigned int textureCount;
	unsigned int* textureIndices;
};

struct Camera
{
    Camera();
    ~Camera();

	float position[4];
	float rotation[4];

	float horizontalFov;
	float clipPlaneNear;
	float clipPlaneFar;
	float aspectRatio;
};

struct Light
{
	enum Type { DIRECTIONAL, POINT, SPOT };

    Light();
    ~Light();

	Type type;

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

struct Texture
{
    Texture();
    ~Texture();

	const char* filename;
};

struct Joint
{
    unsigned int parentIndex;
	float offset[4];
    float roll;
};

struct Armature
{
    Armature();
    ~Armature();

	unsigned int jointCount;
	Joint* joints;
};

// Nodes are sorted in time order
struct Action
{
    
    Action();
    ~Action();

    const char* animationData; // TODO: init to 0 and delete!
};

} // namespace import
} // namespace mini3d

#endif
