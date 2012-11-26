
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#include "mini3dimporter.hpp"

using namespace mini3d::import;

template <typename T> void Read(FILE* file, T* t, unsigned int count = 1) { fread(t, sizeof(T), count, file); }
unsigned short ReadShort(FILE* file) { uint16_t t; Read<uint16_t>(file, &t); return t; }
float ReadFloat(FILE* file) { float t; Read<float>(file, &t); return t; }
const char* ReadString(FILE* file) 
{ 
    unsigned int length = ReadShort(file);
    char* string = new char[length + 1];
    string[length] = 0;
    Read<char>(file, string, length);
    return string;
}

const char* ReadBytes(FILE* file, unsigned int size)
{
    char* data = new char[size];
    Read<char>(file, data, size);
    return data;
}

AssetImporter* Mini3dImporter::LoadSceneFromFile(const char* filename)
{
    AssetImporter* pI = new AssetImporter();
    
	FILE *file = fopen(filename, "rb");
    mini3d_assert(file != 0, "Failed to open file %s", filename);
	
    ////////// MESHES /////////////////////////////////////////////////////////

    // Read mesh count
    pI->meshCount = ReadShort(file);

    // Read mesh name <-> index map
    pI->meshNameIndexMap = new NameIndexMap[pI->meshCount];
    for (unsigned int i = 0; i < pI->meshCount; ++i)
    {
        pI->meshNameIndexMap[i].name = ReadString(file);
        pI->meshNameIndexMap[i].index = ReadShort(file);
    }

    // read meshes
    pI->meshes = new Mesh[pI->meshCount];
    for (unsigned int i = 0; i < pI->meshCount; ++i)
    {
        Mesh* mesh = pI->meshes + i;

        // Get vertex data
        mesh->vertexSizeInBytes = ReadShort(file);
        mesh->vertexCount = ReadShort(file);
        mesh->vertexData = new char[mesh->vertexSizeInBytes * mesh->vertexCount];
        mesh->vertexData = ReadBytes(file, mesh->vertexSizeInBytes * mesh->vertexCount);

        // Get indices
        mesh->indexCount = ReadShort(file);
        mesh->indices = new unsigned int[mesh->indexCount];
        for (unsigned int j = 0; j < mesh->indexCount; ++j)
            mesh->indices[j] = ReadShort(file);
    }


    ////////// ARMATURES //////////////////////////////////////////////////////

    // Read armature count
    pI->armatureCount = ReadShort(file);

    // Read armature name <-> index map
    pI->armatureNameIndexMap = new NameIndexMap[pI->armatureCount];
    for (unsigned int i = 0; i < pI->armatureCount; ++i)
    {
        pI->armatureNameIndexMap[i].name = ReadString(file);
        pI->armatureNameIndexMap[i].index = ReadShort(file);
    }

    // read armature joints
    pI->armatures = new Armature[pI->armatureCount];
    for (unsigned int i = 0; i < pI->armatureCount; ++i)
    {
        Armature* armature = pI->armatures + i;

        // Get the number of armatures
        armature->jointCount = ReadShort(file);

        // Get joints
        armature->joints = new Joint[armature->jointCount];
        for (unsigned int j = 0; j < armature->jointCount; ++j)
        {
            armature->joints[j].parentIndex = ReadShort(file);
            armature->joints[j].offset[0] = ReadFloat(file);
            armature->joints[j].offset[1] = ReadFloat(file);
            armature->joints[j].offset[2] = ReadFloat(file);
            armature->joints[j].offset[3] = 0;
            armature->joints[j].roll = ReadFloat(file);
        }
    }


    ////////// ACTIONS ////////////////////////////////////////////////////////

    // Read action count
    pI->actionCount = ReadShort(file);

    // Read action name <-> index map
    pI->actionNameIndexMap = new NameIndexMap[pI->actionCount];
    for (unsigned int i = 0; i < pI->actionCount; ++i)
    {
        pI->actionNameIndexMap[i].name = ReadString(file);
        pI->actionNameIndexMap[i].index = ReadShort(file);
    }

    // read actions
    pI->actions = new Action[pI->actionCount];
    for (unsigned int i = 0; i < pI->actionCount; ++i)
    {
        Action* action = pI->actions + i;
        uint16_t animationStreamSizeInBytes = ReadShort(file);
        //action->keyframeCount = ReadShort(file);
        action->animationData = ReadBytes(file, animationStreamSizeInBytes);
    }


    const char* string = ReadString(file);

    ////////// MATERIALS //////////////////////////////////////////////////////

    // Read material count
    pI->materialCount = ReadShort(file);

    // Read material name <-> index map
    pI->materialNameIndexMap = new NameIndexMap[pI->materialCount];
    for (unsigned int i = 0; i < pI->materialCount; ++i)
    {
        pI->materialNameIndexMap[i].name = ReadString(file);
        pI->materialNameIndexMap[i].index = ReadShort(file);
    }

    // read materials
    pI->materials = new Material[pI->materialCount];
    for (unsigned int i = 0; i < pI->materialCount; ++i)
    {
        Material* material = pI->materials + i;
        material->textureCount = ReadShort(file);
        material->textureIndices = new unsigned int[material->textureCount];
        for (unsigned int j = 0; j < material->textureCount; ++j)
            material->textureIndices[j] = ReadShort(file);
    }

    ////////// TEXTURES (IMAGES) //////////////////////////////////////////////

    // Read texture count
    pI->textureCount = ReadShort(file);

    // Read texture name <-> index map
    pI->textureNameIndexMap = new NameIndexMap[pI->textureCount];
    for (unsigned int i = 0; i < pI->textureCount; ++i)
    {
        pI->textureNameIndexMap[i].name = ReadString(file);
        pI->textureNameIndexMap[i].index = ReadShort(file);
    }

    // read texture file names
    pI->textures = new Texture[pI->textureCount];
    for (unsigned int i = 0; i < pI->textureCount; ++i)
    {
        Texture* texture = pI->textures + i;
        texture->filename = ReadString(file);
    }


    ////////// SCENES /////////////////////////////////////////////////////////

    // Read scene count
    pI->sceneCount = ReadShort(file);

    // Read scene name <-> index map
    pI->sceneNameIndexMap = new NameIndexMap[pI->sceneCount];
    for (unsigned int i = 0; i < pI->sceneCount; ++i)
    {
        pI->sceneNameIndexMap[i].name = ReadString(file);
        pI->sceneNameIndexMap[i].index = ReadShort(file);
    }

    pI->scenes = new Scene[pI->sceneCount];
    for (unsigned int i = 0; i < pI->sceneCount; ++i)
    {
        Scene* scene = pI->scenes + i;

        
        ////////// OBJECTS ////////////////////////////////////////////////////
        
        scene->objectCount = ReadShort(file);

        // Read scene name <-> index map
        scene->objectNameIndexMap = new NameIndexMap[scene->objectCount];
        for (unsigned int j = 0; j < scene->objectCount; ++j)
        {
            scene->objectNameIndexMap[j].name = ReadString(file);
            scene->objectNameIndexMap[j].index = ReadShort(file);
        }

        scene->objects = new Object[scene->objectCount];
        for (unsigned int j = 0; j < scene->objectCount; ++j)
        {
            Object* object = scene->objects + j;

            object->position[0] = ReadFloat(file);
            object->position[1] = ReadFloat(file);
            object->position[2] = ReadFloat(file);
            object->position[3] = 0;

            object->rotation[0] = ReadFloat(file);
            object->rotation[1] = ReadFloat(file);
            object->rotation[2] = ReadFloat(file);
            object->rotation[3] = ReadFloat(file);

            object->scale[0] = ReadFloat(file);
            object->scale[1] = ReadFloat(file);
            object->scale[2] = ReadFloat(file);
            object->scale[3] = 0;

            object->meshIndex = ReadShort(file);
            object->materialIndex = ReadShort(file);
        }

        ////////// LIGHTS /////////////////////////////////////////////////////

        scene->lightCount = ReadShort(file);

        // Read scene name <-> index map
        scene->lightNameIndexMap = new NameIndexMap[scene->lightCount];
        for (unsigned int j = 0; j < scene->lightCount; ++j)
        {
            scene->lightNameIndexMap[j].name = ReadString(file);
            scene->lightNameIndexMap[j].index = ReadShort(file);
        }

        scene->lights = new Light[scene->lightCount];
        for (unsigned int j = 0; j < scene->lightCount; ++j)
        {
            Light* light = scene->lights + j;

            light->position[0] = ReadFloat(file);
            light->position[1] = ReadFloat(file);
            light->position[2] = ReadFloat(file);
            light->position[3] = 0;

            light->rotation[0] = ReadFloat(file);
            light->rotation[1] = ReadFloat(file);
            light->rotation[2] = ReadFloat(file);
            light->rotation[3] = ReadFloat(file);

            light->angleInnerCone = ReadFloat(file);
            light->angleOuterCone = ReadFloat(file);
            light->clipPlaneNear = ReadFloat(file);
            light->clipPlaneFar = ReadFloat(file);

            light->color[0] = ReadFloat(file);
            light->color[1] = ReadFloat(file);
            light->color[2] = ReadFloat(file);
        }


        ////////// CAMERA /////////////////////////////////////////////////////

        scene->cameraCount = ReadShort(file);

        // Read scene name <-> index map
        scene->cameraNameIndexMap = new NameIndexMap[scene->cameraCount];
        for (unsigned int j = 0; j < scene->cameraCount; ++j)
        {
            scene->cameraNameIndexMap[j].name = ReadString(file);
            scene->cameraNameIndexMap[j].index = ReadShort(file);
        }

        scene->cameras = new Camera[scene->cameraCount];
        for (unsigned int j = 0; j < scene->cameraCount; ++j)
        {
            Camera* camera = scene->cameras + j;

            camera->position[0] = ReadFloat(file);
            camera->position[1] = ReadFloat(file);
            camera->position[2] = ReadFloat(file);
            camera->position[3] = 0;

            camera->rotation[0] = ReadFloat(file);
            camera->rotation[1] = ReadFloat(file);
            camera->rotation[2] = ReadFloat(file);
            camera->rotation[3] = ReadFloat(file);

            camera->horizontalFov = ReadFloat(file);
            camera->clipPlaneNear = ReadFloat(file);
            camera->clipPlaneFar = ReadFloat(file);
            camera->aspectRatio = ReadFloat(file);
        }
    }

    // test that we have read the entire file
    
    long pos = ftell(file);
    fseek (file, 0, SEEK_END);
    mini3d_assert (pos == ftell(file), "Entire file was not parsed. This idicates a parsing error!");

    fclose(file);

    return pI;
}
