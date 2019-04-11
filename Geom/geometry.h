
/////////////////////////////////////////////////////////////////////////////////////////

//                    geometry.lib header

//                    g.j.hawkesford August 2003
//						modified with 2d & 3d vector methods 2006
//
// This program is released under the BSD license. See the file COPYING for details.
//
/////////////////////////////////////////////////////////////////////////////////////////

// modified by Dan Heeks 2018

#pragma once
#ifdef WIN32
#pragma warning( disable : 4996 )
#ifndef WINVER
	#define WINVER 0x501
#endif
#endif

#include <math.h>
#include <algorithm>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <string.h>
#include "Point.h"

using namespace std;

class Point;
class Point3d;
class Point;
class Point3d;
class CLine;
class Circle;
class Line;


enum UNITS_TYPE{
	MM = 0,
	METRES,
	INCHES
};

extern int	  UNITS;					// may be enum UNITS_TYPE (MM METRES or INCHES)
extern double TOLERANCE;				// CAD Geometry resolution (inexact, eg. from import)
extern double TOLERANCE_SQ;				// tolerance squared for faster coding.
extern double TIGHT_TOLERANCE;
extern double UNIT_VECTOR_TOLERANCE;
extern double SMALL_ANGLE;				// small angle tangency test eg isConvex
extern double SIN_SMALL_ANGLE;
extern double COS_SMALL_ANGLE;
extern double RESOLUTION;				// CNC resolution

void set_Tolerances(int mode);
double mm(double value);				// convert to current units from mm

inline bool FEQ(double a, double b, double tolerance = TOLERANCE) {return fabs(a - b) <= tolerance;}
inline bool FNE(double a, double b, double tolerance = TOLERANCE) {return fabs(a - b) > tolerance;}

inline bool FEQZ(double a, double tolerance = TIGHT_TOLERANCE) {return fabs(a) <= tolerance;}
inline bool FNEZ(double a, double tolerance = TIGHT_TOLERANCE) {return fabs(a) > tolerance;}

#define PI 3.1415926535897932384626433832795e0
#define DegreesToRadians (PI / 180.0e0)
#define RadiansToDegrees (180.0e0 / PI)
#define NEARLY_ONE 0.99999999999e0
#define CPTANGENTTOL 1.0e-04			// normalised vector crossproduct tolerance sin A  so A = .0057deg

#define TANTO		-1
#define ANTITANTO	1

#define TANGENT		0

#define NEARINT		1
#define FARINT		-1

#define LEFTINT		1
#define RIGHTINT	-1

#define CFILLET			0	// corner fillet
#define CHAMFER			1	// chamfer

#define GEOFF_LEFT 1
#define NONE 0
#define GEOFF_RIGHT -1


#define LINEAR 0	// linear
#define ACW 1		// anti-clockwise
#define CW -1		// clockwise

const wchar_t* getMessage(const wchar_t* original, int messageGroup, int stringID);
const wchar_t* getMessage(const wchar_t* original);							// dummy
void FAILURE(const wchar_t* str);
void FAILURE(const std::wstring& str);

enum MESSAGE_GROUPS {
	GENERAL_MESSAGES,
	GEOMETRY_ERROR_MESSAGES,
	PARAMSPMP
};

enum GENERAL_MESSAGES {
	MES_TITLE = 0,
	MES_UNFINISHEDCODING,
	MES_ERRORFILENAME,
	MES_LOGFILE,
	MES_LOGFILE1,
	MES_P4CMENU,
	MES_P4CMENUHINT
};

enum GEOMETRY_ERROR_MESSAGES{	// For geometry.lib
	MES_DIFFSCALE = 1000,
	MES_POINTONCENTRE,
	MES_INVALIDARC,
	MES_INDEXOUTOFRANGE,
	MES_BAD_VERTEX_NUMBER,
	MES_BAD_REF_OFFSET,
	MES_BAD_SEC_OFFSET,
	MES_ROLLINGBALL4AXIS_ERROR,
	MES_INVALIDPLANE
};

