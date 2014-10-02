#ifndef QUATERNION_H
#define QUATERNION_H

#include <cmath>
#include <iostream>
#include "vec3.h"

class Quaternion
{
public:
    union {
        struct { float x, y, z, w; };
        float v[4];
    };

    Quaternion(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f);
    Quaternion(const Quaternion &q);
    Quaternion(float radAngle, Vec3 axis);
    Quaternion operator*(const Quaternion &q);
    Quaternion& divideByScalar(float scalar);
    Quaternion operator/(float scalar);
    Quaternion conjugate();
    Quaternion& normalize();
    Quaternion inverse();
	float dotProduct(const Quaternion &q) const;
	Quaternion slerp(const Quaternion &q1, const Quaternion &q2, float t);
    float length();
    void setToAngle(float radAngle, Vec3 axis);

    inline friend ostream& operator<<(ostream& o, const Quaternion& q) {
        o << "(" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ")";
        return o;
    }
};

#endif // QUATERNION_H
