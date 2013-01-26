
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#if 0
#ifndef MINI3D_PHYSICS_PHYSICS_H
#define MINI3D_PHYSICS_PHYSICS_H

#include <cmath>

namespace {

typedef unsigned int uint;

const uint MAX_RIGID_BODIES = 64;


float Sgn(float a)						{ return (a < 0) ? -1 : 1; }

void Clear(float a[3])					{ for(uint i=0;i<3;++i) a[i] = 0; }
void Set(float d[3], const float s[3])	{ for(uint i=0;i<3;++i) d[i] = s[i]; }
void Add(float a[3], const float b[3])	{ for(uint i=0;i<3;++i) a[i] += b[i]; }
void Sub(float a[3], const float b[3])	{ for(uint i=0;i<3;++i) a[i] -= b[i]; }
void Mul(float a[3], const float b)		{ for(uint i=0;i<3;++i) a[i] *= b; }
void Div(float a[3], const float b)		{ for(uint i=0;i<3;++i) a[i] /= b; }

void Sub(float d[3], const float a[3], const float b[3])	{ for(uint i=0;i<3;++i) d[i] = a[i] - b[i]; }


float Dot(const float a[3], const float b[3])				{ float r = 0; for(uint i=0;i<3;++i) r += a[i] * b[i]; return r; }
void Cross(float d[3], const float a[3], const float b[3])	{ d[0] = a[1]*b[2] - a[2]*b[1]; d[1] = a[2]*b[0] - a[0]*b[2]; d[2] = a[0]*b[1] - a[1]*b[0]; }

float Norm(const float a[3])			{ float r = 0; for(uint i=0;i<3;++i) r += a[i] * a[i]; return r; }
float Length(const float a[3])			{ return sqrt(Norm(a)); }

void Normalize(float a[3])				{ float l = Length(a); for(uint i=0;i<3;++i) a[i] /= l; }
void Invert(float a[3])					{ for(uint i=0;i<3;++i) a[i] *= -1; }

bool Equals(const float a[3], const float b[3]) { for(uint i=0;i<3;++i) if (a[i] != b[i]) return false; return true; }

float ZERO_VECTOR[3] = {0,0,0};

const float GRAVITY[3] = {0.0f, 9.82f, 0.0f};

/*
    static Transform LinearInterpolate(Transform* transforms, uint16_t indices[4], float weights[4])
    {
        Transform r(ZERO);

        for (unsigned int i = 0; i < 4; ++i)
        {
            r.pos += transforms[indices[i]].pos * weights[i];
            r.rot += transforms[indices[i]].rot * weights[i];
            r.scale += transforms[indices[i]].scale * weights[i];
        }
        r.rot.Normalize();
        return r;
    }
*/
}



namespace mini3d {
namespace physics {

#define SUPPORTMAPPING void SupportMapping(float d[3], float *p)
struct RigidBody
{
	virtual uint GetType() = 0;
	virtual SUPPORTMAPPING = 0;

	// Orientation
	float position[3]; 
	float velocity[3];
	
	float angle[3]; 
	float angularVelocity[3];

	// Properties
	float inverseMass;
	float inverseInertia[3];
	float bounce;

	// Abstractions
	float radius;
};

#define RIGIDBODYTYPE(type) static const uint TYPE = type; uint GetType() { return TYPE; };
struct Sphere	: public RigidBody { RIGIDBODYTYPE(1); SUPPORTMAPPING { /* never used */ } };
struct Box		: public RigidBody { RIGIDBODYTYPE(2); SUPPORTMAPPING { for(uint i=0;i<3;++i) p[i] = size[i] * Sgn(d[i]); } float size[3]; };

struct Contact { RigidBody* rigidBodies[2]; float points[2][3]; float normal[3]; float distance; };


class Physics
{
	void Update(float deltaTime)
	{
		// apply gravity
		for (unsigned int i = 0; i < mRigidBodyCount; ++i)
		{
			RigidBody* pRigidBody = &mpRigidBodies[i];
	
			// If body reacts to gravity
			if ( pRigidBody->inverseMass > 0 )
			{
				// velocity += (GRAVITY * deltaTime);
				float v[3];
				Set(v, GRAVITY);
				Mul(v, deltaTime);
				Add(pRigidBody->velocity, v);
			}
		}
 
		// collide
		for (unsigned int i = 0; i < mRigidBodyCount; ++i)
		{
			RigidBody* pRigidBody_1 = &mpRigidBodies[i];
			for (unsigned int j = i + 1; i < mRigidBodyCount; ++i)
			{
				RigidBody* pRigidBody_2 = &mpRigidBodies[j];
 
				// distanceAB = pRigidBody_2->mPosition - pRigidBody_1->mPosition;
				float distanceAB[3];
				Set(distanceAB, pRigidBody_2->position);
				Sub(distanceAB, pRigidBody_1->position);

				float lengthAB = Length(distanceAB);

				float directionAB[3];
				Set(directionAB, distanceAB);
				Div(directionAB, lengthAB);

				float directionBA[3];
				Set(directionBA, directionAB);
				Invert(directionBA);

				float support_1[3], support_2[3];

				pRigidBody_1->SupportMapping(support_1, directionAB);
				pRigidBody_2->SupportMapping(support_2, directionBA);

				if (lengthAB <= (Length(support_1) + Length(support_2)))
				{
					// Collision

					if (pRigidBody_1->inverseMass > 0 || pRigidBody_2->inverseMass > 0)
					{
						// Do something
					}

				}
			}
		}
 
		// Integrate
		for (unsigned int i = 0; i < mRigidBodyCount; ++i)
		{
			RigidBody* pRigidBody = &mpRigidBodies[i];
			
			// integrate values
		}
	}

