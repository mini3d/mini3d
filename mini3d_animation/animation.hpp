
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_ANIMATION_ANIMATION_H
#define MINI3D_ANIMATION_ANIMATION_H

#include "internal/animationstream.h"
#include <stdint.h>
#include <limits.h>

// This animation system is very similar to the one described here:
// http://bitsquid.blogspot.se/2009/11/bitsquid-low-level-animation-system.html
// http://bitsquid.blogspot.se/2011/10/low-level-animation-part-2.html
//
// The main difference is that the vector data is not compressed and
// have more track types (location, rotation and scale)
//
// ANIMATION STREAM LAYOUT
//
// HEADER:
// 16 bit animation length in frames
// 16 bit joint count
//
// KEY FRAME:
// 11 bit joint index
//  5 bit track index
// 16 bit frame index (30 fps)
// 32 bit float data x
// 32 bit float data y
// 32 bit float data z
// 32 bit float data w
//
// END TRACK KEY FRAME:
// 11 bit joint index
//  5 bit track index
// 16 bit "infintiy" frame index marker (0xFFFF)
//
// END STREAM KEY FRAME:
// 11 bit "empty" joint index (0x0000)
//  5 bit "empty" track index (0x0000)
// 16 bit "empty" frame index marker (0x0000)
//


namespace mini3d {
namespace animation {

const unsigned int FRAMES_PER_SECOND = 30;
const unsigned int MICROSECONDS_PER_SECOND = 1000000;

const int TRACK_COUNT = 3; // Default is: Location, Rotation, Scale in that order!

const unsigned int LOCATION_TRACK_INDEX = 0;
const unsigned int ROTATION_TRACK_INDEX = 1;
const unsigned int SCALE_TRACK_INDEX = 2;

const unsigned int REPEATE_INDEFINITLY = UINT_MAX;

const float IDENTITY_SCALE[4] = {1,1,1,1};


////////// ANIMATION INTERFACE ////////////////////////////////////////////////

struct IAnimation { virtual void Update(uint64_t time, float (*state)[TRACK_COUNT][4], float weight = 1.0f) = 0; };


////////// ANIMATION //////////////////////////////////////////////////////////

struct KeyFrame;

class Animation : public IAnimation
{
public:

    Animation(const char* pAnimationStream);
    ~Animation();

    uint64_t GetLength() const                      { return (m_length * MICROSECONDS_PER_SECOND) / FRAMES_PER_SECOND; } // TODO: Variable fps?
    uint64_t GetPosition() const                    { return m_time; }
    void SetPosition(uint64_t time)                 { if (time < m_time) Init(); m_time = time; }
    
    void Update(uint64_t deltaTime, float (*state)[TRACK_COUNT][4], float weight = 1.0f);
    static void SetJointMatrix(float m[12], const float loc[4], const float quat[4], const float scale[4]);
    static void ConcatenateJointMatrices(float m[12], const float parent[12]);

private:
    void Init();
    bool ReadKeyFrame(unsigned int timeStamp);
    bool FastForwardStream(unsigned int frameIndex);

    AnimationStream m_pStream;
    KeyFrame (*m_pKeyFrames)[TRACK_COUNT][4]; // We need 4 key frames per track to interpolate the Catmull-Rom splines

    uint64_t m_time;

    unsigned int m_repeatCount;
    unsigned int m_jointCount;
    unsigned int m_length;
};


////////// ANIMATION BLENDING /////////////////////////////////////////////////

struct BlendBase : public IAnimation
{
    BlendBase(int childCount) : 
		m_pChildren(new IAnimation*[childCount]),
		m_pWeights(new float[childCount]),
		m_childCount(childCount)
	{ for (int i=0; i<m_childCount; ++i) { m_pChildren[i] = 0; m_pWeights[i] = 0.0f; } }

	virtual ~BlendBase() { delete m_pChildren; delete m_pWeights; }

    float GetWeight(int index)                      { return m_pWeights[index]; }
    void SetWeight(int index, float weight)         { m_pWeights[index] = weight; }

    IAnimation* GetAnimation(int index)             { return m_pChildren[index]; }
    void SetAnimation(int index, IAnimation* pAnim) { m_pChildren[index] = pAnim; }

    virtual void Update(uint64_t deltaTime, float (*state)[TRACK_COUNT][4], float weight = 1.0f) = 0;

protected:
    IAnimation** m_pChildren;
    float* m_pWeights;
	int m_childCount;
};


////////// ADDITIVE BLENDING //////////////////////////////////////////////////

class AdditiveBlend : public BlendBase
{
	AdditiveBlend(int childCount) : BlendBase(childCount) {}
	
public:
    void Update(uint64_t deltaTime, float (*state)[TRACK_COUNT][4], float weight = 1.0f) 
    {
        for (int i=0; i<m_childCount; ++i) 
            if (m_pChildren[i])
                m_pChildren[i]->Update(deltaTime, state, m_pWeights[i]);
    }
};


////////// CROSS BLENDING /////////////////////////////////////////////////////

const float CROSS_BLEND_EPSILON = 0.001f;

class CrossBlend : public BlendBase
{ 
public:
    CrossBlend(int childCount, float exp = 0.5f) : BlendBase(childCount), m_exp(exp) { }

    float GetExp() const                { return m_transitionFactor; }
    void SetExp(float factor)           { m_transitionFactor = factor; }

    void Update(uint64_t deltaTime, float (*state)[TRACK_COUNT][4], float weight = 1.0f) 
    { 
        m_pWeights[m_targetIndex] = 1.0f;

        for (int i=0; i<m_childCount; ++i)
        {
            if (i != m_targetIndex && m_pChildren[i])
            {
                // TODO: Needs a more complicated formula for exponential decay based on delta time!
                m_pWeights[i] = (m_pWeights[i] > CROSS_BLEND_EPSILON) ? m_pWeights[i] * m_exp : 0.0f;
                m_pWeights[m_targetIndex] -= m_pWeights[i];
                m_pChildren[i]->Update(deltaTime, state, m_pWeights[i] * weight);
            }
        }

        // Update target
        if (m_pChildren[m_targetIndex]) 
            m_pChildren[m_targetIndex]->Update(deltaTime,state, m_pWeights[m_targetIndex] * weight);
    }

private:
    float m_exp;
	float m_transitionFactor;
    unsigned int m_targetIndex;
};

}
}

#endif // MINI3D_ANIMATION_ANIMATION_H

