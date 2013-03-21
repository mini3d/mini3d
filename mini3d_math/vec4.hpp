
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_MATH_VEC4_H
#define MINI3D_MATH_VEC4_H

// TODO: Near equals

#include <cmath>

namespace mini3d {
namespace math {


////////// VECTOR 4 ///////////////////////////////////////////////////////////

class Vec4
{
public:

    float x, y, z, w;

    inline Vec4()                                                              {}
    inline Vec4(float s) : x(s), y(s), z(s), w(s)                              {} 
    inline Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)   {} 
    inline Vec4(const float v[4]) : x(v[0]), y(v[1]), z(v[2]), w(v[3])         {}
    inline Vec4(float v[4]) : x(v[0]), y(v[1]), z(v[2]), w(v[3])               {}
    inline Vec4(const Vec4 &v) : x(v.x), y(v.y), z(v.z), w(v.w)                {}

	inline operator float*()                                            { return &x; }
	inline operator const float*() const                                { return &x; }

    inline const bool operator == ( const Vec4 &v ) const               { return (v.x==x && v.y==y && v.z == z && v.w == w); }
	inline const bool operator != ( const Vec4 &v ) const               { return !(*this == v); }

    

    inline const Vec4 operator -() const                                { return Vec4(-x, -y, -z, -w); }

    inline float operator [](int index) const                           { return *(&x + index); }

    //inline const Vec4& operator = (const Vec4 &v)                       { x = v.x, y = v.y, z = v.z, w = v.w; return *this; }
    inline const Vec4& operator +=(const Vec4 &v)                       { x += v.x, y += v.y, z += v.z, w += v.w; return *this; }
    inline const Vec4& operator -=(const Vec4 &v)                       { x -= v.x, y -= v.y, z -= v.z, w -= v.w; return *this; }
    inline const Vec4& operator *=(const Vec4 &v)                       { x *= v.x, y *= v.y, z *= v.z, w *= v.w; return *this; }
    inline const Vec4& operator /=(const Vec4 &v)                       { x /= v.x, y /= v.y, z /= v.z, w /= v.w; return *this; }
    inline const Vec4& operator *=(float s)                             { x *= s, y *= s, z *= s, w *= s; return *this; }
    inline const Vec4& operator /=(float s)                             { x /= s, y /= s, z /= s, w /= w; return *this; }

    inline const Vec4 operator +(const Vec4 &v2) const                  { return Vec4(x + v2.x, y + v2.y, z + v2.z, w + v2.w); }
    inline const Vec4 operator -(const Vec4 &v2) const                  { return Vec4(x - v2.x, y - v2.y, z - v2.z, w - v2.w); }
    inline const Vec4 operator *(const Vec4 &v) const                   { return Vec4(x * v.x, y * v.y, z * v.z, w * v.w); }
    inline const Vec4 operator /(const Vec4 &v) const                   { return Vec4(x / v.x, y / v.y, z / v.z, w / v.w); }

    inline const Vec4 operator *(float s) const                         { return Vec4(x * s, y * s, z * s, w * s); }
    inline const Vec4 operator /(float s) const                         { return Vec4(x / s, y / s, z / s, w / s); }

    inline float Dot(const Vec4 v) const                                { return x * v.x + y * v.y + z * v.z + w * v.w; }
    inline static float Dot(const Vec4 &v1, const Vec4 &v2)             { return v1.Dot(v2); }

    inline Vec4 Cross3(const Vec4 v) const                              { return Vec4(y*v.z - v.y*z, z*v.x - v.z*x, x*v.y - v.x*y, 0); }
    inline static Vec4 Cross3(const Vec4 &v1, const Vec4 &v2)           { return v1.Cross3(v2); }

    inline void Negate()                                                { x = -x, y = -y, z = -z, w = -w; }
    inline const Vec4 Negated() const                                   { return Vec4(-x, -y, -z, -w); }
    inline float Norm() const                                           { return x*x + y*y + z*z + w*w; }
    inline float Length() const                                         { return sqrt(x*x + y*y + z*z + w*w); }
    inline float Distance(const Vec4 v) const                           { Vec4 diff(x-v.x, y-v.y, z-v.z, w-v.w); return diff.Length(); }
    inline void Normalize()                                             { float s = sqrt(x*x + y*y + z*z + w*w); x /= s, y /= s, z /= s, w /= w; }
    inline const Vec4 Normalized() const                                { float s = sqrt(x*x + y*y + z*z + w*w); return Vec4(x / s, y / s, z / s, w / s); }

};

inline const Vec4 operator *(float s, const Vec4 &v)                    { return v * s; }
inline const Vec4 operator /(float s, const Vec4 &v)                    { return v / s; }

}
}

#endif