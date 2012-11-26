
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license>

#ifdef ANDROID

#ifndef MINI3D_SOUND_ANDROID_OPENSL_ES_H
#define MINI3D_SOUND_ANDROID_OPENSL_ES_H

#include "../isoundservice.hpp"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

namespace mini3d {
namespace sound { 

const unsigned int SOUND_ANDROID_OPENSL_ES_BUFFER_COUNT = 3;

///////// SOUND SERVICE ///////////////////////////////////////////////////////

typedef
class SoundService_android_opensl_es : public ISoundService
{
public:
    SoundService_android_opensl_es(unsigned int channels, unsigned int sampleRate);
    ~SoundService_android_opensl_es();

    short* GetNextPeriodBuffer();
    void AddPeriodBufferToQueue(short* pBuffer);
    BufferDesc GetDescription() const;

private:
    BufferDesc m_desc;
    unsigned int m_nextBufferIndex;
    short* m_pWaveBuffer[SOUND_ANDROID_OPENSL_ES_BUFFER_COUNT];

    // engine interfaces
    SLObjectItf engineObject;
    SLEngineItf engineEngine;

    // output mix interfaces
    SLObjectItf outputMixObject;

    // buffer queue player interfaces
    SLObjectItf bqPlayerObject;
    SLPlayItf bqPlayerPlay;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;


} SoundService;

}
}

#endif // MINI3D_SOUND_ANDROID_OPENSL_ES_H
#endif // ANDROID