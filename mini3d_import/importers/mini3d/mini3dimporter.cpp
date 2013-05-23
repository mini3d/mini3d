
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#include "mini3dimporter.hpp"
#include "../../assetlibrary.hpp"

#include <stdint.h>

using namespace mini3d::import;

template <typename T> void Read(FILE* file, T* t, unsigned int count = 1) { fread(t, sizeof(T), count, file); }
unsigned short ReadShort(FILE* file) { uint16_t t; Read<uint16_t>(file, &t); return t; }
unsigned int ReadInt32(FILE* file) { uint32_t t; Read<uint32_t>(file, &t); return t; }
float ReadFloat(FILE* file) { float t; Read<float>(file, &t); return t; }
char* ReadString(FILE* file) 
{ 
    unsigned int length = ReadShort(file);
    char* string = new char[length + 1];
    string[length] = 0;
    Read<char>(file, string, length);
    return string;
}

char* ReadBytes(FILE* file, unsigned int size)
{
    char* data = new char[size];
    Read<char>(file, data, size);
    return data;
}

AssetLibrary* Mini3dImporter::LoadSceneFromFile(const char* filename)
{
    AssetLibrary* pI = new AssetLibrary();
    
	FILE *file = fopen(filename, "rb");
    mini3d_assert(file != 0, "Failed to open file %s", filename);
	
    ////////// MESHES /////////////////////////////////////////////////////////

    // Read mesh count
    pI->meshes.count = ReadShort(file);

    // read meshes
    pI->meshes.array = new Mesh[pI->meshes.count];
    for (unsigned int i = 0; i < pI->meshes.count; ++i)
    {
        Mesh* mesh = pI->meshes.array + i;
        mesh->index = i;

        // Name
        mesh->name = ReadString(file);

        // Get vertex data
        mesh->vertexSizeInBytes = ReadShort(file);
        mesh->vertexData.count = ReadInt32(file);
        mesh->vertexData.array = ReadBytes(file, mesh->vertexData.count);
        
        // Get index data
        mesh->indexSizeInBytes = ReadShort(file);
        mesh->indexData.count = ReadInt32(file);
        mesh->indexData.array = ReadBytes(file, mesh->indexData.count);
    }


    ////////// ARMATURES //////////////////////////////////////////////////////

    // Read armature count
    pI->armatures.count = ReadShort(file);
    pI->armatures.array = new Armature[pI->armatures.count];

    // read armature joints
    for (unsigned int i = 0; i < pI->armatures.count; ++i)
    {
        Armature* armature = pI->armatures.array + i;
        armature->index = i;

        // Name
        armature->name = ReadString(file);

        // Get the number of joints
        armature->joints.count = ReadShort(file);
        armature->joints.array = new Joint[armature->joints.count];
        
        // Get joints
        for (unsigned int j = 0; j < armature->joints.count; ++j)
        {
            Joint* joint = armature->joints.array + j;
            joint->index = j;

            joint->name = ReadString(file);

            unsigned int parentIndex = ReadShort(file);
            joint->parent = (parentIndex != NO_BONE_PARENT) ? armature->joints.array + parentIndex : 0;

            joint->offset[0] = ReadFloat(file);
            joint->offset[1] = ReadFloat(file);
            joint->offset[2] = ReadFloat(file);
            
            joint->roll[0] =  ReadFloat(file);
            joint->roll[1] =  ReadFloat(file);
            joint->roll[2] =  ReadFloat(file);
            joint->roll[3] =  ReadFloat(file);
        }
    }


    ////////// ACTIONS ////////////////////////////////////////////////////////

    // Read action count
    pI->actions.count = ReadShort(file);
    pI->actions.array = new Action[pI->actions.count];

    // read actions
    for (unsigned int i = 0; i < pI->actions.count; ++i)
    {
        Action* action = pI->actions.array + i;
        action->index = i;

        action->name = ReadString(file);
        action->length = ReadFloat(file);
        
        
        action->channels.count = ReadShort(file);
        action->channels.array = new Channel[action->channels.count];
        
        // Read all channels
        for (unsigned int i = 0; i < action->channels.count; ++i)
        {
            Channel* channel = action->channels.array + i;

            channel->boneName = ReadString(file);
            channel->type = (Channel::Type)ReadShort(file);

            channel->animationData.count = ReadShort(file);
            channel->animationData.array = ReadBytes(file, channel->animationData.count);
        }
    }


    ////////// TEXTURES (IMAGES) //////////////////////////////////////////////

    // Read texture count
    pI->textures.count = ReadShort(file);
    pI->textures.array = new Texture[pI->textures.count];

    // read texture file names
    
    for (unsigned int i = 0; i < pI->textures.count; ++i)
    {
        Texture* texture = pI->textures.array + i;
        texture->index = i;

        texture->name = ReadString(file);
        texture->filename = ReadString(file);
    }


    ////////// MATERIALS //////////////////////////////////////////////////////

    // Read material count
    pI->materials.count = ReadShort(file);
    pI->materials.array = new Material[pI->materials.count];

    // read materials
    for (unsigned int i = 0; i < pI->materials.count; ++i)
    {
        Material* material = pI->materials.array + i;
        material->index = i;

        material->name = ReadString(file);

        material->textures.count = ReadShort(file);
        material->textures.array = new Texture*[material->textures.count];

        // Get texture links
        for (unsigned int j = 0; j < material->textures.count; ++j)
        {
            unsigned int index = ReadShort(file);
            material->textures.array[i] = pI->textures.array + index;
        }
    }


    ////////// SCENES /////////////////////////////////////////////////////////

    // Read scene count
    pI->scenes.count = ReadShort(file);
    pI->scenes.array = new Scene[pI->scenes.count];
    
    // Read scenes
    for (unsigned int i = 0; i < pI->scenes.count; ++i)
    {
        Scene* scene = pI->scenes.array + i;
        scene->index = i;

        scene->name = ReadString(file);

        
        ////////// OBJECTS ////////////////////////////////////////////////////
        
        scene->objects.count = ReadShort(file);
        scene->objects.array = new Object[scene->objects.count];

        for (unsigned int j = 0; j < scene->objects.count; ++j)
        {
            Object* object = scene->objects.array + j;
            object->index = j;

            object->name = ReadString(file);

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

            object->mesh = pI->meshes.array + ReadShort(file);
            object->material = pI->materials.array + ReadShort(file);
        }

        ////////// LIGHTS /////////////////////////////////////////////////////

        scene->lights.count = ReadShort(file);
        scene->lights.array = new Light[scene->lights.count];

        for (unsigned int j = 0; j < scene->lights.count; ++j)
        {
            Light* light = scene->lights.array + j;
            light->index = j;

            light->name = ReadString(file);

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

        scene->cameras.count = ReadShort(file);
        scene->cameras.array = new Camera[scene->cameras.count];

        for (unsigned int j = 0; j < scene->cameras.count; ++j)
        {
            Camera* camera = scene->cameras.array + j;
            camera->index = j;

            camera->name = ReadString(file);

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
    mini3d_assert (pos == ftell(file), "Entire file was not parsed. This indicates a parsing error!");

    fclose(file);

    return pI;
}
