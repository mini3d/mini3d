
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license>

#if defined(__linux) && !defined(ANDROID) && !defined(__APPLE__)

#ifndef MINI3D_SOUND_LINUX_ALSA_H
#define MINI3D_SOUND_LINUX_ALSA_H

#include "../isoundservice.hpp"
#include <alsa/asoundlib.h>

namespace mini3d {
namespace sound { 

const unsigned int SOUND_LINUX_ALSA_BUFFER_COUNT = 3;

///////// SOUND SERVICE ///////////////////////////////////////////////////////

typedef
class SoundService_linux_alsa : public ISoundService
{
public:
    SoundService_linux_alsa(unsigned int channels, unsigned int sampleRate);
    ~SoundService_linux_alsa();

    short* GetNextPeriodBuffer();
    void AddPeriodBufferToQueue(short* pBuffer);
    BufferDesc GetDescription() const;

private:
	snd_pcm_sframes_t GetAvailableBufferSpaceInSamples();

    BufferDesc m_desc;
    unsigned int m_nextBufferIndex;
    short* m_pWaveBuffer[SOUND_LINUX_ALSA_BUFFER_COUNT];

	snd_pcm_t *m_pPcmHandle;

} SoundService;

}
}

#endif // MINI3D_SOUND_LINUX_ALSA_H
#endif // defined(__linux) && !defined(ANDROID) && !defined(__APPLE__)