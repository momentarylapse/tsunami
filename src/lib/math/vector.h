
#ifndef _MATH_VECTOR_INCLUDED_
#define _MATH_VECTOR_INCLUDED_

#include <math.h>

class matrix;
class matrix3;

class vector
{
public:
	float x, y, z;
	vector(){};
	vector(float x, float y, float z);
	// assignment operators
	vector& _cdecl operator += (const vector& v);
	vector& _cdecl operator -= (const vector& v);
	vector& _cdecl operator *= (float f);
	vector& _cdecl operator /= (float f);
	// unitary operator(s)
	vector _cdecl operator - () const;
	// binary operators
	vector _cdecl operator + (const vector &v) const;
	vector _cdecl operator - (const vector &v) const;
	vector _cdecl operator * (float f) const;
	vector _cdecl operator / (float f) const;
	friend vector _cdecl operator * (float f,const vector &v)
	{	return v*f;	}
	bool _cdecl operator == (const vector &v) const;
	bool _cdecl operator != (const vector &v) const;
	float _cdecl operator * (const vector &v) const;
	vector operator ^ (const vector &v) const;
	string _cdecl str() const;

	float _cdecl length() const;
	float _cdecl length_sqr() const;
	float _cdecl length_fuzzy() const;
	void _cdecl normalize();
	vector _cdecl normalized() const;
	vector _cdecl ang2dir() const;
	vector _cdecl dir2ang() const;
	vector _cdecl dir2ang2(const vector &up) const;
	vector _cdecl ortho() const;
	int _cdecl important_plane() const;
	vector _cdecl rotate(const vector &ang) const;
	vector _cdecl transform(const matrix &m) const;
	vector _cdecl transform_normal(const matrix &m) const;
	vector _cdecl untransform(const matrix &m) const;
	vector _cdecl transform3(const matrix3 &m) const;
	void _cdecl _min(const vector &test_partner);
	void _cdecl _max(const vector &test_partner);
	bool _cdecl between(const vector &a, const vector &b) const;
	float _cdecl factor_between(const vector &a, const vector &b) const;
	bool _cdecl bounding_cube(const vector &a, float r) const;

	static const vector ZERO, EX, EY, EZ;
	
	static float _cdecl dot(const vector &v1, const vector &v2);
	static vector _cdecl cross(const vector &v1, const vector &v2);
};
// vectors
vector _cdecl VecAngAdd(const vector &ang1, const vector &ang2);
vector _cdecl VecAngInterpolate(const vector &ang1, const vector &ang2, float t);
float _cdecl VecLineDistance(const vector &p, const vector &l1, const vector &l2);
vector _cdecl VecLineNearestPoint(const vector &p, const vector &l1, const vector &l2);


const vector v_0 = vector(0, 0, 0);


float _vec_length_(const vector &v);
float _vec_length_fuzzy_(const vector &v);
void _vec_normalize_(vector &v);
bool _vec_between_(const vector &v,const vector &a,const vector &b);
float _vec_factor_between_(const vector &v,const vector &a,const vector &b);


#endif
