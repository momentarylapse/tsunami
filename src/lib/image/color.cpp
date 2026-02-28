#include "color.h"
#include "../math/math.h"
#include <cmath>


const color White  = color::from_rgba(1, 1, 1);
const color Black  = color::from_rgba(0, 0, 0);
const color Grey   = color::from_rgba(0.5f, 0.5f, 0.5f);
const color Gray   = color::from_rgba(0.5f, 0.5f, 0.5f);
const color Red    = color::from_rgba(1, 0, 0);
const color Green  = color::from_rgba(0, 1, 0);
const color Blue   = color::from_rgba(0, 0, 1);
const color Yellow = color::from_rgba(1, 1, 0);
const color Orange = color::from_rgba(1, 0.5f, 0);
const color Purple = color::from_rgba(1, 0.5f, 0);


//------------------------------------------------------------------------------------------------//
//                                             colors                                             //
//------------------------------------------------------------------------------------------------//

color color::with_alpha(float _a) const {
	return from_rgba(r, g, b, _a);
}

string color::str() const {
	return format("(%.3f, %.3f, %.3f, %.3f)", r, g, b, a);
}

string color::hex() const {
	return format("#%02x%02x%02x%02x", (int)(r*255), (int)(g*255), (int)(b*255), (int)(a*255));
	return format("#02x02x02x02x", (int)(r*255), (int)(g*255), (int)(b*255), (int)(a*255));
}

// "(1.0, 1.0, 1.0, 1.0)"
// "1.0 1.0 1.0"
// "#rrggbbaa"
color color::parse(const string &s) {
	color c = Black;
	if (s.head(1) == "#") {
		auto bb = s.sub_ref(1).unhex();
		if (bb.num >= 1)
			c.r = (float)(unsigned char)bb[0] / 255.0f;
		if (bb.num >= 2)
			c.g = (float)(unsigned char)bb[1] / 255.0f;
		if (bb.num >= 3)
			c.b = (float)(unsigned char)bb[2] / 255.0f;
		if (bb.num >= 4)
			c.a = (float)(unsigned char)bb[3] / 255.0f;
	} else {
		auto xx = s.replace("(", "").replace(")", "").replace("[", "").replace("]", "").replace(" ", ",").replace(",,", ",").explode(",");
		if (xx.num > 0)
			c.r = xx[0]._float();
		if (xx.num > 1)
			c.g = xx[1]._float();
		if (xx.num > 2)
			c.b = xx[2]._float();
		if (xx.num > 3)
			c.a = xx[3]._float();
	}
	return c;
}

void color::clamp() {
	a = ::clamp(a, 0.0f, 1.0f);
	r = ::clamp(r, 0.0f, 1.0f);
	g = ::clamp(g, 0.0f, 1.0f);
	b = ::clamp(b, 0.0f, 1.0f);
}

// create a color from (alpha, red, green blue)
// (values of set [0..1])
color color::create_save(float r,float g, float b, float a) {
	if (a < 0)
		a = 0;
	else if (a > 1)
		a = 1;
	if (r < 0)
		r = 0;
	else if (r>1)
		r = 1;
	if (g < 0)
		g = 0;
	else if (g > 1)
		g = 1;
	if (b < 0)
		b = 0;
	else if (b > 1)
		b = 1;
	return from_rgba(r, g, b, a);
}

color color::from_rgba(float r, float g, float b, float a) {
	// TODO fix constructor!
	return color(a, r, g, b);
}


color color::from_hsb(float hue, float saturation, float brightness, float a) {
	int h = int(hue*6)%6;
	float f = hue*6.0f - int(hue*6);
	float p = brightness*(1-saturation);
	float q = brightness*(1-saturation*f);
	float t = brightness*(1-saturation*(1-f));
	color c;
	if (h == 0)
		c = from_rgba(brightness,t,p, a);
	if (h == 1)
		c = from_rgba(q,brightness,p, a);
	if (h == 2)
		c = from_rgba(p,brightness,t, a);
	if (h == 3)
		c = from_rgba(p,q,brightness, a);
	if (h == 4)
		c = from_rgba(t,p,brightness, a);
	if (h == 5)
		c = from_rgba(brightness,p,q, a);
	return c;
}

// create a mixed color = a * (1-t)  +  b * t
color color::mix(const color &a,const color &b,float t) {
	return (1-t)*a + t*b;
}

color color::from_int_rgb(int *i) {
	return color(	1,
					(float)i[0]/255.0f,
					(float)i[1]/255.0f,
					(float)i[2]/255.0f);
}

color color::from_int_argb(int *i) {
	return from_rgba((float)i[1]/255.0f,
					(float)i[2]/255.0f,
					(float)i[3]/255.0f,
					(float)i[0]/255.0f);
}

void color::get_int_rgb(int *i) const {
	i[0] = int(r * 255.0f);
	i[1] = int(g * 255.0f);
	i[2] = int(b * 255.0f);
}

void color::get_int_argb(int *i) const {
	i[0] = int(a * 255.0f);
	i[1] = int(r * 255.0f);
	i[2] = int(g * 255.0f);
	i[3] = int(b * 255.0f);
}

namespace {

float l2s(float x) {
	if (x <= 0.0031308f)
		return x * 12.92f;
	return 1.055f * powf(x, 1.0f / 2.4f) - 0.055f;
};

float s2l(float x) {
	if (x <= 0.04045f)
		return x / 12.92f;
	return powf((x + 0.055f) / 1.055f, 2.4f);
};

}

color color::linear_to_srgb() const {
	return from_rgba(l2s(r), l2s(g), l2s(b), a);
}

color color::srgb_to_linear() const {
	return from_rgba(s2l(r), s2l(g), s2l(b), a);
}

bool color::operator ==(const color &c) const {
	return (r == c.r) and (g == c.g) and (b == c.b) and (a == c.a);
}

bool color::operator !=(const color &c) const {
	return !(*this == c);
}

float color::brightness() const {
	return (r + g + b) / 3.0f;
}

float color::hsb_brightness() const {
	return max(r, max(g, b));
}

float color::hue() const {
	float _min = min(r, min(g, b));
	float _max = hsb_brightness();
	if (_max == _min)
		return 0;

	if (r >= g and g >= b)
		return (g - b) / (r - b) / 6;
	if (g >= r and r >= b)
		return  (2 - (r - b) / (g - b)) / 6;
	if (g >= b and b >= r)
		return  (2 + (b - r) / (g - r)) / 6;
	if (b >= g and g >= r)
		return (4 - (g - r) / (b - r)) / 6;
	if (b >= r and r >= g)
		return (4 + (r - g) / (b - g)) / 6;
	//if (r >= b and b >= g)
	return (6 - (b - g) / (r - g)) / 6;
}

float color::saturation() const {
	float _min = min(r, min(g, b));
	float b = hsb_brightness();
	if (b == 0)
		return 0;
	return 1 - _min / b;
}

