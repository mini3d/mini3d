
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license>


#include "sound.hpp"
#include "channelmixing.inl"

#define STB_VORBIS_NO_PUSHDATA_API
#include "stb_vorbis/stb_vorbis.h"

#include <climits>
#include <cstdlib>
#include <cmath>

using namespace mini3d::sound;

///////// THREADING ///////////////////////////////////////////////////////////

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

Mutex::Mutex()                                      { m_hMutex = malloc(sizeof(CRITICAL_SECTION)); InitializeCriticalSection((CRITICAL_SECTION*)m_hMutex); }
Mutex::~Mutex()                                     { DeleteCriticalSection((CRITICAL_SECTION*)m_hMutex); free(m_hMutex); }
void Mutex::Lock()                                  { EnterCriticalSection((CRITICAL_SECTION*)m_hMutex); }
void Mutex::Unlock()                                { LeaveCriticalSection((CRITICAL_SECTION*)m_hMutex); }

DWORD WINAPI StartThreadProc(void* lpParameter)     { delete (IFunctor*)lpParameter; return 0; }
void Thread::Run(IFunctor* pFunc)                   { if (!t) t = CreateThread(0, 0, &StartThreadProc, pFunc, 0, 0); }
void Thread::Join()                                 { WaitForSingleObject((HANDLE)t, INFINITE); t=0; }
void Thread::Sleep(unsigned int milliseconds)       { ::Sleep(milliseconds); }
#endif

#if defined(__linux) || defined(ANDROID) || defined(__APPLE__)
#include <pthread.h>
#include <unistd.h>
// pthread_init() is called from the constructor of each SoundService class for the platforms that need it!
Mutex::Mutex()                                      { m_hMutex = malloc(sizeof(pthread_mutex_t)); pthread_mutex_init((pthread_mutex_t*)m_hMutex, 0); }
Mutex::~Mutex()                                     { pthread_mutex_destroy((pthread_mutex_t*)m_hMutex); free(m_hMutex); }
void Mutex::Lock()                                  { pthread_mutex_lock((pthread_mutex_t*)m_hMutex); }
void Mutex::Unlock()                                { pthread_mutex_unlock((pthread_mutex_t*)m_hMutex); }

void* StartThreadProc(void* lpParameter)            { delete (IFunctor*)lpParameter; return 0; }
void Thread::Run(IFunctor* pFunc)                   { if (!t) t = malloc(sizeof(pthread_t)); pthread_create((pthread_t*)t, 0, &StartThreadProc, pFunc); }
void Thread::Join()                                 { pthread_join(*(pthread_t*)t, 0); free(t); t=0; }
void Thread::Sleep(unsigned int milliseconds)       { usleep(milliseconds * 1000); }
#endif


///////// COMMON FUNCTIONS ////////////////////////////////////////////////////

unsigned char* LoadFile(const char* pFileName, uint &fileSizeInBytes)
{
    // Load the file
    FILE* file = fopen(pFileName, "rb");
    mini3d_assert(file != 0, "Failed to open the file \"%s\". File not found!", pFileName);
    fseek(file , 0 , SEEK_END);
    fileSizeInBytes = (uint)ftell(file);
    rewind(file);
    mini3d_assert(fileSizeInBytes > 0, "Failed to load the file \"%s\". It does not contain any data!", pFileName);
    unsigned char* pBuffer = new unsigned char[fileSizeInBytes];
    fread(&pBuffer[0], 1, (size_t)fileSizeInBytes, file);
    fclose(file);
    return pBuffer;
}

stb_vorbis* GetVorbis(const unsigned char* pFiledata, uint fileSizeInBytes)
{
    int error = VORBIS__no_error;
    stb_vorbis* pVorbis = stb_vorbis_open_memory((unsigned char*)pFiledata, fileSizeInBytes, &error, 0);
    mini3d_assert(error == VORBIS__no_error, "Failed to crate new vorbis stream: %d. Is the file an error-free OGG file?", error);
    return pVorbis;
}



///////// SOUND BUFFER ////////////////////////////////////////////////////////

