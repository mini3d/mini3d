
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

AssetLibrary* AssetLibrary::LoadFromFile(const char* filename)
{
    // find the file name ending
    const char* pos = strrchr(filename, '.');
    mini3d_assert(pos != 0, "Faled to identify file ending for file: %s", filename);

    // Convert file name ending to upper case
    AutoString ending(strdup(pos));
    for(unsigned int i = 0; i < ending.count; ++i) 
        ending.array[i] = toupper(ending.array[i]);

    // Find what parser to use
    if (ending == ".M3D")
    {
        Mini3dImporter* pMini3dImp = new Mini3dImporter();
	    AssetLibrary* pAssetLibrary = pMini3dImp->LoadSceneFromFile(filename);
        delete pMini3dImp;
        return pAssetLibrary;
    }

    mini3d_assert(false, "Failed to find a matching file parser for file: %s", filename);
    return 0;
}
