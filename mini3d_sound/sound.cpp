
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license>


#include "sound.hpp"

// Include Platform specific versions
#include "platform/win32_waveout/soundservice_win32_waveout.hpp"
#include "platform/android_opensl_es/soundservice_android_opensl_es.hpp"
#include "platform/linux_alsa/soundservice_linux_alsa.hpp"
#include "platform/osx_ios_core_audio/soundservice_osx_ios_core_audio.hpp"

#define STB_VORBIS_NO_PUSHDATA_API
#include "stb_vorbis/stb_vorbis.h"

#include <fstream>
#include <iostream>


using namespace mini3d::sound;


///////// WAV //////////////////////////////////////////////////////////////////

struct Header {
    char riffID[4];
    unsigned int size;
    char wavID[4];
    char fmtID[4];
    unsigned int fmtSize;
    unsigned short format;
    unsigned short channels;
    unsigned int sampleRate;
    unsigned int bytePerSec;
    unsigned short blockSize;
    unsigned short bit;
    char dataID[4];
    unsigned int dataSize;
};

Buffer *Wav::load(const char *fileName) {
    Header header;
    
    FILE *file = fopen(fileName, "rb");
    fread((char *)&header, sizeof(header), 1, file);
    
    Buffer *buffer = new Buffer(header.channels, 0);
    
    short buf;
    while (!feof(file)) {
        for (int i = 0; i < header.channels; ++i) {
            fread((void *)&buf, sizeof(buf), 1, file);
            buffer->addSample(i, buf / (float)SHRT_MAX);
        }
    }
    
    fclose(file);
    
    return buffer;
}


///////// BUFFER ///////////////////////////////////////////////////////////////

Buffer::Buffer(size_t channelCount, size_t length)
: mData(std::vector<std::vector<float> >(channelCount, std::vector<float>(length))) {
}

size_t Buffer::getChannelCount() {
    return mData.size();
}

void Buffer::setChannelCount(size_t channelCount) {
    mData.resize(channelCount);
}

size_t Buffer::getLength() {
    return mData.size() == 0 ? 0 : mData[0].size();
}

void Buffer::setLength(size_t length) {
    for(auto& channel : mData)
        channel.resize(length);
}

void Buffer::clear() {
    for(auto& channel : mData)
        fill(channel.begin(), channel.end(), 0);
}

float* Buffer::getDataBuffer(size_t channel) {
    return mData[channel].data();
}
void Buffer::addSample(size_t channel, float value) {
    mData[channel].push_back(value);
}


///////// SOURCE ///////////////////////////////////////////////////////////////

bool Source::isHigherPriority(std::shared_ptr<Source> a, std::shared_ptr<Source> b) {
    return (a->priority < b->priority) || ((a->priority == b->priority) && (a->volume > b->volume));
}

void Source::setMixMatrix(size_t srcChannels, size_t dstChannels) {
    
    float volume = this->volume;
    float balance = this->balance;
    State state = this->state;
    
    volume = state != PLAYING ? 0.0 : volume;
    
    mixMatrix[0][0] = 0.0f;
    mixMatrix[0][1] = 0.0f;
    mixMatrix[1][0] = 0.0f;
    mixMatrix[1][1] = 0.0f;
    
    if (srcChannels == 1) {
        if (dstChannels == 2) {
            mixMatrix[0][0] = volume * 2.0f * (1.0f - balance);
            mixMatrix[0][1] = volume * 2.0f * balance;
        }
    } else if (srcChannels == 2) {
        if (dstChannels == 2 ) {
            mixMatrix[0][0] = volume * std::min(2.0f * (1.0f - balance), 1.0f);
            mixMatrix[1][1] = volume * std::min(2.0f * balance, 1.0f);
        }
    }
}

void Source::zeroMixMatrix(float (&mixMatrix)[2][2]) {
    for (size_t x = 0; x < 2; x++) {
        for (size_t y = 0; y < 2; y++) {
            mixMatrix[x][y] = 0.0f;
        }
    }
}

bool Source::isPlaying() {
    
    if (state == PLAYING) {
        return true;
    }
    
    for (size_t x = 0; x < 2; x++) {
        for (size_t y = 0; y < 2; y++) {
            if (mixMatrix[x][y] > 0.001f) {
                return true;
            }
        }
    }
    
    return false;
}

