
struct plane{
	float a,b,c,d;
};

// planes
void PlaneFromPoints(plane &pl,const vector &a,const vector &b,const vector &c);
void PlaneFromPointNormal(plane &pl,const vector &p,const vector &n);
void PlaneTransform(plane &plo,const matrix &m,const plane &pli);
vector GetNormal(const plane &pl);
bool PlaneIntersectLine(vector &i,const plane &pl,const vector &l1,const vector &l2);
void GetBaryCentric(const vector &P,const vector &A,const vector &B,const vector &C,float &f,float &g);
extern float LineIntersectsTriangleF,LineIntersectsTriangleG;
bool LineIntersectsTriangle(const vector &t1,const vector &t2,const vector &t3,const vector &l1,const vector &l2,vector &col,bool vm);
bool LineIntersectsTriangle2(const plane &pl, const vector &t1,const vector &t2,const vector &t3,const vector &l1,const vector &l2,vector &col,bool vm);
float PlaneDistance(const plane &pl,const vector &p);
void PlaneInverse(plane &pl);
