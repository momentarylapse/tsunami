
#ifndef _IMAGE_COLOR_INCLUDED_
#define _IMAGE_COLOR_INCLUDED_


#include "../base/base.h"

class color
{
public:
	float r,g,b,a;
	color(){};
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
	void _cdecl clamp();
	string _cdecl str() const
	{	return format("(%f, %f, %f, %f)", r, g, b, a);	}

	void _cdecl get_int_rgb(int *i) const;
	void _cdecl get_int_argb(int *i) const;
};
// colors
color _cdecl SetColorSave(float a, float r, float g, float b);
color _cdecl SetColorHSB(float a, float hue, float saturation, float brightness);
color _cdecl ColorInterpolate(const color &a, const color &b, float t);
color _cdecl ColorFromIntRGB(int *i);
color _cdecl ColorFromIntARGB(int *i);

extern const color White;
extern const color Black;
extern const color Grey;
extern const color Gray;
extern const color Red;
extern const color Green;
extern const color Blue;
extern const color Yellow;
extern const color Orange;

#endif
