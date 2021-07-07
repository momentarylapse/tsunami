
#pragma once


#include "math.h"
#include "vector.h"

class vector;
class matrix;

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
void _cdecl GetBaryCentric(const vector &P,const vector &A,const vector &B,const vector &C,float &f,float &g);
extern float LineIntersectsTriangleF,LineIntersectsTriangleG;
bool _cdecl LineIntersectsTriangle(const vector &t1,const vector &t2,const vector &t3,const vector &l1,const vector &l2,vector &col,bool vm);
bool _cdecl LineIntersectsTriangle2(const plane &pl, const vector &t1,const vector &t2,const vector &t3,const vector &l1,const vector &l2,vector &col,bool vm);

#if 0
inline void _plane_from_point_normal_(plane &pl,const vector &p,const vector &n)
{
	pl.n=n;
	pl.d=-vector::dot(n,p);
}

inline bool _plane_intersect_line_(vector &cp,const plane &pl,const vector &l1,const vector &l2)
{
	float e=vector::dot(pl.n,l1);
	float f=vector::dot(pl.n,l2);
	if (e==f) // parallel?
		return false;
	float t=-(pl.d+f)/(e-f);
	//if ((t>=0)&&(t<=1)){
		//cp = l1 + t*(l2-l1);
		cp = l2 + t*(l1-l2);
	return true;
}

inline float _plane_distance_(const plane &pl,const vector &p)
{	return vector::dot(pl.n,p) + pl.d;	}

inline void _get_bary_centric_(const vector &p,const plane &pl,const vector &a,const vector &b,const vector &c,float &f,float &g)
{
	vector ba=b-a,ca=c-a;
	vector pvec=vector::cross(pl.n,ca);
	float det=vector::dot(ba,pvec);
	vector pa;
	if (det>0)
		pa=p-a;
	else{
		pa=a-p;
		det=-det;
	}
	f=vector::dot(pa,pvec);
	vector qvec=vector::cross(pa,ba);
	g=vector::dot(pl.n,qvec);
	float inv_det=1.0f/det;
	f*=inv_det;
	g*=inv_det;
}
#endif
