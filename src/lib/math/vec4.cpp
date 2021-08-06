/*
 * vec4.cpp
 *
 *  Created on: Jul 22, 2021
 *      Author: michi
 */


#include "vec4.h"
#include <math.h>




const vec4 vec4::ZERO = vec4(0, 0, 0, 0);
const vec4 vec4::EX = vec4(1, 0, 0, 0);
const vec4 vec4::EY = vec4(0, 1, 0, 0);
const vec4 vec4::EZ = vec4(0, 0, 1, 0);
const vec4 vec4::EW = vec4(0, 0, 0, 1);

vec4::vec4(float x, float y, float z, float w) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

// assignment operators
void vec4::operator += (const vec4& v) {
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
}

void vec4::operator -= (const vec4& v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
}

void vec4::operator *= (float f) {
	x *= f;
	y *= f;
	z *= f;
	w *= f;
}

void vec4::operator /= (float f) {
	x /= f;
	y /= f;
	z /= f;
	w /= f;
}

// unitary operator(s)
vec4 vec4::operator - () const {
	return vec4(-x, -y, -z, -w);
}

// binary operators
vec4 vec4::operator + (const vec4 &v) const {
	return vec4(x+v.x ,y+v.y, z+v.z, w+v.w);
}

vec4 vec4::operator - (const vec4 &v) const {
	return vec4(x-v.x, y-v.y, z-v.z, w-v.w);
}

vec4 vec4::operator * (float f) const {
	return vec4(x*f, y*f, z*f, w*f);
}

vec4 vec4::operator / (float f) const {
	return vec4(x/f, y/f, z/f, w/f);
}

bool vec4::operator == (const vec4 &v) const {
	return ((x==v.x) and (y==v.y) and (z==v.z) and (w==v.w));
}

bool vec4::operator != (const vec4 &v) const {
	return !((x==v.x) and (y==v.y) and (z==v.z) and (w==v.w));
}

string vec4::str() const {
	return format("(%f, %f, %f, %f)", x, y, z, w);
}

// real length of the vec4
float vec4::length() const {
	return sqrtf( x*x + y*y + z*z + w*w );
}

// scale to length 1
void vec4::normalize() {
	float l = length();
	if (l > 0)
		*this /= l;
	else
		*this = vec4::EW;
}

// scale to length 1
vec4 vec4::normalized() const {
	float l = length();
	if (l == 0)
		return vec4::EW;
	return *this / l;
}





int vec4::argmin() const {
	int n = 0;
	float m = (*this)[0];
	for (int i=1; i<3; i++)
		if ((*this)[i] < m) {
			n = i;
			m = (*this)[i];
		}
	return n;
}

int vec4::argmax() const {
	int n = 0;
	float m = (*this)[0];
	for (int i=1; i<3; i++)
		if ((*this)[i] > m) {
			n = i;
			m = (*this)[i];
		}
	return n;
}

float vec4::sum() const {
	return x + y + z + w;
}

float &vec4::operator[](int index) {
	auto vv = &x;
	return vv[index];
}

float vec4::operator[](int index) const {
	auto vv = &x;
	return vv[index];
}



int ivec4::find(int x) const {
	if (i == x)
		return 0;
	if (j == x)
		return 1;
	if (k == x)
		return 2;
	if (l == x)
		return 3;
	return -1;
}

int &ivec4::operator[](int index) {
	auto vv = &i;
	return vv[index];
}

int ivec4::operator[](int index) const {
	auto vv = &i;
	return vv[index];
}






