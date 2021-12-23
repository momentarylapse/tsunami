
#pragma once


#include "math.h"
#include "vector.h"

class vector;
class matrix;
class vec2;

class plane {
public:
	vector n;
	float d;

	plane(){}
	plane(const vector &n, float d);
	string _cdecl str() const;

	bool _cdecl intersect_line(const vector &l1, const vector &l2, vector &i) const;
	float _cdecl distance(const vector &p) const;
	plane _cdecl inverse() const;
	plane _cdecl transform(const matrix &m) const;
	
	// creation
	static plane _cdecl from_points(const vector &a, const vector &b, const vector &c);
	static plane _cdecl from_point_normal(const vector &p, const vector &n);
};


bool inf_pl(plane p);


// planes
vec2 _cdecl bary_centric(const vector &P, const vector &A, const vector &B, const vector &C);
vec2 bary_centric2(const plane &pl, const vector &P, const vector &A, const vector &B, const vector &C);
extern vec2 line_intersects_triangle_fg;
bool _cdecl line_intersects_triangle(const vector &t1, const vector &t2, const vector &t3, const vector &l1, const vector &l2, vector &col);
bool _cdecl line_intersects_triangle2(const plane &pl, const vector &t1, const vector &t2, const vector &t3, const vector &l1, const vector &l2, vector &col);
