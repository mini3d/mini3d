
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>


#ifndef MINI3D_MATH_TRANSFORM_H
#define MINI3D_MATH_TRANSFORM_H

#include "vec4.hpp"
#include "quat.hpp"

namespace mini3d {
namespace math {


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

    Transform operator *(const Transform &t) { return Transform(t.pos + t.rot.Transform(pos), t.rot*rot, t.scale*scale); }    
    Vec4 operator *(const Vec4 &v) const { return pos + rot.Transform(v * scale); }    

    void RotateAboutPoint(Vec4 v) { pos = v - rot.Transform(v); }

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

};

float Transform::ZERO[3][4] = {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}};
float Transform::IDENTITY[3][4] = {{0,0,0,0}, {0,0,0,1}, {1,1,1,1}};
float Transform::IDENTITY_ZERO_SCALE[3][4] = {{0,0,0,0}, {0,0,0,1}, {0,0,0,0}};
float Transform::ZERO_IDENTITY_SCALE[3][4] = {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}};

}
}

#endif