// TODO: Make fixed delta increments per frame!
void Source::mixBuffers(Buffer* srcBuffer, Buffer* dstBuffer, size_t srcOffset, size_t dstOffset, size_t count, float (&inMixMatrix)[2][2], float (&outMixMatrix)[2][2]) {
    
    size_t srcChannels = srcBuffer->getChannelCount();
    size_t dstChannels = dstBuffer->getChannelCount();
    size_t length = count;
    
    for (size_t x = 0; x < srcChannels; x++) {
        float* src = srcBuffer->getDataBuffer(x);
        
        for (size_t y = 0; y < dstChannels; y++) {
            if (inMixMatrix[x][y] == 0.0f && outMixMatrix[x][y] == 0.0f) {
                continue;
            }
            
            float inVolume = inMixMatrix[x][y];
            float outVolume = outMixMatrix[x][y];
            float deltaVolume = (outVolume - inVolume) / length;
            
            float* dst = dstBuffer->getDataBuffer(y);
            for (int j = 0; j < length; ++j) {
                float volume = inVolume + deltaVolume * j;
                dst[j + dstOffset] += src[j + srcOffset] * volume;
            }
            
            // Make sure the final values gets set back to the inputMatrix so they will be the
            // start values next time around
            inMixMatrix[x][y] = outVolume;
        }
    }
}


///////// SOUND ////////////////////////////////////////////////////////////////

Sound::Sound(const char *fileName)
: audioBuffer(std::shared_ptr<Buffer>(Wav::load(fileName))) {}

Sound::Sound(std::shared_ptr<Buffer> buffer) : audioBuffer(buffer) {}

Sound::~Sound() {}

void Sound::advance(size_t count) {
    offset += count;
}

void Sound::addToBuffer(Buffer *buffer) {
    
    std::shared_ptr<Buffer> currentBuffer = audioBuffer;
    
    setMixMatrix(currentBuffer->getChannelCount(), buffer->getChannelCount());
    
    size_t count = std::min(buffer->getLength(), currentBuffer->getLength() - offset);
    mixBuffers(currentBuffer.get(), buffer, offset, 0, count, oldMixMatrix, mixMatrix);
    
    offset += buffer->getLength();
    
    if (offset >= audioBuffer.get()->getLength()) {
        state = STOPPED;
    }
}


///////// MUSIC ////////////////////////////////////////////////////////////////

const size_t STREAM_BUFFER_SIZE_IN_FRAMES = 1 << 14; // Must be power of two

Music::Music() { Init(); }

Music::Music(const char *filename) {
    std::ifstream file(filename, std::ios::binary);
    mini3d_assert(file.is_open(), "Failed to open the file \"%s\". File not found!", filename);

    fileData = std::vector<unsigned char>(
                                          (std::istreambuf_iterator<char>(file)),
                                          std::istreambuf_iterator<char>());
    
    Init();
}

Music::Music(const char *pSoundData, size_t sizeInBytes)
: fileData((unsigned char *)pSoundData,
           (unsigned char *)pSoundData + sizeInBytes),
m_dataLengthInBytes(sizeInBytes) {
    Init();
}

void Music::Init() {
    m_shutDown = false;
    m_writePosition_f = 0;
    m_readPosition_f = 0;
    m_streamHasEnded = false;
    
    int error = VORBIS__no_error;
    m_pVorbis = stb_vorbis_open_memory(&fileData[0], (int)fileData.size(), &error, 0);
    mini3d_assert(error == VORBIS__no_error, "Failed to crate new vorbis stream: %d. Is the file an error-free OGG file?", error);

    stb_vorbis_info info = stb_vorbis_get_info(m_pVorbis);
    channelCount = info.channels;
    sampleRate = info.sample_rate;
    lengthInFrames = stb_vorbis_stream_length_in_samples(m_pVorbis);
    
    streamBuffer = new Buffer(channelCount, STREAM_BUFFER_SIZE_IN_FRAMES);
    
    m_decodeThread = std::thread(&Music::DecodeThreadFunction, this);
}

void Music::advance(size_t count) { m_readPosition_f += count; }

void Music::addToBuffer(Buffer *buffer) {
    setMixMatrix(streamBuffer->getChannelCount(), buffer->getChannelCount());
    
    size_t total = 0;
    while (total < buffer->getLength()) {
        
        // Don't allow writing more than buffer length, past the
        // m_writePosition_f or writing past the end of the streambuffer
        size_t bufferReadPosition =
        m_readPosition_f % STREAM_BUFFER_SIZE_IN_FRAMES;
        size_t count = std::min(
                                std::min(buffer->getLength(), m_writePosition_f - m_readPosition_f),
                                STREAM_BUFFER_SIZE_IN_FRAMES - bufferReadPosition);
        
        if (count == 0 && m_streamHasEnded) {
            state = STOPPED;
            zeroMixMatrix(oldMixMatrix);
            return;
        }
        
        mixBuffers(streamBuffer, buffer, bufferReadPosition, total, count,
                   oldMixMatrix, mixMatrix);
        
        total += count;
        m_readPosition_f += count;
    }
}

