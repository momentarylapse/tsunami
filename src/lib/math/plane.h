
#pragma once


#include "math.h"
#include "vec3.h"

class vec3;
class mat4;
class vec2;

class plane {
public:
	vec3 n;
	float d;

	plane(){}
	plane(const vec3 &n, float d);
	string _cdecl str() const;

	bool _cdecl intersect_line(const vec3 &l1, const vec3 &l2, vec3 &i) const;
	float _cdecl distance(const vec3 &p) const;
	plane _cdecl inverse() const;
	plane _cdecl transform(const mat4 &m) const;
	
	// creation
	static plane _cdecl from_points(const vec3 &a, const vec3 &b, const vec3 &c);
	static plane _cdecl from_point_normal(const vec3 &p, const vec3 &n);
};


bool inf_pl(plane p);


// planes
vec2 _cdecl bary_centric(const vec3 &P, const vec3 &A, const vec3 &B, const vec3 &C);
vec2 bary_centric2(const plane &pl, const vec3 &P, const vec3 &A, const vec3 &B, const vec3 &C);
extern vec2 line_intersects_triangle_fg;
bool _cdecl line_intersects_triangle(const vec3 &t1, const vec3 &t2, const vec3 &t3, const vec3 &l1, const vec3 &l2, vec3 &col);
bool _cdecl line_intersects_triangle2(const plane &pl, const vec3 &t1, const vec3 &t2, const vec3 &t3, const vec3 &l1, const vec3 &l2, vec3 &col);
