
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license>

#ifdef _WIN32

#ifndef MINI3D_SOUND_WIN32_WAVEOUT_H
#define MINI3D_SOUND_WIN32_WAVEOUT_H

#include "../isoundservice.hpp"

namespace mini3d {
namespace sound { 

const unsigned int SOUND_WIN32_WAVEOUT_BUFFER_COUNT = 3;

///////// SOUND SERVICE ///////////////////////////////////////////////////////

typedef
class SoundService_win32_waveOut : public ISoundService
{
public:
    SoundService_win32_waveOut(unsigned int channels, unsigned int sampleRate);
    ~SoundService_win32_waveOut();

    short* GetNextPeriodBuffer();
    void AddPeriodBufferToQueue(short* pBuffer);
    BufferDesc GetDescription() const;

private:
    BufferDesc m_desc;
    unsigned int m_nextBufferIndex;

    float* m_pMixBuffer;
    short* m_pWaveBuffer[SOUND_WIN32_WAVEOUT_BUFFER_COUNT];
    void* m_phWaveOut;
    void* m_pWaveHeader[SOUND_WIN32_WAVEOUT_BUFFER_COUNT];

} SoundService;


}
}

#endif // MINI3D_PLAYBACKMANAGER_WIN32_WAVEOUT_H
#endif // WIN32