// homogenous 4 x 4 Matrix class
class Matrix{
protected:
public:
	double e[16];
	bool m_unit;												// true if unit matrix
	int m_mirrored;												// 1 if mirrored, 0 if not and -1 if unknown

public:
	// constructors etc...
	Matrix();													// create a unit matrix
	Matrix(const double m[16]);										// from an array
	Matrix(const Point3d &origin, const Point3d &x_axis, const Point3d &y_axis);
	Matrix(const Matrix& m);									// copy constructor

	~Matrix(){};

	//operators
	bool operator==(const Matrix &m)const;
	bool operator!=(const Matrix &m)const { return !(*this == m);}
	const Matrix operator*(const Matrix &m)const;

	// methods
	void	Unit();												// unit matrix
	void	Get(double* p) const;								// get the matrix into p
	void	GetTransposed(double* p) const;						// get the matrix into p
	void	Put(double*p);										// put p[16] into matrix
	void	Translate(double x, double y, double z = 0);		// Translation
	void	Translate(const Point3d& v);						// Translation

	void	Rotate(double sinang, double cosang, Point3d *rotAxis); // Rotation about rotAxis
	void	Rotate(double angle, Point3d *rotAxis);			// Rotation about rotAxis

	void	Rotate(double sinang, double cosang, int Axis);			// Rotation with cp & dp
	void	Rotate(double angle, int Axis);						// Rotation with angle

	void	Scale(double scale);								// Scale
	void	Scale(double scalex, double scaley, double scalez);

	void	Multiply(const Matrix& m);								// Multiply 2 Matrices
	//	void	Transform(Point& p);
	void	Transform(double p0[3]) const;							// Transform p0 thro' this matrix
	void	Transform(double p0[3], double p1[3]) const;				// Transform p0 to p1 thro' this matrix
	void	Transform2d(double p0[2], double p1[2]) const;			// Transform p0 to p1 thro' this matrix

	int		IsMirrored();										// true if matrix has a mirror transformation
	int		IsUnit();											// true if matrix is unit matrix
	void	GetTranslate(double& x, double& y, double& z) const;		// get translation from matrix
	void	GetScale(double& sx, double& sy, double& sz) const;		// get scale from matrix
	bool	GetScale(double& sx) const;								// get scale from matrix (true if uniform scale)
	void	GetRotation(double& ax, double& ay, double& az) const;	// get rotation from matrix

	Matrix	Inverse()const;											// inverts this matrix
};

extern Matrix UnitMatrix;		// a Unit Matrix


#define INVALID_POINT	Point(9.9999999e50, 0)
#define INVALID_POINT3D	Point3d(9.9999999e50, 0, 0)
#define INVALID_CLINE	CLine(INVALID_POINT, 1, 0)
#define INVALID_CIRCLE	Circle(INVALID_POINT, 0)


class Point{
public:
	// can be a position, or a vector
	double x, y;

	Point() :x(0.0), y(0.0){}
	Point(double X, double Y) :x(X), y(Y){}
	Point(const double* p) :x(p[0]), y(p[1]){}
	Point(const Point& p0, const Point& p1) :x(p1.x - p0.x), y(p1.y - p0.y){} // vector from p0 to p1

	const Point operator+(const Point& p)const{ return Point(x + p.x, y + p.y); }
	const Point operator-(const Point& p)const{ return Point(x - p.x, y - p.y); }
	const Point operator*(double d)const{ return Point(x * d, y * d); }
	const Point operator/(double d)const{ return Point(x / d, y / d); }
	bool operator==(const Point& p)const{ return fabs(x - p.x)<TOLERANCE && fabs(y - p.y)<TOLERANCE; }
	bool operator!=(const Point &p)const{ return !(*this == p); }

