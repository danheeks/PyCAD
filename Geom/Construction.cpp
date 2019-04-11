// ***************************************************************************************************************************************
//                    Point, CLine & Circle classes part of geometry.lib
//                    g.j.hawkesford August 2006 for Camtek Gmbh
//
// This program is released under the BSD license. See the file COPYING for details.
//
// ***************************************************************************************************************************************

#include "geometry.h"

	int   UNITS = MM;
	double TOLERANCE = 1.0e-06;
	double TOLERANCE_SQ = TOLERANCE * TOLERANCE;
	double TIGHT_TOLERANCE = 1.0e-09;
	double UNIT_VECTOR_TOLERANCE = 1.0e-10;
	double RESOLUTION = 1.0e-06;

	// dummy functions
	const wchar_t* getMessage(const wchar_t* original, int messageGroup, int stringID){return original;}
	const wchar_t* getMessage(const wchar_t* original){return original;}
	void FAILURE(const wchar_t* str){throw(str);}
	void FAILURE(const std::wstring& str){throw(str);}

	void set_Tolerances(int mode) {
		UNIT_VECTOR_TOLERANCE = 1.0e-10;
		switch (UNITS = mode)
		{
		case MM:
			TOLERANCE = 1.0e-03;					// Peps
			RESOLUTION = 1.0e-03;
			TIGHT_TOLERANCE = 1.0e-06;
			break;
		case INCHES:
			TOLERANCE = 1.0e-04;					// Peps
			RESOLUTION = 1.0e-04;
			TIGHT_TOLERANCE = 1.0e-7;
			break;
		case METRES:
			TOLERANCE = 1.0e-06;					// p4c...SW
			RESOLUTION = 1.0e-06;
			TIGHT_TOLERANCE = 1.0e-09;
			break;
		default:
			FAILURE(L"INVALID UNITS");
		}
		TOLERANCE_SQ = TOLERANCE * TOLERANCE;
	}

	double mm(double value) {
		switch(UNITS) {
			default:
				return value;
			case METRES:
				return value * .001;
			case INCHES:
				return value / 25.4;
		}
	}

	// ostream operators  = non-member overload
	// *********************************************************************************************************
	wostream& operator << (wostream& op, Point& p){
		// for debug - print point to file
		op << L" x=\"" << p.x << L"\" y=\"" << p.y << L"\"";
		return op;
	}

	wostream& operator <<(wostream& op, CLine& cl){
		// for debug - print cline to file
		if(cl.ok == false)
			op << L"(CLine UNSET)";
		else
			op << L"sp=" << cl.p << L" v=" << cl.v;
		return op;
	}

	wostream& operator <<(wostream& op, Plane& pl){
		// for debug - print plane to file stream
		if(pl.ok == false)
			op << L"(Plane UNSET)";
		else
			op << L"d=" << pl.d << L" normal=" << pl.normal;
		return op;
	}

	wostream& operator << (wostream& op, Point3d& p){
		// for debug - print point to file
//		if(p.ok == false)
//			op << "ok=\"false\"";
//		else
			op << L"x=\"" << p.x << L"\" y=\"" << p.y << L"\" z=" << p.z << L"\"";
		return op;

	}

	wostream& operator <<(wostream& op, Circle& c){
		// for debug - print circle to file
		if(c.ok == false)
			op << L"ok=\"false\"";
		else
			op << L" x=\"" << c.pc.x << L"\" y=\"" << c.pc.y << L"\" radius=\"" << c.radius << L"\"";
		return op;
	}

	bool Point3d::operator==(const Point3d &p)const{
		// p1 == p2 (uses TOLERANCE)
		if(FNE(this->x, p.x, TOLERANCE) || FNE(this->y, p.y, TOLERANCE) || FNE(this->z, p.z, TOLERANCE)) return false;
		return true;
	}

	Point3d Point3d::Transformed(const Matrix& m) {
		// transform Point
		Point3d ret;
		m.Transform(&x, &ret.x);
//		ret.ok = true;
		return ret;
	}

	double Point3d::Dist(const Point3d& p)const {												// distance between 2 points
		return Point3d(*this, p).magnitude();
	}

	double Point3d::DistSq(const Point3d& p)const {			// distance squared
		return (this->x - p.x) * (this->x - p.x) + (this->y - p.y) * (this->y - p.y) + (this->z - p.z) * (this->z - p.z);
	}

	Point3d Point3d::Mid(const Point3d& p, double factor)const{
		// Mid
		return Point3d(*this, p) * factor + *this;
	}

	ostream & operator<<(ostream &os, const Point3d &p)
	{
		return os << "Point3d x = " << p.x << ", y = " << p.y << ", z = " << p.z;
	}

	Point Mid(const Point& p0, const Point& p1, double factor){
		// mid or partway between 2 points
		return Point(p0, p1) * factor + p0;
	}
	Point Rel(const Point& p, double x0, double y0) {
		// Relative point
		return Point(p.x + x0, p.y + y0);
	}

	Point Polar(const Point& p, double angle, double r) {
		// polar from this point
		angle *= DegreesToRadians;
		return Point(p.x + r * cos(angle), p.y + r * sin(angle));
	}

	// ***************************************************************************************************************************************
	// clines
	// ***************************************************************************************************************************************
	//const CLine horiz(Point(0, 0), 1, 0);											// define global horizontal line

	double CLine::c() {
		// returns c for ax + by + c = 0 format (peps format where needed)
		return (v.x * p.y - v.y * p.x);
	}
	void CLine::Normalise() {
		// normalise the cline vector
		ok = v.normalise() >= TOLERANCE;
	}

	CLine Normal(const CLine& s) {
		// returns normal to this line
		return CLine(s.p, ~s.v, false);
	}
	const CLine CLine::operator ~(void){
		return CLine(this->p, ~v, false);
	}
	CLine Normal(const CLine& s, const Point& p) {
		// returns normal to this line thro' p
		return CLine(p, ~s.v, false);
	}

	CLine CLine::Transform(Matrix& m) {

		Point p0 = this->p;
		Point p1(p0.x + v.x, p0.y + v.y);
		p0.Transform(m);
		p1.Transform(m);
		return CLine(p0, p1);
	}

	void Point::Transform(const Matrix &m)
	{
		Point p(x, y);
		p = p.Transformed(m);
		x = p.x;
		y = p.y;
	}
	Point Point::Transformed(const Matrix& m) {
		// transform Point
		Point ret;
		m.Transform2d(&x, &ret.x);
		return ret;
	}


	double CLine::Dist(const Point& p0)const {
		// distance between cline & point  >0 cw about point   <0 acw about point
		return this->v ^ Point(p0, this->p);
	}

	Point CLine::Intof(const CLine& s)	{
		// Intof 2 Clines
		return ::Intof(*this, s);
	}

	Point CLine::Intof(int NF, const Circle& c)	{
		// Intof Cline & Circleconst 
		return ::Intof(NF, *this, c);
	}
	Point CLine::Intof(int NF, const Circle& c, Point& otherInters)	{
		// Intof Cline & Circle & other intersection
		return ::Intof(NF, *this, c, otherInters);
	}

	Point Intof(const CLine& s0, const CLine& s1)	{
		// inters of 2 clines  (parameterise lines x = x0 + t * dx)
		double cp = s1.v ^ s0.v;
		if(fabs (cp) > 1.0e-6) {
			double t = (s1.v ^ Point(s0.p, s1.p)) / cp;
			return s0.v * t + s0.p;
		}
		return INVALID_POINT;
	}
	Point XonCLine(CLine& s, double xval) {
		// return point given X on a line
		return Intof(s, CLine(Point(xval,0),0,1,false));
	}
	Point YonCLine(CLine& s, double yval) {
		// return point given Y on a line
		return Intof(s, CLine(Point(0,yval),1,0,false));
	}
	Point Along(const CLine& s, double d) {
		// distance along line
		return Point(s.p.x + d * s.v.x, s.p.y + d * s.v.y);
	}

	Point Along(const CLine& s, double d, Point& p) {
		// distance along line from point
		return Point(p.x + d * s.v.x, p.y + d * s.v.y);
	}
	Point Around(const Circle& c, double d, const Point& p) {
		// distance around circle from point
		CLine radial(c.pc, p);
		if(radial.ok) {
			if(fabs(c.radius) > TOLERANCE ) {
				double a = sin(- d / c.radius);
				double b = cos(- d / c.radius);
				return Point(c.pc.x - c.radius * (radial.v.y * a - radial.v.x * b), c.pc.y + c.radius * (radial.v.y * b + radial.v.x * a));
			}
		}
		return INVALID_POINT;
	}
	CLine AtAngle(double angle, const Point& p0, const CLine& s) {
		// cline at angle [to a cline] thro' a point
		angle *= DegreesToRadians;
		Point v(cos(angle), sin(angle));
		return CLine(p0, v.x * s.v.x - v.y * s.v.y, v.y * s.v.x + v.x * s.v.y);
	}
	CLine Parallel(int side, const CLine& s0, double distance) {
		// parallel to line by distance
		Point v = ~s0.v;
		return CLine(v * ((double)side * distance) + s0.p, s0.v.x, s0.v.y);
	}

	CLine Parallel(const CLine& s0, Point& p) {
		// parallel to line through point
		return CLine(p, s0.v.x, s0.v.y);
	}

	CLine CLine::Bisector(const CLine& s) {
		//  bisector of 2 clines
		return CLine (this->Intof(s), this->v.x + s.v.x, this->v.y + s.v.y);
	}




	// ***************************************************************************************************************************************
	// circle methods
	// ***************************************************************************************************************************************

	Circle::Circle(const Point& p, double rad, bool okay){
		// Circle
		pc = p;
		radius = rad;
		ok = okay;
	}

	Circle::Circle( const Point& p, const Point& pc0){
		ok = true;
		pc = pc0;
		radius = p.Dist(pc0);
	}

	bool Circle::operator==(const Circle &c)const{
		// c1 == c2 (uses TOLERANCE)
		return FEQ(this->radius, c.radius, TOLERANCE) && (this->pc == c.pc);
	}

	Circle Circle::Transform(Matrix& m) { // transform
		Point p0 = this->pc;
		double scale;
		if(m.GetScale(scale) == false) FAILURE(getMessage(L"Differential Scale not allowed for this method", GEOMETRY_ERROR_MESSAGES, MES_DIFFSCALE));
		return Circle(p0.Transformed(m), radius * scale);
	}

	Point	Circle::Intof(int LR, const Circle& c1) {
		// intof 2 circles
		return ::Intof(LR, *this, c1);
	}
	Point	Circle::Intof(int LR, const Circle& c1, Point& otherInters) {
		// intof 2 circles, (returns the other intersection)
		return ::Intof(LR, *this, c1, otherInters);
	}
	int	Circle::Intof(const Circle& c1, Point& leftInters, Point& rightInters) {
		// intof 2 circles, (returns the other intersection)
		return ::Intof(*this, c1, leftInters, rightInters);
	}

	CLine	Circle::Tanto(int AT,  double angle, const CLine& s0) const{
		// cline tanto circle at angle to optional cline
		return ::Tanto(AT, *this, angle, s0);
	}

	CLine Tanto(int AT, const Circle& c, const Point& p) {
		// CLine tangent to a circle through a point
		Point v(p, c.pc);
		double d = v.magnitude();
		CLine s(p, ~v, false);								// initialise cline

		if ( d <  TOLERANCE || d <  fabs(c.radius) - TOLERANCE)	// point inside circle ?
			return INVALID_CLINE;
		else {
			if(d > fabs(c.radius) + TOLERANCE) {				// point outside circle
				v.Rotate(sqrt((d - c.radius) * (d + c.radius)), - AT * c.radius);
				s.v = v;
			}
		}
		s.Normalise();
		return s;
	}

	CLine Tanto(int AT0, const Circle& c0, int AT1, const Circle& c) {
		// cline tanto 2 circles
		CLine s;
		Circle c1 = c;
		c1.radius -= (double) (AT0 * AT1) * c0.radius;
		s = Tanto(AT1, c1, c0.pc);
		s.p.x += (double) AT0 * c0.radius * s.v.y;
		s.p.y -= (double) AT0 * c0.radius * s.v.x;
		return s;
	}

	CLine Tanto(int AT, const Circle& c, double angle, const CLine& s0) {
		// cline at an angle [to a cline] tanto a circle 
		CLine s = AtAngle(angle, c.pc, s0);
		s.p.x += (double) AT * c.radius * s.v.y;
		s.p.y -= (double) AT * c.radius * s.v.x;
		//	s.p += ~s.v * (AT * c.radius);
		s.ok = true;
		return s;
	}
	Point AtAngle(const Circle& c, double angle) {
		// Point at an angle on circle
		angle *= DegreesToRadians;
		return Point(c.pc.x + c.radius * cos(angle), c.pc.y + c.radius * sin(angle));
	}

	Point On(const CLine& s, const Point& p) {
		// returns point that is nearest to s from p
		double t = s.v * Point(s.p, p);
		return s.v * t + s.p;
	}

	Point On(const Circle& c, const Point& p) {
		// returns point that is nearest to c from p
		double r = p.Dist(c.pc);
		if(r < TOLERANCE) FAILURE(getMessage(L",Point on Circle centre - On(Circle& c, Point& p)", GEOMETRY_ERROR_MESSAGES, MES_POINTONCENTRE));
		return(Mid(p, c.pc, (r - c.radius) / r));
	}


	Point Intof( int NF, const CLine& s, const Circle& c) {
		// inters of cline & circle  eg.     p1 = Intof(NEARINT, s1, c1);
		Point otherInters;
		return Intof(NF, s, c, otherInters);
	}

	Point Intof( int NF, const CLine& s, const Circle& c, Point& otherInters) {
		// inters of cline & circle  eg.     p1 = Intof(NEARINT, s1, c1);
		// otherInters returns the other intersection
#if 1
		// solving	x = x0 + dx * t			x = y0 + dy * t
		//			x = xc + R * cos(a)		y = yc + R * sin(a)		for t
		// gives :-  t� (dx� + dy�) + 2t(dx*dx0 + dy*dy0) + (x0-xc)� + (y0-yc)� - R� = 0
		int nRoots;
		double t, tFar, tNear, tOther;
		Point v0(c.pc, s.p);
		if((nRoots = quadratic(1, 2 * (v0 * s.v), v0.magnitudesqd() - c.radius * c.radius, tFar, tNear)) != 0) {
			if(nRoots == 2 && NF == NEARINT) {
				t = tNear;
				tOther = tFar;
			} else {
				t = tFar;
				tOther = (nRoots == 2)?tNear : tFar;
			}
			otherInters =  s.v * tOther + s.p;
			return s.v * t + s.p;
		}
		return INVALID_POINT;
	}
