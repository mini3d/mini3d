
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_MATH_VEC3_H
#define MINI3D_MATH_VEC3_H

// TODO: Near equals

#include <cmath>

namespace mini3d {
namespace math {


////////// VECTOR 4 ///////////////////////////////////////////////////////////

class Vec3
{
public:

    float x, y, z;

    inline Vec3()                                                       {}
    inline Vec3(float s) : x(s), y(s), z(s)                             {} 
    inline Vec3(float x, float y, float z) : x(x), y(y), z(z)           {} 
    inline Vec3(const float v[3]) : x(v[0]), y(v[1]), z(v[2])           {}
    inline Vec3(float v[3]) : x(v[0]), y(v[1]), z(v[2])                 {}
    inline Vec3(const Vec3 &v) : x(v.x), y(v.y), z(v.z)                 {}

	inline operator float*()                                            { return &x; }
	inline operator const float*() const                                { return &x; }

    inline const bool operator == ( const Vec3 &v ) const               { return (v.x==x && v.y==y && v.z == z); }
	inline const bool operator != ( const Vec3 &v ) const               { return !(*this == v); }

    

    inline const Vec3 operator -() const                                { return Vec3(-x, -y, -z); }

    inline float operator [](int index) const                           { return *(&x + index); }

    //inline const Vec3& operator = (const Vec3 &v)                       { x = v.x, y = v.y, z = v.z, w = v.w; return *this; }
    inline const Vec3& operator +=(const Vec3 &v)                       { x += v.x, y += v.y, z += v.z; return *this; }
    inline const Vec3& operator -=(const Vec3 &v)                       { x -= v.x, y -= v.y, z -= v.z; return *this; }
    inline const Vec3& operator *=(const Vec3 &v)                       { x *= v.x, y *= v.y, z *= v.z; return *this; }
    inline const Vec3& operator /=(const Vec3 &v)                       { x /= v.x, y /= v.y, z /= v.z; return *this; }
    inline const Vec3& operator *=(float s)                             { x *= s, y *= s, z *= s; return *this; }
    inline const Vec3& operator /=(float s)                             { x /= s, y /= s, z /= s; return *this; }

    inline const Vec3 operator +(const Vec3 &v2) const                  { return Vec3(x + v2.x, y + v2.y, z + v2.z); }
    inline const Vec3 operator -(const Vec3 &v2) const                  { return Vec3(x - v2.x, y - v2.y, z - v2.z); }
    inline const Vec3 operator *(const Vec3 &v) const                   { return Vec3(x * v.x, y * v.y, z * v.z); }
    inline const Vec3 operator /(const Vec3 &v) const                   { return Vec3(x / v.x, y / v.y, z / v.z); }

    inline const Vec3 operator *(float s) const                         { return Vec3(x * s, y * s, z * s); }
    inline const Vec3 operator /(float s) const                         { return Vec3(x / s, y / s, z / s); }

    inline float Dot(const Vec3 v) const                                { return x * v.x + y * v.y + z * v.z; }
    inline static float Dot(const Vec3 &v1, const Vec3 &v2)             { return v1.Dot(v2); }

    inline Vec3 Cross(const Vec3 v) const                               { return Vec3(y*v.z - v.y*z, z*v.x - v.z*x, x*v.y - v.x*y); }
    inline static Vec3 Cross(const Vec3 &v1, const Vec3 &v2)            { return v1.Cross(v2); }

    inline void Negate()                                                { x = -x, y = -y, z = -z; }
    inline const Vec3 Negated() const                                   { return Vec3(-x, -y, -z); }
    inline float Norm() const                                           { return x*x + y*y + z*z; }
    inline float Length() const                                         { return sqrt(x*x + y*y + z*z); }
    inline float Distance(const Vec3 v) const                           { Vec3 diff(x-v.x, y-v.y, z-v.z); return diff.Length(); }
    inline void Normalize()                                             { float s = sqrt(x*x + y*y + z*z); x /= s, y /= s, z /= s; }
    inline const Vec3 Normalized() const                                { float s = sqrt(x*x + y*y + z*z); return Vec3(x / s, y / s, z / s); }

};

inline const Vec3 operator *(float s, const Vec3 &v)                    { return v * s; }
inline const Vec3 operator /(float s, const Vec3 &v)                    { return v / s; }

}
}

#endif