#ifndef __MATRIX4_H__
#define __MATRIX4_H__

#include <iostream>
#include <string.h>
#include "vec3.h"
#include "vec4.h"
#include "mathshelpers.h"
#include "quaternion.h"
#include <assert.h>

class Matrix4 {
	public:
		union
	    {
	        float data[4][4];
			float array[16];
	    };
	    
		Matrix4(void);
		Matrix4(float elements[16]);
		~Matrix4(void);
        Matrix4& operator=(const Matrix4 &a);

        void toZero();
        void toIdentity();
        Matrix4& transpose();
        Vec3 getTranslation() const;
        void setColumn(int i, const Vec3& column);
        void setRow(int i, const Vec3& row);
        void setTranslation(const Vec3& t);

        static Matrix4 createRotation(float degrees, const Vec3 &axis);
        static Matrix4 createScale(const Vec3 &scale);
        static Matrix4 createTranslation(const Vec3 &translation);
        static Matrix4 createPerspective(float znear, float zfar,float right, float left, float top, float bottom);
        static Matrix4 createOrthographic();
        static Matrix4 createLookAt(const Vec3 &from, const Vec3 &lookingAt);
        static Matrix4 createRotation(const Quaternion &q);
        static Matrix4 createView(const Vec3& up, const Vec3& right, const Vec3& forward, const Vec3& translate);

        static Matrix4 identity();

		inline Matrix4 operator*(const Matrix4 &a) const {	
            Matrix4 out;

			for(unsigned int r = 0; r < 4; ++r) {
				for(unsigned int c = 0; c < 4; ++c) {
					out.array[c + (r*4)] = 0.0f;
					for(unsigned int i = 0; i < 4; ++i) {
                        out.array[c + (r*4)] += this->array[(r*4)+i] * a.array[c+(i*4)];
					}
				}
			}

			return out;
		}

		inline Vec3 operator*(const Vec3 &v) const {
			Vec3 vec;

			float temp;

            vec.x = v.x * array[0] + v.y * array[1] + v.z * array[2]  + array[3];
            vec.y = v.x * array[4] + v.y * array[5] + v.z * array[6]  + array[7];
            vec.z = v.x * array[8] + v.y * array[9] + v.z * array[10] + array[11];

            temp =  v.x * array[12] + v.y * array[13] + v.z * array[14] + array[15];

            vec.x = vec.x / temp;
			vec.y = vec.y / temp;
			vec.z = vec.z / temp;

			return vec;
        }

		inline Vec4 operator*(const Vec4 &v) const {
			return Vec4(
                v.x*array[0] + v.y * array[1] + v.z * array[2]  + v.w * array[3],
                v.x*array[4] + v.y * array[5] + v.z * array[6]  + v.w * array[7],
                v.x*array[8] + v.y * array[9] + v.z * array[10] + v.w * array[11],
                v.x*array[12] + v.y * array[13] + v.z * array[14] + v.w * array[15]
			);
        }

		inline friend std::ostream& operator<<(std::ostream& o, const Matrix4& m) {
			o << "\t" << m.array[0]  << " " << m.array[1]  << " " << m.array[2]  << " " << m.array [3]  << std::endl;
			o << "\t" << m.array[4]  << " " << m.array[5]  << " " << m.array[6]  << " " << m.array [7]  << std::endl;
			o << "\t" << m.array[8]  << " " << m.array[9]  << " " << m.array[10] << " " << m.array [11] << std::endl;
			o << "\t" << m.array[12] << " " << m.array[13] << " " << m.array[14] << " " << m.array [15] << std::endl;
			return o;
		}
};

#endif