#else
		// geometric solution - this is similar to the peps method, and it may offer better tolerancing than above??
		Point intof;
		CLine normal = Normal(s, c.pc);
		intof = s.Intof(normal);
		double d = intof.Dist(c.pc);

		if(fabs(d - c.radius) < TOLERANCE) return intof;						// tangent (near enough for non-large radius I suppose?)

		if(d > c.radius + TOLERANCE) return INVALID_POINT;					// no intersection

		double q = (c.radius - d) * (c.radius + d);
		if(q < 0) return intof;												// line inside tolerance

		return Along(s, -(double)NF * sqrt(q), intof);						// 2 intersections (return near/far case)
	}
#endif
	Point Intof( int intMode, const Circle& c0, const Circle& c1)	{
		// inters of 2 circles		 eg.     p1 = Intof(LEFTINT, c1, c2)
		Point otherInters;
		return Intof(intMode, c0, c1, otherInters);
	}

	Point Intof( int intMode, const Circle& c0, const Circle& c1, Point& otherInters)	{
		// inters of 2 circles		 eg.     p1 = Intof(LEFTINT, c1, c2);u
		Point pLeft, pRight;
		switch(Intof(c0, c1, pLeft, pRight)) {
	default:
		return INVALID_POINT;
	case 1:
		otherInters = pLeft;
		return pLeft;
	case 2:
		if(intMode == LEFTINT) {
			otherInters = pRight;
			return pLeft;
		}else {
			otherInters = pLeft;
			return pRight;
		}
		}
	}

	int Intof(const Circle& c0, const Circle& c1, Point& pLeft, Point& pRight)	{
		// inters of 2 circles
		// returns the number of intersctions
		Point v(c0.pc, c1.pc);
		double d = 	v.normalise();
		if(d < TOLERANCE)	return 0;									// co-incident circles

		double sum = fabs(c0.radius) + fabs(c1.radius);
		double diff = fabs(fabs(c0.radius) - fabs(c1.radius));
		if(d > sum + TOLERANCE || d < diff - TOLERANCE) return 0;

		// dist from centre of this circle to mid intersection
		double d0 = 0.5 * (d + (c0.radius + c1.radius) * (c0.radius - c1.radius) / d);
		if(d0 - c0.radius > TOLERANCE) return 0;						// circles don't intersect

		double h = (c0.radius - d0) * (c0.radius + d0);				// half distance between intersects squared
		if(h < 0) d0 = c0.radius;									// tangent
		pLeft = v * d0 + c0.pc;										// mid-point of intersects
		if(h < TOLERANCE_SQ) return 1;						// tangent
		h = sqrt(h);

		v = ~v;														// calculate 2 intersects
		pRight = v * h + pLeft;
		v = -v;
		pLeft = v * h + pLeft; 
		return 2;
	}

	Circle	Tanto(int NF, CLine& s0, Point& p, double rad) {
		// circle tanto a CLine thro' a point
		double d = s0.Dist(p);
		if(fabs(d) > rad + TOLERANCE) return INVALID_CIRCLE;	// point too far from line
		CLine s0offset = Parallel(RIGHTINT, s0, rad);

		return Circle(Intof(NF, s0offset, Circle(p, rad)), rad);
	}

	Circle	Tanto(int AT1, CLine& s1, int AT2, CLine& s2, double rad) {
		// circle tanto 2 clines with radius
		CLine Offs1	= Parallel(AT1, s1, rad);
		CLine Offs2	= Parallel(AT2, s2, rad);
		Point pc = Intof(Offs1, Offs2);
		return Circle(pc, rad);
	}

	Circle Tanto(int AT1, CLine s1, int AT2, CLine s2, int AT3, CLine s3) {
		// circle tanto 3 CLines
		double s1c = s1.c(), s2c = s2.c(), s3c = s3.c();
		double d =	  s1.v.y * (AT2 * s3.v.x - AT3 * s2.v.x)
			+ s2.v.y * (AT3 * s1.v.x - AT1 * s3.v.x)
			+ s3.v.y * (AT1 * s2.v.x - AT2 * s1.v.x);
		if(fabs(d) < UNIT_VECTOR_TOLERANCE) return INVALID_CIRCLE;
		double radius =  (s1.v.y * (s2.v.x * s3c - s3.v.x * s2c)
			+ s2.v.y * (s3.v.x * s1c - s1.v.x * s3c)
			+ s3.v.y * (s1.v.x * s2c - s2.v.x * s1c)) / d ;
		if(radius < TOLERANCE) return INVALID_CIRCLE;

		CLine Offs1	= Parallel(AT1, s1, radius);
		CLine Offs2	= Parallel(AT2, s2, radius);

		Point p = Intof(Offs1, Offs2);

		return Circle(p, radius);
	}

	Circle	Thro(int LR, const Point& p0, const Point& p1, double rad) {
		// circle thro' 2 points, given radius and side
		CLine thro(p0, p1);
		if(thro.ok) {
			double d = 0.5 * p0.Dist(p1);
			Point pm = Mid(p0, p1);

			if(d > rad + TOLERANCE) return INVALID_CIRCLE;
			else if(d > rad - TOLERANCE) {
				// within tolerance of centre of 2 points
				return Circle(pm, d);
			}
			else {
				// 2 solutions
				return Circle(Along(Normal(thro, pm), (double)LR * sqrt((rad + d) * (rad - d)), pm), rad);
			}
		}
		return INVALID_CIRCLE;
	}

	Circle	Thro(const Point& p0, const Point& p1) {
		// circle thro 2 points (diametric)
		return Circle(p0.Mid(p1), .5*p0.Dist(p1));
	}
	Circle	Thro(const Point& p0, const Point& p1, const Point& p2) {
		// circle thro 3 points
		CLine s0(p0, p1);
		if(!s0.ok) return Thro(p1,p2);		// p0 & p1 coincident

		CLine s1(p0, p2);
		if(!s1.ok) return Thro(p0, p1);		// p0 & p2 coincident

		CLine s2(p2, p1);
		if(!s2.ok) return Thro(p0, p2);		// p1 & p2 coincident

		Point p = Intof(Normal(s0, Mid(p0, p1)),  Normal(s1, Mid(p0, p2)));
		return Circle(p, p0.Dist(p), true);
	}
	Circle	Tanto(int NF, int AT0, const CLine& s0, int AT1, const Circle &c1, double rad) {
		// circle tanto cline & circle with radius
		CLine Offs0	= Parallel(AT0, s0, rad);
		Circle c2 = c1;
		c2.radius += AT1 * rad;
		Point pc = Intof(NF, Offs0, c2);
		return Circle(pc, rad);
	}

	Circle	Tanto( int LR, int AT0, const Circle& c0, const Point& p, double rad) {
		// circle tanto circle & thro' a point
		Circle c2 = c0;
		c2.radius += AT0 * rad;
		Circle c1(p, rad);
		Point pc = Intof(LR, c2, c1);
		return Circle(pc, rad);
	}
	Circle	Tanto(int LR, int AT0, const Circle& c0, int AT1, const Circle& c1, double rad) {
		// circle tanto 2 circles
		Circle c2 = c0;
		Circle c3 = c1;
		c2.radius += AT0 * rad;
		c3.radius += AT1 * rad;
		Point pc = Intof(LR, c2, c3);
		return Circle(pc, rad);
	}

	Circle Parallel(int side, const Circle& c0, double distance) {
		// parallel to circle by distance
		return Circle(c0.pc, c0.radius + (double) side * distance);
	}

	// distance
	double atn360(double dy, double dx) {
		// angle 0 to 2pi
		double ang = atan2(dy, dx);
		return ((ang < 0)? 2 * PI + ang : ang);
	}

	double Dist(const Point& p0, const Circle& c, const Point& p1) {
		// clockwise distance around c from p0 to p1 
		double a0 = atn360(p0.y - c.pc.y, p0.x - c.pc.x);
		double a1 = atn360(p1.y - c.pc.y ,p1.x - c.pc.x);
		if ( a1 > a0 ) a1 -= 2 * PI ;
		return (a0 - a1) * c.radius;
	}
	double Dist(const CLine& s, const Circle& c) {
		// distance between line and circle
		return fabs(s.Dist(c.pc)) - c.radius;
	}
	double Dist(const Circle& c0, const Circle& c1) {
		// distance between 2 circles
		return c0.pc.Dist(c1.pc) - c0.radius - c1.radius;
	}
	double Dist(const Circle& c, const Point& p) {
		// distance between circle and point
		return p.Dist(On(c, p));
	}

	double IncludedAngle(const Point& v0, const Point& v1, int dir) {
		// returns the absolute included angle between 2 vectors in the direction of dir ( 1=acw  -1=cw)
		double inc_ang = v0 * v1;
		if(inc_ang > 1. - UNIT_VECTOR_TOLERANCE) return 0;
		if(inc_ang < -1. + UNIT_VECTOR_TOLERANCE)
			inc_ang = PI;  
		else {									// dot product,   v1 . v2  =  cos ang
			if(inc_ang > 1.0) inc_ang = 1.0;
			inc_ang = acos(inc_ang);									// 0 to pi radians

			if(dir * (v0 ^ v1) < 0) inc_ang = 2 * PI - inc_ang ;		// cp
		}
		return dir * inc_ang;
	}

	double IncludedAngle(const Point3d& v0, const Point3d& v1, const Point3d& normal, int dir) {
		// returns the absolute included angle between 2 vectors in the direction of dir ( 1=acw  -1=cw) about normal
		double inc_ang = v0 * v1;

		if(inc_ang >= -NEARLY_ONE) {									// dot product,   v1 . v2  =  cos ang
			inc_ang = acos(inc_ang);									// 0 to pi radians

			if(dir * (normal * (v0 ^ v1)) < 0) inc_ang = 2 * PI - inc_ang ;		// cp
		}
		else
			inc_ang = PI;	// semi-cicle

		return dir * inc_ang;
	}

	int corner(const Point& v0, const Point& v1, double cpTol) {
		// returns corner
		//						0 (TANGENT) = tangent
		//						1 (LEFT)    = left turn
		//					   -1 (RIGHT)   = right turn
		double cp = v0 ^ v1;
		if(fabs(cp) < cpTol) return TANGENT;

		return (cp > 0)?GEOFF_LEFT : GEOFF_RIGHT;
	}

	int quadratic(double a, double b, double c, double& x0, double& x1) {
		// solves quadratic equation ax² + bx + c = 0
		// returns number of real roots
//		double epsilon = 1.0e-6;
		double epsilon = (UNITS == METRES)?1.0e-09 : 1.0e-06;
		double epsilonsq = epsilon * epsilon;
		if(fabs(a) < epsilon) {
			if(fabs(b) < epsilon) return 0;		// invalid
			x0 = - c / b;
			return 1;
		}
		b /= a;
		c /= a;
		double s = b * b - 4 * c;
		if(s < -epsilon) return 0;				// imaginary roots
		x0 = - 0.5 * b;
		if(s > epsilonsq) {
			s = 0.5 * sqrt(s);
			x1 = x0 - s;
			x0 += s;
			return 2;
		}
		return 1;
	}

	Plane::Plane(const Point3d& p0, const Point3d& p1, const Point3d& p2) {
		// constructor plane from 3 points
		normal = Point3d(p0, p1) ^ Point3d(p0, p2);
		normal.normalise();
		ok = (normal != NULL_VECTOR);
		d = -(normal * Point3d(p0));
	}

	Plane::Plane(const Point3d& p0, const Point3d& v, bool normalise) {
		// constructor plane from point & vector
		normal = v;
		if(normalise == true) normal.normalise();
		d = -(normal * Point3d(p0));
	}

	Plane::Plane(double dist, const Point3d& n) {
		normal = n;
		double mag = normal.normalise();
		if((ok = (normal != NULL_VECTOR))) d = dist / mag;
	}

	double Plane::Dist(const Point3d& p)const{
		// returns signed distance to plane from point p
	return (normal * Point3d(p)) + d;
}

	Point3d Plane::Near(const Point3d& p)const {
		// returns near point to p on the plane
		return - normal * Dist(p) + p;
	}

	bool Plane::Intof(const Line& l, Point3d& intof, double& t) const{
		// intersection between plane and line
		// input this plane, line
		// output intof
		// method returns true for valid intersection
		double den = l.v * this->normal;
		if(fabs(den) < UNIT_VECTOR_TOLERANCE)	return false; // line is parallel to the plane, return false, even if the line lies on the plane

		t = -(normal * Point3d(l.p0) + d) / den;
		intof = l.v * t + l.p0;
		return true;
	}

	bool Plane::Intof(const Plane& pl, Line& intof)const {
		// intersection of 2 planes
		Point3d d = this->normal ^ pl.normal;
		d.normalise();
		intof.ok = false;
		if(d == NULL_VECTOR) return false;		// parallel planes

		intof.v = d;
		intof.length = 1;
		
		double dot = this->normal * pl.normal;

		double den = dot * dot - 1.;
		double a = (this->d - pl.d * dot) / den;
		double b = (pl.d - this->d * dot) / den;
		intof.p0 = this->normal * a + pl.normal * b;
		intof.ok = true;
		return true;
	}

	bool Plane::Intof(const Plane& pl0, const Plane& pl1, Point3d& intof) const{
		// intersection of 3 planes
		Line tmp;
		if(Intof(pl0, tmp)) {
			double t;
			return pl1.Intof(tmp, intof, t);
		}
		return false;
	}



	bool calculate_biarc_points(const Point3d &p0, Point3d v_start, const Point3d &p4, Point3d v_end, Point3d &p1, Point3d &p2, Point3d &p3)
	{
		if (v_start.magnitude() < 0.0000000001)v_start = Point3d(p0, p1);
		if (v_end.magnitude() < 0.0000000001)v_end = Point3d(p3, p4);

		v_start.Normalize();
		v_end.Normalize();

		Point3d v = p0 - p4;

		double a = 2 * (v_start*v_end - 1);
		double c = v*v;
		double b = (v * 2)*(v_start + v_end);

		if (fabs(a) < 0.000000000000001)return false;

		double d = b*b - 4 * a*c;

		if (d < 0.0)return false;

		double sd = sqrt(d);

		double e1 = (-b - sd) / (2.0 * a);
		double e2 = (-b + sd) / (2.0 * a);

		if (e1 > 0 && e2 > 0)return false;

		double e = e1;
		if (e2 > e)e = e2;

		if (e < 0)return false;

		p1 = p0 + v_start * e;
		p3 = p4 - v_end * e;
		p2 = p1 * 0.5 + p3 * 0.5;

		return true;
	}