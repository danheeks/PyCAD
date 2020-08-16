// PythonStuff.cpp
// Copyright 2011, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "PythonStuff.h"

#include "Area.h"
#include "Point.h"
#include "AreaDxf.h"
#include "geometry.h"
#include "Solid.h"
#include "Tris.h"
#include "HeeksGeomDxf.h"
#include "Box.h"

#define HAVE_ACOSH
#define HAVE_ASINH
#define HAVE_ATANH
#define HAVE_LOG1P

#if _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

#ifdef __GNUG__
#pragma implementation
#endif

#include <boost/progress.hpp>
#include <boost/timer.hpp>
#include <boost/foreach.hpp>
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/class.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/python/call.hpp>

#include "clipper.hpp"
using namespace ClipperLib;


namespace bp = boost::python;

boost::python::list getVertices(const CCurve& curve) {
	boost::python::list vlist;
	BOOST_FOREACH(const CVertex& vertex, curve.m_vertices) {
		vlist.append(vertex);
    }
	return vlist;
}

boost::python::list getCurves(const CArea& area) {
	boost::python::list clist;
	BOOST_FOREACH(const CCurve& curve, area.m_curves) {
		clist.append(curve);
    }
	return clist;
}

boost::python::tuple transformed_point(const Matrix &matrix, double x, double y, double z)
{
	Point3d p(x,y,z);
	p = p.Transformed(matrix);

	return bp::make_tuple(p.x,p.y,p.z);
}

void MatrixRotate(Matrix &matrix, double angle)
{
	matrix.Rotate(angle, 3);
}

void MatrixRotateAxis(Matrix &matrix, double angle, const Point3d& axis)
{
	matrix.Rotate(angle, axis);
}

void Point3dTransform(Point3d &p, const Matrix &matrix)
{
	p = p.Transformed(matrix);
}

boost::python::tuple ArbitraryAxes(const Point3d& p)
{
	Point3d x, y;
	p.arbitrary_axes(x, y);
	return bp::make_tuple(x, y);
}

boost::python::object PlaneIntofPlane(const Plane &plane1, const Plane &plane2)
{
	Line line;
	bool result = plane1.Intof(plane2, line);

	if (result)
	{
		return boost::python::object(line);
	}
	else
		return boost::python::object();
}

void LineTransform(Line &line, const Matrix &matrix)
{
	line.p0 = line.p0.Transformed(matrix);
	line.v = line.v.Transformed(matrix);
}

static void print_curve(const CCurve& c)
{
	unsigned int nvertices = (unsigned int)(c.m_vertices.size());
	wprintf(L"number of vertices = %d\n", nvertices);
	int i = 0;
	for(std::list<CVertex>::const_iterator It = c.m_vertices.begin(); It != c.m_vertices.end(); It++, i++)
	{
		const CVertex& vertex = *It;
		wprintf(L"vertex %d type = %d, x = %g, y = %g", i+1, vertex.m_type, vertex.m_p.x / CArea::m_units, vertex.m_p.y / CArea::m_units);
		if(vertex.m_type)wprintf(L", xc = %g, yc = %g", vertex.m_c.x / CArea::m_units, vertex.m_c.y / CArea::m_units);
		wprintf(L"\n");
	}
}

static void print_area(const CArea &a)
{
	for(std::list<CCurve>::const_iterator It = a.m_curves.begin(); It != a.m_curves.end(); It++)
	{
		const CCurve& curve = *It;
		print_curve(curve);
	}
}

static unsigned int num_vertices(const CCurve& curve)
{
	return (unsigned int)(curve.m_vertices.size());
}

static CVertex FirstVertex(const CCurve& curve)
{
	return curve.m_vertices.front();
}

static CVertex LastVertex(const CCurve& curve)
{
	return curve.m_vertices.back();
}

static void set_units(double units)
{
	CArea::m_units = units;
}

static double get_units()
{
	return CArea::m_units;
}

static CArea AreaFromDxf(const char* filepath)
{
	CArea area;
	AreaDxfRead dxf(&area, filepath);
	dxf.DoRead();
	return area;
}

static void append_point(CCurve& c, const Point& p)
{
	c.m_vertices.push_back(CVertex(p));
}

static boost::python::tuple nearest_point_to_curve(CCurve& c1, const CCurve& c2)
{
	double dist = 0.0;
	Point p = c1.NearestPoint(c2, &dist);

	return bp::make_tuple(p, dist);
}


static boost::python::tuple SpanNearestPoint(Span& s1, Span& s2)
{
	double dist = 0.0;
	Point p = s1.NearestPoint(s2, &dist);

	return bp::make_tuple(p, dist);
}

static bool SpanOn(Span& s, Point& p)
{
	return s.On(p, NULL);
}

boost::python::list MakePocketToolpath(const CArea& a, double tool_radius, double extra_offset, double stepover, bool from_center, bool use_zig_zag, double zig_angle)
{
	std::list<CCurve> toolpath;

	CAreaPocketParams params(tool_radius, extra_offset, stepover, from_center, use_zig_zag ? ZigZagPocketMode : SpiralPocketMode, zig_angle);
	a.SplitAndMakePocketToolpath(toolpath, params);

	boost::python::list clist;
	BOOST_FOREACH(const CCurve& c, toolpath) {
		clist.append(c);
    }
	return clist;
}

