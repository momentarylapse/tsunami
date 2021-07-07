#include "complex.h"
#include <math.h>


const complex complex::ZERO = complex(0, 0);
const complex complex::ONE = complex(1, 0);
const complex complex::I = complex(0, 1);

complex::complex(float x, float y) {
	this->x = x;
	this->y = y;
}

// assignment operators
void complex::operator += (const complex& v) {
	x += v.x;
	y += v.y;
}

void complex::operator -= (const complex& v) {
	x -= v.x;
	y -= v.y;
}

void complex::operator *= (float f) {
	x *= f;
	y *= f;
}

void complex::operator /= (float f) {
	x /= f;
	y /= f;
}

void complex::operator *= (const complex &v) {
	*this = complex(x*v.x - y*v.y, y*v.x + x *v.y);
}

void complex::operator /= (const complex &v) {
	*this = complex(x*v.x + y*v.y, y*v.x - x *v.y) / (v.x * v.x + v.y * v.y);
}

// unitary operator(s)
complex complex::operator - () {
	return complex(-x, -y);
}

// binary operators
complex complex::operator + (const complex &v) const {
	return complex(x+v.x, y+v.y);
}

complex complex::operator - (const complex &v) const {
	return complex(x-v.x, y-v.y);
}

complex complex::operator * (float f) const {
	return complex(x*f, y*f);
}

complex complex::operator / (float f) const {
	return complex(x/f, y/f);
}

complex complex::operator * (const complex &v) const {
	return complex(x*v.x - y*v.y, y*v.x + x *v.y);
}

complex complex::operator / (const complex &v) const {
	return complex(x*v.x + y*v.y, y*v.x - x *v.y) / (v.x * v.x + v.y * v.y);
}

bool complex::operator == (const complex &v) const {
	return ((x == v.x) && (y == v.y));
}

bool complex::operator != (const complex &v) const {
	return !((x == v.x) && (y == v.y));
}

float complex::abs() const {
	return sqrt(x*x + y*y);
}

float complex::abs_sqr() const {
	return x*x + y*y;
}

complex complex::bar() const {
	return complex(x, -y);
}

string complex::str() const {
	return format("(%f, %f)", x, y);
}
