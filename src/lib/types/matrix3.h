
struct matrix3
{
public:
	union{
		struct{
			// the squared form of this block is "transposed"!
			float _00,_10,_20;
			float _01,_11,_21;
			float _02,_12,_22;
		};
		float __e[3][3];
#define _e3(i,j)		__e[j][i]
		float e[9];
	};

	matrix3(){};
	matrix3 operator + (const matrix3 &m) const
	{
		matrix3 r;
		for (int i=0;i<9;i++)
			r.e[i]=e[i]+m.e[i];
		return r;
	}
	matrix3 operator - (const matrix3 &m) const
	{
		matrix3 r;
		for (int i=0;i<9;i++)
			r.e[i]=e[i]-m.e[i];
		return r;
	}
	matrix3 operator * (float f) const
	{
		matrix3 r = *this;
		r *= f;
		return r;
	}
	matrix3 operator *= (float f)
	{
		for (int i=0;i<9;i++)
			e[i] *= f;
		return *this;
	}
	friend matrix3 operator * (float f,const matrix3 &m)
	{	return m * f;	}
	matrix3 operator / (float f) const
	{
		matrix3 r = *this;
		r /= f;
		return r;
	}
	matrix3 operator /= (float f)
	{
		for (int i=0;i<9;i++)
			e[i] /= f;
		return *this;
	}
	matrix3 operator * (const matrix3 &m) const
	{
		matrix3 r;
		r._00 = _00*m._00 + _01*m._10 + _02*m._20;
		r._01 = _00*m._01 + _01*m._11 + _02*m._21;
		r._02 = _00*m._02 + _01*m._12 + _02*m._22;
		r._10 = _10*m._00 + _11*m._10 + _12*m._20;
		r._11 = _10*m._01 + _11*m._11 + _12*m._21;
		r._12 = _10*m._02 + _11*m._12 + _12*m._22;
		r._20 = _20*m._00 + _21*m._10 + _22*m._20;
		r._21 = _20*m._01 + _21*m._11 + _22*m._21;
		r._22 = _20*m._02 + _21*m._12 + _22*m._22;
		return r;
	}
	vector operator * (const vector &v) const
	{
		return vector(	v.x*_00 + v.y*_01 + v.z*_02,
						v.x*_10 + v.y*_11 + v.z*_12,
						v.x*_20 + v.y*_21 + v.z*_22);
	}
	friend vector operator * (const vector &v, const matrix3 &m)
	{	return m*v;	}
	string str() const
	{	return format("(%f, %f, %f; %f, %f, %f; %f, %f, %f)", _00, _01, _02, _10, _11, _12, _20, _21, _22);	}
};

// matrix3s
void _cdecl Matrix3Identity(matrix3 &m);
void _cdecl Matrix3Inverse(matrix3 &mo, const matrix3 &mi);
void _cdecl Matrix3Transpose(matrix3 &mo, const matrix3 &mi);
void _cdecl Matrix3Rotation(matrix3 &m, const vector &ang);
void _cdecl Matrix3RotationQ(matrix3 &m, const quaternion &q);