	double dist(const Point &p)const{ double dx = p.x - x; double dy = p.y - y; return sqrt(dx*dx + dy*dy); }
	double Dist(const Point &p)const{ double dx = p.x - x; double dy = p.y - y; return sqrt(dx*dx + dy*dy); }
	double length()const;
	double magnitude()const;
	double magnitudesqd()const;
	double normalize();
	double normalise(){ return normalize(); }
	double operator*(const Point &p)const{ return (x * p.x + y * p.y); }// dot product
	double operator^(const Point &p)const{ return (x * p.y - y * p.x); }// cross product m0.m1.sin a = v0 ^ v1
	Point operator~(void)const{ return Point(-y, x); }// perp to left
	Point operator-(void)const{ return Point(-x, -y); }// v1 = -v0;  (unary minus)
	void Rotate(double cosa, double sina){// rotate vector by angle
		double temp = -y * sina + x * cosa;
		y = x * sina + cosa * y;
		x = temp;
	}
	Point	Mid(const Point& p, double factor = .5)const;
	void Rotate(double angle){ if (fabs(angle) < 1.0e-09)return; Rotate(cos(angle), sin(angle)); }
	void Transform(const Matrix &m);
	Point Transformed(const Matrix& m);
};
const Point operator*(const double &d, const Point &p);
ostream & operator<<(ostream &os, const Point &);

// 3d point class
class Point3d {
	friend wostream& operator <<(wostream& op, Point3d& p);
public:
//		bool	ok;																// true if this point is defined correctly	
	double	x;																// x value
	double	y;																// y value
	double	z;																// z value

	// constructors
	inline	Point3d(){};// {z=0; /*ok=false;*/};												// Point p1
	inline	Point3d(const double* xyz) {x = xyz[0], y = xyz[1]; z = xyz[2];}
	inline	Point3d( double xord, double yord, double zord = 0/*, bool okay = true*/) {	// Point p1(10,30.5);
		x = xord; y = yord; z = zord;/* ok = okay;*/}
	inline	Point3d(const Point3d& p) {											// copy constructor  Point p1(p2);
		x = p.x; y = p.y;  z = p.z;/* ok = p.ok;*/
	}
	inline	Point3d(const Point& p){x = p.x; y = p.y;  z = 0.0;}
	inline	Point3d(const Point3d& p0, const Point3d& p1) {						// vector from p0 to p1
		x = p1.x - p0.x; y = p1.y - p0.y;  z = p1.z - p0.z;/* ok = p.ok;*/
	}

	// destructor
//		~Point3d();

	// operators
	bool operator==(const Point3d &p)const;
	bool operator!=(const Point3d &p)const { return !(*this == p);}
	Point3d operator+(const Point3d &v)const{ return Point3d(x +v.x, y +v.y, z +v.z); }

	const Point3d operator*(double d)const{ return Point3d(x * d, y * d, z * d); }
	const Point3d operator/(double d)const{ return Point3d(x / d, y / d, z / d); }
	const	Point3d& operator+=(const Point3d &v){ x += v.x; y += v.y; z += v.z; return *this; }			// v1 += v0;
	Point3d operator-(const Point3d &v)const{ return Point3d(x - v.x, y - v.y, z - v.z); }				// v2 = v0 - v1;
	const	Point3d& operator-=(const Point3d &v){
		x -= v.x; y = -v.y; z = -v.z; return *this;
	}			// v1 -= v0;
	const	Point3d operator^(const Point3d &v)const{
		return Point3d(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}					// cross product vector

	const	Point3d operator-(void)const{ return Point3d(-x, -y, -z); }										// v1 = -v0;  (unary minus)

	const	double operator*(const Point3d &v)const{ return (x * v.x + y * v.y + z * v.z); }				// dot product	m0 m1 cos a = v0 * v1

	const Point3d& operator*=(double c){ x *= c; y *= c; z *= c; return *this; }								// scalar products

	const Point3d Normalized()const{ double m = magnitude(); if (m < 1.0e-09) { return Point3d(0, 0, 0); } return Point3d(x / m, y / m, z / m); }
	double Normalize(){ double m = magnitude(); if (m < 1.0e-09) { x = y = z = 0; return 0; } x /= m; y /= m; z /= m;	return m; }
	double normalise(){ double m = magnitude(); if (m < 1.0e-09) { x = y = z = 0; return 0; } x /= m; y /= m; z /= m;	return m; }
	inline	double magnitude(void)const{ return(sqrt(x * x + y * y + z * z)); }								// magnitude
	inline	double magnitudeSq(void)const{ return x * x + y * y + z * z; }

	// methods
#ifdef PEPSDLL
	void	ToPeps(int id, bool draw = true);									// copy Point to Peps
#endif
	Point3d Transformed(const Matrix& m);
	double Dist(const Point3d& p)const;													// distance between 2 points
	double DistSq(const Point3d& p)const;													// distance squared between 2 points
	Point3d	Mid(const Point3d& p, double factor = 0.5)const;									// midpoint
	void get(double xyz[3]) {xyz[0] = x; xyz[1] = y; xyz[2] = z;}
	double* getBuffer(){return &this->x;};																		// returns ptr to data
	const double* getBuffer()const{return &this->x;};																		// returns ptr to data
	void arbitrary_axes(Point3d& x, Point3d& y)const;

};

ostream & operator<<(ostream &os, const Point3d &);

#define ORIGIN Point3d(0,0,0)
#define NULL_VECTOR Point3d(0,0,0)
#define Z_VECTOR Point3d(0,0,1)
#define Y_VECTOR Point3d(0,1,0)
#define X_VECTOR Point3d(1,0,0)

// 2D cline x = x0 + t * dx;    y = y0 + t * dy
class CLine{
	friend wostream& operator <<(wostream& op, CLine& cl);
public:
	bool ok;
	Point p;
	Point v;

