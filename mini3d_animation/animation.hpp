
// Copyright (c) <2009-2013> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_MINI3DANIMATION_H
#define MINI3D_MINI3DANIMATION_H

#include "track.hpp"

namespace mini3d {
namespace animation {


////////// ANIMATION //////////////////////////////////////////////////////////


class Animation
{
public:

    enum State { STATE_PLAYING = 0, STATE_PAUSED = 1, STATE_STOPPED = 2 };

    Animation(ITrack** pUpdatables, unsigned int count, float length);
    ~Animation() {};

    void Play();
    void Stop();
    void Pause();

    float GetLength() const                         { return m_length; }
    float GetPosition() const                       { return m_time; }
    void SetPosition(float time)                    { m_time = time; }
    
    void Update(float timeStep, float weight = 1.0f);

private:
    ITrack ** m_pUpdatables;
    unsigned int m_count;
    float m_time;
    float m_length;
    State m_state;
};

}
}

#endif