SoundBuffer::SoundBuffer(const char* pFilename)
{
    uint fileSizeInBytes;
    unsigned char* pFileData = LoadFile(pFilename, fileSizeInBytes);

    stb_vorbis* pVorbis = GetVorbis(pFileData, fileSizeInBytes);
    stb_vorbis_info info = stb_vorbis_get_info(pVorbis);
    m_desc.channelCount = info.channels;
    m_desc.sampleRate = info.sample_rate;
    uint sampleCount = stb_vorbis_stream_length_in_samples(pVorbis);

    m_pBuffer = new float[sampleCount];
    m_desc.lengthInFrames = stb_vorbis_get_samples_float_interleaved(pVorbis, m_desc.channelCount, m_pBuffer, sampleCount);
    stb_vorbis_close(pVorbis);

    delete[] pFileData;
}

SoundBuffer::SoundBuffer(const float* pSoundData, uint lengthInFrames, uint channelCount, uint sampleRate)
{
    m_desc.channelCount = channelCount;
    m_desc.sampleRate = sampleRate;
    m_desc.lengthInFrames = lengthInFrames;

    m_pBuffer = new float[lengthInFrames * channelCount];
    for (uint i = 0; i < lengthInFrames * channelCount; ++i)
        m_pBuffer[i] = pSoundData[i];
}

double SoundBuffer::_internal_get_frame_pointer(float** pBuffer, const double positionInFrames, double requestedFrames)
{
    *pBuffer = 0;
    double servedFrames = requestedFrames;

    // If less than one frame left
    if (positionInFrames + m_desc.channelCount > m_desc.lengthInFrames)
        return 0;

    if (positionInFrames + requestedFrames >= m_desc.lengthInFrames)
        servedFrames = m_desc.lengthInFrames - positionInFrames;

    *pBuffer = m_pBuffer + (uint)positionInFrames * m_desc.channelCount;
    return servedFrames;
}



///////// SOUND STREAM ////////////////////////////////////////////////////////

StreamBuffer::StreamBuffer() { Init(); }

StreamBuffer::StreamBuffer(const char* pFilename)
{ 
    m_pData = (const char*)LoadFile(pFilename, m_dataLengthInBytes); 
    Init(); 
}

StreamBuffer::StreamBuffer(const char* pSoundData, uint sizeInBytes) :
    m_pData(pSoundData), 
    m_dataLengthInBytes(sizeInBytes)
{
    Init();
}

void StreamBuffer::Init()
{
    m_shutDown = false;
    m_writePosition_f = 0;
    m_nextReadPosition_f = 0;
    m_readPosition_f = 0;
    m_streamHasEnded = false;

    m_pVorbis = GetVorbis((unsigned char*)m_pData, m_dataLengthInBytes);
    stb_vorbis_info info = stb_vorbis_get_info(m_pVorbis);
    m_desc.channelCount = info.channels;
    m_desc.sampleRate = info.sample_rate;
    m_desc.lengthInFrames = stb_vorbis_stream_length_in_samples(m_pVorbis);

    m_pBuffer = new float[STREAM_BUFFER_SIZE_IN_FRAMES * m_desc.channelCount];

    m_decodeThread.Run(new Functor<StreamBuffer>(this, &StreamBuffer::DecodeThreadFunction));
}

double StreamBuffer::_internal_get_frame_pointer(float** pBuffer, const double positionInFrames, double requestedFrames)
{
    *pBuffer = 0;
    double servedFrames = requestedFrames;

    Lock guard(&m_mutex);
    
    // Check if the user has requested a new position &&
    // the new position is not in the existing buffer
    if (positionInFrames != m_nextReadPosition_f)
    {
        m_writePosition_f = (uint)positionInFrames;
        stb_vorbis_seek_frame(m_pVorbis, m_writePosition_f);
    }

    m_readPosition_f = positionInFrames;

    // If less than one frame left
    if (m_streamHasEnded && positionInFrames + m_desc.channelCount > m_writePosition_f)
        return 0;

    // Don't allow reading outside the edge of the buffer
    uint readIndex_f = ((uint)positionInFrames) & (STREAM_BUFFER_SIZE_MODULO);
    uint maxRead_f = STREAM_BUFFER_SIZE_IN_FRAMES - readIndex_f;
    servedFrames = servedFrames < maxRead_f ? servedFrames : maxRead_f;

    // Don't allow reading past the write-position
    maxRead_f =  m_writePosition_f - (uint)positionInFrames;
    servedFrames = servedFrames < maxRead_f ? servedFrames : maxRead_f;

    m_nextReadPosition_f = m_readPosition_f + servedFrames;
    *pBuffer = m_pBuffer + readIndex_f * m_desc.channelCount;
    return servedFrames;
}

