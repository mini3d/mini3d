
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license>

// TODO: Sound priorities (only a subset of the total sounds are audible at any time)

#ifndef MINI3D_SOUND_UTILS_H
#define MINI3D_SOUND_UTILS_H

#include <cmath>
#include <climits>
#include <cstdlib>
#include <cfloat>
#include "sound.hpp"

typedef unsigned int uint;

namespace mini3d {
namespace sound {

///////// REVERB FILTER ///////////////////////////////////////////////////////

class ReverbFilter : public SoundFilter
{
public:
    ReverbFilter(SoundOutput* pOutput);
    ~ReverbFilter() { delete m_pReverbBuffer; };

    void ApplyFilter();

private:
    float* m_pReverbBuffer;
    uint m_location_f;
};


///////// LOW PASS FILTER /////////////////////////////////////////////////////

class LowPassFilter : public SoundFilter
{
public:
    LowPassFilter(SoundOutput* pOutput);
    ~LowPassFilter() { delete[] sum; delete[] prev; };

    void ApplyFilter();

private:
    float* sum;
    float* prev;
};


///////// HIGH PASS FILTER ////////////////////////////////////////////////////

class HighPassFilter : public SoundFilter
{
public:
    HighPassFilter(SoundOutput* pOutput);
    ~HighPassFilter() { delete[] prev[0]; delete[] prev[1]; };

    void ApplyFilter();

private:
    float* prev[2];
};

}
}

#endif
