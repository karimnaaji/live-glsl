#include "quaternion.h"

Quaternion::Quaternion(float x, float y, float z, float w) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

Quaternion::Quaternion(float radAngle, Vec3 axis) {
    setToAngle(radAngle, axis);
}

void Quaternion::setToAngle(float radAngle, Vec3 axis) {
    axis.normalize();

    float a = radAngle/2;
    float sina = sin(a);

    x = sina * axis.x;
    y = sina * axis.y;
    z = sina * axis.z;
    w = cos(a);

    normalize();
}

Quaternion::Quaternion(const Quaternion &q) {
    x = q.x;
    y = q.y;
    z = q.z;
    w = q.w;
}

Quaternion Quaternion::operator*(const Quaternion &q1) {
    Quaternion q2(*this);
    q2.w = q1.w*w - q1.x*x - q1.y*y - q1.z*z;
    q2.x = q1.w*x + q1.x*w + q1.y*z - q1.z*y;
    q2.y = q1.w*y - q1.x*z + q1.y*w + q1.z*x;
    q2.z = q1.w*z + q1.x*y - q1.y*x + q1.z*w;
    return q2;
}

Quaternion Quaternion::operator/(float scalar) {
    Quaternion q(*this);
    q.divideByScalar(scalar);
    return q;
}

Quaternion& Quaternion::divideByScalar(float scalar) {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
}

Quaternion& Quaternion::normalize() {
    return this->divideByScalar(length());
}

Quaternion Quaternion::inverse() {
    float l = length();
	return conjugate().divideByScalar(l*l);
}

float Quaternion::dotProduct(const Quaternion &q) const
{
	return x*q.x + y*q.y + z*q.z + w*q.w;
}

Quaternion Quaternion::slerp(const Quaternion &q1, const Quaternion &q2, float t)
{
	Quaternion qr;
	float theta, sint, sintt, sinott, coeffq1, coeffq2;

	// algorithm adapted from Shoemake's paper
	t=t/2.0;

	theta = (float) acos(q1.dotProduct(q2));
	if (theta<0.0) theta=-theta;

	sint = (float) sin(theta);
	sintt = (float) sin(t*theta);
	sinott = (float) sin((1-t)*theta);
	coeffq1 = sinott/sint;
	coeffq2 = sintt/sint;

	qr.x = coeffq1*q1.x + coeffq2*q2.x;
	qr.y = coeffq1*q1.y + coeffq2*q2.y;
	qr.z = coeffq1*q1.z + coeffq2*q2.z;
	qr.w = coeffq1*q1.w + coeffq2*q2.w;

	qr.normalize();

	return qr;
}

float Quaternion::length() {
    return sqrt(w*w + x*x + y*y + z*z);
}

Quaternion Quaternion::conjugate() {
    Quaternion q(*this);
    q.x = -q.x;
    q.y = -q.y;
    q.z = -q.z;
    return q;
}
