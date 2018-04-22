// Copyright (c) <2018> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#define MINI3D_TEST_PHYSICS_VEC3
#ifdef MINI3D_TEST_PHYSICS_VEC3

#include <vector>

#include "../../mini3d_math/vec3.hpp"

using namespace mini3d::math;
using namespace std;

bool testAddition() {
    return Vec3(1,1,1) + Vec3(1,1,1) == Vec3(2,2,2);
};

vector<pair<const char*, bool(*)()>> math_uvec3 = {
    {"Addition", &testAddition} };

#endif
