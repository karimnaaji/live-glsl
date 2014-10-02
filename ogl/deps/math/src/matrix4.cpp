#include "matrix4.h"

Matrix4::Matrix4(void) {
    toIdentity();
}

Matrix4::Matrix4(float elements[16]) {
	memcpy(this->array, elements, 16 * sizeof(float));
}

Matrix4::~Matrix4(void)	{
    toIdentity();
}

void Matrix4::toIdentity() {
    toZero();

	array[0]  = 1.0f;
	array[5]  = 1.0f;
	array[10] = 1.0f;
	array[15] = 1.0f;
}

Matrix4& Matrix4::operator=(const Matrix4 &a) {
	if(this != &a) {
		for(int i = 0; i < 16; i++) {
			array[i] = a.array[i];
		}
	}
	
	return *this;
}


void Matrix4::toZero() {
	for(int i = 0; i < 16; i++) {
		array[i] = 0.0f;
	}
}

Matrix4 Matrix4::identity() {
	Matrix4 m;

    m.toIdentity();
	return m;
}

Matrix4 Matrix4::createOrthographic() {
	Matrix4 m;

    // TODO : Ortho

	return m;
}

Matrix4 Matrix4::createPerspective(float znear, float zfar,float right, float left, float top, float bottom) {
	Matrix4 m;
    m.toZero();

	m.array[0]	= (2.0f * znear) / (right - left);
	m.array[2]	= (right + left) / (right - left);
	m.array[5]	= (2.0f * znear) / (top - bottom);
	m.array[6]	= (top + bottom) / (top - bottom);
	m.array[10] = - (zfar + znear) / (zfar - znear);
	m.array[11] = - (2.0f * znear * zfar) / (zfar - znear);
	m.array[14] = -1.0f;

	return m;
}

Matrix4 Matrix4::createView(const Vec3& up, const Vec3& right, const Vec3& forward, const Vec3& translate) {
    Matrix4 r;

    r.setColumn(0, right);
    r.setColumn(1, up);
    r.setColumn(2, -forward);

    r.transpose();
    r.setTranslation(-(r * translate));

    return r;
}

void Matrix4::setColumn(int i, const Vec3& column) {
    assert(i < 4);
    for(int j = 0; j < 3; ++j) {
        array[i + j*4] = column.v[j];
    }
}

void Matrix4::setRow(int i, const Vec3& column) {
    assert(i < 4);
    for(int j = 0; j < 3; ++j) {
        array[i*4 + j] = column.v[j];
    }
}

void Matrix4::setTranslation(const Vec3& t) {
    setColumn(3, t);
}

Matrix4 Matrix4::createLookAt(const Vec3 &from, const Vec3 &lookingAt) {
    Matrix4 r = Matrix4::createTranslation(-from);
	Matrix4 m;
	Vec3 f = (lookingAt - from);
    Vec3 up = Vec3::up();

    f.normalize();

    Vec3 s = f.cross(up);
    Vec3 u = s.cross(f);

    m.setRow(0, s);
    m.setRow(1, u);
    m.setRow(2, -f);

	return m*r;
}

Matrix4 Matrix4::createRotation(const Quaternion &q) {
    Matrix4 m;

    float xx = q.x * q.x;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float xw = q.x * q.w;

    float yy = q.y * q.y;
    float yz = q.y * q.z;
    float yw = q.y * q.w;

    float zz = q.z * q.z;
    float zw = q.z * q.w;

    m.array[0] = 1 - 2 * (yy + zz);
    m.array[1] = 2 * (xy - zw);
    m.array[2] = 2 * (xz + yw);

    m.array[4] = 2 * (xy + zw);
    m.array[5] = 1 - 2 * (xx + zz);
    m.array[6] = 2 * (yz - xw);

    m.array[8] = 2 * (xz - yw);
    m.array[9] = 2 * (yz + xw);
    m.array[10] = 1 - 2 * (xx + yy);

    return m;
}

Matrix4 Matrix4::createRotation(float degrees, const Vec3 &inaxis) {
	Matrix4 m;

	Vec3 axis = inaxis;

    axis.normalize();

    float c = cos((float)radians(degrees));
    float s = sin((float)radians(degrees));

	m.array[0]  = (axis.x * axis.x) * (1.0f - c) + c;
    m.array[4]  = (axis.y * axis.x) * (1.0f - c) + (axis.z * s);
    m.array[8]  = (axis.z * axis.x) * (1.0f - c) - (axis.y * s);

    m.array[1]  = (axis.x * axis.y) * (1.0f - c) - (axis.z * s);
	m.array[5]  = (axis.y * axis.y) * (1.0f - c) + c;
    m.array[9]  = (axis.z * axis.y) * (1.0f - c) + (axis.x * s);

    m.array[2]  = (axis.x * axis.z) * (1.0f - c) + (axis.y * s);
    m.array[6]  = (axis.y * axis.z) * (1.0f - c) - (axis.x * s);
	m.array[10] = (axis.z * axis.z) * (1.0f - c) + c;

	return m;
}

Matrix4 Matrix4::createScale(const Vec3 &scale) {
	Matrix4 m;

	m.array[0]  = scale.x;
	m.array[5]  = scale.y;
	m.array[10] = scale.z;	

	return m;
}

Matrix4 Matrix4::createTranslation(const Vec3 &translation) {
	Matrix4 m;

    m.array[3] = translation.x;
    m.array[7] = translation.y;
    m.array[11] = translation.z;

	return m;
}

Vec3 Matrix4::getTranslation() const
{
    return Vec3(array[3], array[7], array[11]);
}

Matrix4& Matrix4::transpose() {
    Matrix4 m(*this);

    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            array[i+j*4] = m.array[i*4+j];
        }
    }

    return *this;
}