	// constructors
	inline	CLine()	{ok = false;};
	inline	CLine(const Point& p0, double dx, double dy, bool normalise = true){ p = p0; v = Point(dx, dy); if(normalise) Normalise();};
	inline	CLine(const Point& p0, const Point& v0, bool normalise = true) {p = p0; v = v0; if(normalise) Normalise();};
	inline	CLine( const CLine& s ) {p = s.p; v = s.v;};				// copy constructor  CLine s1(s2);

	// operators
	const	CLine operator~(void);// perp to left
	const	CLine operator=(const Point& p0){p.x=p0.x; p.y=p0.y; return *this;};				// s = p;

	// methods
	double c();																// returns c
	void Normalise();														// normalise dx,dy
#ifdef PEPSDLL
	void ToPeps(int id, bool draw = true);									// to Peps
	void DelPeps(int id);													// delete Peps CLine
#endif
	CLine Transform(Matrix& m);												// transform a CLine
	Point Intof(const CLine& s);													// intersection of 2 clines
	Point Intof(int NF, const Circle& c);											// intersection of cline & circle 
	Point Intof(int NF, const Circle& c, Point& otherInters);	double Dist(const Point& p1)const;	//  ditto & other intersection												
	CLine Bisector(const CLine& s);												// Bisector of 2 Clines

	// destructor
//		~CLine();
};

#define HORIZ_CLINE CLine(Point(0,0), 1.0, 0.0, true)


// 2D circle 
class Circle{
	friend wostream& operator <<(wostream& op, Circle& c);
public:
	bool ok;
	Point pc;
	double	radius;

	// constructors etc...
	inline	Circle() {ok = false;};
	Circle( const Point& p, double r, bool okay = true);							// Circle  c1(Point(10,30), 20);
	Circle( const Point& p, const Point& pc);											// Circle  c1(p[222], p[223]);
	Circle( const Circle& c ){*this = c;}									// copy constructor  Circle c1(c2);
	Circle(const Point& p0, const Point& p1, const Point& p2); // circle through three points

	// methods
#ifdef PEPSDLL
	void ToPeps(int id, bool draw = true);									// to Peps
	void DelPeps(int id);													// delete Peps Circle
#endif
	bool operator==(const Circle &c)const;									// c == cc
	bool operator!=(const Circle &c)const { return !(*this == c);}
	Circle Transform(Matrix& m);											// transform a Circle
	Point	Intof(int LR, const Circle& c1);										// intof 2 circles
	Point	Intof(int LR, const Circle& c1, Point& otherInters);					// intof 2 circles, (returns the other intersection)
	int		Intof(const Circle& c1, Point& leftInters, Point& rightInters);		// intof 2 circles (returns number of intersections & left/right inters)
	CLine	Tanto(int AT,  double angle, const CLine& s0)const;			// a cline tanto this circle at angle
//	~Circle();																// destructor
	bool PointIsOn(const Point& p, double accuracy);
	bool LineIsOn(const Point& p0, const Point& p1, double accuracy);
};

// 2d box class
class Box{
public:
	Point min;
	Point max;
	bool ok;

