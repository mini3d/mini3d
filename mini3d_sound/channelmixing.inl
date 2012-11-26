
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license>

#ifndef MINI3D_SOUND_CHANNEL_MIXING_INL
#define MINI3D_SOUND_CHANNEL_MIXING_INL

#include <cmath>
#include "sound.hpp"

typedef unsigned int uint;

namespace mini3d {
namespace sound {

///////// SURROUND CHANNEL MIXING /////////////////////////////////////////////

// Speaker placement in degrees (counging clockwise with 0 degrees being straight in front)
// http://www.thx.com/consumer/home-entertainment/home-theater/surround-sound-speaker-set-up/

const float MINIMUM_DISTANCE = 0.5f;
const float ATTENUATION_PER_DISTANCE_UNIT = 0.1f;
const float SPEED_OF_SOUND_IN_DISTANCE_UNITS_PER_SECOND = 340.29f;

const float PI = 3.1415926535f;
float SIN(float deg) { return sin(deg * PI / 180.0f); }
float COS(float deg) { return cos(deg * PI / 180.0f); }

// Ears are angled 15 degrees forward
const float EAR_LEFT[2] =               { SIN( -75), COS( -75) };
const float EAR_RIGHT[2] =              { SIN(  75), COS(  75) }; 

const float SPEAKER_FRONT_LEFT[2] =     { SIN( -45), COS( -45) };
const float SPEAKER_FRONT_RIGHT[2] =    { SIN(  45), COS(  45) };
const float SPEAKER_FRONT_CENTER[2] =   { SIN(   0), COS(   0) };
const float SPEAKER_REAR_LEFT[2] =      { SIN(-120), COS(-120) };
const float SPEAKER_REAR_RIGHT[2] =     { SIN( 120), COS( 120) };
const float SPEAKER_SIDE_LEFT[2] =      { SIN( -90), COS( -90) };
const float SPEAKER_SIDE_RIGHT[2] =     { SIN(  90), COS(  90) };

float dot2(const float a[2], const float b[2]) { return a[0] * b[0] + a[1] * b[1]; };
float dot3(const float a[2], const float b[2]) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; };

void CalculateChannelMix(const float pos[3], const float vel[3], float* channelMix, float &pitch, uint inputChannelCount, uint outputChannelCount)
{
    if (inputChannelCount == MONO)
    {
        // Direction and distance to source
        float dist_2 = pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2];
        float dist = sqrt(dist_2);

        float dir[3] = {pos[0] / dist, pos[1] / dist, pos[2] / dist };

        if (dist == 0)
            { dir[0] = dir[1] = dir[2] = 0; }

        if (dist < MINIMUM_DISTANCE)
            dist = MINIMUM_DISTANCE;

        float att = 1.0f + (dist - MINIMUM_DISTANCE) * ATTENUATION_PER_DISTANCE_UNIT;

        // Pitch based on velocity in direction of listener
        float speed = dot3(vel, dir);
        pitch = 1.0f - speed / SPEED_OF_SOUND_IN_DISTANCE_UNITS_PER_SECOND;

        switch(outputChannelCount)
        {
        case MONO:
            *channelMix = att;
            break;

        case STEREO:
            channelMix[0] = (PI - acos(dot2(EAR_LEFT,       dir))) / PI / att;
            channelMix[1] = (PI - acos(dot2(EAR_RIGHT,      dir))) / PI / att;
            break;
        
        case SURROUND_5_1:
            channelMix[0] = (PI - dot2(SPEAKER_FRONT_LEFT,   dir)) / PI / att;
            channelMix[1] = (PI - dot2(SPEAKER_FRONT_RIGHT,  dir)) / PI / att;
            channelMix[2] = (PI - dot2(SPEAKER_FRONT_CENTER, dir)) / PI;
            channelMix[3] = (PI - dot2(SPEAKER_REAR_LEFT,    dir)) / PI;
            channelMix[4] = (PI - dot2(SPEAKER_REAR_RIGHT,   dir)) / PI;
            channelMix[5] = 1.0f; // Sub
            break;

        case SURROUND_7_1:
            channelMix[0] = (PI - dot2(SPEAKER_FRONT_LEFT,   dir)) / PI;
            channelMix[1] = (PI - dot2(SPEAKER_FRONT_RIGHT,  dir)) / PI;
            channelMix[2] = (PI - dot2(SPEAKER_FRONT_CENTER, dir)) / PI;
            channelMix[3] = (PI - dot2(SPEAKER_REAR_LEFT,    dir)) / PI;
            channelMix[4] = (PI - dot2(SPEAKER_REAR_RIGHT,   dir)) / PI;
            channelMix[5] = 1.0f; // Sub
            channelMix[6] = (PI - dot2(SPEAKER_SIDE_LEFT,    dir)) / PI;
            channelMix[7] = (PI - dot2(SPEAKER_SIDE_RIGHT,   dir)) / PI;
            break;
        
        default:
           // No sound
            for (uint i = 0; i < inputChannelCount; ++i)
                for (uint j = 0; j < outputChannelCount; ++i)
                    channelMix[i * outputChannelCount + j] = 0.0f;
            break;
        }
    }
    else if (inputChannelCount == STEREO)
    {
        pitch = 1.0f;

        switch(outputChannelCount)
        {
        case MONO:
            channelMix[0] = 1.0f;
            channelMix[1] = 1.0f;
            break;

        case STEREO:
            // Left channel input
            channelMix[0] = 1.0f;
            channelMix[1] = 0.0f;

            // Right channel input
            channelMix[2] = 0.0f;
            channelMix[3] = 1.0f;
            break;
        
        case SURROUND_5_1:
            // Left channel input
            channelMix[0] = 1.0f;
            channelMix[1] = 0.0f;
            channelMix[2] = 0.1f; // Center
            channelMix[3] = 0.3f;
            channelMix[4] = 0.0f;
            channelMix[5] = 0.5f; // Sub

            // Right channel input
            channelMix[6] = 0.0f;
            channelMix[7] = 1.0f;
            channelMix[8] = 0.1f; // Center
            channelMix[9] = 0.0f;
            channelMix[10] = 0.3f;
            channelMix[11] = 1.0f; // SUB
            break;

        case SURROUND_7_1:
            // Left channel input
            channelMix[0] = 1.0f;
            channelMix[1] = 0.0f;
            channelMix[2] = 0.1f; // Center
            channelMix[3] = 0.3f;
            channelMix[4] = 0.0f;
            channelMix[5] = 1.0f; // Sub
            channelMix[6] = 0.3f;
            channelMix[7] = 0.0f;

            // Right channel input
            channelMix[8] = 0.0f;
            channelMix[9] = 1.0f;
            channelMix[10] = 0.1f; // Center
            channelMix[11] = 0.0f;
            channelMix[12] = 0.3f;
            channelMix[13] = 1.0f; // SUB
            channelMix[14] = 0.0f;
            channelMix[15] = 0.3f;
            break;

        default:
           // No sound
            for (uint i = 0; i < inputChannelCount; ++i)
                for (uint j = 0; j < outputChannelCount; ++i)
                    channelMix[i * outputChannelCount + j] = 0.0f;
            break;
        }
    }
    else
    {
        // No sound
        for (uint i = 0; i < inputChannelCount; ++i)
            for (uint j = 0; j < outputChannelCount; ++i)
                channelMix[i * outputChannelCount + j] = 0.0f;
    }
}

}
}

#endif