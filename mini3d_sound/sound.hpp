
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license>

// TODO: Volume gain for Sound Buffers and Stream Buffers
// TODO: Automatic recording level setting for Sound Output (make louder sound drown quieter sounds by lowering the overall level automatically)
// TODO: Sound Looping

#ifndef MINI3D_SOUND_H
#define MINI3D_SOUND_H

#include "platform/isoundservice.hpp"

void mini3d_assert(bool expression, const char* text, ...);

struct stb_vorbis;

typedef unsigned int uint;
typedef unsigned short ushort;

namespace mini3d {
namespace sound {


///////// THREADING ///////////////////////////////////////////////////////////

struct IFunctor { virtual ~IFunctor() {}; };
template <class T> struct Functor : IFunctor { typedef void (T::*F)(); Functor(T* o, F f) : o(o), f(f) {} ~Functor() { (o->*f)(); } private: T* o; F f; };

struct Mutex { Mutex(); ~Mutex(); void Lock(); void Unlock(); private: void* m_hMutex; };
struct Lock { Lock(Mutex* m) { x=m; x->Lock(); } ~Lock() { x->Unlock(); } private: Mutex* x; };
struct Thread { Thread() : t(0) {} void Run(IFunctor* pFunc); void Join(); static void Sleep (unsigned int ms); void* t; };


///////// MINI3D SOUND ////////////////////////////////////////////////////////

const uint SAMPLE_RATE_44100_HZ = 44100;
const uint SAMPLE_RATE_22050_HZ = 22050;

const uint MONO = 1;
const uint STEREO = 2;
const uint SURROUND_5_1 = 6;
const uint SURROUND_7_1 = 8;

const uint MAX_TOTAL_SOUND_SOURCES = 256;
const uint MAX_TOTAL_SOUND_FILTERS = 16;

const uint STREAM_BUFFER_SIZE_IN_FRAMES = 1 << 14; // Must be power of two
const uint STREAM_BUFFER_SIZE_MODULO = STREAM_BUFFER_SIZE_IN_FRAMES - 1; // Allows us to do a fast bitwize AND operation instead of an expensive modulo...

// These are used to determine the size of channel mix arrays in SoundSource::internal_mix()
const uint MAX_INPUT_CHANNELS = 2; // Max number of channels in ogg-files used for input
const uint MAX_OUTPUT_CHANNELS = 8; // Max number of surround-channels in output.

#define LOCKED Lock guard(&m_mutex)

///////// FORWARD DECLARATIONS ////////////////////////////////////////////////

class SoundBuffer;
class StreamBuffer;
class SoundSource;
class SoundFilter;
class SoundOutput;


///////// SHARED OBJECT ///////////////////////////////////////////////////////

class SharedObject
{
public:
    SharedObject() : m_allocCount(1)   {}
    virtual ~SharedObject() {}

    virtual void Alloc()                            { LOCKED; ++m_allocCount; }
    virtual void Release()                          { m_mutex.Lock(); if (--m_allocCount) m_mutex.Unlock(); else delete this; }

    virtual uint internal_get_alloc_count()         { LOCKED; return m_allocCount; }

protected:
    mutable Mutex m_mutex;
    uint m_allocCount;
};


///////// SOUND BUFFER ////////////////////////////////////////////////////////

class SoundBuffer : public SharedObject
{
public:
    SoundBuffer() : m_pBuffer(0)                    {}
    SoundBuffer(const char* pFilename);
    SoundBuffer(const float* pSoundData, uint lengthInFrames, uint channelCount, uint sampleRate);
    
    BufferDesc GetDescription() const               { return m_desc; }

    // For use by Sound Source only!
    // returns acutual readable frames
    virtual double _internal_get_frame_pointer(float** pBuffer, const double positionInFrames, double requestedFrames);

protected:
    // Reference Counted, disposed using Release();
    virtual ~SoundBuffer()                          { delete[] m_pBuffer; }

    float* m_pBuffer;
    BufferDesc m_desc;
};


///////// STREAM BUFFER ///////////////////////////////////////////////////

// Only use this with a single SoundSource at a time!
class StreamBuffer : public SoundBuffer
{
public:
    StreamBuffer();
    StreamBuffer(const char* pFilename);
    StreamBuffer(const char* pSoundData, uint dataSizeInBytes);

    void Release()                                  { LOCKED; if (--m_allocCount == 0) m_shutDown = true; }

    // For use by Sound Source only!
    double _internal_get_frame_pointer(float** pBuffer, const double positionInFrames, double requestedFrames);

private:
    void Init();
    void DecodeThreadFunction();

private:
    // Reference Counted, disposed using Release();
    ~StreamBuffer() {};

    Thread m_decodeThread;
    stb_vorbis* m_pVorbis;    
    const char* m_pData;
    double m_readPosition_f;
    double m_nextReadPosition_f;
    uint m_dataLengthInBytes;
    uint m_writePosition_f;
    bool m_streamHasEnded;
    bool m_shutDown;
};


///////// SOUND FILTER ////////////////////////////////////////////////////////

class SoundFilter : public SharedObject
{
public:
    SoundFilter(SoundOutput* pOutput);

    virtual void ApplyFilter() = 0; // Override this in a sub-class to implement the effect

    SoundFilter* GetFilter() const                  { LOCKED; return m_pFilter; }
    void SetFilter(SoundFilter* pFilter)            { LOCKED; if (m_pFilter) m_pFilter->Release(); m_pFilter = pFilter; if (pFilter) pFilter->Alloc(); }

    // Called by sound output to mix sound sources into this buffer
    void internal_mix();

