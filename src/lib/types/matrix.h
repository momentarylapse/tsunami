
#ifndef _TYPES_MATRIX_INCLUDED_
#define _TYPES_MATRIX_INCLUDED_

//#define _element(row,col)	e[row+col*4]

class matrix;
class vector;
class quaternion;
matrix MatrixMultiply2(const matrix &m2, const matrix &m1);


struct matrix
{
public:
	union{
		struct{
			// the squared form of this block is "transposed"!
			float _00,_10,_20,_30;
			float _01,_11,_21,_31;
			float _02,_12,_22,_32;
			float _03,_13,_23,_33;
		};
		float __e[4][4];
//#define _e(i,j)		__e[j][i]
		float e[16];
	};

	matrix(){};
	matrix(const float f[16]){
		for (int i=0;i<16;i++)
			e[i]=f[i];
	};
	matrix(const vector &a, const vector &b, const vector &c){
		_00 = a.x;	_01 = b.x;	_02 = c.x;	_03 = 0;
		_10 = a.y;	_11 = b.y;	_12 = c.y;	_13 = 0;
		_20 = a.z;	_21 = b.z;	_22 = c.z;	_23 = 0;
		_30 = 0;	_31 = 0;	_32 = 0;	_33 = 1;
	}
	matrix operator + (const matrix &m) const
	{
		matrix r;
		for (int i=0;i<16;i++)
			r.e[i]=e[i]+m.e[i];
		return r;
	}
	matrix operator - (const matrix &m) const
	{
		matrix r;
		for (int i=0;i<16;i++)
			r.e[i]=e[i]-m.e[i];
		return r;
	}
	matrix operator * (const matrix &m) const
	{	return MatrixMultiply2(*this, m);	}
	/*matrix operator * (float f) const
	{
		matrix r;
		for (int i=0;i<16;i++)
			r.e[i]=e[i]*f;
		return r;
	}
	friend matrix operator * (float f, const matrix &m)
	{	return m*f;	}*/
	matrix operator *= (const matrix &m)
	{
		matrix r = (*this * m);
		*this = r;
		return *this;
	}
	vector operator * (const vector &v) const
	{
		return vector(	v.x*_00 + v.y*_01 + v.z*_02 + _03,
						v.x*_10 + v.y*_11 + v.z*_12 + _13,
						v.x*_20 + v.y*_21 + v.z*_22 + _23);
	}
	vector transform_normal(const vector &v) const
	{
		return vector(	v.x*_00 + v.y*_01 + v.z*_02,
						v.x*_10 + v.y*_11 + v.z*_12,
						v.x*_20 + v.y*_21 + v.z*_22);
	}
	vector project(const vector &v) const
	{	return (*this * v) / (v.x*_30 + v.y*_31 + v.z*_32 + _33);	}
	string str() const
	{	return format("(%f, %f, %f, %f; %f, %f, %f, %f; %f, %f, %f, %f; %f, %f, %f, %f)", _00, _01, _02, _03, _10, _11, _12, _13, _20, _21, _22, _23, _30, _31, _32, _33);	}

	// kaba
	void imul(const matrix &m)
	{	*this *= m;	}
	matrix mul(const matrix &m) const
	{	return *this * m;	}
	vector mul_v(const vector &v) const
	{	return *this * v;	}
};
// matrices
void _cdecl MatrixMultiply(matrix &m,const matrix &m2,const matrix &m1);
void _cdecl MatrixIdentity(matrix &m);
void _cdecl MatrixInverse(matrix &mo,const matrix &mi);
void _cdecl MatrixTranspose(matrix &mo,const matrix &mi);
void _cdecl MatrixTranslation(matrix &m,const vector &v);
void _cdecl MatrixRotationX(matrix &m,float w);
void _cdecl MatrixRotationY(matrix &m,float w);
void _cdecl MatrixRotationZ(matrix &m,float w);
void _cdecl MatrixRotation(matrix &m,const vector &ang);
void _cdecl MatrixRotationView(matrix &m,const vector &ang);
void _cdecl MatrixRotationQ(matrix &m,const quaternion &q);
void _cdecl MatrixScale(matrix &m,float fx,float fy,float fz);
void _cdecl MatrixReflect(matrix &m,const plane &pl);

matrix _cdecl MatrixMultiply2(const matrix &m2,const matrix &m1);
matrix _cdecl MatrixRotation2(const vector &ang);

const float f_m_id[16] = { 1,0,0,0 , 0,1,0,0 , 0,0,1,0 , 0,0,0,1 };
const matrix m_id = matrix(f_m_id);

#endif
