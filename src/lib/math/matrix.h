
#ifndef _MATH_MATRIX_INCLUDED_
#define _MATH_MATRIX_INCLUDED_

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
	matrix(const float f[16]);
	matrix(const vector &a, const vector &b, const vector &c);
	matrix operator + (const matrix &m) const;
	matrix operator - (const matrix &m) const;
	matrix operator * (const matrix &m) const;
	matrix operator * (float f) const;
	friend matrix operator * (float f, const matrix &m)
	{	return m*f;	}
	matrix operator *= (const matrix &m);
	vector operator * (const vector &v) const;
	float determinant() const;
	vector transform_normal(const vector &v) const;
	vector untransform(const vector &v) const;
	vector project(const vector &v) const;
	vector unproject(const vector &v) const;
	string str() const;

	// kaba
	void imul(const matrix &m);
	matrix mul(const matrix &m) const;
	vector mul_v(const vector &v) const;
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
void _cdecl MatrixPerspective(matrix &m, float fovy, float aspect, float z_near, float z_far);

matrix _cdecl MatrixMultiply2(const matrix &m2,const matrix &m1);
matrix _cdecl MatrixRotation2(const vector &ang);

const float f_m_id[16] = { 1,0,0,0 , 0,1,0,0 , 0,0,1,0 , 0,0,0,1 };
const matrix m_id = matrix(f_m_id);


inline vector *_matrix_get_translation_(const matrix &m)
{	return (vector*)&m._03;	} // (_03, _13, _23) happens to be aligned the right way...


#endif
