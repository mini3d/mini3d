
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_MATH_TRANSFORM_H
#define MINI3D_MATH_TRANSFORM_H

#include "vec4.hpp"
#include "quat.hpp"

namespace mini3d {
namespace math {

// Note that scaling only applies to the most local transform!
// To avoid misstakes only use uniform scaling

// TODO: Switch to this http://wscg.zcu.cz/wscg2012/short/A29-full.pdf and single float for uniform scaling

////////// TRANSFORM //////////////////////////////////////////////////////////

struct Transform
{
    static float ZERO[3][4];
    static float IDENTITY[3][4];
    static float IDENTITY_ZERO_SCALE[3][4];
    static float ZERO_IDENTITY_SCALE[3][4];
    
    Vec4 pos;
    Quat rot;
    Vec4 scale;

    Transform() { };
    Transform(const Transform &t) : pos(t.pos), rot(t.rot), scale(t.scale) { };
    Transform(const float pos[4], const float rot[4], const float scale[4]) : pos(pos), rot(rot), scale(scale) {}
    Transform(const float posRotScale[3][4]) : pos(posRotScale[0]), rot(posRotScale[1]), scale(posRotScale[2]) {}

    Transform operator *(const Transform &t)    { return Transform(*this * t.pos, rot*t.rot, scale*t.scale); } 
    Vec4 operator *(const Vec4 &v) const        { return pos + rot.Transform(v * scale); }    

    void RotateAboutPoint(Vec4 v)               { pos = v - rot.Transform(v); }

    static Transform LookAtRH(const Vec4 eye, const Vec4 at, const Vec4 up)
    {
        Vec4 n = (eye - at).Normalized();
        Vec4 u = up.Cross3(n).Normalized();
        Vec4 v = n.Cross3(u);

        float w = sqrtf(1.0f + u.x + v.y + n.z) * 0.5f;
        float w4 = 1.0f / (4.0f * w);
        
        return Transform(   Vec4(-u.Dot(eye), -v.Dot(eye), -n.Dot(eye), 0), // Position
                            Quat((v.z-n.y) * w4, (n.x-u.z) * w4, (u.y-v.x) * w4, -w), // Rotation
                            Vec4(1,1,1,0)); // Scale
    }

    void ToMatrix(float m[16]) 
    {
        // http://www.flipcode.com/documents/matrfaq.html#Q54

        float xx      = rot.x * rot.x;
        float xy      = rot.x * rot.y;
        float xz      = rot.x * rot.z;
        float xw      = rot.x * rot.w;

        float yy      = rot.y * rot.y;
        float yz      = rot.y * rot.z;
        float yw      = rot.y * rot.w;

        float zz      = rot.z * rot.z;
        float zw      = rot.z * rot.w;

        m[0]  = (1 - 2 * ( yy + zz )) * scale.x;
        m[1]  =     2 * ( xy - zw );
        m[2]  =     2 * ( xz + yw );
        m[3]  = pos.x;

        m[4]  =     2 * ( xy + zw );
        m[5]  = (1 - 2 * ( xx + zz )) * scale.y;
        m[6]  =     2 * ( yz - xw );
        m[4]  = pos.y;

        m[8]  =     2 * ( xz - yw );
        m[9]  =     2 * ( yz + xw );
        m[10] = (1 - 2 * ( xx + yy )) * scale.z;
        m[12] = pos.z;

        m[12] = m[13] = m[14] = 0;
        m[15] = 1;
    }
};

float Transform::ZERO[3][4] = {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}};
float Transform::IDENTITY[3][4] = {{0,0,0,0}, {0,0,0,1}, {1,1,1,1}};
float Transform::IDENTITY_ZERO_SCALE[3][4] = {{0,0,0,0}, {0,0,0,1}, {0,0,0,0}};
float Transform::ZERO_IDENTITY_SCALE[3][4] = {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}};

}
}

#endif