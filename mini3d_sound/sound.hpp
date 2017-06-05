
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license>

// TODO: Volume gain for Sound Buffers and Stream Buffers
// TODO: Automatic recording level setting for Sound Output (make louder sound drown quieter sounds by lowering the overall level automatically)
// TODO: Sound Looping

#ifndef MINI3D_SOUND_H
#define MINI3D_SOUND_H

#include "platform/isoundservice.hpp"

#include <atomic>
#include <memory>
#include <algorithm>
#include <vector>
#include <thread>

void mini3d_assert(bool expression, const char* text, ...);

struct stb_vorbis;

namespace mini3d {
namespace sound {


///////// MINI3D SOUND /////////////////////////////////////////////////////////

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


///////// WAV //////////////////////////////////////////////////////////////////

class Buffer;
    
class Wav {
    public:
        static Buffer *load(const char *fileName);
};

    
///////// BUFFER ///////////////////////////////////////////////////////////////

class Buffer {
    
public:
    Buffer(size_t channelCount, size_t length);
    
    size_t getChannelCount();
    void setChannelCount(size_t channelCount);
    
    size_t getLength();
    void setLength(size_t length);
    
    void clear();
    
    float* getDataBuffer(size_t channel);
    void addSample(size_t channel, float value);
    
    std::vector<std::vector<float>> mData;
};


///////// SOURCE ///////////////////////////////////////////////////////////////

class Source {
    
public:
    enum State { STOPPED, PLAYING, PAUSED };
    
    static constexpr float AUDIBLE_THRESHOLD = 0.001f;
    
    // Playback Control
    void play() { state = PLAYING; }
    void pause() { state = PAUSED; }
    void stop() { state = STOPPED; }
    
    State getPlaybackState() { return state; }
    
    float getBalance() { return balance; }
    void setBalance(float value) { balance = std::min(std::max(value, 0.0f), 1.0f); }
    
    float getVolume() { return volume; }
    void setVolume(float value) { volume = value; }
    
    size_t getPriority() { return priority; };
    void setPriority(size_t value) { priority = value; };
    
    virtual void addToBuffer(Buffer *buffer) = 0;
    virtual void advance(size_t count) = 0;
    
    static bool isHigherPriority(std::shared_ptr<Source> a, std::shared_ptr<Source> b);
    
    // Only from mixer thread
    
    bool isPlaying();
    
    
protected:
    
    static void mixBuffers(Buffer* srcBuffer, Buffer* dstBuffer, size_t srcOffset, size_t dstOffset, size_t count, float (&inMixMatrix)[2][2], float (&outMixMatrix)[2][2]);
    static void zeroMixMatrix(float (&mixMatrix)[2][2]);
    void setMixMatrix(size_t srcChannels, size_t dstChannels);
    
    // Shared between threads
    
    std::atomic<State> state{State::PLAYING};
    std::atomic<float> volume{1.0f};
    std::atomic<float> balance{0.5f};
    std::atomic<size_t> priority{0};
    
    // Only background thread
    
    float fadeInOutVolume = 1.0f;
    float mixMatrix[2][2];
    float oldMixMatrix[2][2];
    
};


///////// SOUND ////////////////////////////////////////////////////////////////

class Sound : public Source {
    
public:
    
    Sound(const char *fileName);
    Sound(std::shared_ptr<Buffer> buffer);
    virtual ~Sound();
    
    // Buffer mixing
    
    void addToBuffer(Buffer *buffer);
    void advance(size_t count);
    
private:
    
    size_t offset = 0;
    std::shared_ptr<Buffer> audioBuffer;
};


///////// MUSIC ////////////////////////////////////////////////////////////////

class Music : public Source {
public:
    Music();
    Music(const char* filename);
    Music(const char* pSoundData, size_t dataSizeInBytes);
    virtual ~Music() {};
    
    // Buffer mixing
    void addToBuffer(Buffer *buffer);
    void advance(size_t count);
    
private:
    void Init();
    void DecodeThreadFunction();
    
private:
    
    // Mix thread only
    std::atomic<size_t> m_readPosition_f;
    
    // Decode thread only
    std::thread m_decodeThread;
    stb_vorbis* m_pVorbis;
    std::vector<unsigned char> fileData;
    std::atomic<size_t> m_writePosition_f;
    
    // Common static
    size_t channelCount;
    size_t sampleRate;
    size_t lengthInFrames;
    size_t m_dataLengthInBytes;
    
    // Common atomic
    std::atomic<bool> m_shutDown;
    std::atomic<bool> m_streamHasEnded;
    std::atomic<size_t> offset { 0 };
    
    // Common mutex
    std::mutex bufferMutex;
    Buffer* streamBuffer; //bufferMutex
    
};


///////// MIXER ////////////////////////////////////////////////////////////////

class Mixer : public Source {
    
public:
    Mixer();
    ~Mixer();
    
    void addSource(std::shared_ptr<Source> source);
    bool isPlaying();
    
    void advance(size_t count);
    void addToBuffer(Buffer* buffer);
    
private:
    
    static bool isFinished(const std::shared_ptr<Source> &source);
    
    std::vector<std::shared_ptr<Source>> sources;
    size_t totalVoices;
};

    
///////// OUTPUT ///////////////////////////////////////////////////////////////

class BufferDesc;
class ISoundService;
    
class Output {
    
public:
    Output();
    ~Output();
    
    void setSource(std::shared_ptr<Source> source);
    void shutDown();
    
private:
    std::thread thread;
    std::shared_ptr<Source> source;
    std::atomic<bool> isShutDown;
    
    static void Mix(Output *output, int id);
    
    static void innerMix(Buffer &mixBuffer, BufferDesc* desc, Output *output,
                         ISoundService *service);
    
    static float limit(float value);
};

}
}

#endif // _MINI3D_SOUND_H
