
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#include "animation.hpp"
#include <cmath>

// TODO: SSE Optimize - Like a Boss! :)

const unsigned int JOINT_INDEX_BITS = 12;
const unsigned int TRACK_INDEX_BITS = 4;
const unsigned int FRAME_INDEX_UNKNOWN = 0xffff;
const unsigned int JOINT_INDEX_UNKNOWN = 0xffff;

// Simple vector class to allow same interface for floating point as well as Intel SSE / Arm NEON
struct Vec4 { 
    float x,y,z,w; 

    Vec4 operator*(float s)    { Vec4 r = {x*s, y*s, z*s, w*s }; return r; }
    Vec4 operator*(Vec4 v)     { Vec4 r = {x*v.x, y*v.y, z*v.z, w*v.w }; return r; }
    Vec4 operator+(Vec4 v)     { Vec4 r = {x+v.x, y+v.y, z+v.z, w+v.w }; return r; }
};

// Keyframe struct
struct mini3d::animation::KeyFrame { 
    unsigned int frameIndex; 
    Vec4 vec; 
    KeyFrame() : frameIndex(FRAME_INDEX_UNKNOWN) { vec.x = vec.y = vec.z = vec.w = 0; }; 
};

using namespace mini3d::animation;

Animation::Animation(const char* pAnimationStream) : 
    m_pStream(pAnimationStream), 
    m_pKeyFrames(0),
    m_time(0)
{
    Init();
}

Animation::~Animation() 
{ 
    delete[] m_pKeyFrames; 
}

void Animation::Init()
{
    m_pStream.Reset();
    
    m_length = m_pStream.Read<uint16_t>();
    m_jointCount = m_pStream.Read<uint16_t>();
    unsigned int totalTrackCount = m_pStream.Read<uint16_t>();

    if (m_pKeyFrames == 0)
        m_pKeyFrames = new KeyFrame[m_jointCount][TRACK_COUNT][4];

    // Get the initial set of keyframes
    int lastJointInfo = -1;
    while(m_pStream.Peek<uint16_t>() > lastJointInfo)
    {
        lastJointInfo = m_pStream.Peek<uint16_t>();

        ReadKeyFrame(FRAME_INDEX_UNKNOWN);
        ReadKeyFrame(FRAME_INDEX_UNKNOWN);
        ReadKeyFrame(FRAME_INDEX_UNKNOWN);

        // Set the first node to zero (first node will not be if we reset the stream during playback)
        unsigned int jointIndex = lastJointInfo >> TRACK_INDEX_BITS;
        unsigned int trackIndex = (lastJointInfo & ((1 << TRACK_INDEX_BITS) - 1));
        m_pKeyFrames[jointIndex][trackIndex][0] = KeyFrame();
    }
}

void Animation::Update(uint64_t deltaTime, float (*state)[TRACK_COUNT][4], float weight)
{
    m_time += deltaTime;

    // Check if we have reached the end of the stream including all repetitions, if so, clamp m_time
    if (m_time > GetLength() * m_repeatCount)
        m_time = GetLength() * m_repeatCount;

    uint64_t time = m_time;

    // If we are going past the end of the animation (for one of the repetitions), start the animation stream over with Init()
    if (time / GetLength() != (time - deltaTime) / GetLength()) Init();

    // Get the time offset within the current repetition
    time %= GetLength();

    // Get the frame index corresponding to time at 30 fps
    uint16_t frameIndex = (time > 0) ? (uint16_t)((time * FRAMES_PER_SECOND) / MICROSECONDS_PER_SECOND) : 0;

    // read the animation stream until all joint channels contain the correct interval
    while(ReadKeyFrame(frameIndex));    

    // Get the interpolated value for each animation channel at time frameIndex
    for (unsigned int jointIndex = 0; jointIndex < m_jointCount; ++jointIndex)
    {
        for (unsigned int trackIndex = 0; trackIndex < TRACK_COUNT; ++trackIndex)
        {
            Vec4 vec;
            KeyFrame* keyFrames = m_pKeyFrames[jointIndex][trackIndex];
            
            // If we are at a frame before the first "proper" keyframe, just extend it with constant interpolation
            if (frameIndex < keyFrames[1].frameIndex)
                vec = keyFrames[0].vec;
            // if we are at a time after the last "proper" keyframe, just extend it with constant interpolation
            else if (frameIndex > keyFrames[2].frameIndex)
                vec = keyFrames[3].vec;
            // We are int the interval between points 1 and 2. Do a Catmull-Rom spline interpolation
            else 
            {
                // Catmull-Rom spline Interpolation
                float t = (float)(frameIndex - keyFrames[1].frameIndex) / (float)(keyFrames[2].frameIndex - keyFrames[1].frameIndex);
                float t2 = t*t;
                float t3 = t2*t;
            
                float a = -0.5f*t3 + t2 - 0.5f*t;
                float b =  1.5f*t3 - 2.5f*t2 + 1.0f;
                float c = -1.5f*t3 + 2.0f*t2 + 0.5f*t;
                float d =  0.5f*t3 - 0.5f*t2;

                // interpolate each component
                vec = keyFrames[1].vec * b + keyFrames[2].vec * c;

                // keyFrames[0].frameIndex == FRAME_INDEX_INFINITY if we are between the first two key frames of the track!
                if (keyFrames[0].frameIndex != FRAME_INDEX_UNKNOWN)
                    vec = vec + keyFrames[0].vec * a;

                // keyFrames[3].frameIndex == FRAME_INDEX_INFINITY if we are between the last two key frames of the track!
                if (keyFrames[3].frameIndex != FRAME_INDEX_UNKNOWN)
                    vec = vec + keyFrames[3].vec * d;
            }

            // Add the vec to the target using th given weight
            Vec4* target = (Vec4*)state[jointIndex][trackIndex];
            *target = *target + vec * weight;
        }
    }
}

bool Animation::ReadKeyFrame(unsigned int frameIndex)
{
    unsigned int jointInfo, jointIndex, trackIndex;

    jointInfo = m_pStream.Peek<uint16_t>();
    jointIndex = jointInfo >> TRACK_INDEX_BITS;
    trackIndex = (jointInfo & ((1 << TRACK_INDEX_BITS) - 1));
    KeyFrame* keyFrames = m_pKeyFrames[jointIndex][trackIndex];

    // If we are still in the interval between keyframe 1 and 2, just return
    if (frameIndex < keyFrames[2].frameIndex) return false;

    // Jump over the jointInfo that we peeked earlier
    m_pStream.FastForward<uint16_t>();

    // Advance the key frame table
    keyFrames[0] = keyFrames[1];
    keyFrames[1] = keyFrames[2];
    keyFrames[2] = keyFrames[3];

    // Store the time-stamp for the new keyframe
    keyFrames[3].frameIndex = m_pStream.Read<uint16_t>();

    // Stream End Keyframes dont have any data
    if (keyFrames[3].frameIndex == FRAME_INDEX_UNKNOWN) return true;

    // Read the vec
    keyFrames[3].vec = m_pStream.Read<Vec4>();

    return true;
}