void StreamBuffer::DecodeThreadFunction()
{
    uint writeable_f = 0;
    uint sleepPeriod = 1000 * STREAM_BUFFER_SIZE_IN_FRAMES / m_desc.sampleRate / 16; // 1/16 the time it takes to play the whole stream buffer in ms

    float* pBuffer = new float[STREAM_BUFFER_SIZE_IN_FRAMES * m_desc.channelCount];

    for (;;)
    {
        { 
            Lock guard(&m_mutex);
            writeable_f = STREAM_BUFFER_SIZE_IN_FRAMES - (m_writePosition_f - (uint)m_readPosition_f);

            if (m_shutDown == true)
                break;
        }

        if (writeable_f >= STREAM_BUFFER_SIZE_IN_FRAMES >> 2)
        {
            uint framesRead = stb_vorbis_get_samples_float_interleaved(m_pVorbis, m_desc.channelCount, pBuffer, writeable_f * m_desc.channelCount);

            Lock guard(&m_mutex);

            if (framesRead == 0)
                m_streamHasEnded = true;

            for (uint i = 0; i < framesRead; ++i, ++m_writePosition_f)
                for (uint j = 0; j < m_desc.channelCount; ++j)
                    m_pBuffer[(m_writePosition_f & STREAM_BUFFER_SIZE_MODULO) * m_desc.channelCount + j] = pBuffer[i * m_desc.channelCount + j];
        
        }

        Thread::Sleep(sleepPeriod);
    }

    stb_vorbis_close(m_pVorbis);
    delete[] m_pData;
    delete[] pBuffer;
    delete this;
}


///////// SOUND SOURCE ////////////////////////////////////////////////////////

SoundSource::SoundSource(SoundOutput* pOutput, SoundBuffer* pBuffer, SoundFilter* pFilter) :
    m_pOutput(pOutput)
{
    Init();
    
    m_state_user.pBuffer = pBuffer; 
    if (pBuffer) 
        pBuffer->Alloc();
    
    m_state_user.pFilter = pFilter;
    if (pFilter) 
        pBuffer->Alloc();
}

// Always called from the mix-thread of the sound output after all mixing is done!
SoundSource::~SoundSource()
{ 
    if (m_state_user.pBuffer)
        m_state_user.pBuffer->Release();
    
    if (m_state_mix.pBuffer)
         m_state_user.pBuffer->Release();
    
    if (m_state_user.pFilter) 
        m_state_user.pFilter->Release();

    if (m_state_mix.pFilter) 
        m_state_mix.pFilter->Release();
}

void SoundSource::Init()
{
    m_state_user.pBuffer = 0;
    m_state_user.pFilter = 0;
    m_state_user.state = STATE_PAUSED;
    m_state_user.vol = 1.0f; 
    m_state_user.pitch = 1.0f; 
    m_state_user.offset_f = 0.0;
    m_state_user.pos[0] = m_state_user.pos[1] = m_state_user.pos[2] = 0.0f;
    m_state_user.vel[0] = m_state_user.vel[1] = m_state_user.vel[2] = 0.0f;

    m_state_mix = m_state_user;
    m_fadeVol = 1.0f;

    if (!m_pOutput->internal_add(this))
        m_pOutput = 0;
}