    // Called by sound source to get a buffer to mix into
    float* internal_get_buffer()                    { return m_pBuffer; }


protected:
    // Shared resource, disposed using Release()!
    ~SoundFilter()                                  { delete m_pBuffer; if (m_pFilter) m_pFilter->Release();} 

    SoundOutput* m_pOutput;
    float* m_pBuffer;
    SoundFilter* m_pFilter;
};


///////// SOUND SOURCE ////////////////////////////////////////////////////////

class SoundSource : public SharedObject
{
public:
    enum State                                      { STATE_PLAYING, STATE_PAUSED, STATE_STOPPED };

    SoundSource(SoundOutput* pOutput, SoundBuffer* pBuffer = 0, SoundFilter* pFilter = 0);

    void Play()                                     { LOCKED; if(m_pOutput) m_state_user.state = STATE_PLAYING; }
    void Pause()                                    { LOCKED; if (m_state_user.state == STATE_PLAYING) m_state_user.state = STATE_PAUSED; }
    void Stop()                                     { LOCKED; m_state_user.state = STATE_STOPPED; }

    State GetState() const                          { LOCKED; return m_state_user.state; }

    SoundBuffer* GetBuffer() const                  { LOCKED; return m_state_user.pBuffer; }
    void SetBuffer(SoundBuffer* pBuffer)            { LOCKED; if (m_state_user.pBuffer) m_state_user.pBuffer->Release(); m_state_user.pBuffer = pBuffer; if (pBuffer) pBuffer->Alloc(); }

    SoundFilter* GetFilter() const                  { LOCKED; return m_state_user.pFilter; }
    void SetFilter(SoundFilter* pFilter)            { LOCKED; if (m_state_user.pFilter) m_state_user.pFilter->Release(); m_state_user.pFilter = pFilter; if (pFilter) pFilter->Alloc(); }

    double GetOffsetInFrames() const                { LOCKED; return m_state_user.offset_f; }
    virtual void SetOffsetInFrames(double offset)   { LOCKED; m_state_user.offset_f = offset; }

    float GetVolume() const                         { LOCKED; return m_state_user.vol; }
    void SetVolume(float volume)                    { LOCKED; m_state_user.vol = volume; }

    void Get3dPosition(float pos[3])                { LOCKED; pos[0] = m_state_user.pos[0]; pos[1] = m_state_user.pos[1]; pos[2] = m_state_user.pos[2]; }
    void Set3dPosition(const float pos[3])          { LOCKED; m_state_user.pos[0] = pos[0]; m_state_user.pos[1] = pos[1]; m_state_user.pos[2] = pos[2]; }

    void Get3dVelocity(float vel[3])                { LOCKED; vel[0] = m_state_user.vel[0]; vel[1] = m_state_user.vel[1]; vel[2] = m_state_user.vel[2]; }
    void Set3dVelocity(const float vel[3])          { LOCKED; m_state_user.vel[0] = vel[0]; m_state_user.vel[1] = vel[1]; m_state_user.vel[2] = vel[2]; }

    float GetPitch() const                          { LOCKED; return m_state_user.pitch; }
    void SetPitch(float pitch)                      { LOCKED; m_state_user.pitch = pitch; }
    
    // For use by Sound Output only!
    bool internal_get_has_stopped()                 { LOCKED; return ((m_state_user.state == STATE_STOPPED) && (m_state_mix.state == STATE_STOPPED)); }
    void internal_mix();

private:
    void Init();
     
    // Shared resource, disposed using Release()!
    ~SoundSource();

    SoundOutput* m_pOutput;

    struct InternalState
    {
        SoundBuffer* pBuffer;
        SoundFilter* pFilter;
        double offset_f;
        float pos[3];
        float vel[3];
        float pitch;
        float vol;
        State state;
    };

    InternalState m_state_user;
    InternalState m_state_mix;

    float m_fadeVol;
};  


///////// SOUND OUTPUT ///////////////////////////////////////////////////////

class SoundOutput : SharedObject
{
public: 
    SoundOutput(unsigned int channels, unsigned int sampleRate);

    float GetVolume() const                         { LOCKED; return m_vol; }
    void SetVolume(float volume)                    { LOCKED; m_vol = volume; }

    BufferDesc GetDescription()                     { return m_pService->GetDescription(); }

    void Release()                                  { LOCKED; if (--m_allocCount == 0) m_shutDown = true; }

    // Called by Sound Source and Sound Filter constructors. No need to call manually!
    bool internal_add(SoundSource* pSource);
    bool internal_add(SoundFilter* pFilter);

    // Called by sound source and sound filter to get a buffer to mix into
    float* internal_get_buffer()                    { return m_pPeriodBuffer; };

private:
    void MixThreadFunction();

    ~SoundOutput() {}; // Shared resource, disposed using Release()!

    Thread m_mixThread;
    ISoundService* m_pService;
    
    float* m_pPeriodBuffer; // only used by the mix thread while mixing

    SoundSource* m_pSources[MAX_TOTAL_SOUND_SOURCES];
    SoundFilter* m_pFilters[MAX_TOTAL_SOUND_FILTERS];

    uint m_sourceCount;
    uint m_filterCount;

    float m_vol;
    float m_oldVol;

    bool m_shutDown;
};


}
}

#endif // _MINI3D_SOUND_H

// Include Platform specific versions
#include "platform/win32_waveout/soundservice_win32_waveout.hpp"
#include "platform/android_opensl_es/soundservice_android_opensl_es.hpp"
#include "platform/linux_alsa/soundservice_linux_alsa.hpp"
#include "platform/osx_ios_core_audio/soundservice_osx_ios_core_audio.hpp"
