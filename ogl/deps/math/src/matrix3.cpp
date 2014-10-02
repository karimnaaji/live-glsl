#include "matrix3.h"

Matrix3::Matrix3()
{
    toIdentity();
}

Matrix3::Matrix3(float elements[9]) {
    memcpy(this->array, elements, 9 * sizeof(float));
}

Matrix3 Matrix3::getInverse(const Matrix4& m1)
{
    float a11 =   m1.array[10] * m1.array[5] - m1.array[6] * m1.array[9];
    float a21 = - m1.array[10] * m1.array[1] + m1.array[2] * m1.array[9];
    float a31 =   m1.array[6]  * m1.array[1] - m1.array[2] * m1.array[5];
    float a12 = - m1.array[10] * m1.array[4] + m1.array[6] * m1.array[8];
    float a22 =   m1.array[10] * m1.array[0] - m1.array[2] * m1.array[8];
    float a32 = - m1.array[6]  * m1.array[0] + m1.array[2] * m1.array[4];
    float a13 =   m1.array[9]  * m1.array[4] - m1.array[5] * m1.array[8];
    float a23 = - m1.array[9]  * m1.array[0] + m1.array[1] * m1.array[8];
    float a33 =   m1.array[5]  * m1.array[0] - m1.array[1] * m1.array[4];

    float det = m1.array[0] * a11 + m1.array[1] * a12 + m1.array[2] * a13;

    if (det == 0.0)
    {
        return Matrix3::identity();
    }

    Matrix3 m2;

    float idet = 1.0 / det;

    m2.array[0] = idet * a11; m2.array[1] = idet * a21; m2.array[2] = idet * a31;
    m2.array[3] = idet * a12; m2.array[4] = idet * a22; m2.array[5] = idet * a32;
    m2.array[6] = idet * a13; m2.array[7] = idet * a23; m2.array[8] = idet * a33;

    return m2;
}

void Matrix3::transpose()
{
    float tmp;

    tmp = array[1]; array[1] = array[3]; array[3] = tmp;
    tmp = array[2]; array[2] = array[6]; array[6] = tmp;
    tmp = array[5]; array[5] = array[7]; array[7] = tmp;
}

Matrix3 Matrix3::identity()
{
    Matrix3 m;
    m.toIdentity();
    return m;
}

void Matrix3::toZero() {
    for(int i = 0; i < 9; i++) {
        array[i] = 0.0f;
    }
}

void Matrix3::toIdentity()
{
    toZero();
    array[0] = array[4] = array[8] = 1; 
}
