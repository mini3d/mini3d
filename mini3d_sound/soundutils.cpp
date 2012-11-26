
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license>

// TODO: Sound priorities (only a subset of the total sounds are audible at any time)

#include "soundutils.hpp"

#include <cmath>
#include <climits>
#include <cstdlib>
#include <cfloat>


using namespace mini3d::sound;

///////// REVERB FILTER ///////////////////////////////////////////////////////

ReverbFilter::ReverbFilter(SoundOutput* pOutput) :  SoundFilter(pOutput), m_location_f(0) 
{
    m_pReverbBuffer = new float[4096 * 32];

    for (uint i = 0; i < 4096 * 32; ++i)
        m_pReverbBuffer[i] = 0.0f;
}

void ReverbFilter::ApplyFilter()
{
    BufferDesc desc = m_pOutput->GetDescription();

    uint MODULO = (4096 * 32) - 1;
    uint size_s = desc.lengthInFrames * desc.channelCount;
    for (uint i = 0; i < size_s; i += desc.channelCount)
    {
        for (uint j = 0; j < desc.channelCount; ++j)
        {
            m_pReverbBuffer[m_location_f] = atanf(1.0f * m_pBuffer[i + j] +
                                            0.10f * m_pReverbBuffer[(m_location_f + MODULO + 1 - 1300 * desc.channelCount) & MODULO] +
                                            0.30f * m_pReverbBuffer[(m_location_f + MODULO + 1 - 4096 * desc.channelCount) & MODULO] +
                                            0.50f * m_pReverbBuffer[(m_location_f + MODULO + 1 - 22050 * desc.channelCount) & MODULO]
                                            ) * 1.95f / 3.141592653f;
            
            m_pBuffer[i + j] = m_pReverbBuffer[m_location_f];
            m_location_f = (m_location_f + 1) & MODULO;
        }
    }
}


///////// LOW PASS FILTER /////////////////////////////////////////////////////

LowPassFilter::LowPassFilter(SoundOutput* pOutput) :  SoundFilter(pOutput) 
{
    uint channels = pOutput->GetDescription().channelCount;
    sum = new float[channels];
    prev = new float[channels];

    for (uint i = 0; i < channels; ++i)
    {
        sum[i] = 0;
        prev[i] = 0;
    }
}

void LowPassFilter::ApplyFilter()
{
    BufferDesc desc = m_pOutput->GetDescription();
    uint size_s = desc.lengthInFrames * desc.channelCount;
    for (uint i = 0; i < size_s; i += desc.channelCount)
    {
        for (uint j = 0; j < desc.channelCount; ++j)
        {
            sum[j] = 0.90f * sum[j] + (1 - 0.90f) * prev[j];
            prev[j]  = m_pBuffer[i + j];
            m_pBuffer[i + j] = sum[j] * 1.2f;
        }
    }
}


///////// HIGH PASS FILTER ////////////////////////////////////////////////////

HighPassFilter::HighPassFilter(SoundOutput* pOutput) :  SoundFilter(pOutput) 
{
    uint channels = pOutput->GetDescription().channelCount;
    prev[0] = new float[channels];
    prev[1] = new float[channels];

    for (uint i = 0; i < channels; ++i)
    {
        prev[0][i] = 0;
        prev[1][i] = 0;
    }
}

void HighPassFilter::ApplyFilter()
{
    BufferDesc desc = m_pOutput->GetDescription();
    uint size_s = desc.lengthInFrames * desc.channelCount;
    for (uint i = 0; i < size_s; i += desc.channelCount)
    {
        for (uint j = 0; j < desc.channelCount; ++j)
        {
            float highPass = 0.7f * m_pBuffer[i + j] -  0.7f * prev[1][j] + 0.3f * prev[0][j];

            prev[1][j] = prev[0][j];
            prev[0][j] = m_pBuffer[i + j];

            m_pBuffer[i + j] = highPass * 1.5f;
        }
    }
}

