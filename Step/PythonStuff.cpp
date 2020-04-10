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


#include <boost/progress.hpp>
#include <boost/timer.hpp>
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

void SetStepFileObjectType(int type){ StepFileObject::m_type = type; }
void SetVertexType(int type){ HVertex::m_type = type; }
void SetEdgeType(int type){ CEdge::m_type = type; }
void SetFaceType(int type){ CFace::m_type = type; }
void SetLoopType(int type){ CLoop::m_type = type; }
void SetSolidType(int type){ CSolid::m_type = type; }
void SetWireType(int type){ CWire::m_type = type; }


	BOOST_PYTHON_MODULE(step) {
		
		boost::python::def("SetResPath", SetResPath);
		boost::python::def("SetApp", SetApp);
		boost::python::def("CreateStepFileObject", CreateStepFileObject, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("CreateVertexObject", CreateVertexObject, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("CreateEdgeObject", CreateEdgeObject, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("CreateFaceObject", CreateFaceObject, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("CreateLoopObject", CreateLoopObject, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("CreateStepFileObject", CreateStepFileObject, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("CreateStepFileObject", CreateStepFileObject, boost::python::return_value_policy<boost::python::reference_existing_object>());
		boost::python::def("SetStepFileObjectType", SetStepFileObjectType);
		boost::python::def("SetVertexType", SetVertexType);
		boost::python::def("SetEdgeType", SetEdgeType);
		boost::python::def("SetFaceType", SetFaceType);
		boost::python::def("SetLoopType", SetLoopType);
		boost::python::def("SetSolidType", SetSolidType);
		boost::python::def("SetWireType", SetWireType);
	}
