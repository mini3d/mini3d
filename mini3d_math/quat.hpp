
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_MATH_QUAT_H
#define MINI3D_MATH_QUAT_H

#include "vec4.hpp"

namespace mini3d {
namespace math {

////////// QUATERNION /////////////////////////////////////////////////////////

class Quat
{
public:

    float x, y, z, w;

    inline Quat()                                                              {}
    inline Quat(float s) : x(s), y(s), z(s), w(s)                              {} 
    inline Quat(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)   {} 
    inline Quat(const float q[4]) : x(q[0]), y(q[1]), z(q[2]), w(q[3])         {}
    inline Quat(const Quat &q) : x(q.x), y(q.y), z(q.z), w(q.w)                {}

    static Quat FromAxisAngle(float x, float y, float z, float a)
    {
        // http://www.flipcode.com/documents/matrfaq.html#Q56

        float sin_a = sin( a * 0.5f );
        float cos_a = cos( a * 0.5f );

        return Quat( x * sin_a, y * sin_a, z * sin_a, cos_a).Normalized();
    }

	inline operator float*()                                            { return &x; }
	inline operator const float*() const                                { return &x; }

    inline const bool operator == ( const Quat &q ) const               { return (q.x==x && q.y==y && q.z == z && q.w == w); }
	inline const bool operator != ( const Quat &q ) const               { return !(*this == q); }

    inline const Quat operator -() const                                { return Quat(-x, -y, -z, -w); }

    inline float operator [](int index) const                           { return *(&x + index); }

    inline const Quat& operator +=(const Quat &q)                       { x += q.x, y += q.y, z += q.z, w += q.w; return *this; }
    inline const Quat& operator -=(const Quat &q)                       { x -= q.x, y -= q.y, z -= q.z, w -= q.w; return *this; }
    inline const Quat& operator *=(const Quat &q)                       { *this = this->operator *(q); return *this; }
    inline const Quat& operator /=(const Quat &q)                       { *this = this->operator /(q); return *this; }
    inline const Quat& operator *=(float s)                             { x *= s, y *= s, z *= s, w *= s; return *this; }
    inline const Quat& operator /=(float s)                             { x /= s, y /= s, z /= s, w /= w; return *this; }

    inline const Quat operator +(const Quat &q2) const                  { return Quat(x + q2.x, y + q2.y, z + q2.z, w + q2.w); }
    inline const Quat operator -(const Quat &q2) const                  { return Quat(x - q2.x, y - q2.y, z - q2.z, w - q2.w); }
    inline const Quat operator *(const Quat &q) const                   { return Quat(w*q.x+x*q.w+y*q.z-z*q.y, w*q.y-x*q.z+y*q.w+z*q.x, w*q.z+x*q.y-y*q.x+z*q.w, w*q.w-x*q.x-y*q.y-z*q.z); }
    inline const Quat operator /(const Quat &q) const                   { return operator *(q.Conjugated()); }

    inline const Quat operator *(float s) const                         { return Quat(x * s, y * s, z * s, w * s); }
    inline const Quat operator /(float s) const                         { return Quat(x / s, y / s, z / s, w / s); }

    inline Vec4 Transform(const Vec4 v) const                           { Vec4 r(x,y,z,0); return v + (r+r).Cross3(r.Cross3(v) + v*w); }
    inline static Vec4 Transform(const Quat &q, const Vec4 &v)          { return q.Transform(v); }

    inline void Conjugate()                                             { x = -x, y = -y, z = -z; }
    inline const Quat Conjugated() const                                { return Quat(-x, -y, -z, w); }
    inline void Negate()                                                { x = -x, y = -y, z = -z, w = -w; }
    inline const Quat Negated() const                                   { return Quat(-x, -y, -z, -w); }
    inline void Normalize()                                             { float s = sqrt(x*x + y*y + z*z + w*w); x /= s, y /= s, z /= s, w /= s; }
    inline const Quat Normalized() const                                { float s = sqrt(x*x + y*y + z*z + w*w); return Quat(x / s, y / s, z / s, w / s); }

    // TODO: Assignment operator
    // TODO: Near equals
};

inline const Quat operator *(float s, const Quat &q)                    { return q * s; }
inline const Quat operator /(float s, const Quat &q)                    { return q / s; }

}
}

#endif