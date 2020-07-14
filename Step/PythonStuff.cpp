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
#include "EllipseDrawing.h"

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

HeeksObj* NewSphere()
{
	gp_Trsf mat = make_matrix(theApp->GetDrawMatrix(true)->e);
	CSphere* new_object = new CSphere(gp_Pnt(0, 0, 0).Transformed(mat), 5, NULL, HeeksColor(240, 191, 191), 1.0f);
	return new_object;
}

HeeksObj* NewCuboid()
{
	gp_Trsf mat = make_matrix(theApp->GetDrawMatrix(false)->e);
	CCuboid* new_object = new CCuboid(gp_Ax2(gp_Pnt(0, 0, 0).Transformed(mat), gp_Dir(0, 0, 1).Transformed(mat), gp_Dir(1, 0, 0).Transformed(mat)), 10, 10, 10, NULL, HeeksColor(191, 240, 191), 1.0f);
	return new_object;
}

HeeksObj* NewCyl()
{
	gp_Trsf mat = make_matrix(theApp->GetDrawMatrix(true)->e);
	CCylinder* new_object = new CCylinder(gp_Ax2(gp_Pnt(0, 0, 0).Transformed(mat), gp_Dir(0, 0, 1).Transformed(mat), gp_Dir(1, 0, 0).Transformed(mat)), 5, 10, NULL, HeeksColor(191, 191, 240), 1.0f);
	return new_object;
}

HeeksObj* NewCone()
{
	gp_Trsf mat = make_matrix(theApp->GetDrawMatrix(true)->e);
	CCone* new_object = new CCone(gp_Ax2(gp_Pnt(0, 0, 0).Transformed(mat), gp_Dir(0, 0, 1).Transformed(mat), gp_Dir(1, 0, 0).Transformed(mat)), 10, 5, 20, NULL, HeeksColor(240, 240, 191), 1.0f);
	return new_object;
}

void SetStepFileObjectType(int type){ StepFileObject::m_type = type; }
void SetVertexType(int type){ HVertex::m_type = type; }
void SetEdgeType(int type){ CEdge::m_type = type; }
void SetFaceType(int type){ CFace::m_type = type; }
void SetLoopType(int type){ CLoop::m_type = type; }
void SetSolidType(int type){ CSolid::m_type = type; }
void SetWireType(int type){ CWire::m_type = type; }
int GetVertexType(){ return HVertex::m_type; }
int GetEdgeType(){ return CEdge::m_type; }
int GetFaceType(){ return CFace::m_type; }
int GetLoopType(){ return CLoop::m_type; }
int GetSolidType(){ return CSolid::m_type; }
int GetWireType(){ return CWire::m_type; }

void CreateSweepPy(boost::python::list &list, HeeksObj* profile, const HeeksColor& color)
{
	std::list<HeeksObj*> o_list;
	for (int i = 0; i < len(list); ++i)
	{
		o_list.push_back(boost::python::extract<HeeksObj*>(list[i]));
	}
	CreateSweep(o_list, profile, true, color);
}

void CutShapes()
{
	std::list<HeeksObj*> objects;
	theApp->GetSelection(objects);
	CShape::CutShapes(objects);
}

void FuseShapes()
{
	std::list<HeeksObj*> objects;
	theApp->GetSelection(objects);
	CShape::FuseShapes(objects);
}

void CommonShapes()
{
	std::list<HeeksObj*> objects;
	theApp->GetSelection(objects);
	CShape::CommonShapes(objects);
}

void FilletOrChamferEdges(double rad, bool chamfer_not_fillet)
{
	std::list<HeeksObj*> objects;
	theApp->GetSelection(objects);
	CShape::FilletOrChamferEdges(objects, rad, chamfer_not_fillet);
}

void SetEllipseDrawing()
{
	ellipse_drawing.drawing_mode = EllipseDrawingMode;
	theApp->SetInputMode(&ellipse_drawing);
}

	BOOST_PYTHON_MODULE(step) {

		boost::python::class_<CShape, boost::python::bases<IdNamedObjList>, boost::noncopyable >("Shape", boost::python::no_init)
			;
		boost::python::class_<CSolid, boost::python::bases<CShape>, boost::noncopyable >("Solid", boost::python::no_init)
			;
		boost::python::class_<CCuboid, boost::python::bases<CSolid>, boost::noncopyable >("Cuboid", boost::python::no_init)
			.def_readwrite("width", &CCuboid::m_x)
			.def_readwrite("height", &CCuboid::m_y)
			.def_readwrite("depth", &CCuboid::m_z)
			.def("OnApplyProperties", &CCuboid::OnApplyProperties)
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
		boost::python::def("GetVertexType", GetVertexType);
		boost::python::def("GetEdgeType", GetEdgeType);
		boost::python::def("GetFaceType", GetFaceType);
		boost::python::def("GetLoopType", GetLoopType);
		boost::python::def("GetSolidType", GetSolidType);
		boost::python::def("GetWireType", GetWireType);
		boost::python::def("WriteSolids", WriteSolids);
		boost::python::def("CreateRuledSurface", PickCreateRuledSurface);
		boost::python::def("CreateExtrusion", CreateExtrusionOrRevolution);
		boost::python::def("CreateSweep", CreateSweepPy);
		boost::python::def("ImportSTEPFile", ImportSTEPFile);
		boost::python::def("CutShapes", CutShapes);
		boost::python::def("FuseShapes", FuseShapes);
		boost::python::def("CommonShapes", CommonShapes);
		boost::python::def("FilletOrChamferEdges", FilletOrChamferEdges);
		boost::python::def("SetEllipseDrawing", SetEllipseDrawing);
	}
	