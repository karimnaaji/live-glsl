#ifndef __VECTOR4_H__
#define __VECTOR4_H__

#include <iostream>

using namespace std;

class Vec4 {
	public:
        union {
		    struct { float x, y, z, w; };
            struct { float r, g, b, a; };
            float v[4];
        };

		Vec4(void) {
			x = y = z = w = 1.0f;
		}

		Vec4(float x, float y, float z, float w) {
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}

        float operator[](const int dim) const {
            if(dim == 0) return x;
            else if(dim == 1) return y;
            else if(dim == 2) return z;
            else return w;
        }

        inline friend ostream& operator<<(ostream& o, const Vec4& vec) {
            o << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w <<  ")";
            return o;
        }
};

#endif
