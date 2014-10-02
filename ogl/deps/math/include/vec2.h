#ifndef __VEC2_H__
#define __VEC2_H__

#include <iostream>
#include <cmath>

using std::ostream;

class Vec2 {
    public:
        union {
            struct { float x, y; };
            float v[2];
        };
        
        Vec2();
        Vec2(float x, float y);
        Vec2(const Vec2& v);
        Vec2(const Vec2& from,const Vec2 & to);
    
        Vec2& operator=(const Vec2 & v);    
        Vec2& operator+=(const Vec2 & v);
        Vec2 operator+(const float a) const;
        Vec2 operator-(const float a) const;
        Vec2 operator+(const Vec2 & v) const;
        Vec2& operator-=(const Vec2 & v);
        Vec2 operator-(const Vec2 & v) const;
        Vec2& operator*=(const float a);
        Vec2 operator*(const float a)const;
        Vec2& operator/=(const float a);
        Vec2 operator/(const float a)const;
        float operator[](const int dim) const;
        friend Vec2 operator*(const float a,const Vec2 & v);

        float length()const;
        Vec2& normalize();


		inline friend ostream& operator<<(ostream& o, const Vec2& vec) {
            o << "(" << vec.x << ", " << vec.y << ")";
            return o;
        }
};
#endif 