	bool GenerateContact(RigidBody* rb1, RigidBody* rb2, Contact &contact)
	{
		contact.rigidBodies[0] = rb1;
		contact.rigidBodies[1] = rb2;

		if (rb1->GetType() == rb2->GetType())
		{
			contact.distance;
			contact.normal;
			contact.rigidBodies[0];
			contact.rigidBodies[1];
			contact.points[0];
			contact.points[1];

			// contact->distance = simplex->position - position + simplex->points[0];
			float distance[3]; Set(distance, rb2->position); Sub(distance, rb1->position);

			// Sphere vs Sphere collision
			switch(rb1->GetType())
			{
				case Sphere::TYPE:
					contact.distance = Length(distance) - (rb1->radius + rb2->radius);

					// contact->normal = Normalize(distance);
					Set(contact.normal, distance); Normalize(contact.normal);
					
					// point = contact.normal * rb1.radius
					Set(contact.points[0], contact.normal); Mul(contact.points[0], rb1->radius);
					Set(contact.points[1], contact.normal); Mul(contact.points[1], -rb2->radius);
					return true;
				case Box::TYPE:

					// Array of normals [direction, inverse, xyz]
					float N[3][2][3], supportPoint[4][2][3], portalPoint[4][3];

					// Get the initial support points and derive the minkowski portal points
					Set(N[0][0], distance); Normalize(N[0][0]);	
					Set(N[0][1], N[0][0]); Invert(N[0][1]);

					Cross(N[2][0], N[0][0], distance); Normalize(N[2][0]);
					Set(N[2][1], N[2][0]); Invert(N[2][1]);

					Cross(N[1][0], N[2][0], N[0][0]); Normalize(N[1][0]);
					Set(N[2][1], N[2][0]); Invert(N[2][1]);

					rb1->SupportMapping(N[0][0], supportPoint[0][0]);
					rb2->SupportMapping(N[0][1], supportPoint[0][1]);
					Set(portalPoint[0], supportPoint[0][0]); Sub(portalPoint[0], supportPoint[0][1]);

					rb1->SupportMapping(N[1][0], supportPoint[1][0]);
					rb2->SupportMapping(N[1][1], supportPoint[1][1]);
					Set(portalPoint[1], supportPoint[1][0]); Sub(portalPoint[1], supportPoint[1][1]);
					
					bool invert = (Dot(portalPoint[1], N[1][0]) < 0);
					rb1->SupportMapping(N[2][ invert], supportPoint[2][0]);
					rb2->SupportMapping(N[2][!invert], supportPoint[2][1]);
					Set(portalPoint[2], supportPoint[2][0]); Sub(portalPoint[2], supportPoint[2][1]);

					// normal for the current and previous portal
					// the previous portal is kept to check that the new portal is sufficiently different from the old one in each iteration.
					float Np[3][2];

					for (;;)
					{
						// Get the new portal from s1 and s2
						float P[2][3]; 
						Set(P[0], portalPoint[1]); Sub(P[0], portalPoint[0]);
						Set(P[1], portalPoint[2]); Sub(P[1], portalPoint[0]);
						
						Cross(Np[0], P[0], P[1]);
						Set(Np[1], Np[0]); Invert(Np[1]);
						
						// Get the support most point in the direction of the portal normal
						bool invert = (Dot(Np[0], distance) < 0);
						rb1->SupportMapping(Np[ invert], supportPoint[3][0]);
						rb2->SupportMapping(Np[!invert], supportPoint[3][1]);
						Set(portalPoint[3], supportPoint[3][0]); Sub(portalPoint[3], supportPoint[3][1]);

						// If the new point is the same as one of the old ones we have refined the portal as much as possible
						if (Equals(portalPoint[3], portalPoint[0]) || Equals(portalPoint[3], portalPoint[1]) || Equals(portalPoint[3], portalPoint[2])) break;
						
						// Find out what part of the portal is closest rb2->pos. This is the portal we should keep.
						float L[3][3]; for (uint i=0; i<3; ++i) Sub(L[i], portalPoint[i], portalPoint[3]);

						float o[3]; Sub(o, distance, portalPoint[3]);
						
						// Dot product d that is negative represents the vector that we should replace
						for (uint i=0; i<3; ++i) if (Dot(L[i], o) < 0) { Set(portalPoint[i], portalPoint[3]); Set(supportPoint[i][0], supportPoint[3][0]); Set(supportPoint[i][1], supportPoint[3][1]);  break;}
					}

					// Find closest distance between point and 

			}
		}
		else
		{
			
		}

		return false;
	}


private:
	unsigned int mRigidBodyCount;
	RigidBody mpRigidBodies[MAX_RIGID_BODIES];
};

}
}


#endif
#endif