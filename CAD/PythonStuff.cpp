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

#include "../Geom/geometry.h"

namespace bp = boost::python;

void DoSomeOpenGL1(float x, float y)
{
#if 1
	// set viewing projection
	glMatrixMode(GL_PROJECTION);
	glFrustum(-0.8, 0.8, -0.8, 0.8, 0.4, 8.0);

	// position viewer
	glMatrixMode(GL_MODELVIEW);
	glTranslatef(0.0, 0.0, -2.0);

	// position object
	glRotatef(y, 1.0, 0.0, 0.0);
	glRotatef(x, 0.0, 1.0, 0.0);

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glLineWidth(1);
	glDepthMask(1);
	glEnable(GL_CULL_FACE);


	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
#endif
}

void DrawCube(float x)
{
	// draw six faces of a cube
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(0.5 + x, 0.5, 0.5);
	glVertex3f(-0.5 + x, 0.5, 0.5);
	glVertex3f(-0.5 + x, -0.5, 0.5);
	glVertex3f(0.5 + x, -0.5, 0.5);

	glNormal3f(0.0, 0.0, -1.0);
	glVertex3f(-0.5 + x, -0.5, -0.5);
	glVertex3f(-0.5 + x, 0.5, -0.5);
	glVertex3f(0.5 + x, 0.5, -0.5);
	glVertex3f(0.5 + x, -0.5, -0.5);

	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(0.5 + x, 0.5, 0.5);
	glVertex3f(0.5 + x, 0.5, -0.5);
	glVertex3f(-0.5 + x, 0.5, -0.5);
	glVertex3f(-0.5 + x, 0.5, 0.5);

	glNormal3f(0.0, -1.0, 0.0);
	glVertex3f(-0.5 + x, -0.5, -0.5);
	glVertex3f(0.5 + x, -0.5, -0.5);
	glVertex3f(0.5 + x, -0.5, 0.5);
	glVertex3f(-0.5 + x, -0.5, 0.5);

	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(0.5 + x, 0.5, 0.5);
	glVertex3f(0.5 + x, -0.5, 0.5);
	glVertex3f(0.5 + x, -0.5, -0.5);
	glVertex3f(0.5 + x, 0.5, -0.5);

	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(-0.5 + x, -0.5, -0.5);
	glVertex3f(-0.5 + x, -0.5, 0.5);
	glVertex3f(-0.5 + x, 0.5, 0.5);
	glVertex3f(-0.5 + x, 0.5, -0.5);
	glEnd();
}

void DoSomeOpenGL2()
{
	//clear color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawCube(0.0);

	DrawCube(2.0);
}

void cad_glRotatef(float a, float b, float c, float d)
{
	glRotatef(a, b, c, d);
}

geoff_geometry::Point3d camera_point(0, 0, 0);

void SetCameraPoint(const geoff_geometry::Point3d& p)
{
	camera_point = p;
}

wxFrame* NewFrame()
{
	return new wxFrame(NULL, -1, _T("test"));
}

geoff_geometry::Point3d GetCameraPoint()
{
	return camera_point;
}

	BOOST_PYTHON_MODULE(cad) {

		bp::def("DoSomeOpenGL1", DoSomeOpenGL1);
		bp::def("DoSomeOpenGL2", DoSomeOpenGL2);
		bp::def("glRotatef", cad_glRotatef);
		bp::def("SetCameraPoint", SetCameraPoint);
		bp::def("GetCameraPoint", GetCameraPoint);
		bp::def("NewFrame", NewFrame, bp::return_value_policy<bp::reference_existing_object>());
	}
