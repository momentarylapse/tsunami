#include "plane.h"
#include "mat4.h"
#include "vec2.h"


plane::plane(const vec3 &_n, float _d) {
	n = _n;
	d = _d;
}

bool plane::operator==(const plane& o) const {
	return n == o.n and d == o.d;
}

string plane::str() const {
	return format("(%f, %f, %f, %f)", n.x, n.y, n.z, d);
}

vec2 line_intersects_triangle_fg;

// plane containing a, b, c
plane plane::from_points(const vec3 &a,const vec3 &b,const vec3 &c) {
	plane pl;
	pl.n = vec3::cross(b - a, c - a);
	pl.n.normalize();
	pl.d = - vec3::dot(pl.n, a);
	return pl;
}

// plane containing p an having the normal vector n
plane plane::from_point_normal(const vec3 &p,const vec3 &n) {
	plane pl;
	pl.n = n;
	pl.d = - vec3::dot(n, p);
	return pl;
}

// rotate and move a plane with a matrix
//    please don't scale...
plane plane::transform(const mat4 &m) const {
	plane plo;
	// transform the normal vector  (n' = R n)
	plo.n = m.transform_normal(n);
	// offset (d' = d - < T, n' >)
	plo.d= d - plo.n.x*m._03 - plo.n.y*m._13 - plo.n.z*m._23;
	return plo;
}

// intersection of plane <pl> and the line through l1 and l2
// (false if parallel!)
bool plane::intersect_line(const vec3 &l1, const vec3 &l2, vec3 &i) const  {
	float _d = -d;
	float e = vec3::dot(n, l1);
	float f = vec3::dot(n, l2);
	if (e==f) // parallel?
		return false;
	float t= (_d-f) / (e-f);
	//if ((t>=0)&&(t<=1)){
		//i = l1 + t*(l2-l1);
		i = l2 + t * (l1-l2);
		return true;
}

// reflect the plane on itself
plane plane::inverse() const {
	return plane(-n, d);
}

vec2 bary_centric2(const plane &pl, const vec3 &P, const vec3 &A, const vec3 &B, const vec3 &C) {
	// relative to: A
	vec3 ba = B - A;
	vec3 ca = C - A;
	vec3 dir = pl.n; // normal vector
	vec3 pvec = vec3::cross(dir, ca); // length: |ca|         direction: triangle area, orthogonal to ca
	float det = vec3::dot(ba, pvec); // = |ba| * |ca| * cos( pvec->ba )   -> = area of parallelogram
	vec3 pa;
	if (det > 0) {
		pa = P - A;
	} else {
		pa = A - P;
		det = -det;
	}
	float f = vec3::dot(pa, pvec);
	vec3 qvec = vec3::cross(pa,ba);
	float g = vec3::dot(dir, qvec);
	float inv_det = 1.0f / det;
	return vec2(f, g) * inv_det;
}

// P = A + f*( B - A ) + g*( C - A )
vec2 bary_centric(const vec3 &P, const vec3 &A, const vec3 &B, const vec3 &C) {
	plane pl = plane::from_points(A, B, C); // plane of the triangle
	return bary_centric2(pl, P, A, B, C);
}

// do the triangle(t1,t2,t3) and the line(l1,l2) intersect?
//   intersection = col
bool line_intersects_triangle(const vec3 &t1,const vec3 &t2,const vec3 &t3,const vec3 &l1,const vec3 &l2,vec3 &col) {
	plane p = plane::from_points(t1,t2,t3);
	return line_intersects_triangle2(p, t1, t2, t3, l1, l2, col);
}

// do the triangle(t1,t2,t3) and the line(l1,l2) intersect?
//   intersection = col
bool line_intersects_triangle2(const plane &pl,const vec3 &t1,const vec3 &t2,const vec3 &t3,const vec3 &l1,const vec3 &l2,vec3 &col) {
	if (!pl.intersect_line(l1, l2, col))
		return false;
	line_intersects_triangle_fg = bary_centric2(pl, col,t1,t2,t3);
	if ((line_intersects_triangle_fg.x>0) and (line_intersects_triangle_fg.y>0) and (line_intersects_triangle_fg.x+line_intersects_triangle_fg.y<1))
		return true;
	return false;
}

// distance <point p> to <plane pl>
float plane::distance(const vec3 &p) const {
	return vec3::dot(n, p) + d;
}


bool inf_pl(plane p) {
	return (inf_v(p.n) or inf_f(p.d));
}

