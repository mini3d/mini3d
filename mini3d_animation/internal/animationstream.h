
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under t    he MIT Software License <www.mini3d.org/license.php>

#ifndef MINI3D_ANIMATION_ACTION_STREAM_H
#define MINI3D_ANIMATION_ACTION_STREAM_H

#include <stdint.h>

// Wraps the byte array into a Animation Stream object so
// we can read from it using the >> operator;
class AnimationStream
{
public:
    AnimationStream(const char* pAnimationStream) : _animation(pAnimationStream), _it(pAnimationStream) {}
    
    AnimationStream& operator>>(uint16_t& val)      { val = *(uint16_t*)_it; _it += sizeof(uint16_t); return *this; }
    AnimationStream& operator>>(float& val)         { val = *(float*)_it; _it += sizeof(float); return *this; }

    
    template <typename T> T Read()                  { T val = *(T*)_it; _it += sizeof(T); return val; }

    template <typename T> void Rewind()             { _it -= sizeof(T); }
    template <typename T> void FastForward()        { _it += sizeof(T); }

    template <typename T> T Peek()                  { return *(T*)_it; }
    void Reset()                                    
    { 
        _it = _animation; 
    }

private:
    const char* _animation;
    const char* _it;
};

#endif // MINI3D_ANIMATION_ACTION_STREAM_H