boost::python::list SplitArea(const CArea& a)
{
	std::list<CArea> areas;
	a.Split(areas);

	boost::python::list alist;
	BOOST_FOREACH(const CArea& a, areas) {
		alist.append(a);
    }
	return alist;
}

void dxfArea(CArea& area, const char* str)
{
	area = CArea();
}

boost::python::list getCurveSpans(const CCurve& c)
{
	boost::python::list span_list;
	const Point *prev_p = NULL;

	for(std::list<CVertex>::const_iterator VIt = c.m_vertices.begin(); VIt != c.m_vertices.end(); VIt++)
	{
		const CVertex& vertex = *VIt;

		if(prev_p)
		{
			span_list.append(Span(*prev_p, vertex));
		}
		prev_p = &(vertex.m_p);
	}

	return span_list;
}

Span getFirstCurveSpan(const CCurve& c)
{
	if(c.m_vertices.size() < 2)return Span();

	std::list<CVertex>::const_iterator VIt = c.m_vertices.begin();
	const Point &p = (*VIt).m_p;
	VIt++;
	return Span(p, *VIt, true);
}

Span getLastCurveSpan(const CCurve& c)
{
	if(c.m_vertices.size() < 2)return Span();

	std::list<CVertex>::const_reverse_iterator VIt = c.m_vertices.rbegin();
	const CVertex &v = (*VIt);
	VIt++;

	return Span((*VIt).m_p, v, c.m_vertices.size() == 2);
}

bp::tuple TangentialArc(const Point &p0, const Point &p1, const Point &v0)
{
  Point c;
  int dir;
  tangential_arc(p0, p1, v0, c, dir);

  return bp::make_tuple(c, dir);
}

boost::python::list spanIntersect(const Span& span1, const Span& span2) {
	boost::python::list plist;
	std::list<Point> pts;
	span1.Intersect(span2, pts);
	BOOST_FOREACH(const Point& p, pts) {
		plist.append(p);
    }
	return plist;
}

//Matrix(boost::python::list &l){}


boost::shared_ptr<Matrix> matrix3point_constructor(const Point3d& o, const Point3d& x, const Point3d& y) {
	return boost::shared_ptr<Matrix>(new Matrix(o, Point3d(x.x, x.y, x.z), Point3d(y.x, y.y, y.z)));
}

boost::shared_ptr<Matrix> matrix_constructor(const boost::python::list& lst) {
	double m[16] = {1,0,0,0,0,1,0,0, 0,0,1,0, 0,0,0,1};

  boost::python::ssize_t n = boost::python::len(lst);
  int j = 0;
  for(boost::python::ssize_t i=0;i<n;i++) {
    boost::python::object elem = lst[i];
	m[j] = boost::python::extract<double>(elem.attr("__float__")());
	j++;
	if(j>=16)break;
  }

  return boost::shared_ptr<Matrix>( new Matrix(m) );
}

boost::shared_ptr<Plane> plane_constructor(const Point3d& p, const Point3d& v) {
	return boost::shared_ptr<Plane>(new Plane(p, Point3d(v.x, v.y, v.z)));
}


boost::python::list InsideCurves(const CArea& a, const CCurve& curve) {
	boost::python::list plist;

	std::list<CCurve> curves_inside;
	a.InsideCurves(curve, curves_inside);
	BOOST_FOREACH(const CCurve& c, curves_inside) {
		plist.append(c);
    }
	return plist;
}

boost::python::list CurveIntersections(const CCurve& c1, const CCurve& c2) {
	boost::python::list plist;

	std::list<Point> pts;
	c1.CurveIntersections(c2, pts);
	BOOST_FOREACH(const Point& p, pts) {
		plist.append(p);
    }
	return plist;
}

boost::python::object CurveGetMaxCutterRadius(const CCurve& c, bool outside)
{
	double radius;
	if (c.GetMaxCutterRadius(radius, outside))
	{
		return boost::python::object(radius);
	}
	return boost::python::object(); // None
}

CBox2D CurveGetBox(const CCurve &c)
{
	CBox2D box;
	c.GetBox(box);
	return box;
}

Point3d CBoxCenter(const CBox& box)
{
	double x[3];
	box.Centre(x);
	return Point3d(x);
}

boost::python::list AreaIntersections(const CArea& a, const CCurve& c2) {
	boost::python::list plist;

	std::list<Point> pts;
	a.CurveIntersections(c2, pts);
	BOOST_FOREACH(const Point& p, pts) {
		plist.append(p);
    }
	return plist;
}

boost::python::list CTrisGetMachiningAreas(const CTris& tris)
{
	boost::python::list plist;
	std::list<CMachiningArea> areas;
	tris.GetMachiningAreas(areas);
	BOOST_FOREACH(CMachiningArea& a, areas) {
		plist.append(a);
	}
	return plist;
}

Point3d LineIntersectPlane(const Line& line, const Plane& plane)
{
	Point3d intof;
	double t;
	plane.Intof(line, intof, t);
	return intof;
}

