#include "vec2.h"

Vec2::Vec2() {
    x = 0;
    y = 0;
}

Vec2::Vec2(float x_, float y_) {
    x = x_;
    y = y_;
}

Vec2::Vec2(const Vec2& v) {
    x = v.x;
    y = v.y;
}

Vec2::Vec2(const Vec2& from,const Vec2& to) {
    x = to.x - from.x;
    y = to.y - from.y;
}

Vec2& Vec2::operator= (const Vec2& v) {
    x = v.x;
    y = v.y;
    return *this;
}

Vec2& Vec2::operator+= (const Vec2& v) {
    x += v.x;
    y += v.y;
    return *this;
}

Vec2 Vec2::operator+ (const Vec2& v) const {
    Vec2 t = *this;
    t += v;
    return t;
}

Vec2 Vec2::operator+(const float a) const {
    Vec2 t = *this;
    t.x += a;
    t.y += a;
    return t;
}

Vec2 Vec2::operator-(const float a) const {
    Vec2 t = *this;
    t.x -= a;
    t.y -= a;
    return t;
}

Vec2 & Vec2::operator-= (const Vec2& v) {
    x -= v.x;
    y -= v.y;
    return *this;
}

Vec2 Vec2::operator-(const Vec2& v) const {
    Vec2 t = *this;
    t -= v;
    return t;
}

Vec2 & Vec2::operator*=(const float a) {
    x *= a;
    y *= a;
    return *this;
}

Vec2 Vec2::operator*(const float a)const {
    Vec2 t = *this;
    t *= a;
    return t;
}

Vec2 operator*(const float a, const Vec2 & v) {
    return Vec2(v.x*a, v.y*a);
}

Vec2& Vec2::operator/=(const float a) {
    x /= a;
    y /= a;
    return *this;
}

Vec2 Vec2::operator/(const float a)const {
    Vec2 t = *this;
    t /= a;
    return t;
}

float Vec2::length() const {
    return sqrt(x*x+y*y);
}

Vec2& Vec2::normalize() {
    (*this) /= length();
    return (*this);
}

float Vec2::operator[](const int dim) const {
    if(dim == 0) return x;
    else return y;
}
