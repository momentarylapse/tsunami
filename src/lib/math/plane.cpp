#include "math.h"

//------------------------------------------------------------------------------------------------//
//                                             planes                                             //
//------------------------------------------------------------------------------------------------//


plane::plane(const vector &_n, float _d) {
	n = _n;
	d = _d;
}

string plane::str() const {
	return format("(%f, %f, %f, %f)", n.x, n.y, n.z, d);
}

float LineIntersectsTriangleF, LineIntersectsTriangleG;

// plane containing a, b, c
plane plane::from_points(const vector &a,const vector &b,const vector &c) {
	plane pl;
	pl.n = (b-a) ^ (c - a);
	pl.n.normalize();
	pl.d = - (pl.n*a);
	return pl;
}

// plane containing p an having the normal vector n
plane plane::from_point_normal(const vector &p,const vector &n) {
	plane pl;
	pl.n = n;
	pl.d = -(n*p);
	return pl;
}

// rotate and move a plane with a matrix
//    please don't scale...
plane plane::transform(const matrix &m) const {
	plane plo;
	// transform the normal vector  (n' = R n)
	plo.n = n.transform_normal(m);
	// offset (d' = d - < T, n' >)
	plo.d= d - plo.n.x*m._03 - plo.n.y*m._13 - plo.n.z*m._23;
	return plo;
}

// intersection of plane <pl> and the line through l1 and l2
// (false if parallel!)
bool plane::intersect_line(const vector &l1, const vector &l2, vector &i) const  {
	float _d = -d;
	float e = n*l1;
	float f = n*l2;
	if (e==f) // parallel?
		return false;
	float t=(_d-f)/(e-f);
	//if ((t>=0)&&(t<=1)){
		//i = l1 + t*(l2-l1);
		i = l2 + t*(l1-l2);
		return true;
}

// reflect the plane on itself
plane plane::inverse() const {
	return plane(-n, d);
}

// P = A + f*( B - A ) + g*( C - A )
void GetBaryCentric(const vector &P,const vector &A,const vector &B,const vector &C,float &f,float &g) {
	// Bezugs-System: A
	vector ba=B-A,ca=C-A,dir;
	plane pl = plane::from_points(A,B,C); // Ebene des Dreiecks
	dir=pl.n; // Normalen-Vektor
	vector pvec;
	pvec=vector::cross(dir,ca); // Laenge: |ca|         Richtung: Dreiecks-Ebene, orth zu ca
	float det=vector::dot(ba,pvec); // = |ba| * |ca| * cos( pvec->ba )   -> =Flaeche des Parallelogramms
	vector pa;
	if (det>0) {
		pa=P-A;
	} else {
		pa=A-P;
		det=-det;
	}
	f=vector::dot(pa,pvec);
	vector qvec;
	qvec=vector::cross(pa,ba);
	g=vector::dot(dir,qvec);
	//float t=VecDotProduct(ca,qvec);
	float InvDet=1.0f/det;
	//t*=InvDet;
	f*=InvDet;
	g*=InvDet;
}

// wird das Dreieck(t1,t2,t3) von der Geraden(l1,l2) geschnitten?
// Schnittpunkt = col
bool LineIntersectsTriangle(const vector &t1,const vector &t2,const vector &t3,const vector &l1,const vector &l2,vector &col,bool vm) {
	plane p = plane::from_points(t1,t2,t3);
	if (!p.intersect_line(l1, l2, col))
		return false;
	GetBaryCentric(col,t1,t2,t3,LineIntersectsTriangleF,LineIntersectsTriangleG);
	if ((LineIntersectsTriangleF>0)&&(LineIntersectsTriangleG>0)&&(LineIntersectsTriangleF+LineIntersectsTriangleG<1))
		return true;
	return false;
}
 
// wird das Dreieck(t1,t2,t3) von der Geraden(l1,l2) geschnitten?
// Schnittpunkt = col
// ------------ GetBaryCentric....
bool LineIntersectsTriangle2(const plane &pl,const vector &t1,const vector &t2,const vector &t3,const vector &l1,const vector &l2,vector &col,bool vm) {
	if (!pl.intersect_line(l1, l2, col))
		return false;
	GetBaryCentric(col,t1,t2,t3,LineIntersectsTriangleF,LineIntersectsTriangleG);
	if ((LineIntersectsTriangleF>0)&&(LineIntersectsTriangleG>0)&&(LineIntersectsTriangleF+LineIntersectsTriangleG<1))
		return true;
	return false;
}

// distance <point p> to <plane pl>
float plane::distance(const vector &p) const {
	return n * p + d;
}

