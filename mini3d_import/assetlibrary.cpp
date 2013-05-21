
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#include "assetlibrary.hpp"
#include "importers/mini3d/mini3dimporter.hpp"

#include <cstring>
#include <cstdio>
#include <cctype>

void mini3d_assert(bool expression, const char* text, ...);


using namespace mini3d::import;

AssetLibrary::AssetLibrary() :
    sceneCount(0),
    scenes(0),
    meshCount(0),
    meshes(0),
    materialCount(0),
    materials(0),
    textureCount(0),
    textures(0),
    armatureCount(0),
    armatures(0),
    actionCount(0),
    actions(0)
{
}

AssetLibrary* AssetLibrary::LoadFromFile(const char* filename)
{
    const char* pos = strrchr(filename, '.');
    mini3d_assert(pos != 0, "Faled to identify file ending for file: %s", filename);

    char fileEnding[FILENAME_MAX];
    strcpy(fileEnding, pos+ 1);
    
    for(int i = 0; fileEnding[i] != 0; ++i) fileEnding[i] = tolower(fileEnding[i]);

    if (!strcmp(fileEnding, "m3d"))
    {
        Mini3dImporter* pMini3dImp = new Mini3dImporter();
	    AssetLibrary* pAssetLibrary = pMini3dImp->LoadSceneFromFile(filename);
        delete pMini3dImp;
        return pAssetLibrary;
    }

    mini3d_assert(false, "Failed to find a matching file parser for file: %s", filename);
    return 0;
}

AssetLibrary::~AssetLibrary()
{
    delete[] scenes;
    delete[] meshes;
    delete[] materials;
    delete[] textures;
    delete[] armatures;
    delete[] actions;
}

Scene::Scene() :
    objectCount(0),
    objects(0),
    lightCount(0),
    lights(0),
    cameraCount(0),
    cameras(0)
{
}

Scene::~Scene()
{
}

Object::Object() {}
Object::~Object()
{
}

Mesh::Mesh() :
    vertexCount(0),
    vertexData(0)
{
}


Mesh::~Mesh()
{
    delete[] vertexData;
}

Material::Material() :
    textureCount(0),
    textureIndices(0),
    shaderIndex(0)
{
}

Material::~Material()
{
    delete[] textureIndices;
}

Camera::Camera() {}
Camera::~Camera()
{
}

Light::Light() {}
Light::~Light()
{
}

Texture::Texture() : filename(0) {}
Texture::~Texture()
{
    delete[] filename;
}

Armature::Armature() :
    jointCount(0),
    joints(0)
{
}

Armature::~Armature()
{
    delete[] joints;
}


Action::Action() : channels(0)
{
}

Action::~Action()
{
    delete[] channels;
}

Action::Channel::Channel() : animationData(0) {}

Action::Channel::~Channel()
{
    delete[] animationData;
}