void Music::DecodeThreadFunction() {
    size_t sleepPeriod =
    1000 * STREAM_BUFFER_SIZE_IN_FRAMES / sampleRate /
    16; // 1/16 the time it takes to play the whole stream buffer in ms
    
    while (!m_shutDown) {
        
        // Check how much we can write
        size_t remaining = STREAM_BUFFER_SIZE_IN_FRAMES -
        (m_writePosition_f - m_readPosition_f);
        
        // Loop write until we have written the proper amount
        while (remaining > 0 && isPlaying()) {
            
            // Don't write outside the edge of the ring buffer
            size_t bufferWritePosition =
            m_writePosition_f % STREAM_BUFFER_SIZE_IN_FRAMES;
            size_t count = std::min(remaining, STREAM_BUFFER_SIZE_IN_FRAMES -
                                    bufferWritePosition);
            
            float *buffers[2] = {
                streamBuffer->getDataBuffer(0) + bufferWritePosition,
                streamBuffer->getDataBuffer(1) + bufferWritePosition};
            
            // C++ atomic memory model guarantees that the streamBuffer update
            // is seen correctly thanks to the implicit strong memory barrier of
            // the atomic writePosition update below!
            int read = stb_vorbis_get_samples_float(
                                                    m_pVorbis, (int)channelCount, (float **)&buffers, (int)count);
            
            if (read == 0) {
                m_shutDown = true;
            }
            
            remaining -= read;
            m_writePosition_f += read;
        }
        
        usleep((useconds_t)sleepPeriod * 1000);
    }
    
    stb_vorbis_close(m_pVorbis);
    delete this;
}


///////// MIXER ////////////////////////////////////////////////////////////////

Mixer::Mixer() : totalVoices(2) {}

Mixer::~Mixer() {}

void Mixer::addSource(std::shared_ptr<Source> source) {
    sources.push_back(source);
}

bool Mixer::isPlaying() { return !sources.empty(); }

bool Mixer::isFinished(const std::shared_ptr<Source> &source) {
    return source.use_count() == 1 && !source->isPlaying();
}

void Mixer::advance(size_t count) {
    // Remove all sources that are no longer referenced
    sources.erase(std::remove_if(sources.begin(), sources.end(), isFinished), sources.end());
    
    for (std::shared_ptr<Source> source : sources) {
        source->advance(count);
    }
}

template<typename Iter, typename Predicate>
void insertion_sort(Iter first, Iter last, Predicate p) {
    for (Iter it = first; it != last; ++it)
        std::rotate(std::upper_bound(first, it, *it, p), it, std::next(it));
}

void Mixer::addToBuffer(Buffer *buffer) {
    
    // Remove all sources that are no longer referenced
    sources.erase(std::remove_if(sources.begin(), sources.end(), isFinished), sources.end());
    
    // Prioritize sounds
    insertion_sort(sources.begin(), sources.end(), Source::isHigherPriority);
    
    // Add all sources to buffer
    size_t end = std::min(totalVoices, sources.size());
    
    for (size_t i = 0; i < end; i++) {
        sources[i]->addToBuffer(buffer);
    }
    
    for (size_t i = totalVoices; i < sources.size(); i++) {
        sources[i]->advance(buffer->getLength());
    }
}


///////// OUTPUT ///////////////////////////////////////////////////////////////

Output::Output() : thread(&Mix, this, 0), isShutDown(false) {}
Output::~Output() {
    setSource(0);
    isShutDown = true;
    thread.join();
}

void Output::setSource(std::shared_ptr<Source> source) { this->source = source; }
void Output::shutDown() { isShutDown = true; }

void Output::Mix(Output *output, int id) {
    
    ISoundService *service = new SoundService_osx_ios_core_audio(2, 44100);
    BufferDesc desc = service->GetDescription();
    Buffer mixBuffer(desc.channelCount, desc.lengthInFrames);
    
    innerMix(mixBuffer, &desc, output, service);
    
    delete service;
}

void Output::innerMix(Buffer &mixBuffer, BufferDesc* desc, Output *output,
                      ISoundService *service) {
    
    while (!output->isShutDown) {
        short *pBuffer = service->GetNextPeriodBuffer();
        
        mixBuffer.clear();
        
        std::shared_ptr<Source> source = output->source;
        
        if (source.get()) {
            source->addToBuffer(&mixBuffer);
        }
        
        for (int i = 0; i < desc->channelCount; ++i) {
            float *pSrc = mixBuffer.getDataBuffer(i);
            for (int j = 0; j < desc->lengthInFrames; ++j) {
                pBuffer[j * 2 + i] = (short)(limit(pSrc[j] * 0.5) * SHRT_MAX);
            }
        }
        
        service->AddPeriodBufferToQueue(pBuffer);
    }
}

float Output::limit(float value) {
    return value > 1.0 ? 1.0 : value < -1.0 ? -1.0 : value;
};

