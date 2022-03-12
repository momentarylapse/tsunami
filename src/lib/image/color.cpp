#include "color.h"
#include "../math/math.h"


const color White  = color(1, 1, 1, 1);
const color Black  = color(1, 0, 0, 0);
const color Grey   = color(1, 0.5f, 0.5f, 0.5f);
const color Gray   = color(1, 0.5f, 0.5f, 0.5f);
const color Red    = color(1, 1, 0, 0);
const color Green  = color(1, 0, 1, 0);
const color Blue   = color(1, 0, 0, 1);
const color Yellow = color(1, 1, 1, 0);
const color Orange = color(1, 1, 0.5f, 0);
const color Purple = color(1, 1, 0.5f, 0);


//------------------------------------------------------------------------------------------------//
//                                             colors                                             //
//------------------------------------------------------------------------------------------------//

color color::with_alpha(float _a) const {
	return color(_a, r, g, b);
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
	} else {
		auto xx = s.replace("(", "").replace(")", "").replace(",", "").explode(" ");
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
	return color(a, r, g, b);
}

color color::hsb(float hue, float saturation, float brightness, float a) {
	int h=int(hue*6)%6;
	float f=hue*6.0f-int(hue*6);
	float p=brightness*(1-saturation);
	float q=brightness*(1-saturation*f);
	float t=brightness*(1-saturation*(1-f));
	color c;
	if (h==0)	c=color(a,brightness,t,p);
	if (h==1)	c=color(a,q,brightness,p);
	if (h==2)	c=color(a,p,brightness,t);
	if (h==3)	c=color(a,p,q,brightness);
	if (h==4)	c=color(a,t,p,brightness);
	if (h==5)	c=color(a,brightness,p,q);
	return c;
}

// create a mixed color = a * (1-t)  +  b * t
color color::interpolate(const color &a,const color &b,float t) {
	return (1-t)*a + t*b;
}

color color::from_int_rgb(int *i) {
	return color(	1,
					(float)i[0]/255.0f,
					(float)i[1]/255.0f,
					(float)i[2]/255.0f);
}

color color::from_int_argb(int *i) {
	return color(	(float)i[0]/255.0f,
					(float)i[1]/255.0f,
					(float)i[2]/255.0f,
					(float)i[3]/255.0f);
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

bool color::operator ==(const color &c) const {
	return (r == c.r) and (g == c.g) and (b == c.b) and (a == c.a);
}

bool color::operator !=(const color &c) const {
	return !(*this == c);
}
