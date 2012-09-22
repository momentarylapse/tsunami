
struct complex
{
	float x, y;
	complex(){};
	complex(float x, float y)
	{	this->x = x;	this->y = y;	}
	// assignment operators
	complex& operator += (const complex& v)
	{	x+=v.x;	y+=v.y;	return *this;	}
	complex& operator -= (const complex& v)
	{	x-=v.x;	y-=v.y;	return *this;	}
	complex& operator *= (float f)
	{	x*=f;	y*=f;	return *this;	}
	complex& operator /= (float f)
	{	x/=f;	y/=f;	return *this;	}
	complex& operator *= (const complex &v)
	{	*this = complex( x*v.x - y*v.y , y*v.x + x *v.y );	return *this;	}
	complex& operator /= (const complex &v)
	{	*this = complex( x*v.x + y*v.y , y*v.x - x *v.y ) / (v.x * v.x + v.y * v.y);	return *this;	}
	// unitary operator(s)
	complex operator - ()
	{	return complex(-x,-y);	}
	// binary operators
	complex operator + (const complex &v) const
	{	return complex( x+v.x , y+v.y );	}
	complex operator - (const complex &v) const
	{	return complex( x-v.x , y-v.y );	}
	complex operator * (float f) const
	{	return complex( x*f , y*f );	}
	complex operator / (float f) const
	{	return complex( x/f , y/f );	}
	complex operator * (const complex &v) const
	{	return complex( x*v.x - y*v.y , y*v.x + x *v.y );	}
	complex operator / (const complex &v) const
	{	return complex( x*v.x + y*v.y , y*v.x - x *v.y ) / (v.x * v.x + v.y * v.y);	}
	friend complex operator * (float f,const complex &v)
	{	return v*f;	}
	bool operator == (const complex &v) const
	{	return ((x==v.x)&&(y==v.y));	}
	bool operator != (const complex &v) const
	{	return !((x==v.x)&&(y==v.y));	}
	float abs() const
	{	return sqrt(x*x + y*y);	}
	float abs_sqr() const
	{	return x*x + y*y;	}
	string str() const
	{	return format("(%f, %f)", x, y);	}
};

// complex
const complex c_i = complex(0, 1);
