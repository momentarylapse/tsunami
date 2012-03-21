#include "types.h"

//------------------------------------------------------------------------------------------------//
//                                             planes                                             //
//------------------------------------------------------------------------------------------------//

float LineIntersectsTriangleF, LineIntersectsTriangleG;

// plane containing a, b, c
void PlaneFromPoints(plane &pl,const vector &a,const vector &b,const vector &c)
{
	vector ba=b-a,ca=c-a,n;
	n=VecCrossProduct(ba,ca);
	VecNormalize(n);
	pl.a=n.x;	pl.b=n.y;	pl.c=n.z;
	pl.d=-(n*a);
}

// plane containing p an having the normal vector n
void PlaneFromPointNormal(plane &pl,const vector &p,const vector &n)
{
	pl.a=n.x;	pl.b=n.y;	pl.c=n.z;
	pl.d=-(n*p);
}

// rotate and move a plane with a matrix
//    please don't scale...
void PlaneTransform(plane &plo,const matrix &m,const plane &pli)
{
	// transform the normalvector  (n' = R n)
	plo.a= pli.a*m._00 + pli.b*m._01 + pli.c*m._02;
	plo.b= pli.a*m._10 + pli.b*m._11 + pli.c*m._12;
	plo.c= pli.a*m._20 + pli.b*m._21 + pli.c*m._22;
	// offset (d' = d - < T, n' >)
	plo.d= pli.d - plo.a*m._03 - plo.b*m._13 - plo.c*m._23;
}

// return the normal vector of a plane
vector GetNormal(const plane &pl)
{
	vector n=vector(pl.a,pl.b,pl.c);
	return n;
}

// intersection of plane <pl> and the line through l1 and l2
// (false if parallel!)
bool PlaneIntersectLine(vector &i,const plane &pl,const vector &l1,const vector &l2)
{
	vector n=vector(pl.a,pl.b,pl.c);
	float d=-pl.d;
	float e=VecDotProduct(n,l1);
	float f=VecDotProduct(n,l2);
	if (e==f) // parallel?
		return false;
	float t=(d-f)/(e-f);
	//if ((t>=0)&&(t<=1)){
		//i = l1 + t*(l2-l1);
		i = l2 + t*(l1-l2);
		return true;
}

// reflect the plane on itself
void PlaneInverse(plane &pl)
{
	pl.a=-pl.a;
	pl.b=-pl.b;
	pl.c=-pl.c;
}

// P = A + f*( B - A ) + g*( C - A )
void GetBaryCentric(const vector &P,const vector &A,const vector &B,const vector &C,float &f,float &g)
{
	// Bezugs-System: A
	vector ba=B-A,ca=C-A,dir;
	plane pl;
	PlaneFromPoints(pl,A,B,C); // Ebene des Dreiecks
	dir=GetNormal(pl);//vector(pl.a,pl.b,pl.c); // Normalen-Vektor
	vector pvec;
	pvec=VecCrossProduct(dir,ca); // Laenge: |ca|         Richtung: Dreiecks-Ebene, orth zu ca
	float det=VecDotProduct(ba,pvec); // = |ba| * |ca| * cos( pvec->ba )   -> =Flaeche des Parallelogramms
	vector pa;
	if (det>0)
		pa=P-A;
	else
	{
		pa=A-P;
		det=-det;
	}
	f=VecDotProduct(pa,pvec);
	vector qvec;
	qvec=VecCrossProduct(pa,ba);
	g=VecDotProduct(dir,qvec);
	//float t=VecDotProduct(ca,qvec);
	float InvDet=1.0f/det;
	//t*=InvDet;
	f*=InvDet;
	g*=InvDet;
}

// wird das Dreieck(t1,t2,t3) von der Geraden(l1,l2) geschnitten?
// Schnittpunkt = col
bool LineIntersectsTriangle(const vector &t1,const vector &t2,const vector &t3,const vector &l1,const vector &l2,vector &col,bool vm)
{
	plane p;
	PlaneFromPoints(p,t1,t2,t3);
	if (!PlaneIntersectLine(col,p,l1,l2))
		return false;
	GetBaryCentric(col,t1,t2,t3,LineIntersectsTriangleF,LineIntersectsTriangleG);
	if ((LineIntersectsTriangleF>0)&&(LineIntersectsTriangleG>0)&&(LineIntersectsTriangleF+LineIntersectsTriangleG<1))
		return true;
	return false;
}
 
// wird das Dreieck(t1,t2,t3) von der Geraden(l1,l2) geschnitten?
// Schnittpunkt = col
// ------------ GetBaryCentric....
bool LineIntersectsTriangle2(const plane &pl,const vector &t1,const vector &t2,const vector &t3,const vector &l1,const vector &l2,vector &col,bool vm)
{
	if (!PlaneIntersectLine(col,pl,l1,l2))
		return false;
	GetBaryCentric(col,t1,t2,t3,LineIntersectsTriangleF,LineIntersectsTriangleG);
	if ((LineIntersectsTriangleF>0)&&(LineIntersectsTriangleG>0)&&(LineIntersectsTriangleF+LineIntersectsTriangleG<1))
		return true;
	return false;
}

// distance <point p> to <plane pl>
float PlaneDistance(const plane &pl,const vector &p)
{
	return pl.a*p.x + pl.b*p.y + pl.c*p.z + pl.d;
}

