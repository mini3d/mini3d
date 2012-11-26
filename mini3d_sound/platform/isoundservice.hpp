
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license>

#ifndef MINI3D_SOUND_SOUNDSERVICE_H
#define MINI3D_SOUND_SOUNDSERVICE_H

typedef unsigned int uint;

namespace mini3d {
namespace sound {


///////// BUFFER DESCRIPTION //////////////////////////////////////////////////

struct BufferDesc { uint lengthInFrames, channelCount, sampleRate; };


///////// SOUND SERVICE INTERFACE /////////////////////////////////////////////

struct ISoundService
{
    virtual short* GetNextPeriodBuffer() = 0;
    virtual void AddPeriodBufferToQueue(short* pBuffer) = 0;
    virtual BufferDesc GetDescription() const = 0;
    virtual ~ISoundService() {}; // Shared Object, dispose using Release()!
};

}
}

#endif // MINI3D_SOUND_SOUNDSERVICE_H