	Box() { min.x = min.y = 1.0e61; max.x = max.y = -1.0e61; ok = false;};
	Box(Point& pmin, Point& pmax) { min = pmin; max = pmax; ok = true;};

	bool outside(const Box& b)const;		// returns true if box is outside box
	void combine(const Box& b);		// combines this with b
};

// 3d box class
class Box3d{
public:
	Point3d min;
	Point3d max;
	bool ok;

	Box3d() { min.x = min.y = min.z = 1.0e61; max.x = max.y = max.z = -1.0e61; ok = false;};
	Box3d(const Point3d& pmin, const Point3d& pmax) { min = pmin; max = pmax; ok = true;};

	bool outside(const Box3d& b)const;		// returns true if box is outside box
	void combine(const Box3d& b);		// combines this with b
};

inline void MinMax(const Point& p, Point& pmin, Point& pmax) {
	if(p.x > pmax.x) pmax.x = p.x;
	if(p.y > pmax.y) pmax.y = p.y;
	if(p.x < pmin.x) pmin.x = p.x;
	if(p.y < pmin.y) pmin.y = p.y;
};

inline void MinMax(const Point3d& p, Point3d& pmin, Point3d& pmax) {
	if(p.x > pmax.x) pmax.x = p.x;
	if(p.y > pmax.y) pmax.y = p.y;
	if(p.z > pmax.z) pmax.z = p.z;
	if(p.x < pmin.x) pmin.x = p.x;
	if(p.y < pmin.y) pmin.y = p.y;
	if(p.z < pmin.z) pmin.z = p.z;
};



// general
double	atn360(double dx, double dy);									// angle 0 to 2pi

// distance functions
//double Dist(double px, double py, double p1x, double p1y);				// diatance between 2 points (2d)
//double Dist(Point& p0, Point& p1);										// distance between 2 points (3d)
//double Dist(CLine& s, Point& p1);											// distance between cline & point

double Dist(const Point3d *p, const Point3d *vl, const Point3d *pf);							// distance from line (p, vl) and pf
double DistSq(const Point3d *p, const Point3d *vl, const Point3d *pf);						// distance squared from line (p, vl) and pf
double Dist(const Circle& c, const Point& p);											// distance between c & p
double Dist(const Point& p0, const Circle& c, const Point& p1);								// clockwise distance around c from p0 to p1
double Dist(const CLine& s, const Circle& c);											// distance between line and circle
double Dist(const Circle& c0, const Circle& c1);										// distance between 2 circles
double IncludedAngle(const Point& v0, const Point& v1, int dir = 1);				// angle between 2 vectors
double IncludedAngle(const Point3d& v0, const Point3d& v1, const Point3d& normal, int dir = 1);
inline	double IncludedAngle(const CLine& s0, const CLine& s1, int dir = 1) {			// angle between 2 Clines
	return IncludedAngle(s0.v, s1.v, dir);
}


// point definitions
Point	Mid(const Point& p0, const Point& p1, double factor = 0.5);					//// midpoint
Point	Rel(const Point& p, double x, double y);								// relative point
Point	Polar(const Point& p, double angle, double r);						// polar from this point
Point	AtAngle(const Circle& c, double angle);								// Point at angle on a circle
Point	XonCLine(const CLine& s, double xval);								// returns point that has X on this line
Point	YonCLine(const CLine& s, double yval);								// returns point that has Y on this line
Point	Intof(const CLine& s0, const CLine& s1);									//// intof 2 clines
Point	Intof(int NF, const CLine& s, const Circle& c);								//// intof of circle & a cline
Point	Intof(int NF, const CLine& s, const Circle& c, Point& otherInters);			//// intof of circle & a cline (returns the other intersection)
Point	Intof(int LR, const Circle& c0, const Circle& c1);							//// intof 2 circles
Point	Intof(int LR, const Circle& c0, const Circle& c1, Point& otherInters);		//// intof 2 circles, (returns the other intersection)
int		Intof(const Circle& c0, const Circle& c1, Point& pLeft, Point& pRight);		////    ditto
Point	Along(const CLine& s, double d);										// distance along Cline
Point	Along(const CLine& s, double d, const Point& p);								// distance along Cline from point
Point	Around(const Circle& c, double d, const Point& p);								// distance around a circle from point
Point	On(const CLine& s,  const Point& p);											// returns a point on s nearest to p
Point	On(const Circle& c, const Point& p);											// returns a point on c nearest to p

// cline definitons

CLine	AtAngle(double angle, const Point& p, const CLine& s = HORIZ_CLINE);		// cline at angle to line thro' point
CLine	Tanto(int AT, const Circle& c,  double angle, const CLine& s0 = HORIZ_CLINE);//// cline tanto circle at angle to optional cline
CLine	Tanto(int AT, const Circle& c, const Point& p);								// cline tanto circle thro' a point
CLine	Tanto(int AT0, const Circle& c0, int AT1, const Circle& c1);					// cline tanto 2 circles
CLine	Normal(const CLine& s);													// noirmal to cline
CLine	Normal(const CLine& s, const Point& p);										// normal to cline thro' p
CLine	Parallel(int LR, const CLine& s, double distance);						// parallel to cline by distance
CLine	Parallel(const CLine& cl, const Point& p);										// parallel to cline thro' a point


// circle definitions
Circle	Thro(const Point& p0, const Point& p1);										// circle thro 2 points (diametric)
Circle	Thro(const Point& p0, const Point& p1, const Point& p2);							// circle thro 3 points
Circle	Tanto(int NF, const CLine& s0, const Point& p, double rad);					// circle tanto a CLine thro' a point with radius
Circle	Thro(int LR, const Point& p0, const Point& p1, double rad);					// circle thro' 2 points with radius
Circle	Tanto(int AT1, const CLine& s1, int AT2, const CLine& s2, double rad);		// circle tanto 2 clines with radius
Circle	Tanto(int AT1, const CLine& s1, int AT2, const CLine& s2, int AT3, const CLine& s3);	// circle tanto 3 clines
Circle	Tanto(int LR, int AT, const Circle& c, const Point& p, double rad);			// circle tanto circle & thro' a point
Circle	Tanto(int NF, int AT0, const CLine& s0, int AT1, const Circle& c1, double rad);// circle tanto cline & circle with radius
Circle	Tanto(int LR, int AT0, const Circle& c0, int AT1, const Circle& c1, double rad);// circle tanto 2 circles with radius
Circle	Tanto(int LR, int AT1 , const Circle& c1 , int AT2 , const Circle& c2, int AT3 , const Circle c3); // tanto 3 circles
int		apolloniusProblem(int AT1 , const Circle& c1 , int AT2 , const Circle& c2, int AT3 , const Circle& c3, Circle& Solution1, Circle& Solution2);
int		apolloniusProblem(int AT1 , const Circle& c1 , int AT2 , const Circle& c2, int AT3 , const CLine& cl3, Circle& Solution1, Circle& Solution2);
int		apolloniusProblem(int AT1 , const Circle& c1 , int AT2 , const CLine& cl2, int AT3 , const CLine& cl3, Circle& Solution1, Circle& Solution2);

//		Circle	Tanto(int AT0, int NF, int AT1, CLine s1, int AT2, CLine s2);	// circle tanto circle, and 2 clines
Circle	Parallel(int LR, const Circle& c, double distance);					// parallel to circle by a distance


// misc
inline double Radians(double degrees) {return degrees * PI / 180;}
inline double Degrees(double radians) { return radians * 180 / PI;}
int quadratic(double a, double b, double c, double& x0, double& x1);	// solve quadratic

int corner(const Point& v0, const Point& v1, double cpTol = CPTANGENTTOL);	// corner (TANGENT, LEFT, RIGHT)

Line IsPtsLine(const double* a, int n, double tolerance, double* deviation);

class Plane {
	friend wostream& operator <<(wostream& op, Plane& pl);

public:						// ax + by + cz + d = 0
	bool ok;
	double d;				// distance of plane to origin
	Point3d normal;				// normal to plane a = n.dx, b = n.dy, c = n.dz
	// constructors
	Plane(){ok = false;};
	Plane(double dist, const Point3d& n);
	Plane(const Point3d& p0, const Point3d& p1, const Point3d& p2);
	Plane(const Point3d& p0, const Point3d& n, bool normalise = true);

