
#ifndef _TYPES_COLOR_INCLUDED_
#define _TYPES_COLOR_INCLUDED_

struct color
{
public:
	float r,g,b,a;
	color(){};
	color(float a,float r,float g,float b)
	{	this->a=a;	this->r=r;	this->g=g;	this->b=b;	}
	/*color& operator = (const color& c)
	{	a=c.a;	r=c.r;	g=c.g;	b=c.b;	return *this;	}*/
	color& operator += (const color& c)
	{	a += c.a;	r += c.r;	g += c.g;	b += c.b;	return *this;	}
	color& operator -= (const color& c)
	{	a -= c.a;	r -= c.r;	g -= c.g;	b -= c.b;	return *this;	}
	color operator + (const color& c) const
	{	return color(a + c.a, r + c.r, g + c.g, b + c.b);	}
	color operator - (const color& c) const
	{	return color(a - c.a, r - c.r, g - c.g, b - c.b);	}
	color operator * (float f) const
	{	return color(a*f , r*f , g*f , b*f);	}
	friend color operator * (float f, color &c)
	{	return c * f;	}
	void operator *= (float f)
	{	a *= f;	r *= f;	g *= f;	b *= f;	}
	color operator * (const color &c) const
	{	return color(a*c.a , r*c.r , g*c.g , b*c.b);	}
	void operator *= (const color &c)
	{	a*=c.a;	r*=c.r;	g*=c.g;	b*=c.b;	}
	void clamp();
	string str()
	{	return format("(%f, %f, %f, %f)", r, g, b, a);	}
};
// colors
color _cdecl SetColorSave(float a, float r, float g, float b);
color _cdecl SetColorHSB(float a, float hue, float saturation, float brightness);
color _cdecl ColorScale(const color &c, float f);
color _cdecl ColorInterpolate(const color &a, const color &b, float t);
color _cdecl ColorMultiply(const color &a, const color &b);
color _cdecl ColorFromIntRGB(int *i);
color _cdecl ColorFromIntARGB(int *i);
void _cdecl Color2IntRGB(const color &c, int *i);
void _cdecl Color2IntARGB(const color &c, int *i);

static color White  = color(1, 1, 1, 1);
static color Black  = color(1, 0, 0, 0);
static color Grey   = color(1, 0.5f, 0.5f, 0.5f);
static color Gray   = color(1, 0.5f, 0.5f, 0.5f);
static color Red    = color(1, 1, 0, 0);
static color Green  = color(1, 0, 1, 0);
static color Blue   = color(1, 0, 0, 1);
static color Yellow = color(1, 1, 1, 0);
static color Orange = color(1, 1, 0.5f, 0);

#endif
