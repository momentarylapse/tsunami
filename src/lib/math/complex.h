
#pragma once

class complex {
public:
	float x, y;
	complex(){};
	complex(float x, float y);
	// assignment operators
	complex& operator += (const complex& v);
	complex& operator -= (const complex& v);
	complex& operator *= (float f);
	complex& operator /= (float f);
	complex& operator *= (const complex &v);
	complex& operator /= (const complex &v);
	// unitary operator(s)
	complex operator - ();
	// binary operators
	complex operator + (const complex &v) const;
	complex operator - (const complex &v) const;
	complex operator * (float f) const;
	complex operator / (float f) const;
	complex operator * (const complex &v) const;
	complex operator / (const complex &v) const;
	friend complex operator * (float f,const complex &v)
	{	return v*f;	}
	bool operator == (const complex &v) const;
	bool operator != (const complex &v) const;
	float _cdecl abs() const;
	float _cdecl abs_sqr() const;
	complex _cdecl bar() const;
	string _cdecl str() const;

	static const complex ZERO;
	static const complex ONE;
	static const complex I;
};


