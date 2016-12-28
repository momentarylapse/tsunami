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


//------------------------------------------------------------------------------------------------//
//                                             colors                                             //
//------------------------------------------------------------------------------------------------//


void color::clamp()
{
	a = clampf(a, 0, 1);
	r = clampf(r, 0, 1);
	g = clampf(g, 0, 1);
	b = clampf(b, 0, 1);
}

// create a color from (alpha, red, green blue)
// (values of set [0..1])
color SetColorSave(float a,float r,float g, float b)
{
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

color SetColorHSB(float a,float hue,float saturation,float brightness)
{
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
color ColorInterpolate(const color &a,const color &b,float t)
{
	return (1-t)*a + t*b;
}

color ColorFromIntRGB(int *i)
{
	return color(	1,
					(float)i[0]/255.0f,
					(float)i[1]/255.0f,
					(float)i[2]/255.0f);
}

color ColorFromIntARGB(int *i)
{
	return color(	(float)i[0]/255.0f,
					(float)i[1]/255.0f,
					(float)i[2]/255.0f,
					(float)i[3]/255.0f);
}

void color::get_int_rgb(int *i) const
{
	i[0] = int(r * 255.0f);
	i[1] = int(g * 255.0f);
	i[2] = int(b * 255.0f);
}

void color::get_int_argb(int *i) const
{
	i[0] = int(a * 255.0f);
	i[1] = int(r * 255.0f);
	i[2] = int(g * 255.0f);
	i[3] = int(b * 255.0f);
}
