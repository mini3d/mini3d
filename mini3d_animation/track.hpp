
// Copyright (c) <2009-2013> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_MINI3DANIMATIONTRACK_H
#define MINI3D_MINI3DANIMATIONTRACK_H

namespace mini3d {
namespace animation {


////////// ANIMATION TRACK /////////////////////////////////////////////////////

template <typename T> struct Keyframe { float time; T value; };

struct ITrack { virtual void Update(float time, float weight = 1.0f) = 0; virtual ~ITrack() {}; };

template <typename T> struct Track : ITrack { 

    Track(T* pTarget, Keyframe<T>* keyframes, unsigned int count);
    ~Track() {};
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
    bool hasM0; // if we are at the first interval, there will be no first derivative!
    bool hasM1; // if we are at the last interval, there will be no derivative at the end!
    T m[2]; // derivatives at the edges of the current interval
    T p[2]; // values at the edges of the current interval
};


////////// TEMPLATE TYPE IMPLEMENTATIONS //////////////////////////////////////

template <typename T>
Track<T>::Track(T* pTarget, Keyframe<T>* keyframes, unsigned int count) : pTarget(pTarget), kf(keyframes), count(count), index(1)
{
    startTime = kf[0].time;
    endTime = kf[count - 1].time;

    UpdateIntervalCache();
}

template <typename T>
void Track<T>::Update(float time, float weight)
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
        
    *pTarget = (p[0] * (2*t3 - 3*t2 + 1)) + (p[1] * (-2*t3 + 3*t2));

    if (hasM0) *pTarget = *pTarget + m[0] * (t3 - 2*t2 + t);
    if (hasM1) *pTarget = *pTarget + m[1] * (t3 - t2);
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
    hasM0 = hasM1 = false;
        
    if (index > 1)
    {
        float m0Length = (kf[index].time - kf[index - 2].time);
        if (m0Length)
        {
            hasM0 = true;
            m[0] = ((kf[index].value - kf[index - 2].value) / m0Length) * intervalLength;
        }
    }

    if (index < count - 1)
    {
        float m1Length = (kf[index + 1].time - kf[index - 1].time);
        if (m1Length)
        {
            hasM1 = true;
            m[1] = ((kf[index + 1].value - kf[index - 1].value) / m1Length) * intervalLength;
        }
        
    }
}

}
}


#endif