void CTrisAddTriangle(CTris& tris, const Point3d& p0, const Point3d& p1, const Point3d& p2)
{
	float x[9] = { (float)p0.x, (float)p0.y, (float)p0.z, (float)p1.x, (float)p1.y, (float)p1.z, (float)p2.x, (float)p2.y, (float)p2.z };
	tris.AddTri(x);
}

boost::python::list GetTrianglesAsCurveList(const CTris& tris)
{
	boost::python::list clist;
	for (std::list<CTri>::const_iterator It = tris.m_tris.begin(); It != tris.m_tris.end(); It++)
	{
		const CTri& tri = *It;
		CCurve c;
		c.append(Point(tri.x[0][0], tri.x[0][1]));
		c.append(Point(tri.x[1][0], tri.x[1][1]));
		c.append(Point(tri.x[2][0], tri.x[2][1]));
		c.append(Point(tri.x[0][0], tri.x[0][1]));
		clist.append(c);
	}
	return clist;
}

double AreaGetArea(const CArea& a)
{
	return a.GetArea();
}

CBox2D AreaGetBox(const CArea& a)
{
	CBox2D box;
	a.GetBox(box);
	return box;
}

extern int oct_ele_count;
int get_oct_ele_count()
{
	return oct_ele_count;
}

static void set_tolerance(double tolerance)
{
	TOLERANCE = tolerance;
}

static double get_tolerance()
{
	return TOLERANCE;
}

static void set_accuracy(double accuracy)
{
	CArea::m_accuracy = accuracy;
}

static double get_accuracy()
{
	return CArea::m_accuracy;
}

static void set_fitarcs(bool fitarcs)
{
	CArea::m_fit_arcs = fitarcs;
}

static bool get_fitarcs()
{
	return CArea::m_fit_arcs;
}

#ifdef OPEN_CASCADE_INCLUDED

CBox SolidGetBox(const CSolid& solid)
{
	CBox box;
	solid.GetBox(box);
	return box;
}

#endif

CBox CTrisGetBox(const CTris& solid)
{
	CBox box;
	solid.GetBox(box);
	return box;
}

size_t CTrisNumTris(const CTris& solid)
{
	return solid.m_tris.size();
}

void CTrisProject(const CTris& solid, const CArea& area, const std::string& dxf_file_path)
{
	std::list<Line> lines;
	solid.Project(area, lines);

	// write dxf file
	CDxfWrite dxf_writer(dxf_file_path.c_str());
	// add the spans transformed back to the input span's plane
	for (std::list<Line>::iterator It = lines.begin(); It != lines.end(); It++)
	{
		Line& line = *It;
		Point3d e = line.p0 + line.v;
		dxf_writer.WriteLine(line.p0.getBuffer(), e.getBuffer(), "0");
	}
}


#ifdef OPEN_CASCADE_INCLUDED

static Standard_Boolean TriangleIsValid(const Point3d& P1, const Point3d& P2, const Point3d& P3)
{
	Point3d V1(P1, P2);								// V1=(P1,P2)
	Point3d V2(P2, P3);								// V2=(P2,P3)
	Point3d V3(P3, P1);								// V3=(P3,P1)

	if ((V1.SquareMagnitude() > 1.e-10) && (V2.SquareMagnitude() > 1.e-10) && (V3.SquareMagnitude() > 1.e-10))
	{
		V1.Cross(V2);								// V1 = Normal	
		if (V1.SquareMagnitude() > 1.e-10)
			return Standard_True;
		else
			return Standard_False;
	}
	else
		return Standard_False;

}

CTris* CSolidMakeStl(const CSolid& solid, double deviation)
{
	CTris* new_tris = new CTris;

	BRepTools::Clean(solid.GetShape());
	BRepMesh_IncrementalMesh(solid.GetShape(), deviation);

	for (TopExp_Explorer explorer(solid.GetShape(), TopAbs_FACE); explorer.More(); explorer.Next())
	{
		TopoDS_Face face = TopoDS::Face(explorer.Current());

		CTri tri;

		// Get triangulation
		TopLoc_Location L;
		Handle_Poly_Triangulation facing = BRep_Tool::Triangulation(face, L);
		Matrix tr = L;

		if (facing.IsNull()){
		}
		else
		{
			Poly_Connect pc(facing);
			{
				const TColgp_Array1OfPnt& Nodes = facing->Nodes();
				const Poly_Array1OfTriangle& triangles = facing->Triangles();
				Standard_Integer nnn = facing->NbTriangles();					// nnn : nombre de triangles
				Standard_Integer nt, n1, n2, n3 = 0;						// nt  : triangle courant
				// ni  : sommet i du triangle courant
				for (nt = 1; nt <= nnn; nt++)
				{
					if (face.Orientation() == TopAbs_REVERSED)			// si la face est "reversed"
						triangles(nt).Get(n1, n3, n2);						// le triangle est n1,n3,n2
					else
						triangles(nt).Get(n1, n2, n3);						// le triangle est n1,n2,n3

					if (TriangleIsValid(Nodes(n1), Nodes(n2), Nodes(n3)))
					{
						Point3d v1 = Nodes(n1).Transformed(tr);
						Point3d v2 = Nodes(n2).Transformed(tr);
						Point3d v3 = Nodes(n3).Transformed(tr);

						float x[9] = { v1.X(), v1.Y(), v1.Z(), v2.X(), v2.Y(), v2.Z(), v3.X(), v3.Y(), v3.Z() };
						new_tris->AddTri(x);
					}
				}
			}
		}

	}

	return new_tris;

}

