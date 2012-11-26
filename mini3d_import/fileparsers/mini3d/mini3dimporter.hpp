
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_MINI3DIMPORTER_H
#define MINI3D_MINI3DIMPORTER_H

#include <cstdio>
#include "../../assetimporter.hpp"

void mini3d_assert(bool expression, const char* text, ...);

namespace mini3d {
namespace import {

struct AssetImporter;

class Mini3dImporter
{
public:
    AssetImporter* LoadSceneFromFile(const char* filename);

};

}
}


#endif // MINI3D_MINI3DFILEPARSER_H