
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license>

#ifdef _WIN32
#include "soundservice_win32_waveout.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <cstdlib>

// TODO: Is this included with visual studio?
#include <audiodefs.h>

//#define WAVE_FORMAT_EXTENSIBLE 0xFFFE;

void mini3d_assert(bool expression, const char* text, ...);

using namespace mini3d::sound; 

const unsigned int SOUND_WIN32_WAVEOUT_BUFFER_SIZE_IN_FRAMES = 2048;

SoundService_win32_waveOut::SoundService_win32_waveOut(unsigned int channels, unsigned int sampleRate) : 
    m_phWaveOut(0),
    m_nextBufferIndex(0)
{
    m_desc.channelCount = channels;
    m_desc.sampleRate = sampleRate;
    m_desc.lengthInFrames = SOUND_WIN32_WAVEOUT_BUFFER_SIZE_IN_FRAMES;

    WAVEFORMATEX wfx;
    wfx.nSamplesPerSec = m_desc.sampleRate;
    wfx.wBitsPerSample = 16;
    wfx.nChannels = m_desc.channelCount;
    wfx.wFormatTag = (channels > 2) ? WAVE_FORMAT_EXTENSIBLE : WAVE_FORMAT_PCM;
    wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
    wfx.cbSize = 32; /* size of _extra_ info */

    WAVEFORMATEXTENSIBLE wfext;
    wfext.Format = wfx;
    wfext.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    wfext.Samples.wReserved = 0;
    wfext.Samples.wValidBitsPerSample = 0;
    wfext.Samples.wSamplesPerBlock = 16;
    wfext.dwChannelMask = 0;

    // Please see: WAVEFORMATEXTENSIBLE documentation
    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd757714

    if (m_desc.channelCount == 6)
        wfext.dwChannelMask = 0x1 | 0x2 | 0x4 | 0x8 | 0x10 | 0x20;
    else if (m_desc.channelCount == 8)
        wfext.dwChannelMask = 0x1 | 0x2 | 0x4 | 0x8 | 0x10 | 0x20 | 0x200 | 0x400;
    else
        wfext.dwChannelMask = 0;

    m_phWaveOut = malloc(sizeof(HWAVEOUT));

    MMRESULT result;
    while (MMSYSERR_NOERROR != (result = waveOutOpen((HWAVEOUT*)m_phWaveOut, WAVE_MAPPER, (WAVEFORMATEX*)&wfext, 0, 0, CALLBACK_NULL)))
    {
        --m_desc.channelCount;

        if (m_desc.channelCount == 0)
            break;
    }
    mini3d_assert(result == MMSYSERR_NOERROR, "unable to open WAVE_MAPPER device\n");

    unsigned int periodSize_s = SOUND_WIN32_WAVEOUT_BUFFER_SIZE_IN_FRAMES * m_desc.channelCount;

    for (unsigned int i = 0; i < SOUND_WIN32_WAVEOUT_BUFFER_COUNT; ++i)
    {
        m_pWaveBuffer[i] = new short[periodSize_s];
        m_pWaveHeader[i] = malloc(sizeof(WAVEHDR));
    }

    m_pMixBuffer = new float[periodSize_s];
}

SoundService_win32_waveOut::~SoundService_win32_waveOut()
{
    while(waveOutClose(*(HWAVEOUT*)m_phWaveOut) == WAVERR_STILLPLAYING);
    free(m_phWaveOut);

    for (unsigned int i = 0; i < SOUND_WIN32_WAVEOUT_BUFFER_COUNT; ++i)
    {
        delete[] m_pWaveBuffer[i];
        free(m_pWaveHeader[i]);
    }

    delete[] m_pMixBuffer;
}

BufferDesc SoundService_win32_waveOut::GetDescription() const { return m_desc; }

short* SoundService_win32_waveOut::GetNextPeriodBuffer()
{
    unsigned int periodSize_s = m_desc.lengthInFrames * m_desc.channelCount;
    unsigned int bufferIndex = m_nextBufferIndex;
    m_nextBufferIndex = (m_nextBufferIndex + 1) % SOUND_WIN32_WAVEOUT_BUFFER_COUNT;

    HWAVEOUT hWaveOut = *(HWAVEOUT*)m_phWaveOut;
    LPWAVEHDR pWaveHdr = (LPWAVEHDR)m_pWaveHeader[m_nextBufferIndex];

    while(waveOutUnprepareHeader(hWaveOut, pWaveHdr, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
        Sleep(2);

    return m_pWaveBuffer[m_nextBufferIndex];
}

void SoundService_win32_waveOut::AddPeriodBufferToQueue(short* pBuffer)
{
    unsigned int periodSize_s = m_desc.lengthInFrames * m_desc.channelCount;

    // compress the sound into the next audio buffer
    for (unsigned int i = 0; i < periodSize_s; ++i)
        m_pWaveBuffer[m_nextBufferIndex][i] = pBuffer[i];

    HWAVEOUT hWaveOut = *(HWAVEOUT*)m_phWaveOut;
    LPWAVEHDR pWaveHdr = (LPWAVEHDR)m_pWaveHeader[m_nextBufferIndex];

    ZeroMemory(pWaveHdr, sizeof(WAVEHDR));
    pWaveHdr->dwBufferLength = periodSize_s * sizeof(short);
    pWaveHdr->lpData = (char*)m_pWaveBuffer[m_nextBufferIndex];

    waveOutPrepareHeader(hWaveOut, pWaveHdr, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, pWaveHdr, sizeof(WAVEHDR));
}


#endif
