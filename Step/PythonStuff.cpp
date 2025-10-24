#include "stdafx.h"


#include <Python.h>

#ifdef WIN32

#include "windows.h"
#include <GL/gl.h>
#include <GL/glu.h>

#else
#include </usr/include/GL/gl.h>
#include </usr/include/GL/glu.h>
#endif

#include <boost/foreach.hpp>
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/class.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/python/call.hpp>

#include "strconv.h"
#include "StepFileObject.h"
#include "Solid.h"
#include "Face.h"
#include "Edge.h"
#include "Vertex.h"
#include "Loop.h"
#include "Wire.h"
#include "Sphere.h"
#include "Cuboid.h"
#include "Cylinder.h"
#include "Cone.h"
#include "App.h"
#include "RuledSurface.h"
#include "HEllipse.h"
#include "HSpline.h"
#include "ConversionTools.h"
#include "DigitizedPoint.h"

std::wstring step_dir;

CApp* theApp = NULL;

void SetResPath(const std::wstring& dir)
{
	step_dir = dir;
}

void SetApp(CApp* app)
{
	theApp = app;
}

HeeksObj* CreateStepFileObject(){ return new StepFileObject(); }
HeeksObj* CreateVertexObject(){ return new HVertex(); }
HeeksObj* CreateEdgeObject(){ return new CEdge(); }
HeeksObj* CreateFaceObject(){ return new CFace(); }
HeeksObj* CreateLoopObject(){ return new CLoop(); }
HeeksObj* CreateSolidObject(){ return new CSolid(); }
HeeksObj* CreateWireObject(){ return new CWire(); }
HeeksObj* CreateEllipse(){ return new HEllipse(); }
HeeksObj* CreateSpline(){ return new HSpline(); }


HeeksObj* NewSphere()
{
	CSphere* new_object = new CSphere(gp_Pnt(0, 0, 0), 5, NULL, HeeksColor(240, 191, 191), 1.0f);
	return new_object;
}

HeeksObj* NewCuboid()
{
	CCuboid* new_object = new CCuboid(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0)), 10, 10, 10, NULL, HeeksColor(191, 240, 191), 1.0f);
	return new_object;
}

CCylinder* NewCyl()
{
	CCylinder* new_object = new CCylinder(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0)), 5, 10, NULL, HeeksColor(191, 191, 240), 1.0f);
	return new_object;
}

HeeksObj* NewCone()
{
	CCone* new_object = new CCone(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0)), 10, 5, 20, NULL, HeeksColor(240, 240, 191), 1.0f);
	return new_object;
}

void SetStepFileObjectType(int type){ StepFileObject::m_type = type; }
void SetVertexType(int type){ HVertex::m_type = type; }
void SetEdgeType(int type){ CEdge::m_type = type; }
void SetFaceType(int type){ CFace::m_type = type; }
void SetLoopType(int type){ CLoop::m_type = type; }
void SetSolidType(int type){ CSolid::m_type = type; }
void SetWireType(int type){ CWire::m_type = type; }
void SetEllipseType(int type){ HEllipse::m_type = type; }
void SetSplineType(int type){ HSpline::m_type = type; }
int GetVertexType(){ return HVertex::m_type; }
int GetEdgeType(){ return CEdge::m_type; }
int GetFaceType(){ return CFace::m_type; }
int GetLoopType(){ return CLoop::m_type; }
int GetSolidType(){ return CSolid::m_type; }
int GetWireType(){ return CWire::m_type; }
int GetEllipseType(){ return HEllipse::m_type; }
int GetSplineType(){ return HSpline::m_type; }

void CreateSweepPy(boost::python::list &list, HeeksObj* profile, const HeeksColor& color)
{
	std::list<HeeksObj*> o_list;
	for (int i = 0; i < len(list); ++i)
	{
		o_list.push_back(boost::python::extract<HeeksObj*>(list[i]));
	}
	CreateSweep(o_list, profile, true, color);
}

HeeksObj* CutShapes(boost::python::list& list)
{
	std::list<HeeksObj*> o_list;
	for (int i = 0; i < len(list); ++i)
	{
		o_list.push_back(boost::python::extract<HeeksObj*>(list[i]));
	}
	return CShape::CutShapes(o_list);
}

