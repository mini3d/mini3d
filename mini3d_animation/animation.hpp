
// Copyright (c) <2009-2013> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_MINI3DANIMATION_H
#define MINI3D_MINI3DANIMATION_H

namespace mini3d {
namespace animation {

////////// ANIMATABLE TYPES ///////////////////////////////////////////////////

// Types used as animation targets must have the following member functions
// Default constructor that zero initializes!
// T operator +(T rhs);
// T operator -(T rhs);
// T operator *(float scalar);
// T operator /(float scalar);


struct Vec4a { 
    float x,y,z,w;

    Vec4a() : x(0), y(0), z(0), w(0) {}
    Vec4a(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vec4a operator*(float s)    { return Vec4a(x*s, y*s, z*s, w*s); }
    Vec4a operator/(float s)    { return Vec4a(x/s, y/s, z/s, w/s); }
    Vec4a operator+(Vec4a v)     { return Vec4a(x+v.x, y+v.y, z+v.z, w+v.w); }
    Vec4a operator-(Vec4a v)     { return Vec4a(x-v.x, y-v.y, z-v.z, w-v.w); }
};


////////// ANIMATION //////////////////////////////////////////////////////////

template <typename T> struct Keyframe { float time; T value; };

struct ITrack { virtual void Update(float time, float weight = 1.0f) = 0; };

template <typename T> struct Track : ITrack { 

    Track(T* pTarget, Keyframe<T>* keyframes, unsigned int count);
    void Update(float time, float weight = 1.0f);

private:
    void UpdateIntervalCache();
    float Clamp(float value, float min, float max) { return (value < min) ? min : (value > max) ? max : value; }


private:
    T* pTarget;
    Keyframe<T>* kf;
    unsigned int count;
    unsigned int index; // index of current keyframe interval

    // Cache for current keyframe interval
    float startTime;
    float endTime;
    float intervalStartTime;
    float intervalEndTime;
    float invIntervalLength;
    T m[2]; // derivatives at the edges of the current interval
    T p[2]; // values at the edges of the current interval

};


////////// ANIMATION //////////////////////////////////////////////////////////

class Animation
{
public:

    Animation(ITrack** pUpdatables, unsigned int count, float length);
    ~Animation();

    float GetLength() const                         { return m_length; }
    float GetPosition() const                       { return m_time; }
    void SetPosition(float time)                    { m_time = time; }
    
    void Update(float time, float weight = 1.0f);

private:
    ITrack ** m_pUpdatables;
    float m_time;

    float m_length;
};

template <typename T>
Track<T>::Track(T* pTarget, Keyframe<T>* keyframes, unsigned int count) : pTarget(pTarget), kf(keyframes), count(count), index(1)
{
    startTime = kf[0].time;
    endTime = kf[count - 1].time;

    UpdateIntervalCache();
}

template <typename T>
void Track<T>::Update(float time, float weight = 1.0f)
{
    time = Clamp(time, startTime, endTime);

    // if time has started over (if for example we have looped back to the beginning)
    if (time < intervalStartTime)
    {
        index = 0;
        UpdateIntervalCache();
    }

    // if we have left the current keyframe interval
    if (time > intervalEndTime)
    {
        // Fast forward to the correct keyframe interval
        while(time > kf[index].time)
            ++index;

        UpdateIntervalCache();
    }

    // Catmull-Rom spline interpolation of keyframes
    // http://en.wikipedia.org/wiki/Cubic_Hermite_spline

    float t = (time - intervalStartTime) * invIntervalLength;
    float t2 = t*t;
    float t3 = t2*t;
        
    float h00 =  2*t3 - 3*t2 + 1;
    float h10 =    t3 - 2*t2 + t;
    float h01 = -2*t3 + 3*t2 ;
    float h11 =    t3 - t2;

    *pTarget = p[0] * h00 + m[0] * h10 + p[1] * h01 + m[1] * h11;
}

template <typename T>
void Track<T>::UpdateIntervalCache()
{
    intervalEndTime = kf[index].time;
    intervalStartTime = kf[index - 1].time;

    float intervalLength = intervalEndTime - intervalStartTime;
    invIntervalLength = (intervalLength != 0) ? 1 / intervalLength : 0;

    // keyframe values
    p[0] = kf[index - 1].value;
    p[1] = kf[index].value;

    // keyframe derivatives
    m[0] = m[1] = T();
        
    if (index > 1)
    {
        float m0Length = (kf[index].time - kf[index - 2].time);
        m[0] = m0Length ? ((kf[index].value - kf[index - 2].value) / m0Length) * intervalLength : T();
    }

    if (index < count - 1)
    {
        float m1Length = (kf[index + 1].time - kf[index - 1].time);
        m[1] = m1Length ? ((kf[index + 1].value - kf[index - 1].value) / m1Length) * intervalLength : T();
    }
}

}
}

#endif
