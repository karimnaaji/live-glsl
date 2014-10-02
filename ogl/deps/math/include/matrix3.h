#ifndef __MATRIX3_H__
#define __MATRIX3_H__

#include "matrix4.h"

using namespace std;

class Matrix3 {
	public:
	    union
	    {
	        float data[3][3];
	        float array[9];
	    };

	    Matrix3();
		Matrix3(float elements[9]);

        void transpose();
        void toIdentity();
        void toZero();

        static Matrix3 identity();
        static Matrix3 getInverse(const Matrix4& m);

        inline Vec3 operator*(const Vec3 &v) const {
            Vec3 vec;
            vec.x = v.x * array[0] + v.y * array[1] + v.z * array[2];
            vec.y = v.x * array[3] + v.y * array[4] + v.z * array[5];
            vec.z = v.x * array[6] + v.y * array[7] + v.z * array[8];
            return vec;
        }

        inline friend std::ostream& operator<<(std::ostream& o, const Matrix3& m) {
            cout << "\t" << m.array[0]  << " " << m.array[1]  << " " << m.array[2]  << std::endl;
            cout << "\t" << m.array[3]  << " " << m.array[4]  << " " << m.array[5]  << std::endl;
            cout << "\t" << m.array[6]  << " " << m.array[7]  << " " << m.array[8] << std::endl;
            return o;
        }
};

#endif
