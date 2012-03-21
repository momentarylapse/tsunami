
struct vector
{
public:
	float x,y,z;
	vector(){};
	vector(float x,float y,float z)
	{	this->x=x;	this->y=y;	this->z=z;	}
	// assignment operators
	vector& operator += (const vector& v)
	{	x+=v.x;	y+=v.y;	z+=v.z;	return *this;	}
	vector& operator -= (const vector& v)
	{	x-=v.x;	y-=v.y;	z-=v.z;	return *this;	}
	vector& operator *= (float f)
	{	x*=f;	y*=f;	z*=f;	return *this;	}
	vector& operator /= (float f)
	{	x/=f;	y/=f;	z/=f;	return *this;	}
	// unitary operator(s)
	vector operator - () const
	{	return vector(-x,-y,-z);	}
	// binary operators
	vector operator + (const vector &v) const
	{	return vector( x+v.x , y+v.y , z+v.z );	}
	vector operator - (const vector &v) const
	{	return vector( x-v.x , y-v.y , z-v.z );	}
	vector operator * (float f) const
	{	return vector( x*f , y*f , z*f );	}
	vector operator / (float f) const
	{	return vector( x/f , y/f , z/f );	}
	friend vector operator * (float f,const vector &v)
	{	return v*f;	}
	bool operator == (const vector &v) const
	{	return ((x==v.x)&&(y==v.y)&&(z==v.z));	}
	bool operator != (const vector &v) const
	{	return !((x==v.x)&&(y==v.y)&&(z==v.z));	}
	float operator * (vector v) const
	{	return x*v.x + y*v.y + z*v.z;	}
	vector operator ^ (vector v) const
	{	return vector( y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x );	}
};
// vectors
float _cdecl VecLength(const vector &v);
float VecLengthFuzzy(const vector &v);
float VecLengthSqr(const vector &v);
bool VecBetween(const vector &v, const vector &a, const vector &b);
float VecFactorBetween(const vector &v, const vector &a, const vector &b);
bool VecBoundingBox(const vector &a, const vector &b, float d);
void _cdecl VecNormalize(vector &o);
float _cdecl VecDotProduct(const vector &v1, const vector &v2);
vector _cdecl VecCrossProduct(const vector &v1, const vector &v2);
vector _cdecl VecAng2Dir(const vector &ang);
vector _cdecl VecDir2Ang(const vector &dir);
vector _cdecl VecDir2Ang2(const vector &dir, const vector &up);
vector _cdecl VecAngAdd(const vector &ang1, const vector &ang2);
vector _cdecl VecAngInterpolate(const vector &ang1, const vector &ang2, float t);
vector _cdecl VecRotate(const vector &v, const vector &ang);
vector _cdecl VecOrtho(const vector &v);
int VecImportantPlane(const vector &v);
float VecLineDistance(const vector &p, const vector &l1, const vector &l2);
vector VecLineNearestPoint(vector &p, vector &l1,vector &l2);
void _cdecl VecTransform(vector &vo, const matrix &m, const vector &vi);
void _cdecl VecNormalTransform(vector &vo, const matrix &m, const vector &vi);
void _cdecl VecTransform3(vector &vo, const matrix3 &m, const vector &vi);
void VecMin(vector &v, const vector &test_partner);
void VecMax(vector &v, const vector &test_partner);

const vector v0 = vector(0, 0, 0);
const vector e_x = vector(1, 0, 0);
const vector e_y = vector(0, 1, 0);
const vector e_z = vector(0, 0, 1);

