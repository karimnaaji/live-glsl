#ifndef __VEC3_H__
#define __VEC3_H__

#include <cmath>
#include <iostream>
#include "mathshelpers.h"
#include "vec4.h"

using std::ostream;

class Vec3 {
    public:
        union {
            struct { float x, y, z; };
            struct { float r, g, b; };
            float v[3];
        };
        
        Vec3();
        Vec3(float x,float y,float z);
        Vec3(float val);
        Vec3(const Vec3& v);
        Vec3(const Vec4& v);
        Vec3(const Vec3& from,const Vec3& to);
        
        Vec3& operator=(const Vec3& v);
        Vec3& operator+=(const Vec3& v);
        Vec3 operator+(const Vec3& v) const;        
        Vec3& operator-=(const Vec3& v);
        Vec3 operator-(const Vec3& v) const;        
        Vec3 operator-() const;
        Vec3& operator*=(const float a);
        Vec3 operator*(const float a)const;
        Vec3 operator*(const Vec3& v) const;
        Vec3& operator/=(const float a);
        Vec3 operator/(const float a)const;
        float operator[](const int dim) const;
        friend Vec3 operator*(const float a, const Vec3& v);

        static Vec3 zero();
        static Vec3 vClamp(Vec3 v, float low, float high);
        static Vec3 abs(Vec3 v);
        static Vec3 up();
        Vec3 cross(const Vec3& v) const;
        float length() const;
        float dot(const Vec3& v) const;
        Vec3& normalize();

        Vec2 sphericalCoord() const;

		inline friend ostream& operator<<(ostream& o, const Vec3& vec) {
            o << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
            return o;
        }
};

#endif 
