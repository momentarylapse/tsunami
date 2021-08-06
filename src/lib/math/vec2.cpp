/*
 * vec2.cpp
 *
 *  Created on: Jul 22, 2021
 *      Author: michi
 */



#include "vec2.h"
#include <math.h>



const vec2 vec2::ZERO = vec2(0, 0);
const vec2 vec2::EX = vec2(1, 0);
const vec2 vec2::EY = vec2(0, 1);

vec2::vec2(float x, float y) {
	this->x = x;
	this->y = y;
}

// assignment operators
void vec2::operator += (const vec2& v) {
	x += v.x;
	y += v.y;
}

void vec2::operator -= (const vec2& v) {
	x -= v.x;
	y -= v.y;
}

void vec2::operator *= (float f) {
	x *= f;
	y *= f;
}

void vec2::operator /= (float f) {
	x /= f;
	y /= f;
}

// unitary operator(s)
vec2 vec2::operator - () const {
	return vec2(-x, -y);
}

// binary operators
vec2 vec2::operator + (const vec2 &v) const {
	return vec2(x+v.x ,y+v.y);
}

vec2 vec2::operator - (const vec2 &v) const {
	return vec2(x-v.x, y-v.y);
}

vec2 vec2::operator * (float f) const {
	return vec2(x*f, y*f);
}

vec2 vec2::operator / (float f) const {
	return vec2(x/f, y/f);
}

bool vec2::operator == (const vec2 &v) const {
	return ((x==v.x) and (y==v.y));
}

bool vec2::operator != (const vec2 &v) const {
	return !((x==v.x) and (y==v.y));
}

string vec2::str() const {
	return format("(%f, %f)", x, y);
}

// real length of the vec2
float vec2::length() const {
	return sqrtf( x*x + y*y );
}

// scale to length 1
void vec2::normalize() {
	float l = length();
	if (l > 0)
		*this /= l;
	else
		*this = vec2::EY;
}

// scale to length 1
vec2 vec2::normalized() const {
	float l = length();
	if (l == 0)
		return vec2::EY;
	return *this / l;
}

vec2 vec2::ortho() const {
	return {y, -x};
}

float vec2::cross(const vec2 &a, const vec2 &b) {
	return a.x * b.y - a.y * b.x;
}

float vec2::dot(const vec2 &a, const vec2 &b) {
	return a.x * b.x + a.y * b.y;
}


// P = A + f*( B - A ) + g*( C - A )
vec2 vec2::bary_centric(const vec2 &P, const vec2 &A, const vec2 &B, const vec2 &C) {
	vec2 ba = B - A;
	vec2 ca = C - A;
	vec2 pvec = ca.ortho();
	float det = vec2::dot(ba, pvec);
	vec2 pa;
	if (det>0) {
		pa = P - A;
	} else {
		pa = A - P;
		det = -det;
	}
	vec2 fg;
	fg.x = vec2::dot(pa, pvec);
	fg.y = vec2::cross(pa, ba);
	return fg / det;
}