#endif

static std::string Point__str__(const Point& self) {
	std::ostringstream ss;
	ss << self;
	return ss.str();
}

static std::string Point3d__str__(const Point3d& self) {
	std::ostringstream ss;
	ss << self;
	return ss.str();
}

static std::string CVertex__str__(const CVertex& self) {
	std::ostringstream ss;
	ss << self;
	return ss.str();
}

static std::string Span__str__(const Span& self) {
	std::ostringstream ss;
	ss << self;
	return ss.str();
}

static std::string CBox__str__(const CBox& self) {
	std::ostringstream ss;
	ss << self;
	return ss.str();
}

static std::string Matrix__str__(const Matrix& self) {
	std::ostringstream ss;
	ss << self;
	return ss.str();
}

boost::python::object CurveIsACircle(const CCurve& curve, double tol)
{
	Circle circle;
	if (curve.IsACircle(circle, tol))
	{
		return boost::python::object(circle);
	}
	else
	{
		return boost::python::object(); // None
	}
}

BOOST_PYTHON_MODULE(geom) {
	/// class Point
	/// a 2D point, with x and y values, which can also be used as a vector
	bp::class_<Point>("Point", "2D Point can be used as a vector too.") 
        .def(bp::init<double, double>()) /// function Point///params float x, float y/// make a new Point from x and y values///return Point
        .def(bp::init<Point>())
        .def(bp::other<double>() * bp::self)
        .def(bp::self * bp::other<double>())//, "returns a Point with x and y multiplied by the by the multiplier\nyou can also do multiplication the other way round; p2 = 3.0 * p\nreturn Point")
        .def(bp::self / bp::other<double>())/// function / ///params float divider/// returns a Point with x and y divided by the by the divider///return Point
        .def(bp::self * bp::other<Point>())/// function *///params Point p2/// returns the dot product of this point and p2///return float
        .def(bp::self - bp::other<Point>())/// function - ///params Point p2/// returns a Point wit:///x = this points x - p2.x///y = this points y - p2.y///you can also use "-" to return the Point with (-x, -y)///return Point
        .def(bp::self + bp::other<Point>())/// function + ///params Point p2/// returns a Point with:///x = this points x + p2.x///y = this points y + p2.y///return Point
        .def(bp::self ^ bp::other<Point>())/// function ^///params Point p2/// returns the 2D cross product of this point and p2///return float
        .def(bp::self == bp::other<Point>())/// function ==///params Point p2/// returns true if x == p2.x and y == p2.y///return Boolean
        .def(bp::self != bp::other<Point>())/// function !=///params Point p2/// returns true if x != p2.x or y != p2.y///return Boolean
        .def(-bp::self)
        .def(~bp::self)/// function ~/// returns a vector 90 degrees to the left of this one///return Point
        .def("Dist", &Point::dist) /// function Dist /// return float/// params Point p2 /// returns the distance to another point
        .def("Length", &Point::length) /// function Length /// return float///returns the length of the vector
        .def("Normalize", &Point::normalize) /// function Normalize /// makes the vector into a unit vector /// this will leave a (0, 0) vector as it is///returns the length before operation///return float
		.def("Rotate", static_cast< void (Point::*)(double, double) >(&Point::Rotate)) /// function Rotate///params float cosa, float sina /// rotates the vector about (0,0) given cosine and sine of the angle
		.def("Rotate", static_cast< void (Point::*)(double) >(&Point::Rotate)) /// function Rotate///params float angle/// rotates the vector about (0,0) by given angle ( in radians )
		.def("Transform", &Point::Transform)/// function Transform///params Matrix m/// transforms the point by the matrix
        .def_readwrite("x", &Point::x)/// variable x///type float/// the x value of the point
        .def_readwrite("y", &Point::y)/// variable y///type float/// the y value of the point
		.def("__str__", Point__str__);

	/// class Vertex
	/// a Curve has a list of these///it can have type 0, 1, -1 for line, ccw arc, cw arc, and a centre point
	bp::class_<CVertex>("Vertex") 
        .def(bp::init<CVertex>())
        .def(bp::init<int, Point, Point>())/// function Vertex///params int type, Point p, Point c/// make a new Vertex with type, end point and center point/// type 0 = line, 1 = ccw arc, -1 = cw arc
        .def(bp::init<Point>())/// function Vertex///params Point p/// make a new Vertex with end point/// adds a point
        .def(bp::init<int, Point, Point, int>())
        .def_readwrite("type", &CVertex::m_type)/// variable type///type int///0 - line///1 - ccw arc///-1 - cw arc
        .def_readwrite("p", &CVertex::m_p)/// variable p///type Point///the end point of the span
        .def_readwrite("c", &CVertex::m_c)/// variable p///type Point///the center point of the span, for arcs
        .def_readwrite("user_data", &CVertex::m_user_data)
		.def("__str__", CVertex__str__);
	;

	/// class Span
	/// this is just a Vertex with a start point, which defines a Span
	/// and provides functions to work on it
	/// but these are not stored in a Curve ( Curve stores a list of Vertex objects )
	/// You can use curve.GetSpans() to get a list of these from a curve.
	bp::class_<Span>("Span") 
        .def(bp::init<Span>())
        .def(bp::init<Point, CVertex, bool>())///function Span///params Point p, Vertex v, Boolean start_span///return Span///makes a new Span with a start point, an end Vertex, and a flag set to True if this is the start span of a curve
		.def("NearestPoint", static_cast< Point (Span::*)(const Point& p)const >(&Span::NearestPoint))///function NearestPoint///params Point p///return Point///returns the nearest point on this span to the given point
		.def("NearestPoint", &SpanNearestPoint)///function NearestPoint///params Span s2///return Point///return float///returns nearest point on this span to span s2 and returns the distance of that point to s2
		.def("GetBox", &Span::GetBox, "returns the box that fits round the span")
		.def("IncludedAngle", &Span::IncludedAngle)/// function IncludedAngle///return float///returns the included angle of the arc, 1 for ccw, -1 for cw
		.def("GetArea", &Span::GetArea)
		.def("On", &SpanOn)/// function On///params Point p///returns True if point lies on span, else False
		.def("MidPerim", &Span::MidPerim)///function MidPerim///return Point///params float param///returns the point, which is the given fraction ( of the span ) along the span
		.def("MidParam", &Span::MidParam)///function MidParam///return Point///params float perim///returns the point, which is the given distance along the span
		.def("Length", &Span::Length)///function Length///return float///returns the length of the span
		.def("GetVector", &Span::GetVector)///function GetVector///returns the tangential vector ( the vector pointing in the direction of travel )///at the given fraction along the span///return Point///params float
		.def("Intersect", &spanIntersect)///function Intersect///return list///params Span s2///returns a list of intersection points between this span and s2///ordered along this span
		.def("GetRadius", &Span::GetRadius)
        .def_readwrite("p", &Span::m_p)///variable p///type Point///the start point of this span
		.def_readwrite("v", &Span::m_v)///variable v///type Vertex///the Vertex describing the span type, end and center
		.def("__str__", Span__str__);
	;

	/// class Curve
	/// defined by a list of Vertex objects
	/// if you want a closed curve, you need to add a point at the end that is at the same place as the start point
	bp::class_<CCurve>("Curve", "this is a curve") 
        .def(bp::init<CCurve>())///function Curve///return Curve///makes a new blank Curve
        .def("GetVertices", &getVertices)///function GetVertices///return list///returns the list of Vertex objects
        .def("Append",&CCurve::append)///function Append///params Vertex v///adds a Vertex to the list of vertices
        .def("Append",&append_point)///function Append///params Point p///adds a Point to the list of vertices ( makes a Vertex from it )
        .def("Text", &print_curve)///function Text///for debugging, prints a text definition of the Curve
		.def("NearestPoint", static_cast< Point (CCurve::*)(const Point& p)const >(&CCurve::NearestPoint))///function NearestPoint///params Point p///return Point///returns the nearest point on the curve to the given point
		.def("NearestPoint", &nearest_point_to_curve)///function NearestPoint///params Curve c///return Point///return float///returns the nearest point on the curve to the given point and returns the distance of it to this curve
		.def("Reverse", &CCurve::Reverse)///function Reverse///reverses this curve
		.def("NumVertices", &num_vertices)///function NumVertices///return float///returns the number of vertices; quicker than getting the list of vertices
		.def("FirstVertex", &FirstVertex)///function FirstVertex///return Vertex///returns the first vertex
		.def("LastVertex", &LastVertex)///function LastVertex///return Vertex///returns the last vertex
		.def("GetArea", &CCurve::GetArea)///function GetArea///return float///returns the area enclosed by the curve
		.def("IsClockwise", &CCurve::IsClockwise)///function IsClockwise///returns True if this curve is closed and clockwise
		.def("IsClosed", &CCurve::IsClosed)///function IsClosed///returns True if this curve is closed
        .def("ChangeStart",&CCurve::ChangeStart)///function ChangeStart///params Point p///changes the start of this curve, keeps it closed if it was closed
        .def("ChangeEnd",&CCurve::ChangeEnd)///function ChangeEnd///params Point p///changes the end point of this curve, doesn't keep closed kurves closed
        .def("Offset",&CCurve::Offset)///function Offset///params float leftwards_value///offsets the curve by given amount to the left, keeps closed curves closed
        .def("OffsetForward",&CCurve::OffsetForward)
        .def("GetSpans",&getCurveSpans)///function GetSpans///return list///makes and returns a list of Span objects
        .def("GetFirstSpan",&getFirstCurveSpan)///function GetFirstSpan///return Span///returns a Span for the start of the curve
        .def("GetLastSpan",&getLastCurveSpan)///function GetLastSpan///return Span///returns a Span for the end of the curve
        .def("Break",&CCurve::Break)///function Break///inserts a Point at given point
        .def("Perim",&CCurve::Perim)///function Perim///return float///returns the length of the curve ( its perimeter )
        .def("PerimToPoint",&CCurve::PerimToPoint)///function PerimToPoint///return Point///params float p///returns the Point at the given distance around the Curve
        .def("PointToPerim",&CCurve::PointToPerim)///function PointToPerim///return float///params Point p///returns the distance around the Curve at the given Point
		.def("FitArcs",&CCurve::FitArcs)///function FitArcs///replaces little lines with arcs where possible
        .def("UnFitArcs",&CCurve::UnFitArcs)///function UnFitArcs///replaces arcs with lots of little lines
        .def("Intersections",&CurveIntersections)///function Intersections///return list///params Curve c2///returns a list of all the intersections between this Curve and the given Curve///ordered along this Curve
		.def("GetMaxCutterRadius", &CurveGetMaxCutterRadius)
		.def("GetBox", &CurveGetBox)///function GetBox///return Box///returns the box that fits round the curve
		.def("Transform", &CCurve::Transform)
		.def("IsACircle", CurveIsACircle)
		
		;

	/// class Box
	/// a 2D box used for returning the extents of a Span or Curve
	bp::class_<CBox2D>("Box") 
        .def(bp::init<CBox2D>())
		.def(bp::init<const Point&, const Point&>())
		.def("MinX", &CBox2D::MinX)///function MinX///return float///returns the minimum x value
		.def("MaxX", &CBox2D::MaxX)///function MaxX///return float///returns the maximum x value
		.def("MinY", &CBox2D::MinY)///function MinY///return float///returns the minimum y value
		.def("MaxY", &CBox2D::MaxY)///function MaxY///return float///returns the maximum y value
		.def("Width", &CBox2D::Width)
		.def("Height", &CBox2D::Height)
		.def("InsertPoint", static_cast< void(CBox2D::*)(const Point&) >(&CBox2D::Insert))
		.def("InsertBox", static_cast< void(CBox2D::*)(const CBox2D&) >(&CBox2D::Insert))
		.def_readwrite("minxy", &CBox2D::m_minxy)
		.def_readwrite("maxxy", &CBox2D::m_maxxy)
		;

	/// class Box3D
	/// a 3D box used for returning the extents of a Solid
	bp::class_<CBox>("Box3D")
		.def(bp::init<CBox>())
		.def(bp::init<double, double, double, double, double, double>())
		.def("MinX", &CBox::MinX)///function MinX///return float///returns the minimum x value
		.def("MaxX", &CBox::MaxX)///function MaxX///return float///returns the maximum x value
		.def("MinY", &CBox::MinY)///function MinY///return float///returns the minimum y value
		.def("MaxY", &CBox::MaxY)///function MaxY///return float///returns the maximum y value
		.def("MinZ", &CBox::MinZ)///function MinZ///return float///returns the minimum z value
		.def("MaxZ", &CBox::MaxZ)///function MaxZ///return float///returns the maximum z value
		.def("InsertBox", static_cast< void (CBox::*)(const CBox&) >(&CBox::Insert))///function Insert
		.def("InsertPoint", static_cast< void (CBox::*)(double, double, double) >(&CBox::Insert))///function Insert
		.def("Center", &CBoxCenter)
		.def("Radius", &CBox::Radius)
		.def_readwrite("valid", &CBox::m_valid)
		.def("Width", &CBox::Width)
		.def("Height", &CBox::Height)
		.def("Depth", &CBox::Depth)
		.def("__str__", CBox__str__);
	;

	/// class Area
	/// a list of Curve objects that can represent an area with optional islands
	bp::class_<CArea>("Area") 
        .def(bp::init<CArea>())///function Area///makes an new empty Area///return Area
        .def("GetCurves", &getCurves)///function GetCurves///return list///returns the list of Curve objects
        .def("Append",&CArea::append)///function Append///params Curve c///adds a curve to the area///you must add outside curves first, followed by island curves///you must make sure your outside curves are anti-clockwise and island curves clockwise, or use Reorder function ( see below )
        .def("Subtract",&CArea::Subtract)///function Subtract///params Area a2///cuts a2 away from this area
        .def("Intersect",&CArea::Intersect)///function Intersect///params Area a2///leaves the area that is common to both this area and a2
        .def("Union",&CArea::Union)///function Union///params Area a2///joins a2 to this area
        .def("Offset",&CArea::Offset)///function Offset///params float inwards_value///offset the area inwards by the value///use a negative value to offset outwards///this can change the number of curves of the area///when you offset too far there will be no curves left
        .def("FitArcs",&CArea::FitArcs)///function FitArcs///replaces little lines with arcs where possible
        .def("Text", &print_area)///function Text///for debugging, prints a text definition of the Area
		.def("NumCurves", &CArea::num_curves)///function NumCurves///return int///returns the number of curves in this area
		.def("NearestPoint", &CArea::NearestPoint)///function NearestPoint///return Point///params Point p///returns the nearest point on this areas curves to the given point
		.def("GetBox", &AreaGetBox)///function GetBox///return Box///returns the box that fits round the area
		.def("Reorder", &CArea::Reorder)///function Reorder///This will reorder and reverse the curves where necessary
		.def("Split", &SplitArea)///function Split///return list///splits up the area, where it has multiple outside curves and makes a list of separate areas///if no splitting occurs the list contains a copy of this area
		.def("InsideCurves", &InsideCurves)
		.def("Thicken", &CArea::Thicken)///function Thicken///replaces the area with united obrounds with given radius around each span///params float radius
        .def("Intersections",&AreaIntersections)///function Intersections///return list///params Curve c///returns a list of intersection points with this area and the given curve///ordered along the given curve
		.def("GetArea", &AreaGetArea)///function GetArea///return float///returns the area enclosed by the area
		.def("WriteDxf", static_cast< void(*)(const CArea& area, const std::string& dxf_file_path) >(&WriteDxfFile))///function WriteDxf///writes a dxf file///params string filepath
		.def("Swept", &CArea::Swept)
		.def("Transform", &CArea::Transform)
		.def("GetTriangles", &CArea::GetTriangles)
		;

	///class Matrix
	/// defines a 4x4 transformation matrix
	bp::class_<Matrix > ("Matrix")
        .def(bp::init<Matrix>())
		.def("__init__", bp::make_constructor(&matrix3point_constructor))
		.def("__init__", bp::make_constructor(&matrix_constructor))///function Matrix///return Matrix///params list values///makes a Matrix from a list of 16 floats
	    .def("TransformedPoint", &transformed_point)///function TransformedPoint///return float///return float///return float///params float x, float y, float z///transforms a 3D point by the matrix/// given x, y, z vlaues///returns x, y, z
		.def("Multiply", &Matrix::Multiply)///function Multiply///params Matrix m///transforms this matrix by the given one
		.def("Inverse", &Matrix::Inverse)///function Inverse
		.def("Rotate", &MatrixRotate)
		.def("RotateAxis", &MatrixRotateAxis)
		.def("Translate", static_cast< void (Matrix::*)(const Point3d&) >(&Matrix::Translate))
		.def("Scale", static_cast< void (Matrix::*)(double) >(&Matrix::Scale))
		.def("Scale3", static_cast< void (Matrix::*)(double, double, double) >(&Matrix::Scale))
		.def("__str__", Matrix__str__);
	;

	///class Point3d
	bp::class_<Point3d>("Point3D")
		.def(bp::init<Point3d>())
		.def(bp::init<double, double, double>())///function Point3d///params float x, float y, float z///return Point3d///makes a new Point3d with given x, y, z values
		.def("Transform", &Point3dTransform)
		.def("Transformed", &Point3d::Transformed)
		.def_readwrite("x", &Point3d::x)
		.def_readwrite("y", &Point3d::y)
		.def_readwrite("z", &Point3d::z)
		.def(bp::self * bp::other<double>())/// function *///params float multiplier/// returns a Point with x and y multiplied by the by the multiplier///you can also do multiplication the other way round; p2 = 3.0 * p///return Point
		.def(bp::self / bp::other<double>())/// function / ///params float divider/// returns a Point with x and y divided by the by the divider///return Point
		.def(bp::self * bp::other<Point3d>())/// function *///params Point p2/// returns the dot product of this point and p2///return float
		.def(bp::self - bp::other<Point3d>())/// function - ///params Point p2/// returns a Point wit:///x = this points x - p2.x///y = this points y - p2.y///you can also use "-" to return the Point with (-x, -y)///return Point
		.def(bp::self + bp::other<Point3d>())/// function + ///params Point p2/// returns a Point with:///x = this points x + p2.x///y = this points y + p2.y///return Point
		.def(bp::self ^ bp::other<Point3d>())/// function ^///params Point p2/// returns the 2D cross product of this point and p2///return float
		.def(bp::self == bp::other<Point3d>())/// function ==///params Point p2/// returns true if x == p2.x and y == p2.y///return Boolean
		.def(bp::self != bp::other<Point3d>())/// function !=///params Point p2/// returns true if x != p2.x or y != p2.y///return Boolean
		.def(-bp::self)
		.def("Normalized", &Point3d::Normalized)
		.def("Dist", &Point3d::Dist)
		.def("Length", &Point3d::magnitude)
		.def("ArbitraryAxes", &ArbitraryAxes)
		.def("__str__", Point3d__str__);
	;

	///class Plane
	bp::class_<Plane>("Plane")
		.def(bp::init<Plane>())
		.def("__init__", bp::make_constructor(&plane_constructor))
		.def("Intof", &PlaneIntofPlane)
		.def_readwrite("normal", &Plane::normal)
		;

	///class Line
	bp::class_<Line>("Line")
		.def(bp::init<Line>())
		.def(bp::init<const Point3d &, const Point3d &>())
		.def(bp::init<const Point3d &, const Point3d &>())
		.def("Transform", &LineTransform)
		.def_readwrite("p", &Line::p0)
		.def_readwrite("v", &Line::v)
		.def("IntersectPlane", &LineIntersectPlane)
		;

	///class Circle
	bp::class_<Circle>("Circle")
		.def(bp::init<Circle>())
		.def(bp::init<const Point&, double>())
		.def(bp::init<const Point&, const Point&>())
		.def(bp::init<const Point&, const Point&, const Point&>())
		.def("Transform", &LineTransform)
		.def_readwrite("c", &Circle::pc)
		.def_readwrite("radius", &Circle::radius)
		;

	/// class Stl
	/// a collection of triangles, usually read in from an stl file
	bp::class_<CTris>("Stl")
		.def(bp::init<CTris>())///function Stl///makes a new empty Stl
		.def(bp::init<const std::wstring&>())///function Stl///params string stl_file_path///makes a Stl by reading an stl file
		.def("MakeSection", &CTris::MakeSection)///function MakeSection///params Point s, Point e, dxf_file_path///makes a dxf file with a drawing of the section through this solid/// using the given line to cut it
		.def("WriteStl", &CTris::WriteStl)///function WriteStl///params string stl_file_path///makes a stl file for this solid
		.def("BooleanCut", &CTris::BooleanCut, bp::return_value_policy<bp::manage_new_object>())
		.def("BooleanUnion", &CTris::BooleanUnion, bp::return_value_policy<bp::manage_new_object>())
		.def("BooleanCommon", &CTris::BooleanCommon, bp::return_value_policy<bp::manage_new_object>())
		.def("SplitTriangles", &CTris::SplitTriangles)
		.def("Shadow", &CTris::Shadow2Mat)
		.def("Project", &CTrisProject)
		.def("Transform", &CTris::Transform)
		.def("GetBox", &CTrisGetBox)
		.def("NumTris", &CTrisNumTris)
		.def("GetMachiningAreas", &CTrisGetMachiningAreas)
		.def("Add", &CTrisAddTriangle)
		.def("GetFlattenedSurface", &CTris::GetFlattenedSurface, bp::return_value_policy<bp::manage_new_object>())
		.def("GetTrianglesAsCurveList", &GetTrianglesAsCurveList)
		.def(bp::self += bp::other<CTris>())
		;

#ifdef OPEN_CASCADE_INCLUDED

	/// class Solid
	/// a solid model, read in from an step file
	bp::class_<CSolid>("Solid")
		.def(bp::init<CSolid>())///function Solid///makes a new empty Solid
		.def(bp::init<CArea, double>())///function Solid///params Area area, double thickness///makes a new Solid extruded from an Area by the given thickness
		.def(bp::init<CCurve, double>())///function Solid///params Curve curve, double thickness///makes a new Solid extruded from an Curve by the given thickness
		.def(bp::init<const std::wstring&>())///function Solid///params string step_file_path///makes a Solid by reading an step file
		.def("WriteStep", &CSolid::WriteStep)///function WriteStep///params string step_file_path///makes a step file for this solid
		.def("BooleanCut", &CSolid::BooleanCut, bp::return_value_policy<bp::manage_new_object>())
		.def("BooleanUnion", &CSolid::BooleanUnion, bp::return_value_policy<bp::manage_new_object>())
		.def("BooleanCommon", &CSolid::BooleanCommon, bp::return_value_policy<bp::manage_new_object>())
		.def("Translate", &CSolid::Translate)
		.def("Transform", &CSolid::Transform)
		.def("GetBox", &SolidGetBox)
		.def("MakeStl", &CSolidMakeStl, bp::return_value_policy<bp::manage_new_object>())
		.def("IsNull", &CSolid::IsNull)
		;

#endif

	bp::enum_<FaceFlatType>("FaceFlatType")
		.value("Flat", FaceFlatTypeFlat)
		.value("UpButNotFlat", FaceFlatTypeUpButNotFlat)
		.value("Down", FaceFlatTypeDown)
		;

	/// class MachiningArea
	bp::class_<CMachiningArea>("MachiningArea")
		.def(bp::init<CMachiningArea>())
		.def_readwrite("area", &CMachiningArea::m_area)
		.def_readwrite("top", &CMachiningArea::m_top)
		.def_readwrite("bottom", &CMachiningArea::m_bottom)
		.def_readwrite("face_type", &CMachiningArea::m_face_type)
	;

	/// endclass

    bp::def("set_units", set_units, "function called set_units", bp::args("units"));
    bp::def("get_units", get_units);
    bp::def("AreaFromDxf", AreaFromDxf);///function AreaFromDxf///return Area///params str filepath///creates an Area from a dxf file
    bp::def("TangentialArc", TangentialArc);///function TangentialArc /// return Point /// return int/// params Point p0, Point p1, Point v0///given start point, end point and start vector/// returns the center point and span type
	bp::def("oct_ele_count", get_oct_ele_count);
	bp::def("set_tolerance", set_tolerance);
	bp::def("get_tolerance", get_tolerance);
	bp::def("set_accuracy", set_accuracy);
	bp::def("get_accuracy", get_accuracy);
	bp::def("set_fitarcs", set_fitarcs);
	bp::def("get_fitarcs", get_fitarcs);
}
