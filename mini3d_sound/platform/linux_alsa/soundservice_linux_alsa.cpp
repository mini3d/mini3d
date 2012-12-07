
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license>

#if defined(__linux) && !defined(ANDROID) && !defined(__APPLE__)

#include "soundservice_linux_alsa.hpp"
#include <unistd.h>

void mini3d_assert(bool expression, const char* text, ...);

using namespace mini3d::sound; 

const unsigned int SOUND_LINUX_ALSA_BUFFER_SIZE_IN_FRAMES = 2048;

SoundService_linux_alsa::SoundService_linux_alsa(unsigned int channels, unsigned int sampleRate) : 
    m_nextBufferIndex(0)
{

    m_desc.channelCount = channels;
    m_desc.sampleRate = sampleRate;
    m_desc.lengthInFrames = SOUND_LINUX_ALSA_BUFFER_SIZE_IN_FRAMES;

	int err;

	err = snd_pcm_open(&m_pPcmHandle, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	mini3d_assert(err >= 0, "Could not open Alsa PCM device: %s", snd_strerror(err));

	snd_pcm_hw_params_t* hwparams;
	snd_pcm_hw_params_alloca(&hwparams);

	err = snd_pcm_hw_params_any(m_pPcmHandle, hwparams);
	mini3d_assert(err >= 0, "Could not create configuration for Alsa PCM Device. %s", snd_strerror(err));
	 
	err = snd_pcm_hw_params_set_access(m_pPcmHandle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	mini3d_assert(err >= 0, "Could not set buffer format and access for Alsa PCM Device. %s", snd_strerror(err));
  
	err = snd_pcm_hw_params_set_format(m_pPcmHandle, hwparams, SND_PCM_FORMAT_S16_LE);
	mini3d_assert(err >= 0, "Could not set sample format for Alsa PCM Device. %s", snd_strerror(err));

	err = snd_pcm_hw_params_set_rate_near(m_pPcmHandle, hwparams, &m_desc.sampleRate, 0);
	mini3d_assert(err >= 0, "Could not set samle rate for Alsa PCM Device. %s", snd_strerror(err));

	err = snd_pcm_hw_params_set_channels(m_pPcmHandle, hwparams, m_desc.channelCount);
	mini3d_assert(err >= 0, "Could not set number of channels for Alsa PCM Device. %s", snd_strerror(err));

	err = snd_pcm_hw_params_set_buffer_size(m_pPcmHandle, hwparams, SOUND_LINUX_ALSA_BUFFER_SIZE_IN_FRAMES * SOUND_LINUX_ALSA_BUFFER_COUNT * m_desc.channelCount);
	mini3d_assert(err >= 0, "Could not set buffer size for Alsa PCM Device. %s", snd_strerror(err));

	err = snd_pcm_hw_params_set_period_size_integer(m_pPcmHandle, hwparams);
	mini3d_assert(err >= 0, "Could not set integer periods for Alsa PCM Device. %s", snd_strerror(err));

	snd_pcm_uframes_t bufferSize;
	err = snd_pcm_hw_params_get_buffer_size(hwparams, &bufferSize);
	mini3d_assert(err >= 0, "Could not get buffer size for Alsa PCM Device. %s", snd_strerror(err));

	// TODO: Is this needed?
	snd_pcm_uframes_t periodSize = SOUND_LINUX_ALSA_BUFFER_SIZE_IN_FRAMES;
	err = snd_pcm_hw_params_set_period_size_max(m_pPcmHandle, hwparams, &periodSize, 0);
	mini3d_assert(err >= 0, "Could not set max period size for Alsa PCM Device. %s", snd_strerror(err));
 
	err = snd_pcm_hw_params(m_pPcmHandle, hwparams);
	mini3d_assert(err >= 0, "Could not set configuration parameters for Alsa PCM Device. %s", snd_strerror(err));
    
    unsigned int periodSize_s = m_desc.lengthInFrames * m_desc.channelCount;

    for (unsigned int i = 0; i < SOUND_LINUX_ALSA_BUFFER_COUNT; ++i)
        m_pWaveBuffer[i] = new short[periodSize_s];        
}

SoundService_linux_alsa::~SoundService_linux_alsa()
{
	// free the alsa pcm device
	snd_pcm_drop(m_pPcmHandle);
	snd_pcm_close(m_pPcmHandle);
    
    for (unsigned int i = 0; i < SOUND_LINUX_ALSA_BUFFER_COUNT; ++i)
        delete[] m_pWaveBuffer[i];
}

BufferDesc SoundService_linux_alsa::GetDescription() const { return m_desc; }

snd_pcm_sframes_t SoundService_linux_alsa::GetAvailableBufferSpaceInSamples()
{
	snd_pcm_sframes_t avail = snd_pcm_avail_update(m_pPcmHandle);
	if (avail < 0)
	{
		snd_pcm_prepare(m_pPcmHandle);
		avail = snd_pcm_avail_update(m_pPcmHandle);
		mini3d_assert(avail >= 0, "Broken playback stream on Alsa PCM device: %s", snd_strerror(avail));
	}
	return avail;
}

short* SoundService_linux_alsa::GetNextPeriodBuffer()
{
    unsigned int periodSize_s = m_desc.lengthInFrames * m_desc.channelCount;
	
	while(GetAvailableBufferSpaceInSamples() < m_desc.lengthInFrames * m_desc.channelCount)
		usleep(5000);

    return m_pWaveBuffer[m_nextBufferIndex];
}

void SoundService_linux_alsa::AddPeriodBufferToQueue(short* pBuffer)
{
	int framesWritten = snd_pcm_writei(m_pPcmHandle, m_pWaveBuffer[m_nextBufferIndex], m_desc.lengthInFrames);
	if (framesWritten < 0)	
	{ 
		snd_pcm_prepare(m_pPcmHandle); 
		framesWritten = snd_pcm_writei(m_pPcmHandle, m_pWaveBuffer[m_nextBufferIndex], m_desc.lengthInFrames); 
	}
	mini3d_assert(framesWritten >= 0, "Could not initialize write to Alsa PCM device: %s", snd_strerror(framesWritten));
	mini3d_assert(framesWritten == m_desc.lengthInFrames, "Failed to write all expected data to device Alsa PCM device: %s", snd_strerror(framesWritten));

    m_nextBufferIndex = (m_nextBufferIndex + 1) % SOUND_LINUX_ALSA_BUFFER_COUNT;
}


#endif // defined(__linux) && !defined(ANDROID) && !defined(__APPLE__)
