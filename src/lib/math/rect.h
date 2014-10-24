
#ifndef _MATH_RECT_INCLUDED_
#define _MATH_RECT_INCLUDED_

// types
class rect
{
public:
	float x1, x2, y1, y2;
	rect(){};
	rect(float x1, float x2, float y1, float y2);
	string _cdecl str() const;
	float _cdecl width() const;
	float _cdecl height() const;
	float _cdecl area() const;
	bool _cdecl inside(float x, float y) const;

	bool _cdecl operator==(const rect &r) const;
	bool _cdecl operator!=(const rect &r) const;
};

// rects
const rect r_id = rect(0, 1, 0, 1);

#endif