void SoundSource::internal_mix()
{
    BufferDesc outputDesc;
    BufferDesc bufferDesc;

    float* pTargetBuffer;
    float pitch;
    float volume;
    float volStep;
    float newFadeVol;
    double stepSize;
    float channelMix[MAX_INPUT_CHANNELS * MAX_OUTPUT_CHANNELS];
    float channelMixPrev[MAX_INPUT_CHANNELS * MAX_OUTPUT_CHANNELS];
    float channelMixStep[MAX_INPUT_CHANNELS * MAX_OUTPUT_CHANNELS];
    uint channelCount;

    {
        Lock guard(&m_mutex);

        // Skip playback if 
        if ((m_state_mix.pBuffer == 0 && m_state_user.pBuffer == 0) || 
           (m_state_mix.state == STATE_PAUSED && m_state_user.state == STATE_PAUSED) ||
           (m_state_mix.state == STATE_STOPPED && m_state_user.state == STATE_STOPPED))
        {
            return;
        }

        // If we are returning from a paused or stopped state, just sync the state_mix with the state_user
        if ((m_state_user.state == STATE_PLAYING) && (m_state_mix.state != STATE_PLAYING))
        {
            m_state_mix.pitch = m_state_user.pitch;
            m_state_mix.pos[0] = m_state_user.pos[0];
            m_state_mix.pos[1] = m_state_user.pos[1];
            m_state_mix.pos[2] = m_state_user.pos[2];
            m_state_mix.vel[0] = m_state_user.vel[0];
            m_state_mix.vel[1] = m_state_user.vel[1];
            m_state_mix.vel[2] = m_state_user.vel[2];
            m_state_mix.vol = m_state_user.vol;
        }

        // CHOOSE BUFFER AND FILTER
        // 1. Use m_state_mix SoundBuffer/SoundFilter if it was not 0 (so that we can fade out if there is a change)
        // 2. If m_state_mix SoundBuffer/SoundFilter was 0, use the m_state_user one instead to make changes take effect right away
        // and alloc it to make sure it is not lost!
        if (m_state_mix.pBuffer == 0 && m_state_user.pBuffer != 0)
        {
            m_state_mix.pBuffer = m_state_user.pBuffer;
            m_state_mix.pBuffer->Alloc();
        }

        if (m_state_mix.pFilter == 0 && m_state_user.pFilter != 0)
        {
            m_state_mix.pFilter = m_state_user.pFilter;
            m_state_mix.pFilter->Alloc();
        }

        // FADING IN AND OUT
        newFadeVol = 1.0f;

        // Fade out if
        // 1. We have a new SoundBuffer
        // 2. We have a new SoundFilter
        // 3. We have jumped to a new playback position
        // 4. Playback has been stopped
        // 5. Playback has been paused
        if ((m_state_mix.pBuffer != m_state_user.pBuffer) ||
            (m_state_mix.pFilter != m_state_user.pFilter) ||
            (m_state_mix.offset_f != m_state_user.offset_f) ||
            ((m_state_mix.state == STATE_PLAYING) && (m_state_user.state != STATE_PLAYING)))
        {
            newFadeVol = 0.0f;
        }

        outputDesc = m_pOutput->GetDescription();
        bufferDesc = m_state_mix.pBuffer->GetDescription();

        // Set the step by which we will increase (or decrease if negative) the previous volume each frame during the mix
        volume = (m_state_mix.vol * m_fadeVol);
        volStep = ((m_state_user.vol * newFadeVol) - volume) / outputDesc.lengthInFrames;

        // CALCULATE CHANNEL MIXING FROM POSITION AND VELOCITY
        // Get the channel mix setup. An array describing how much of each input channel goes into each outputchannel.
        CalculateChannelMix(m_state_mix.pos, m_state_mix.vel, channelMixPrev, pitch, bufferDesc.channelCount, outputDesc.channelCount);
        CalculateChannelMix(m_state_user.pos, m_state_user.vel, channelMix, pitch, bufferDesc.channelCount, outputDesc.channelCount);

        channelCount = bufferDesc.channelCount * outputDesc.channelCount;
        for (uint i = 0; i < channelCount; ++i)
            channelMixStep[i] = (channelMix[i] - channelMixPrev[i]) / outputDesc.lengthInFrames;

        // CALCULATE PITCH AND STEP SIZE
        // Set how big of a step we take in the input buffer for each frame in the input buffer (for pitch and doppler effects)
        stepSize = ((double)bufferDesc.sampleRate / (double)outputDesc.sampleRate) * m_state_user.pitch * pitch;

        // Update mix state (except buffer, filter and offset which are handled later)
        m_state_mix.pitch = m_state_user.pitch;
        m_state_mix.pos[0] = m_state_user.pos[0];
        m_state_mix.pos[1] = m_state_user.pos[1];
        m_state_mix.pos[2] = m_state_user.pos[2];
        m_state_mix.state = m_state_user.state;
        m_state_mix.vel[0] = m_state_user.vel[0];
        m_state_mix.vel[1] = m_state_user.vel[1];
        m_state_mix.vel[2] = m_state_user.vel[2];
        m_state_mix.vol = m_state_user.vol;
    }

    pTargetBuffer = (m_state_mix.pFilter) ? m_state_mix.pFilter->internal_get_buffer() : m_pOutput->internal_get_buffer();

    uint outChannels = outputDesc.channelCount;
    uint inChannels = bufferDesc.channelCount;

    double requested_f = outputDesc.lengthInFrames * stepSize;
    double recieved_f = 0;
    float* pSamples;
    double readCount_f = 0;
    
    // While we need more frames to fill the period && the stream has not ended (pSamples != 0)
    // TODO: Cleanupt
    for (;;)
    {
        recieved_f = m_state_mix.pBuffer->_internal_get_frame_pointer(&pSamples, m_state_mix.offset_f + readCount_f, requested_f);

        if (pSamples == 0 || recieved_f == 0)
            break;

        // Dont do the mixing if volume is too low
        // TODO: Check the distance to the source also when determining this!
        if ((abs(volume) > 0.001) || (abs(volStep) > 0.001 / outputDesc.lengthInFrames))
        {
            for (double frame = 0; frame < recieved_f; frame += stepSize)
            {
                // TODO: make frame fixed point to increase performance if needed (...and it has any effect!)
                uint inIndex = (uint)frame * inChannels;
                for (uint inChannel = 0; inChannel < inChannels; ++inChannel)
                {
                    double sample = pSamples[inIndex + inChannel] * volume;
                    float* pChannelMix = channelMixPrev + (inChannel * outChannels);
                    for (uint outChannel = 0; outChannel < outChannels; ++outChannel)
                        pTargetBuffer[outChannel] += (float)(sample * pChannelMix[outChannel]);
                }
                pTargetBuffer += outChannels;
                volume += volStep;

                for (uint i = 0; i < channelCount; ++i)
                    channelMixPrev[i] += channelMixStep[i];
            }
        }

        readCount_f += recieved_f;
        requested_f -= recieved_f;

        if (requested_f <= 0)
            break;
    }

    {
        Lock guard(&m_mutex);

        // Release any old buffers or filters
        if (m_state_mix.pBuffer != m_state_user.pBuffer)
        {
            m_state_mix.pBuffer->Release();
            m_state_mix.pBuffer = 0;
        }

        if (m_state_mix.pFilter != m_state_user.pFilter)
        {
            m_state_mix.pFilter->Release();
            m_state_mix.pFilter = 0;
        }

        // Either step forward readCount_f steps or make a jump in the playback
        if (m_state_user.offset_f == m_state_mix.offset_f)
        {
            m_state_user.offset_f += readCount_f;
            m_state_mix.offset_f += readCount_f;
        }
        else
        {
            m_state_mix.offset_f = m_state_user.offset_f;
        }
    }
    
    m_fadeVol = newFadeVol;

    // Check if we have reached the end of the sound
    if (pSamples == 0 || m_state_mix.offset_f > bufferDesc.lengthInFrames)
        Stop();
}