	// methods
	double Dist(const Point3d& p)const;							// signed distance of point to plane
	bool Intof(const Line& l, Point3d& intof, double& t)const;		// intersection of plane & line (0 >= t <= 1 if intersect within line) 
	bool Intof(const Plane& pl, Line& intof)const;					// intersection of 2 planes
	bool Intof(const Plane& pl0, const Plane& pl1, Point3d& intof)const;	// intersection of 3 planes
	Point3d Near(const Point3d& p)const;							// returns near point to p on the plane
	bool On(const Point3d &p)const;
	void Mirrored(Matrix* m);										// returns a matrix for a mirror about this
	void Transform(const Matrix& m);
};




#ifdef WIN32
#pragma warning(default:4522)
#endif

void tangential_arc(const Point &p0, const Point &p1, const Point &v0, Point &c, int &dir);


inline double CPTOL(double offset, double maxOffset) {
	// this returns a suitable tolerance for a cross product
	// the cp for normalised vectors is the sin of the included angle between the vectors
	//
	// this function takes the machine resolution from RESOLUTION

	offset = fabs(offset);

	if(offset <= RESOLUTION) offset = maxOffset;	// no known offset so guess one from the application

	return RESOLUTION / offset;
}



int Intof(const Line& v0, const Line& v1, Point3d& intof);							// intof 2 lines
double Dist(const Line& l, const Point3d& p, Point3d& pnear, double& t);			// distance from a point to a line
Point3d Near(const Line& l, const Point3d& p, double& t );							// near point to a line & t in 0-length range

// 3d line segment
class Line{
public:
	Point3d p0;				// start
	Point3d v;				// vector (not normalised)
	double length;			// line length
	Box3d box;
	bool ok;

