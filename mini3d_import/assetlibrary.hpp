
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

struct NamedResource { const char* name; NamedResource() { name = 0; } virtual ~NamedResource() { delete[] name; } };

inline static int stringComparer(const void* a, const void* b) { return strcmp((const char*)a, ((NamedResource*)b)->name); }

struct Object: public NamedResource
{
    Object();
    ~Object();

  	float position[4];
	float rotation[4];
	float scale[4];

	unsigned int meshIndex;
    unsigned int materialIndex;
};

struct Camera : public NamedResource
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

struct Light : public NamedResource
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

struct Scene : public NamedResource
{
    Scene();
    ~Scene();

    Camera* GetCameraByName(const char* name)       { return (Camera*) bsearch(name, cameras, cameraCount, sizeof(Camera), &stringComparer); }
    Light* GetLightByName(const char* name)         { return (Light*) bsearch(name, lights, lightCount, sizeof(Light), &stringComparer); }
    Object* GetObjectByName(const char* name)       { return (Object*) bsearch(name, objects, objectCount, sizeof(Object), &stringComparer); }

	unsigned int cameraCount;
	Camera* cameras;

	unsigned int lightCount;
	Light* lights;

	unsigned int objectCount;
	Object* objects;
};

struct Mesh : public NamedResource
{
    Mesh();
    ~Mesh();

	unsigned int vertexCount;
    unsigned int vertexSizeInBytes;
    const char* vertexData;
	
	unsigned int indexCount;
	unsigned int* indices;
};

struct Material : public NamedResource
{
    Material();
    ~Material();

	int shaderIndex;
	
	unsigned int textureCount;
	unsigned int* textureIndices;
};

struct Texture : public NamedResource
{
    Texture();
    ~Texture();

	const char* filename;
};

struct Joint : public NamedResource
{
    const char* name;
    unsigned int parentIndex;
	float offset[4];
    float roll;
};

struct Armature : public NamedResource
{
    Armature();
    ~Armature();

	unsigned int jointCount;
	Joint* joints;
};

// Nodes are sorted in time order
struct Action : public NamedResource
{
    struct Channel
    {
        Channel();
        ~Channel();

        const char* boneName;
        const char* targetName;
        unsigned int subindexCount;
        unsigned int keyframeCount;
        float* animationData;
    };
    

    Action();
    ~Action();
    
    unsigned int channelCount;
    Channel* channels;
};

struct AssetLibrary
{
    static AssetLibrary* LoadFromFile(const char* filename);
    AssetLibrary();
    ~AssetLibrary();
    
    Scene* GetSceneByName(const char* name)         { return (Scene*) bsearch(name, scenes, sceneCount, sizeof(Scene), &stringComparer); }
    Mesh* GetMeshByName(const char* name)           { return (Mesh*) bsearch(name, meshes, meshCount, sizeof(Mesh), &stringComparer); }
    Material* GetMaterialByName(const char* name)   { return (Material*) bsearch(name, materials, materialCount, sizeof(Material), &stringComparer); }
    Armature* GetArmatureByName(const char* name)   { return (Armature*) bsearch(name, armatures, armatureCount, sizeof(Armature), stringComparer); }
    Texture* GetTextureByName(const char* name)     { return (Texture*) bsearch(name, textures, textureCount, sizeof(Texture), &stringComparer); }
    Action* GetActionByName(const char* name)       { return (Action*) bsearch(name, actions, actionCount, sizeof(Action), &stringComparer); }

    unsigned int sceneCount;
    Scene* scenes;

	unsigned int meshCount;
	Mesh* meshes;

	unsigned int materialCount;
	Material* materials;

	unsigned int armatureCount;
    Armature* armatures;

	unsigned int textureCount;
    Texture* textures;

	unsigned int actionCount;
    Action* actions;
};


} // namespace import
} // namespace mini3d

#endif