///////// SOUND OUTPUT ///////////////////////////////////////////////////////

SoundOutput::SoundOutput(unsigned int channels, unsigned int sampleRate) : 
    m_vol(1.0),
    m_shutDown(false),
    m_filterCount(0),
    m_sourceCount(0)
{
    m_pService = new SoundService(channels, sampleRate);
    m_mixThread.Run(new Functor<SoundOutput>(this, &SoundOutput::MixThreadFunction));

}

bool SoundOutput::internal_add(SoundSource* pSource)
{ 
    LOCKED; 

    mini3d_assert(m_sourceCount < MAX_TOTAL_SOUND_SOURCES, 
        "Overflowing sound source array in sound output.\n"
        "Please make sure you are realeasing sound sources correctly. "
        "If you are, consider increasing the MAX_TOTAL_SOUND_SOURCES constant.");

    if (m_sourceCount < MAX_TOTAL_SOUND_SOURCES) 
    { 
        m_pSources[m_sourceCount++] = pSource; pSource->Alloc(); 
        return true; 
    } 

    return false; 
}

bool SoundOutput::internal_add(SoundFilter* pFilter)
{ 
    LOCKED; 

    mini3d_assert(m_filterCount < MAX_TOTAL_SOUND_FILTERS, 
        "Overflowing sound filter array in sound output."
        "Please make sure you are realeasing sound filters correctly. "
        "If you are, consider increasing the MAX_TOTAL_SOUND_FILTERS constant.");

    if (m_filterCount < MAX_TOTAL_SOUND_FILTERS) 
    { 
        m_pFilters[m_filterCount++] = pFilter; pFilter->Alloc(); 
        return true; 
    } 

    return false; 
}


