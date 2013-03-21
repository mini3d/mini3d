
// Copyright (c) <2009-2013> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_MINI3DANIMATIONDATATYPES_H
#define MINI3D_MINI3DANIMATIONDATATYPES_H

namespace mini3d {
namespace animation {

////////// ANIMATABLE TYPES ///////////////////////////////////////////////////

// Built in datatypes such as float and int can be animated just as they are
// To use other data-types such as 32-bit color values, vectors or quaternions
// they need to support the following operations:
   
// * T operator +(T rhs);
// * T operator -(T rhs);
// * T operator *(float scalar);
// * T operator /(float scalar);

// This is here as an example. You can use the data types in mini3d::math for
// animation if you want to!

struct float4 { 
    float x,y,z,w;

    float4() : x(0), y(0), z(0), w(0) {}
    float4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    float4 operator*(float s)     { return float4(x*s, y*s, z*s, w*s); }
    float4 operator/(float s)     { return float4(x/s, y/s, z/s, w/s); }
    float4 operator+(float4 v)    { return float4(x+v.x, y+v.y, z+v.z, w+v.w); }
    float4 operator-(float4 v)    { return float4(x-v.x, y-v.y, z-v.z, w-v.w); }
};


}
}

#endif