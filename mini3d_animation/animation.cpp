
// Copyright (c) <2009-2013> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#include "animation.hpp"

using namespace mini3d::animation;

Animation::Animation(ITrack** pUpdatables, unsigned int count, float length) : m_pUpdatables(pUpdatables), m_count(count), m_length(length)
{
    m_state = STATE_STOPPED;
}

void Animation::Play()
{
    if (m_state == STATE_STOPPED || m_time > m_length)
        SetPosition(0);

    m_state = STATE_PLAYING;
}

void Animation::Pause()
{
    m_state = STATE_PAUSED;
}

void Animation::Stop()
{
    m_state = STATE_STOPPED;
}

void Animation::Update(float timeStep, float weight)
{ 
    m_time += timeStep;

    if (m_state == STATE_PLAYING)
    {
        for (unsigned int i = 0; i < m_count; ++i)
            m_pUpdatables[i]->Update(m_time, weight);
    }
};