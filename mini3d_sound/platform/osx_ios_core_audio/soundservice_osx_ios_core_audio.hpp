
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license>

#ifdef __APPLE__

#ifndef MINI3D_SOUND_OSX_IOS_CORE_AUDIO_H
#define MINI3D_SOUND_OSX_IOS_CORE_AUDIO_H

#include "../isoundservice.hpp"
#include <AudioToolbox/AudioQueue.h>
#include <thread>

namespace mini3d {
namespace sound { 

const unsigned int OSX_IOS_CORE_AUDIO_BUFFER_COUNT = 3;

///////// SOUND SERVICE ///////////////////////////////////////////////////////

typedef
class SoundService_osx_ios_core_audio : public ISoundService
{
public:
    SoundService_osx_ios_core_audio(unsigned int channels, unsigned int sampleRate);
    ~SoundService_osx_ios_core_audio();

    short* GetNextPeriodBuffer();
    void AddPeriodBufferToQueue(short* pBuffer);
    BufferDesc GetDescription() const;


public: // These are manipulated from the AQBufferCallback function
	unsigned int m_freeBuffers;
    std::mutex m_mutex;

private:
	unsigned int GetFreeBufferCount();

    BufferDesc m_desc;
    unsigned int m_nextBufferIndex;

    AudioQueueRef m_pQueue;
    AudioQueueBufferRef m_pBuffers[OSX_IOS_CORE_AUDIO_BUFFER_COUNT];

} SoundService;

}
}

#endif // MINI3D_SOUND_OSX_IOS_CORE_AUDIO_H
#endif // __APPLE__
