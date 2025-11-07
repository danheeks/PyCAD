// PythonStuff.cpp
// Copyright 2011, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "PythonStuff.h"

#include "Area.h"
#include "Point.h"
#include "AreaDxf.h"
#include "geometry.h"
#include "Tris.h"
#include "HeeksGeomDxf.h"
#include "Box.h"
#include "Mesh.h"

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

bool SplitAtZ(double z, CTris& new_tris);

boost::python::object CTrisSplitAtZ(CTris& tris, double z)
{
	CTris new_tris;
	if (tris.SplitAtZ(z, new_tris))
	{
		return boost::python::object(new_tris);
	}
	else
	{
		return boost::python::object(); // None
	}
}


boost::python::object LineIntersectPlane(const Line& line, const Plane& plane)
{
	Point3d intof;
	double t;
	if (plane.Intof(line, intof, t))
	{
		return boost::python::object(intof);
	}
	else
	{
		return boost::python::object(); // None
	}
}

void CTrisAddTriangle(CTris& tris, const Point3d& p0, const Point3d& p1, const Point3d& p2)
{
	float x[9] = { (float)p0.x, (float)p0.y, (float)p0.z, (float)p1.x, (float)p1.y, (float)p1.z, (float)p2.x, (float)p2.y, (float)p2.z };
	tris.AddTri(x);
}

