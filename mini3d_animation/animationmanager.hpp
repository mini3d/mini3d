
// Copyright (c) <2009-2013> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_MINI3DANIMATIONMANAGER_H
#define MINI3D_MINI3DANIMATIONMANAGER_H

#include "animation.hpp"

// TODO: Should be a dynamic array in animation manager for holding animations
const unsigned int ANIMATION_MANAGER_MAX_ANIMATIONS_COUNT = 256;

namespace mini3d {
namespace animation {

////////// ANIMATION MANAGER ///////////////////////////////////////////////////

class AnimationManager
{
public:

    AnimationManager() : m_animationCount(0) {}

    void AddAnimation(Animation* pAnim)    { m_animations[m_animationCount++] = pAnim; }
    void RemoveAnimation(Animation* pAnim) { for (unsigned int i = 0; i < m_animationCount; ++i) if (m_animations[i] == pAnim) m_animations[i] = m_animations[--m_animationCount]; }
    void Update(float timeStep)            { for (unsigned int i = 0; i < m_animationCount; ++i) m_animations[i]->Update(timeStep, 1.0f); }

private:
    Animation* m_animations[ANIMATION_MANAGER_MAX_ANIMATIONS_COUNT];
    unsigned int m_animationCount;

};


}
}

#endif
