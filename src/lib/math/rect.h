
#ifndef _MATH_RECT_INCLUDED_
#define _MATH_RECT_INCLUDED_

// types
struct rect
{
public:
	float x1, x2, y1, y2;
	rect(){};
	rect(float x1, float x2, float y1, float y2);
	string str() const;
	float width() const;
	float height() const;
	float area() const;
	bool inside(float x, float y) const;
};

// rects
const rect r_id = rect(0, 1, 0, 1);

#endif