	// constructors
	Line() {ok = false;};
	//Line(const Point3d& p0, const Point3d& v0, bool boxed = true);
	Line(const Point3d& p0, const Point3d& p1);

	// methods
	void minmax();
	Point3d Near(const Point3d& p, double& t)const;				// near point to line from point (0 >= t <= 1) in range
	int Intof(const Line& l, Point3d& intof)const { return ::Intof(*this, l, intof); };	// intof 2 lines
	bool Intof(const Point3d& p)const;
	bool IntofFinite(const Point3d& p)const;
	int IntofFinite(const Line& l, Point3d& intof, Point3d& other_intof)const;
	bool atZ(double z, Point3d& p)const;						// returns p at z on line
	bool Shortest(const Line& l2, Line& lshort, double& t1, double& t2)const;	// calculate shortest line between this & l2
};


class Triangle3d {
	Point3d vert1;    // first vertex
	Point3d vert2;    // second vertex
	Point3d vert3;    // third vertex
	Point3d v0;      // vector from vert1 to vert2
	Point3d v1;      // vector from vert1 to vert3

	Box3d box;        // box around triangle

public:
	// constructor
	Triangle3d(){};
	Triangle3d(const Point3d& vert1, const Point3d& vert2, const Point3d& vert3);

	// methods
	bool Intof(const Line& l, Point3d& intof)const; // returns intersection triangle to line
	bool Intof(const Plane& pl, Point3d& ps, Point3d& pe)const; // intersection plane to triangle
	bool Intof(const Triangle3d& t, std::list<Point3d>& intof_list)const; // returns intersection triangle to triangle
	bool inside(const Point3d& intof)const;
	bool Inside(const Point3d& intof)const;
	Box3d& getBox(void) { return box; };
	const Point3d& getVert1(void) { return vert1; }
	const Point3d& getVert2(void) { return vert2; }
	const Point3d& getVert3(void) { return vert3; }

	const Point3d& getV0(void) { return v0; }
	const Point3d& getV1(void) { return v1; }
	void getNormal(Point3d* normal)const;
	double getArea()const;
};


bool calculate_biarc_points(const Point3d &p0, Point3d v_start, const Point3d &p4, Point3d v_end, Point3d &p1, Point3d &p2, Point3d &p3);
