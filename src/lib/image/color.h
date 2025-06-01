
#ifndef _IMAGE_COLOR_INCLUDED_
#define _IMAGE_COLOR_INCLUDED_


#include "../base/base.h"

struct color {
	float r,g,b,a;
	color() {};
	color(float a,float r,float g,float b)
	{	this->a=a;	this->r=r;	this->g=g;	this->b=b;	}
	/*color& operator = (const color& c)
	{	a=c.a;	r=c.r;	g=c.g;	b=c.b;	return *this;	}*/
	color& _cdecl operator += (const color& c)
	{	a += c.a;	r += c.r;	g += c.g;	b += c.b;	return *this;	}
	color& _cdecl operator -= (const color& c)
	{	a -= c.a;	r -= c.r;	g -= c.g;	b -= c.b;	return *this;	}
	color _cdecl operator + (const color& c) const
	{	return color(a + c.a, r + c.r, g + c.g, b + c.b);	}
	color _cdecl operator - (const color& c) const
	{	return color(a - c.a, r - c.r, g - c.g, b - c.b);	}
	color _cdecl operator * (float f) const
	{	return color(a*f , r*f , g*f , b*f);	}
	friend color _cdecl operator * (float f, const color &c)
	{	return c * f;	}
	void _cdecl operator *= (float f)
	{	a *= f;	r *= f;	g *= f;	b *= f;	}
	color _cdecl operator * (const color &c) const
	{	return color(a*c.a , r*c.r , g*c.g , b*c.b);	}
	void _cdecl operator *= (const color &c)
	{	a*=c.a;	r*=c.r;	g*=c.g;	b*=c.b;	}
	bool operator==(const color &c) const;
	bool operator!=(const color &c) const;
	void _cdecl clamp();
	color with_alpha(float a) const;
	string _cdecl str() const;
	string _cdecl hex() const;

	color lin_to_srgb() const;
	color srgb_to_lin() const;

	void _cdecl get_int_rgb(int *i) const;
	void _cdecl get_int_argb(int *i) const;
	float brightness() const;
	float hsb_brightness() const;
	float hue() const;
	float saturation() const;


	static color _cdecl create_save(float r, float g, float b, float a);
	static color _cdecl hsb(float hue, float saturation, float brightness, float a);
	static color _cdecl interpolate(const color &a, const color &b, float t);
	static color _cdecl from_int_rgb(int *i);
	static color _cdecl from_int_argb(int *i);
	static color _cdecl parse(const string &s);
};

extern const color White;
extern const color Black;
extern const color Grey;
extern const color Gray;
extern const color Red;
extern const color Green;
extern const color Blue;
extern const color Yellow;
extern const color Orange;
extern const color Purple;

#endif