HeeksObj* FuseShapes(boost::python::list& list)
{
	std::list<HeeksObj*> o_list;
	for (int i = 0; i < len(list); ++i)
	{
		o_list.push_back(boost::python::extract<HeeksObj*>(list[i]));
	}
	return CShape::FuseShapes(o_list);
}

HeeksObj* CommonShapes(boost::python::list& list)
{
	std::list<HeeksObj*> o_list;
	for (int i = 0; i < len(list); ++i)
	{
		o_list.push_back(boost::python::extract<HeeksObj*>(list[i]));
	}
	return CShape::CommonShapes(o_list);
}

void FilletOrChamferEdges(double rad, bool chamfer_not_fillet)
{
	std::list<HeeksObj*> objects;
	theApp->GetSelection(objects);
	CShape::FilletOrChamferEdges(objects, rad, chamfer_not_fillet);
}

#if 0
port this to python
void SetEllipseDrawing()
{
	ellipse_drawing.drawing_mode = EllipseDrawingMode;
	theApp->SetInputMode(&ellipse_drawing);
}
#endif

void SetShowFaceNormals(bool value)
{
	CSettings::showFaceNormals = value;
}

bool GetShowFaceNormals()
{
	return CSettings::showFaceNormals;
}

HeeksObj* NewSketchFromFace(HeeksObj* object)
{
	HeeksObj* new_object = theApp->CreateNewSketch();
	if (object->GetType() == CFace::m_type)
	{
		ConvertFaceToSketch2(((CFace*)object)->Face(), new_object, 0.1);
	}
	return new_object;
}

void FaceRadiusChange(HeeksObj* object, double new_radius)
{
	if (object->GetType() == CFace::m_type)
	{
		ChangeFaceRadius(((CFace*)object)->Face(), new_radius);
	}
}

