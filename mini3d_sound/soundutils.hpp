
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



// TODO: Fix!
///////// SOUND PRIORITY POOL /////////////////////////////////////////////////

/*

// Pool of SoundSources for controlling priority (only the most prioritized ones will play).
class SourcePriorityPool : public SoundSourceBase
{

    static const uint PRIORITY_NOT_IN_USE = UINT_MAX;

    struct PrioritizedSoundSource { SoundSource* pSound; uint prio; };

    static int CompareSources (const void * a, const void * b)
    {
        PrioritizedSoundSource* pA = *(PrioritizedSoundSource**)a;
        PrioritizedSoundSource* pB = *(PrioritizedSoundSource**)b;

        if (pA == 0) return (pB == 0) ? 0 : -1;
        if (pB == 0) return 1;
        return (int)pA->prio - (int)pB->prio;
    }

    SourcePriorityPool(uint sourceCount, uint liveCount, uint channelCount) : 
        m_sourceCount(sourceCount), 
        m_liveCount(liveCount), 
        m_usedCount(0)
    { 
        m_pSources = new PrioritizedSoundSource[sourceCount]; 
        m_pSourcesCopy = new PrioritizedSoundSource[sourceCount]; 
        
        for (uint i = 0; i < m_sourceCount; ++i)
            m_pSources[i].pSound = new SoundSource();

        for (uint i = 0; i < m_sourceCount; ++i)
            m_pSourcesCopy[i].pSound = new SoundSource();
    }

    ~SourcePriorityPool() 
    { 
        for (uint i = 0; i < m_sourceCount; ++i)
            m_pSources[i].pSound->Release();

        for (uint i = 0; i < m_sourceCount; ++i)
            m_pSourcesCopy[i].pSound->Release();

        delete[] m_pSources;
        delete[] m_pSourcesCopy;
    };

    void MixIntoPeriod(float* pPeriod, uint periodIndex, BufferDesc periodDesc)
    {
        Lock guard(&m_mutex);

        // Find sounds that have stopped playing and mark them as available
        for (uint i = 0; i < m_sourceCount; ++i)
        {
            if (m_pSources[i].pSound->GetState() == SoundSource::STATE_STOPPED)
            {
                PrioritizedSoundSource tmp = m_pSources[i];
                tmp.prio = PRIORITY_NOT_IN_USE;
                m_pSources[i] = m_pSources[--m_usedCount];
                m_pSources[m_usedCount + 1] = tmp;
            }
        }

        // Sort the playback list
        qsort(m_pSources, m_sourceCount, sizeof(PrioritizedSoundSource*), &CompareSources);

        // Set the volume of non prioritized sounds to 0
        for (uint i = 0; i < m_sourceCount; ++i)
            m_pSources[i].pSound->SetVolume(i <= m_liveCount ? 1.0f : 0.0f);
    }

    void PlaySound(SoundBuffer* pBuffer, float* channelMix, uint channelCount, float pitch, uint priority)
    {
        Lock guard(&m_mutex);

        PrioritizedSoundSource* pSource = &m_pSources[m_usedCount++];
        pSource->prio = priority;
        pSource->pSound->SetSoundBuffer(pBuffer);
        pSource->pSound->SetChannelMix(channelMix, channelCount);
        pSource->pSound->SetPitch(pitch);
        pSource->pSound->Play();
    }

    PrioritizedSoundSource** GetCurrentlyPlayingSounds(uint &count)
    {
        Lock guard(&m_mutex);

        for (uint i = 0; i < m_sourceCount; ++i)
            m_pSourcesCopy[i] =  m_pSources[i];
    }

private:
    Mutex m_mutex;

    PrioritizedSoundSource* m_pSources;
    PrioritizedSoundSource* m_pSourcesCopy;
    uint m_usedCount;
    uint m_sourceCount;
    uint m_liveCount;
};

*/

}
}

#endif