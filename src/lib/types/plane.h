
#ifndef _TYPES_PLANE_INCLUDED_
#define _TYPES_PLANE_INCLUDED_

struct plane
{
	vector n;
	float d;

	plane(){}
	plane(const vector &p, const vector &n)
	{	this->n = n;	d = - (p*n);	}
	plane(const vector &a, const vector &b, const vector &c)
	{
		n = (b-a) ^ (c - a);
		n.normalize();
		d= - (n*a);
	}
	string str() const
	{	return format("(%f, %f, %f, %f)", n.x, n.y, n.z, d);	}

	bool intersect_line(const vector &l1, const vector &l2, vector &i) const;
	float distance(const vector &p) const;
	void inverse();
};

// planes
void PlaneFromPoints(plane &pl,const vector &a,const vector &b,const vector &c);
void PlaneFromPointNormal(plane &pl,const vector &p,const vector &n);
void PlaneTransform(plane &plo,const matrix &m,const plane &pli);
void GetBaryCentric(const vector &P,const vector &A,const vector &B,const vector &C,float &f,float &g);
extern float LineIntersectsTriangleF,LineIntersectsTriangleG;
bool LineIntersectsTriangle(const vector &t1,const vector &t2,const vector &t3,const vector &l1,const vector &l2,vector &col,bool vm);
bool LineIntersectsTriangle2(const plane &pl, const vector &t1,const vector &t2,const vector &t3,const vector &l1,const vector &l2,vector &col,bool vm);

#endif
