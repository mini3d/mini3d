


// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license>

#ifdef __APPLE__

#include "soundservice_osx_ios_core_audio.hpp"
#include <unistd.h>

void mini3d_assert(bool expression, const char* text, ...);

using namespace mini3d::sound; 

const unsigned int OSX_IOS_CORE_AUDIO_BUFFER_SIZE_IN_FRAMES = 2048;

void AQBufferCallback(void* pVoid, AudioQueueRef inAQ, AudioQueueBufferRef inCompleteAQBuffer)
{
	SoundService_osx_ios_core_audio* pSoundService = (SoundService_osx_ios_core_audio*)pVoid;
	Lock guard(&pSoundService->m_mutex);
    ++pSoundService->m_freeBuffers;
}

SoundService_osx_ios_core_audio::SoundService_osx_ios_core_audio(unsigned int channels, unsigned int sampleRate) : 
	m_freeBuffers(OSX_IOS_CORE_AUDIO_BUFFER_COUNT),
    m_nextBufferIndex(0)
{

    m_desc.channelCount = channels;
    m_desc.sampleRate = sampleRate;
    m_desc.lengthInFrames = OSX_IOS_CORE_AUDIO_BUFFER_SIZE_IN_FRAMES;

    unsigned int periodSize_s = m_desc.lengthInFrames * m_desc.channelCount;

	AudioStreamBasicDescription audioStreamDesc;
	
	audioStreamDesc.mSampleRate = m_desc.sampleRate;
	audioStreamDesc.mFormatID = kAudioFormatLinearPCM;
	audioStreamDesc.mFormatFlags = kAudioFormatFlagIsSignedInteger; //kAudioFormatFlagsAreAllClear;
	audioStreamDesc.mBytesPerPacket = m_desc.channelCount * 2;
	audioStreamDesc.mFramesPerPacket = 1;
	audioStreamDesc.mBytesPerFrame = audioStreamDesc.mBytesPerPacket;
	audioStreamDesc.mChannelsPerFrame = m_desc.channelCount;
	audioStreamDesc.mBitsPerChannel = 16;
	audioStreamDesc.mReserved = 0;

    // Setup
    OSStatus status = AudioQueueNewOutput(&audioStreamDesc, &AQBufferCallback, this, 0, 0, 0, &m_pQueue);
    mini3d_assert(status == 0, "Could not initialize CoreAudio Output Queue!");

    for (unsigned int i = 0; i < OSX_IOS_CORE_AUDIO_BUFFER_COUNT; ++i)
    {
        OSStatus status = AudioQueueAllocateBuffer(m_pQueue, periodSize_s * 2, &m_pBuffers[i]);
        mini3d_assert(status == 0, "Could not allocate CoreAudio Queue Buffer!");
		m_pBuffers[i]->mAudioDataByteSize = periodSize_s * 2;
    }
	
	status = AudioQueueStart(m_pQueue, 0);
    mini3d_assert(status == 0, "Could not Start CoreAudio Queue!");
}

SoundService_osx_ios_core_audio::~SoundService_osx_ios_core_audio()
{
	OSStatus status = AudioQueueStop(m_pQueue, 0);
    mini3d_assert(status == 0, "Could not Start CoreAudio Queue!");

    status = AudioQueueDispose(m_pQueue, true);
    mini3d_assert(status == 0, "Could not dispose CoreAudio Output Queue!");

    for (unsigned int i = 0; i < OSX_IOS_CORE_AUDIO_BUFFER_COUNT; ++i)
    {
        AudioQueueFreeBuffer(m_pQueue, m_pBuffers[i]);
        mini3d_assert(status == 0, "Could not free CoreAudio Queue Buffer!");
    }
}

BufferDesc SoundService_osx_ios_core_audio::GetDescription() const { return m_desc; }

unsigned int SoundService_osx_ios_core_audio::GetFreeBufferCount()
{
    Lock guard(&m_mutex);
    return m_freeBuffers;
}

short* SoundService_osx_ios_core_audio::GetNextPeriodBuffer()
{
	while(GetFreeBufferCount() == 0)
		usleep(5000);

    return (short*)(m_pBuffers[m_nextBufferIndex]->mAudioData);
}

void SoundService_osx_ios_core_audio::AddPeriodBufferToQueue(short* pBuffer)
{
    OSStatus status = AudioQueueEnqueueBuffer(m_pQueue, m_pBuffers[m_nextBufferIndex], 0, 0);
    mini3d_assert(status == 0, "Could not enqueue Core Audio buffer!");

    m_nextBufferIndex = (m_nextBufferIndex + 1) % OSX_IOS_CORE_AUDIO_BUFFER_COUNT;

    Lock guard(&m_mutex);
    --m_freeBuffers;
}


#endif // __APPLE__
