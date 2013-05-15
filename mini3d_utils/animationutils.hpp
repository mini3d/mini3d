
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_UTILS_ANIMATIONUTILS_H
#define MINI3D_UTILS_ANIMATIONUTILS_H

#include "../mini3d/animation.hpp"
#include "../mini3d/import.hpp"
#include "../mini3d/math.hpp"

using namespace mini3d::import;
using namespace mini3d::animation;
using namespace mini3d::math;

namespace mini3d {
namespace utils {

struct AnimationUtils
{
    static Animation* AnimationFromAction(Action* action, Transform* target);
    static Animation* BoneAnimationFromAction(Action* action, Armature* armature, Transform targets[4]);
    static void BoneTransformsToMatrices(float boneMatrices[4][16], Transform transforms[4], Armature* armature);
};

}
}

#endif