void SoundOutput::MixThreadFunction()
{
    BufferDesc desc = m_pService->GetDescription();
    uint periodSize_s = desc.lengthInFrames * desc.channelCount;

    float volume;
    uint count;

    m_pPeriodBuffer = new float[periodSize_s];

    while(m_shutDown == false)
    {
        // Wait for free output-buffer
        short* pSoundServiceBuffer = m_pService->GetNextPeriodBuffer();

        for (unsigned int i = 0; i < periodSize_s; ++i) 
            m_pPeriodBuffer[i] = 0.0f;

        { LOCKED; count = m_sourceCount; }
        for (uint i = 0; i < count; ++i)
        {
            m_pSources[i]->internal_mix();
            
            if ((m_pSources[i]->internal_get_alloc_count() == 1) && (m_pSources[i]->internal_get_has_stopped()))
            {
                m_pSources[i]->Release();
                { LOCKED; m_pSources[i] = m_pSources[--m_sourceCount]; }
            }
        }

        { LOCKED; count = m_filterCount; }
        for (int i = m_filterCount - 1; i >= 0; --i)
        {
            m_pFilters[i]->internal_mix();

            if ((m_pFilters[i]->internal_get_alloc_count() == 1))
            {
                m_pFilters[i]->Release();
                { LOCKED; m_pFilters[i] = m_pFilters[--m_filterCount]; }
            }
        }

        // Apply volume control
        { Lock guard(&m_mutex); volume = m_vol; }

        // If the volume will make a difference
        if (abs(volume - 1.0) > 0.0001)
            for (uint i = 0; i < periodSize_s; ++i)
                m_pPeriodBuffer[i] *= volume;

        // compress the sound into the next audio buffer
        for (unsigned int i = 0; i < periodSize_s; ++i)
            pSoundServiceBuffer[i] = (short)(atanf(m_pPeriodBuffer[i] * 0.5f) * 1.95f / 3.141592653f * SHRT_MAX);

        // send to sound card buffer
        m_pService->AddPeriodBufferToQueue(pSoundServiceBuffer);
    }

    for (uint i = 0; i < m_sourceCount; ++i)
        m_pSources[i]->Release();

    for (uint i = 0; i < m_filterCount; ++i)
        m_pFilters[i]->Release();

    delete m_pService;
    delete m_pPeriodBuffer;
    delete this;
}




///////// SOUND FILTER ////////////////////////////////////////////////////////

SoundFilter::SoundFilter(SoundOutput* pOutput) :
    m_pOutput(pOutput),
    m_pFilter(0)
{ 
    BufferDesc desc = pOutput->GetDescription();
    m_pBuffer = new float[desc.lengthInFrames * desc.channelCount];

    uint size_s = desc.lengthInFrames * desc.channelCount;

    for(uint i = 0; i < size_s; ++i)
        m_pBuffer[i] = 0.0f;

    if (!m_pOutput->internal_add(this))
        m_pOutput = 0;
}

void SoundFilter::internal_mix()
{
    ApplyFilter();

    float* pPeriodBuffer = (m_pFilter) ? m_pFilter->internal_get_buffer() : m_pOutput->internal_get_buffer();

    BufferDesc desc = m_pOutput->GetDescription();
    uint size_s = desc.lengthInFrames * desc.channelCount;

    for(uint i = 0; i < size_s; ++i)
    {
        pPeriodBuffer[i] += m_pBuffer[i];
        m_pBuffer[i] = 0;
    }
}