HSpline* NewSplineFromPoints(boost::python::list &list, Point3d *start_vector = NULL, Point3d *end_vector = NULL)
{
	std::list<gp_Pnt> p_list;
	for (int i = 0; i < len(list); ++i)
	{
		Point3d p = boost::python::extract<Point3d>(list[i]);
		gp_Pnt gp(p.x, p.y, p.z);
		p_list.push_back(gp);
	}

	gp_Vec startVec;
	gp_Vec* pStartVec = NULL;
	if (start_vector != NULL)
	{
		startVec.SetX(start_vector->x);
		startVec.SetY(start_vector->y);
		startVec.SetZ(start_vector->z);
		pStartVec = &startVec;
	}

	gp_Vec endVec;
	gp_Vec* pEndVec = NULL;
	if (end_vector != NULL)
	{
		endVec.SetX(end_vector->x);
		endVec.SetY(end_vector->y);
		endVec.SetZ(end_vector->z);
		pEndVec = &endVec;
	}

	HSpline* new_object = new HSpline(p_list, &theApp->GetCurrentColor(), pStartVec, pEndVec);
	return new_object;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(NewSplineFromPointsOverloads, NewSplineFromPoints, 1, 3)


boost::python::object GetFacePlane(const CFace* face)
{
	gp_Pln returned_plane;
	if (face->IsAPlane(&returned_plane))
	{
		Plane plane(G2P(returned_plane.Location()), D2P(returned_plane.Axis().Direction()));
		return boost::python::object(plane);
	}
	else
	{
		return boost::python::object(); // None
	}
}

boost::python::list ShapeGetFaces(const CShape* shape) {
	boost::python::list olist;
	for (HeeksObj *object = shape->m_faces->GetFirstChild(); object; object = shape->m_faces->GetNextChild())
	{
		olist.append(boost::python::pointer_wrapper<CFace*>((CFace*)object));
	}
	return olist;
}


bool DigitizedPointGetEllipse(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, gp_Elips& e)
{
	double d = d2.m_point.Dist(d1.m_point);
	e.SetLocation(P2G(d1.m_point));
	e.SetMajorRadius(d);
	e.SetMinorRadius(d / 2);

	Point3d vec = d2.m_point - d1.m_point;
	vec = vec / d;
	double rot = atan2(vec.y, vec.x);

	gp_Dir up(0, 0, 1);
	gp_Pnt zp(0, 0, 0);
	e.Rotate(gp_Ax1(P2G(d1.m_point), up), rot);

	gp_Dir x_axis = e.XAxis().Direction();
	gp_Dir y_axis = e.YAxis().Direction();
	double maj_r = d;

	//We have to rotate the incoming vector to be in our coordinate system
	gp_Pnt cir = P2G(d3.m_point - d1.m_point);
	cir.Rotate(gp_Ax1(zp, up), -rot + M_PI / 2);

	double nradius = 1 / sqrt((1 - (1 / maj_r)*(1 / maj_r)*cir.Y()*cir.Y()) / (cir.X() * cir.X()));
	if (nradius < maj_r)
		e.SetMinorRadius(nradius);
	else
	{
		e.SetMajorRadius(nradius);
		e.SetMinorRadius(maj_r);
		e.Rotate(gp_Ax1(P2G(d1.m_point), up), M_PI / 2);
	}


	return true;
}


void SetEllipse(HEllipse* object, const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3)
{
	gp_Elips e;
	DigitizedPointGetEllipse(d1, d2, d3, e);
	object->SetEllipse(e);
}

static boost::shared_ptr<HEllipse> initHEllipse(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3)
{
	gp_Elips e;
	DigitizedPointGetEllipse(d1, d2, d3, e);
	return boost::shared_ptr<HEllipse>(new HEllipse(e, &theApp->GetCurrentColor()));
}


boost::python::list CreateExtrusionOrRevolution(boost::python::list& list, double height_or_angle, bool solid_if_possible, bool revolution_not_extrusion, double taper_angle_for_extrusion, const HeeksColor &color)
{
	std::list<TopoDS_Shape> faces_or_wires;

	std::list<HeeksObj*> objects;
	for (int i = 0; i < len(list); ++i)
	{
		objects.push_back(boost::python::extract<HeeksObj*>(list[i]));
	}

	ConvertToFaceOrWire(objects, faces_or_wires, (fabs(taper_angle_for_extrusion) <= 0.0000001) && solid_if_possible);

	std::list<TopoDS_Shape> new_shapes;
	Matrix m = *(theApp->GetDrawMatrix(false));
	gp_Trsf trsf = make_matrix(m.e);
	if (revolution_not_extrusion)
	{
		CreateRevolutions(faces_or_wires, new_shapes, gp_Ax1(gp_Pnt(0, 0, 0).Transformed(trsf), gp_Vec(1, 0, 0).Transformed(trsf)), height_or_angle);
	}
	else
	{
		CreateExtrusions(faces_or_wires, new_shapes, gp_Vec(0, 0, height_or_angle).Transformed(trsf), taper_angle_for_extrusion, solid_if_possible);
	}

	boost::python::list olist;
	if (new_shapes.size() > 0)
	{
		for (std::list<TopoDS_Shape>::iterator It = new_shapes.begin(); It != new_shapes.end(); It++){
			TopoDS_Shape& shape = *It;
			HeeksObj* new_object = CShape::MakeObject(shape, revolution_not_extrusion ? L"Revolved Solid" : L"Extruded Solid", SOLID_TYPE_UNKNOWN, color, 1.0f);
			olist.append(boost::python::pointer_wrapper<CShape*>((CShape*)new_object));
		}
	}

	for (std::list<TopoDS_Shape>::iterator It = faces_or_wires.begin(); It != faces_or_wires.end(); It++)
	{
		TopoDS_Shape shape = *It;
		shape.Free();
	}

	return olist;
}

	BOOST_PYTHON_MODULE(step) {

		boost::python::class_<CShape, boost::python::bases<IdNamedObjList>, boost::noncopyable >("Shape", boost::python::no_init)
			.def("GetFaces", ShapeGetFaces)
			.def("OnApplyProperties", &CShape::OnApplyProperties)
			;
		
		boost::python::class_<CSolid, boost::python::bases<CShape>, boost::noncopyable >("Solid", boost::python::no_init);

		boost::python::class_<CFace, boost::python::bases<HeeksObj>, boost::noncopyable >("Face", boost::python::no_init)
			.def("GetPlane", GetFacePlane)
			.def("GetParentBody", &CFace::GetParentBody, boost::python::return_value_policy<boost::python::reference_existing_object>())
			;

		boost::python::class_<CEdge, boost::python::bases<HeeksObj>, boost::noncopyable >("Edge", boost::python::no_init)
			.def("Blend", &CEdge::Blend)
			;

		boost::python::class_<CCuboid, boost::python::bases<CSolid>, boost::noncopyable >("Cuboid", boost::python::no_init)
			.def_readwrite("width", &CCuboid::m_x)
			.def_readwrite("height", &CCuboid::m_y)
			.def_readwrite("depth", &CCuboid::m_z)
			;

		boost::python::class_<CCylinder, boost::python::bases<CSolid>, boost::noncopyable >("Cylinder", boost::python::no_init)
			.def_readwrite("radius", &CCylinder::m_radius)
			.def_readwrite("height", &CCylinder::m_height)
			;

		boost::python::class_<HEllipse, boost::python::bases<HeeksObj> >("Ellipse", boost::python::no_init)
			.def("__init__", boost::python::make_constructor(&initHEllipse))
			.def("SetFromDigitized", SetEllipse)
			;

		boost::python::class_<HSpline, boost::python::bases<EndedObject> >("Spline", boost::python::no_init)
			.def("GetStartTangent", &HSpline::GetStartTangent)
			.def("GetEndTangent", &HSpline::GetEndTangent)
			;

		boost::python::def("SetResPath", SetResPath);
		boost::python::def("SetApp", SetApp);
		boost::python::def("CreateStepFileObject", CreateStepFileObject, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("CreateVertexObject", CreateVertexObject, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("CreateEdgeObject", CreateEdgeObject, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("CreateFaceObject", CreateFaceObject, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("CreateLoopObject", CreateLoopObject, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("CreateSolidObject", CreateSolidObject, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("CreateWireObject", CreateWireObject, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("CreateEllipse", CreateEllipse, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("CreateSpline", CreateSpline, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("NewSphere", NewSphere, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("NewCyl", NewCyl, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("NewCone", NewCone, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("NewCuboid", NewCuboid, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("SetStepFileObjectType", SetStepFileObjectType);
		boost::python::def("SetVertexType", SetVertexType);
		boost::python::def("SetEdgeType", SetEdgeType);
		boost::python::def("SetFaceType", SetFaceType);
		boost::python::def("SetLoopType", SetLoopType);
		boost::python::def("SetSolidType", SetSolidType);
		boost::python::def("SetWireType", SetWireType);
		boost::python::def("SetEllipseType", SetEllipseType);
		boost::python::def("SetSplineType", SetSplineType);
		boost::python::def("GetVertexType", GetVertexType);
		boost::python::def("GetEdgeType", GetEdgeType);
		boost::python::def("GetFaceType", GetFaceType);
		boost::python::def("GetLoopType", GetLoopType);
		boost::python::def("GetSolidType", GetSolidType);
		boost::python::def("GetWireType", GetWireType);
		boost::python::def("GetEllipseType", GetEllipseType);
		boost::python::def("GetSplineType", GetSplineType);
		boost::python::def("WriteSolids", WriteSolids);
		boost::python::def("CreateRuledSurface", PickCreateRuledSurface);
		boost::python::def("CreateExtrusion", CreateExtrusionOrRevolution);
		boost::python::def("CreateSweep", CreateSweepPy);
		boost::python::def("ImportSolidsFile", ImportSolidsFile);
		boost::python::def("ExportSolidsFile", ExportSolidsFile);
		boost::python::def("CutShapes", CutShapes, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("FuseShapes", FuseShapes, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("CommonShapes", CommonShapes, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("FilletOrChamferEdges", FilletOrChamferEdges);
		boost::python::def("SketchToFace", SketchToFace);
		boost::python::def("SetShowFaceNormals", SetShowFaceNormals);
		boost::python::def("GetShowFaceNormals", GetShowFaceNormals); 
		boost::python::def("NewSketchFromFace", NewSketchFromFace, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("FaceRadiusChange", FaceRadiusChange);
		boost::python::def("NewSplineFromPoints", &NewSplineFromPoints, (boost::python::arg("points"), boost::python::arg("start_vector") = (Point3d*)NULL,	boost::python::arg("end_vector") = (Point3d*)NULL), boost::python::return_value_policy<boost::python::reference_existing_object>());
	}
	