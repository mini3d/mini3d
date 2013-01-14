
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license>

#ifdef ANDROID

#include "soundservice_android_opensl_es.hpp"
#include <unistd.h>

void mini3d_assert(bool expression, const char* text, ...);

using namespace mini3d::sound; 

const unsigned int SOUND_ANDROID_OPENSL_ES_BUFFER_SIZE_IN_FRAMES = 2048;

SoundService_android_opensl_es::SoundService_android_opensl_es(unsigned int channels, unsigned int sampleRate) : 
    m_nextBufferIndex(0)
{

    m_desc.channelCount = channels;
    m_desc.sampleRate = sampleRate;
    m_desc.lengthInFrames = SOUND_ANDROID_OPENSL_ES_BUFFER_SIZE_IN_FRAMES;

    SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    mini3d_assert(SL_RESULT_SUCCESS == result, "Could not create OpenSL engine");

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    mini3d_assert(SL_RESULT_SUCCESS == result, "Could not realize OpenSL engine");

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    mini3d_assert(SL_RESULT_SUCCESS == result, "Could not get OpenSL engine interface");

    // create output mix
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, 0, 0);
    mini3d_assert(SL_RESULT_SUCCESS == result, "Could not create OpenSL output mix object");

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    mini3d_assert(SL_RESULT_SUCCESS == result, "Could not realize OpenSL output mix object");

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, SOUND_ANDROID_OPENSL_ES_BUFFER_COUNT};

    SLDataFormat_PCM format_pcm;
    format_pcm.formatType = SL_DATAFORMAT_PCM;
    format_pcm.numChannels = m_desc.channelCount;
    format_pcm.samplesPerSec = m_desc.sampleRate * 1000; // OpenSL ES specifies sampling rates in milli-Hertz
    format_pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16; 
    format_pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
    format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;
    format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;

    
    if (m_desc.channelCount == 1)
        format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
    else if (m_desc.channelCount == 2)
        format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    else if (m_desc.channelCount == 6)
        format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT | SL_SPEAKER_FRONT_CENTER | SL_SPEAKER_LOW_FREQUENCY | SL_SPEAKER_BACK_LEFT | SL_SPEAKER_BACK_RIGHT;
    else if (m_desc.channelCount == 8)
        format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT | SL_SPEAKER_FRONT_CENTER | SL_SPEAKER_LOW_FREQUENCY | SL_SPEAKER_BACK_LEFT | SL_SPEAKER_BACK_RIGHT | SL_SPEAKER_SIDE_LEFT | SL_SPEAKER_SIDE_RIGHT;
    else
        format_pcm.channelMask = 0;

    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk, 1, ids, req);
    mini3d_assert(SL_RESULT_SUCCESS == result, "Unable to create player object. Are the requested channel count and sample rate values within reason?");

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    mini3d_assert(SL_RESULT_SUCCESS == result, "Unable to realize player object.");

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    mini3d_assert(SL_RESULT_SUCCESS == result, "Unable to get player object interface");

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);
    mini3d_assert(SL_RESULT_SUCCESS == result, "Unable to get buffer queue interface");

    // will start playing automatically when buffers are queued
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    mini3d_assert(SL_RESULT_SUCCESS == result, "Unable to set play state");
    
    unsigned int periodSize_s = SOUND_ANDROID_OPENSL_ES_BUFFER_SIZE_IN_FRAMES * m_desc.channelCount;

    for (unsigned int i = 0; i < SOUND_ANDROID_OPENSL_ES_BUFFER_COUNT; ++i)
        m_pWaveBuffer[i] = new short[periodSize_s];        
}

SoundService_android_opensl_es::~SoundService_android_opensl_es()
{
    (*bqPlayerObject)->Destroy(bqPlayerObject);  
    (*outputMixObject)->Destroy(outputMixObject);
    (*engineObject)->Destroy(engineObject);
    
    for (unsigned int i = 0; i < SOUND_ANDROID_OPENSL_ES_BUFFER_COUNT; ++i)
        delete[] m_pWaveBuffer[i];
}

BufferDesc SoundService_android_opensl_es::GetDescription() const { return m_desc; }

short* SoundService_android_opensl_es::GetNextPeriodBuffer()
{
    unsigned int periodSize_s = m_desc.lengthInFrames * m_desc.channelCount;
    SLresult result;
	SLAndroidSimpleBufferQueueState state;
	
	result = (*bqPlayerBufferQueue)->GetState(bqPlayerBufferQueue, &state); 

	while(state.count > SOUND_ANDROID_OPENSL_ES_BUFFER_COUNT - 1)
	{ 
		usleep(5000);
		(*bqPlayerBufferQueue)->GetState(bqPlayerBufferQueue, &state);
	}

    return m_pWaveBuffer[m_nextBufferIndex];
}

void SoundService_android_opensl_es::AddPeriodBufferToQueue(short* pBuffer)
{
    SLresult result;
    unsigned int periodSize_s = m_desc.lengthInFrames * m_desc.channelCount;

	// for streaming playback, replace this test by logic to find and fill the next buffer
	result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, m_pWaveBuffer[m_nextBufferIndex], periodSize_s * 2);
    mini3d_assert(SL_RESULT_SUCCESS == result, "");

    m_nextBufferIndex = (m_nextBufferIndex + 1) % SOUND_ANDROID_OPENSL_ES_BUFFER_COUNT;
}


#endif // ANDROID
