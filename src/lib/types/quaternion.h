
struct quaternion
{
public:
	float x,y,z,w;
	quaternion(){};
	quaternion(const float w,const vector &v)
	{	this->w=w;	this->x=v.x;	this->y=v.y;	this->z=v.z;	}
	bool operator == (const quaternion& q) const
	{	return ((x==q.x)&&(y==q.y)&&(z==q.z)&&(w==q.w));	}
	bool operator != (const quaternion& q) const
	{	return !((x==q.x)&&(y==q.y)&&(z==q.z)&&(w!=q.w));	}
	quaternion& operator += (const quaternion& q)
	{	x+=q.x;	y+=q.y;	z+=q.z;	w+=q.w;	return *this;	}
	quaternion& operator -= (const quaternion& q)
	{	x-=q.x;	y-=q.y;	z-=q.z;	w-=q.w;	return *this;	}
	quaternion operator + (const quaternion &q) const
	{
		quaternion r;
		r.x=q.x+x;
		r.y=q.y+y;
		r.z=q.z+z;
		r.w=q.w+w;
		return r;
	}
	quaternion operator - (const quaternion &q) const
	{
		quaternion r;
		r.x=q.x-x;
		r.y=q.y-y;
		r.z=q.z-z;
		r.w=q.w-w;
		return r;
	}
	quaternion operator * (float f) const
	{
		quaternion r;
		r.x=x*f;
		r.y=y*f;
		r.z=z*f;
		r.w=w*f;
		return r;
	}
	quaternion operator * (const quaternion &q) const
	{
		quaternion r;
		r.w = w*q.w - x*q.x - y*q.y - z*q.z;
		r.x = w*q.x + x*q.w + y*q.z - z*q.y;
		r.y = w*q.y + y*q.w + z*q.x - x*q.z;
		r.z = w*q.z + z*q.w + x*q.y - y*q.x;
		return r;
	}
	friend quaternion operator * (float f,const quaternion &q)
	{	return q*f;	}
};
// qaternions
void _cdecl QuaternionRotationA(quaternion &q, const vector &axis, float w);
void _cdecl QuaternionRotationV(quaternion &q, const vector &ang);
//void QuaternionRotationView(quaternion &q, const vector &ang);
void _cdecl QuaternionRotationM(quaternion &q, const matrix &m);
void _cdecl QuaternionInverse(quaternion &qo, const quaternion &qi);
void _cdecl QuaternionMultiply(quaternion &q, const quaternion &q2, const quaternion &q1);
void _cdecl QuaternionInterpolate(quaternion &q, const quaternion &q1, const quaternion &q2, float t);
void _cdecl QuaternionInterpolate(quaternion &q, const quaternion &q1, const quaternion &q2, const quaternion &q3, const quaternion &q4, float t);
vector _cdecl QuaternionToAngle(const quaternion &q);
void _cdecl QuaternionScale(quaternion &q, float f);
void _cdecl QuaternionNormalize(quaternion &qo, const quaternion &qi);
vector GetAxis(const quaternion &q);
float GetAngle(const quaternion &q);

const quaternion q_id=quaternion(1, v0);