CMesh* CTrisGetMesh(const CTris& tris)
{
	return new CMesh(tris);
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

boost::python::list CTrisGetTriangles(const CTris& tris)
{
	boost::python::list clist;
	for (std::list<CTri>::const_iterator It = tris.m_tris.begin(); It != tris.m_tris.end(); It++)
	{
		const CTri& tri = *It;
		clist.append(bp::make_tuple(bp::make_tuple(tri.x[0][0], tri.x[0][1], tri.x[0][2]), bp::make_tuple(tri.x[1][0], tri.x[1][1], tri.x[1][2]), bp::make_tuple(tri.x[2][0], tri.x[2][1], tri.x[2][2])));
	}
	return clist;
}

boost::python::list CTrisSplit(const CTris& t)
{
	std::list<CTris> tri_list;
	t.Split(tri_list);
	boost::python::list python_list;
	for (std::list<CTris>::const_iterator It = tri_list.begin(); It != tri_list.end(); It++)
	{
		const CTris& tris = *It;
		python_list.append(tris);
	}
	return python_list;
}

boost::python::tuple MeshGetFaces(const CMesh& mesh)
{
	boost::python::list vlist;
	boost::python::list flist;

	int next_vertex_index = 0;
	std::map<const CMeshVertex*, int> vertex_map;
	for (std::list<CMeshFace*>::const_iterator It = mesh.m_faces.begin(); It != mesh.m_faces.end(); It++)
	{
		const CMeshFace* face = *It;

		boost::python::list ilist;

		unsigned int size = face->m_vertices.size();
		for (unsigned int i = 0; i < size; i++)
		{
			const CMeshVertex* v = face->m_vertices[i];
			std::map<const CMeshVertex*, int>::const_iterator FindIt = vertex_map.find(v);
			int index = -1;
			if (FindIt == vertex_map.end()){
				index = next_vertex_index;
				vertex_map.insert(std::make_pair(v, next_vertex_index));
				vlist.append(boost::python::make_tuple(v->m_x[0], v->m_x[1], v->m_x[2]));
				next_vertex_index++;
			}
			else
			{
				index = FindIt->second;
			}
			ilist.append(index);
		}

		flist.append(ilist);
	}

	return boost::python::make_tuple(vlist, flist);
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

boost::python::list AreaGetTrianglesList(const CArea& a)
{
	std::list<CTris> tri_list;
	a.GetTriangles(tri_list);
	boost::python::list python_list;
	for (std::list<CTris>::const_iterator It = tri_list.begin(); It != tri_list.end(); It++)
	{
		const CTris& tris = *It;
		python_list.append(tris);
	}
	return python_list;
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

static std::string Circle__str__(const Circle& self) {
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

static std::string Curve__str__(const CCurve& self) {
	std::ostringstream ss;
	ss << self;
	return ss.str();
}

static std::string CBox__str__(const CBox& self) {
	std::ostringstream ss;
	ss << self;
	return ss.str();
}

static std::string Area__str__(const CArea& self) {
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


static CCurve MakeCircleCurveAtPoint(const Point& centre, double radius)
{
	Point p0 = centre + Point(radius, 0.0);
	Point p1 = centre + Point(-radius, 0.0);
	CCurve curve;
	curve.append(p0);
	curve.append(CVertex(1, p1, centre));
	curve.append(CVertex(1, p0, centre));
	return curve;
}

BOOST_PYTHON_MODULE(geom) {

	bp::docstring_options local_docstring_options(true, true, false); // This will enable user-defined docstrings and python signatures, while disabling the C++ signatures

	bp::class_<Point>("Point", "Point((float)x, (float)y)\n\n2D Point\nCan also be used as a vector"
		"\nTo make a vector from p1 to p2, use v = p2 - p1"
		"\nUse p1 * p2 for dot product"
		"\nUse p1 ^ p2 for cross product"
		"\nUse ~p to return a vector at 90 degrees to the left of p")
		.def(bp::init<double, double>())
		.def(bp::init<Point>())
		.def(bp::other<double>() * bp::self)
		.def(bp::self * bp::other<double>())
		.def(bp::self / bp::other<double>())
		.def(bp::self * bp::other<Point>())
		.def(bp::self - bp::other<Point>())
		.def(bp::self + bp::other<Point>())
		.def(bp::self ^ bp::other<Point>())
		.def(bp::self == bp::other<Point>())
		.def(bp::self != bp::other<Point>())
		.def(-bp::self)
		.def(~bp::self)
		.def("Dist", &Point::dist, bp::args("p2"), "returns the distance between this point and p2")
		.def("Length", &Point::length, "returns the length of the vector")
		.def("Normalize", &Point::normalize, "makes the vector into a unit vector\nthis will leave a (0, 0) vector as it is\nreturns the (float)length before operation")
		.def("Rotate", static_cast<void (Point::*)(double, double)>(&Point::Rotate), bp::args("cosa, sina"), "rotates the vector about (0,0) given cosine and sine of the angle")
		.def("Rotate", static_cast<void (Point::*)(double)>(&Point::Rotate), bp::args("angle"), "rotates the vector about (0,0) by given angle ( in radians )")
		.def("Transform", &Point::Transform, bp::args("m"), "transforms the point by the matrix")
		.def_readwrite("x", &Point::x, "the x value of the point")
		.def_readwrite("y", &Point::y, "the y value of the point")
		.def("__str__", Point__str__);
	;

	bp::class_<CVertex>("Vertex", "Vertex((int)type, (Point)p, (Point)c) make a new Vertex with type, end point and center point; type 0 = line, 1 = ccw arc, -1 = cw arc"
		"Vertex((Point)p) make a Vertex with end point")
		.def(bp::init<CVertex>())
		.def(bp::init<int, Point, Point>())
		.def(bp::init<Point>())
		.def(bp::init<int, Point, Point, int>())
		.def_readwrite("type", &CVertex::m_type, "0 - line, 1 - ccw arc, -1 - cw arc")
		.def_readwrite("p", &CVertex::m_p, "the end point of the span")
		.def_readwrite("c", &CVertex::m_c, "the center point of the span, for arcs")
		.def_readwrite("user_data", &CVertex::m_user_data)
		.def("__str__", CVertex__str__);
	;

	bp::class_<Span>("Span", "Span((Point)p, (Vertex)v, (bool)start_span\n\nA Span has start Point and a Vertex\nThese re not stored in a Curve ( Curve stores a list of Vertex objects )\nYou can use curve.GetSpans() to get a list of these from a curve")
		.def(bp::init<Span>())
		.def(bp::init<Point, CVertex, bool>())
		.def("NearestPoint", static_cast<Point(Span::*)(const Point& p)const>(&Span::NearestPoint), bp::args("p"), "returns the nearest point on this span to the given point")
		.def("NearestPoint", &SpanNearestPoint, bp::args("s2"), "returns tuple (nearest point on this span to span s2, distance of that point to s2)")
		.def("GetBox", &Span::GetBox, "returns the box that fits round the span")
		.def("IncludedAngle", &Span::IncludedAngle, "returns the included angle of the arc in radians, + for ccw, - for cw")
		.def("GetArea", &Span::GetArea, "returns the area under the span to the X axis")
		.def("On", &SpanOn, bp::args("p"), "returns True if point lies on span, else False")
		.def("MidPerim", &Span::MidPerim, bp::args("param"), "returns the point which is the given fraction ( of the span ) along the span")
		.def("MidParam", &Span::MidParam, bp::args("perim"), "returns the point, which is the given distance along the span")
		.def("Length", &Span::Length, bp::args("p"), "returns the length of the span")
		.def("GetVector", &Span::GetVector, "returns the tangential vector ( the vector pointing in the direction of travel )\nat the given fraction along the span")
		.def("Intersect", &spanIntersect, bp::args("s2"), "returns a list of intersection points between this span and s2\nordered along this span")
		.def("GetRadius", &Span::GetRadius, "returns the radius of the arc, or 0.0 if it's a line")
		.def_readwrite("p", &Span::m_p, "the start point of this span")
		.def_readwrite("v", &Span::m_v, "the Vertex describing the span type, end and center")
		.def("__str__", Span__str__);
	;

	bp::class_<CCurve>("Curve", "Curve()\n\ndefined by a list of Vertex objects\nif you want a closed curve, you need to add a point at the end that is at the same place as the start point") 
        .def(bp::init<CCurve>())
        .def("GetVertices", &getVertices, "returns the list of Vertex objects")
		.def("Append", &CCurve::append, bp::args("v"), "adds a Vertex to the list of vertices")
		.def("Append", &append_point, bp::args("p"), "adds a Point to the list of vertices ( makes a Vertex from it )")
        .def("Text", &print_curve, "for debugging, prints a text definition of the Curve")
		.def("NearestPoint", static_cast< Point(CCurve::*)(const Point& p)const >(&CCurve::NearestPoint), bp::args("p"), "returns the nearest point on the curve to the given point")
		.def("NearestPoint", &nearest_point_to_curve, bp::args("c2"), "returns the nearest point on the curve to the given curve, c2, and returns the distance of it to this curve")
		.def("Reverse", &CCurve::Reverse, "reverses this curve")
		.def("NumVertices", &num_vertices, "returns the number of vertices; quicker than getting the list of vertices")
		.def("FirstVertex", &FirstVertex, "returns the first vertex")
		.def("LastVertex", &LastVertex, "returns the last vertex")
		.def("GetArea", &CCurve::GetArea, "returns the area enclosed by the curve")
		.def("IsClockwise", &CCurve::IsClockwise, "returns True if this curve is closed and clockwise, else False")
		.def("IsClosed", &CCurve::IsClosed, "returns True if this curve is closed, else False")
		.def("ChangeStart", &CCurve::ChangeStart, bp::args("p"), "changes the start of this curve, keeps it closed if it was closed")
		.def("ChangeEnd", &CCurve::ChangeEnd, bp::args("p"), "changes the end point of this curve, doesn't keep closed kurves closed")
		.def("Offset", &CCurve::Offset, bp::args("leftwards_value"), "offsets the curve by given amount to the left, keeps closed curves closed")
		.def("OffsetForward", &CCurve::OffsetForward, bp::args("forwards_value", "refit_arcs"), "for drag-knife compensation")
        .def("GetSpans",&getCurveSpans, "returns a list of Span objects")
        .def("GetFirstSpan",&getFirstCurveSpan, "returns a Span for the start of the curve")
        .def("GetLastSpan",&getLastCurveSpan, "returns a Span for the end of the curve")
		.def("Break", &CCurve::Break, bp::args("p"), "inserts a Point at given point")
        .def("Perim",&CCurve::Perim, "returns the length of the curve ( its perimeter )")
		.def("PerimToPoint", &CCurve::PerimToPoint, bp::args("perim"), "returns the Point at the given distance around the Curve")
		.def("PointToPerim", &CCurve::PointToPerim, bp::args("p"), "returns the distance around the Curve at the given Point")
		.def("FitArcs",&CCurve::FitArcs, "replaces little lines with arcs where possible")
        .def("UnFitArcs",&CCurve::UnFitArcs, "replaces arcs with lots of little lines")
		.def("Intersections", &CurveIntersections, bp::args("c2"), "returns a list of all the intersections between this Curve and the given Curve\nordered along this Curve")
		.def("GetBox", &CurveGetBox, "returns the box that fits round the curve")
		.def("Transform", &CCurve::Transform, bp::args("m"), "transforms the curve by the matrix\na curve is only 2D though, so don't rotate in 3D")
		.def("IsACircle", CurveIsACircle, bp::args("tol"), "returns True if all the spans are arcs of the same direction and fit the same circle\n to given tolerance")
		.def("__str__", Curve__str__);
	;

	bp::class_<CBox2D>("Box", "Box((Point)minxy, (Point)maxxy)\n\na 2D box used for returning the extents of a Span or Curve") 
        .def(bp::init<CBox2D>())
		.def(bp::init<const Point&, const Point&>())
		.def("MinX", &CBox2D::MinX, "returns the minimum x value")
		.def("MaxX", &CBox2D::MaxX, "returns the maximum x value")
		.def("MinY", &CBox2D::MinY, "returns the minimum y value")
		.def("MaxY", &CBox2D::MaxY, "returns the maximum y value")
		.def("Width", &CBox2D::Width, "returns the width of the box in the X axis")
		.def("Height", &CBox2D::Height, "returns the height of the box in the Y axis")
		.def("InsertPoint", static_cast< void(CBox2D::*)(const Point&) >(&CBox2D::Insert), bp::args("p"), "makes the box bigger to include the given point")
		.def("InsertBox", static_cast< void(CBox2D::*)(const CBox2D&) >(&CBox2D::Insert), bp::args("b2"), "makes the box bigger to include the given box")
		.def_readwrite("minxy", &CBox2D::m_minxy, "the X, Y coordinate of the bottom left of the box")
		.def_readwrite("maxxy", &CBox2D::m_maxxy, "the X, Y coordinate of the top right of the box")
		.def_readwrite("valid", &CBox2D::m_valid, "if False, the box is empty and all the other values are invalid")
		;

	bp::class_<CBox>("Box3D", "Box3D((float)minx, (float)miny, (float)minz, (float)maxx, (float)maxy, (float)maxz)\n\na 3D box used for returning the extents of a Solid")
		.def(bp::init<CBox>())
		.def(bp::init<double, double, double, double, double, double>())
		.def("MinX", &CBox::MinX, "returns the minimum x value")
		.def("MaxX", &CBox::MaxX, "returns the maximum x value")
		.def("MinY", &CBox::MinY, "returns the minimum y value")
		.def("MaxY", &CBox::MaxY, "returns the maximum y value")
		.def("MinZ", &CBox::MinZ, "returns the minimum z value")
		.def("MaxZ", &CBox::MaxZ, "returns the maximum z value")
		.def("InsertBox", static_cast< void (CBox::*)(const CBox&) >(&CBox::Insert), bp::args("b2"), "makes the box bigger to include the given box")
		.def("InsertPoint", static_cast< void (CBox::*)(double, double, double) >(&CBox::Insert), bp::args("p"), "makes the box bigger to include the given point")
		.def("Center", &CBoxCenter, "returns the Point3D at the mid X,Y,Z")
		.def("Radius", &CBox::Radius, "returns the radius of a sphere that would enclose the box exactly if centres at Center()")
		.def_readwrite("valid", &CBox::m_valid, "if False, the box is empty and all the other values are invalid")
		.def("Width", &CBox::Width, "returns the width of the box in the X axis")
		.def("Height", &CBox::Height, "returns the height of the box in the Y axis")
		.def("Depth", &CBox::Depth, "returns the depth of the box in the Z axis")
		.def("__str__", CBox__str__);
	;

	bp::class_<CArea>("Area", "Area()\n\n a list of Curve objects that can represent an area with optional islands")
        .def(bp::init<CArea>())///function Area///makes an new empty Area///return Area
        .def("GetCurves", &getCurves, "returns the list of Curve objects")
		.def("Append", &CArea::append, bp::args("c"), "adds a curve to the area\nyou must add outside curves first, followed by island curves\nyou must make sure your outside curves are anti-clockwise and island curves clockwise\nor use Reorder function")
		.def("Subtract", &CArea::Subtract, bp::args("a2"), "cuts a2 away from this area")
		.def("Intersect", &CArea::Intersect, bp::args("a2"), "leaves the area that is common to both this area and a2")
		.def("Union", &CArea::Union, bp::args("a2"), "joins a2 to this area")
		.def("Offset", &CArea::Offset, bp::args("inwards_value"), "offset the area inwards by the value\nuse a negative value to offset outwards\nthis can change the number of curves of the area\nwhen you offset too far inwards there will be no curves left")
		.def("Thicken", &CArea::Thicken, bp::args("value"), "offset the thin curves outwards by the value to make sausages")
		.def("FitArcs", &CArea::FitArcs, "replaces little lines with arcs where possible. to tolerance set by set_accuracy")
		.def("UnFitArcs", &CArea::UnFitArcs, "replaces arcs with little lines to tolerance set by set_accuracy")
		.def("Text", &print_area, "for debugging, prints a text definition of the Area")
		.def("NumCurves", &CArea::num_curves, "returns the number of curves in this area")
		.def("NearestPoint", &CArea::NearestPoint, bp::args("p"), "returns the nearest point on this area's curves to the given point")
		.def("GetBox", &AreaGetBox, "returns the (Box)box that fits round the area")
		.def("Reorder", &CArea::Reorder, "This reorders and reverses the curves where necessary")
		.def("Split", &SplitArea, "splits up the area, where it has multiple outside curves and makes a list of separate areas\nif no splitting occurs the list contains a copy of this area")
		.def("InsideCurves", &InsideCurves, bp::args("c"), "chops up the given curve with this area\nreturns a list of new curves which are the sections inside the area")
		.def("Thicken", &CArea::Thicken, bp::args("radius"), "replaces the area with united obrounds with given radius around each span")
		.def("Intersections", &AreaIntersections, bp::args("c"), "returns a list of intersection points with this area and the given curve\nordered along the given curve")
		.def("GetArea", &AreaGetArea, "returns the (float)area enclosed by the area")
		.def("WriteDxf", static_cast< void(*)(const CArea& area, const std::string& dxf_file_path) >(&WriteDxfFile), bp::args("filepath"), "writes a dxf file with this area in")
		.def("Swept", &CArea::Swept, bp::args("v"), "returns an area that is this area swept along the given vector")
		.def("Transform", &CArea::Transform, bp::args("m"), "transforms this area by the matrix\nan area is only 2D though, so don't rotate in 3D")
		.def("GetTrianglesList", &AreaGetTrianglesList, "returns a list of Stl objects with triangles that fill each separate area")
		.def("__str__", Area__str__);
	;

	bp::class_<Matrix > ("Matrix", "Matrix((Point)o, (Point)x_vector, (Point)y_vector)\nMatrix([list of 16 floats])\n\ndefines a 4x4 transformation matrix")
        .def(bp::init<Matrix>())
		.def("__init__", bp::make_constructor(&matrix3point_constructor))
		.def("__init__", bp::make_constructor(&matrix_constructor))
		.def("TransformedPoint", &transformed_point, bp::args("x", "y", "z"), "transforms a 3D point by the matrix given x, y, z values\nreturns x, y, z")
		.def("Multiply", &Matrix::Multiply, bp::args("m2"), "transforms this matrix by the given one")
		.def("Inverse", &Matrix::Inverse, "returns a Matrix which is the inverse of this matrix\nthe matrix which reverts the effect of this matrix")
		.def("Rotate", &MatrixRotate, bp::args("angle"), "rotates this matrix by the given angle in radians around the z axis anti-clockwise")
		.def("RotateAxis", &MatrixRotateAxis, bp::args("angle", "axis"), "rotates this matrix by the given angle in radians around the given axis anti-clockwise\nwhen looking backwards along the given vector")
		.def("Translate", static_cast< void (Matrix::*)(const Point3d&) >(&Matrix::Translate), bp::args("shift"), "translates this matrix by the given shift vector")
		.def("Scale", static_cast< void (Matrix::*)(double) >(&Matrix::Scale), bp::args("value"), "scales the vector uniformly about 0, 0, 0 by the given scale factor")
		.def("Scale3", static_cast< void (Matrix::*)(double, double, double) >(&Matrix::Scale), bp::args("x", "y", "z"), "scales the vector differentially about 0, 0, 0 by the given scale factors")
		.def("__str__", Matrix__str__);
	;

	bp::class_<Point3d>("Point3D", "Point((float)x, (float)y, (float)z)\n\n3D Point\nCan also be used as a vector"
		"\nTo make a vector from p1 to p2, use v = p2 - p1"
		"\nUse p1 * p2 for dot product"
		"\nUse p1 ^ p2 for cross product")
		.def(bp::init<Point3d>())
		.def(bp::init<double, double, double>())
		.def("Transform", &Point3dTransform, bp::args("m"), "transforms the point by the matrix")
		.def("Transformed", &Point3d::Transformed, bp::args("m"), "returns a Point3D transformed by the matrix")
		.def_readwrite("x", &Point3d::x, "the x value of the point")
		.def_readwrite("y", &Point3d::y, "the y value of the point")
		.def_readwrite("z", &Point3d::z, "the z value of the point")
		.def(bp::self * bp::other<double>())
		.def(bp::self / bp::other<double>())
		.def(bp::self * bp::other<Point3d>())
		.def(bp::self - bp::other<Point3d>())
		.def(bp::self + bp::other<Point3d>())
		.def(bp::self ^ bp::other<Point3d>())
		.def(bp::self == bp::other<Point3d>())
		.def(bp::self != bp::other<Point3d>())
		.def(-bp::self)
		.def("Normalized", &Point3d::Normalized, "returns a vector which is this vector scaled to a unit vector\nfor a (0, 0, 0) vector, this will return Point3D(0, 0, 0)")
		.def("Normalize", &Point3d::Normalize, "scales this vector to a unit vector")
		.def("Dist", &Point3d::Dist, bp::args("p2"), "returns the distance between this point and p2")
		.def("Length", &Point3d::magnitude, "returns the length of the vector")
		.def("ArbitraryAxes", &ArbitraryAxes, "returns a tuple of (x_axis, y_axis) unit vectors, which have the same relationship to this vector\nas x-axis and y-axis have to z-axis")
		.def("__str__", Point3d__str__);
	;

	bp::class_<Plane>("Plane", "Plane((Point)point_on_plane, (Point)normal_vector)\n\nAn infinite plane")
		.def(bp::init<Plane>())
		.def("__init__", bp::make_constructor(&plane_constructor))
		.def("Intof", &PlaneIntofPlane, bp::args("pl2"), "if the two planes intersect this returns the Line of intersection, else returns None")
		.def_readwrite("normal", &Plane::normal, "unit vector normal to plane")
		.def_readwrite("d", &Plane::d, "distance of plane to origin\nuse normal * (-d) to get a point on the plane")
		;

	bp::class_<Line>("Line", "Line((Point3D)p1, (Point3D)p2) - an infinite line through p1 and p2")
		.def(bp::init<Line>())
		.def(bp::init<const Point3d &, const Point3d &>())
		.def("Transform", &LineTransform, bp::args("m"), "transforms this Line by the matrix")
		.def_readwrite("p", &Line::p0, "the point on the line")
		.def_readwrite("v", &Line::v, "the vector along the line")
		.def("IntersectPlane", &LineIntersectPlane, bp::args("pl"), "returns the intersection (Point3D)point of this line with given plane, if intersection exists\nelse returns None")
		;

	bp::class_<Circle>("Circle", "Circle((Point)center, (float)radius - circle with given center point and radius"
		"\nCircle((Point)point_on_circle, (Point)center) - circle with given point on circle and center point"
		"\nCircle((Point)p1, (Point)p2, (Point)p3) - circle through 3 points")
		.def(bp::init<Circle>())
		.def(bp::init<const Point&, double>())
		.def(bp::init<const Point&, const Point&>())
		.def(bp::init<const Point&, const Point&, const Point&>())
		.def("Transform", &LineTransform, bp::args("m"), "transforms this Circle by the matrix")
		.def_readwrite("c", &Circle::pc, "center point of the circle")
		.def_readwrite("radius", &Circle::radius, "radius of the circle")
		.def("__str__", Circle__str__);
	;

	bp::class_<CTris>("Stl", "Stl() - empty collection of triangles\nStl(file_path) - collection of triangles read in from an .stl file")
		.def(bp::init<CTris>())
		.def(bp::init<const std::wstring&>())
		.def("MakeSection", &CTris::MakeSection, bp::args("s", "e", "dxf_file_path"), "makes a dxf file with a drawing of the section through this solid/nusing the given line to cut it")
		.def("WriteStl", &CTris::WriteStl, bp::args("stl_file_path"), "writes an stl file for this solid")
		.def("BooleanCut", &CTris::BooleanCut, bp::return_value_policy<bp::manage_new_object>(), bp::args("stl2"), "returns a new Stl object which is this Stl cut by the given Stl")
		.def("BooleanUnion", &CTris::BooleanUnion, bp::return_value_policy<bp::manage_new_object>(), bp::args("stl2"), "returns a new Stl object which is this Stl united with the given Stl")
		.def("BooleanCommon", &CTris::BooleanCommon, bp::return_value_policy<bp::manage_new_object>(), bp::args("stl2"), "returns a new Stl object which is common volume between this Stl and the given Stl")
		.def("SplitTriangles", &CTris::SplitTriangles, bp::arg("s2"), "intersects this Stl with the given Stl")
		.def("Shadow", &CTris::Shadow2Mat, bp::args("m", "just_up_allowed"), "returns an Area representing the shadow of this Stl object\nif just_up_allowed is true, then ignore downward facing triangles")
		.def("Project", &CTrisProject, bp::args("area", "dxf_file_path"), "writes a dxf file with the given Area projected down onto the Stl object")
		.def("Transform", &CTris::Transform, bp::args("m"), "transforms this Stl by the matrix")
		.def("GetBox", &CTrisGetBox, "returns the (Box3D)box that fits round the area")
		.def("NumTris", &CTrisNumTris, "returns the number of triangles in this Stl")
		.def("GetMachiningAreas", &CTrisGetMachiningAreas, "joins up triangles of the same FaceFlatType and returns a list of MachiningArea objects")
		.def("SplitAtZ", &CTrisSplitAtZ, "split at z height, returns new Stl object, for triangles above z, or None if split not done")
		.def("Add", &CTrisAddTriangle, bp::args("p1", "p2", "p3"), "Add a triangles given 3 Point3D objects")
		.def("GetFlattenedSurface", &CTris::GetFlattenedSurface, bp::return_value_policy<bp::manage_new_object>(), "returns a new Stl with all the triangles unfolded into a flat shape")
		.def("GetTrianglesAsCurveList", &GetTrianglesAsCurveList, "returns a list of Curve objects, each one being a closed triangle")
		.def("GetMesh", &CTrisGetMesh, bp::return_value_policy<bp::manage_new_object>(), "returns a mesh")
		.def("GetTriangles", &CTrisGetTriangles, "returns the list of tuples of tuples")
		.def("Split", &CTrisSplit, "returns a list of new Stl objects")
		.def("Unwrap", &CTris::Unwrap, bp::return_value_policy<bp::manage_new_object>(), "returns a new Stl with all the triangles unwrapped from a bar at a given radius")
		.def("SplitToSmallerTriangles", &CTris::SplitToSmallerTriangles, bp::return_value_policy<bp::manage_new_object>(), "returns a new Stl with all the triangles no bigger than max_length")

		.def(bp::self += bp::other<CTris>())
		;

	bp::class_<CMesh>("Mesh", "Mesh(() - mesh of triangles")
		.def(bp::init<CMesh>())
		.def("GetFaces", &MeshGetFaces, "a tuple with a list of vertices and a list of faces with vertex indexes")
		;

	bp::enum_<FaceFlatType>("FaceFlatType", "face type for MachiningArea")
		.value("Flat", FaceFlatTypeFlat)
		.value("UpButNotFlat", FaceFlatTypeUpButNotFlat)
		.value("Down", FaceFlatTypeDown)
		;

	bp::class_<CMachiningArea>("MachiningArea", "MachiningArea() - empty MachiningArea\n\nUse Stl.GetMachiningAreas() to get a list of these from an Stl")
		.def(bp::init<CMachiningArea>())
		.def_readwrite("area", &CMachiningArea::m_area, "Area - 2D area of all the similar triangles joined together")
		.def_readwrite("top", &CMachiningArea::m_top, "z height of the top of the machining area")
		.def_readwrite("bottom", &CMachiningArea::m_bottom, "z height of the bottom of the machining area")
		.def_readwrite("face_type", &CMachiningArea::m_face_type, "see FaceFlatType")
	;

    bp::def("set_units", set_units, "function called set_units", bp::args("units"));
    bp::def("get_units", get_units);
    bp::def("AreaFromDxf", AreaFromDxf, bp::args("filepath"), "returns an Area created from a dxf file");
	bp::def("TangentialArc", TangentialArc, bp::args("p1", "p2", "v"), "given start point, end point and start vector\nreturns the center point and span type");
	bp::def("oct_ele_count", get_oct_ele_count, "just for debugging");
	bp::def("set_tolerance", set_tolerance, "set the tolerance used for various geometry things like comparing tow points");
	bp::def("get_tolerance", get_tolerance, "get the tolerance used for various geometry things like comparing tow points");
	bp::def("set_accuracy", set_accuracy, "set the tolerance used for fitting arcs");
	bp::def("get_accuracy", get_accuracy, "get the tolerance used for fitting arcs");
	bp::def("set_fitarcs", set_fitarcs, "set to True if Area.FitArcs() is to be called automatically for boolean Area operations and Offset");
	bp::def("get_fitarcs", get_fitarcs, "see set_fitarcs for description");
	bp::def("MakeCircleCurveAtPoint", MakeCircleCurveAtPoint, bp::args("p", "r"), "given centre point and radius\nreturns a Curve");
	
}
