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
#include "Viewport.h"
#include "Viewpoint.h"

namespace bp = boost::python;

bool OnInit()
{
	return theApp.OnInit();
}

	BOOST_PYTHON_MODULE(cad) {

		bp::class_<CViewport>("Viewport")
			.def(bp::init<int, int>())
			.def("glCommands", &CViewport::glCommands)
			.def("WidthAndHeightChanged", &CViewport::WidthAndHeightChanged)
			.def("OnMouseEvent", &CViewport::OnMouseEvent)
			.def_readwrite("m_need_update", &CViewport::m_need_update)
			.def_readwrite("m_need_refresh", &CViewport::m_need_refresh)
			.def_readwrite("m_orthogonal", &CViewport::m_orthogonal)
			;

		bp::class_<MouseEvent>("MouseEvent")
			.def(bp::init<MouseEvent>())
			.def_readwrite("m_event_type", &MouseEvent::m_event_type)
			.def_readwrite("m_x", &MouseEvent::m_x)
			.def_readwrite("m_y", &MouseEvent::m_y)
			.def_readwrite("m_leftDown", &MouseEvent::m_leftDown)
			.def_readwrite("m_middleDown", &MouseEvent::m_middleDown)
			.def_readwrite("m_rightDown", &MouseEvent::m_rightDown)
			.def_readwrite("m_controlDown", &MouseEvent::m_controlDown)
			.def_readwrite("m_shiftDown", &MouseEvent::m_shiftDown)
			.def_readwrite("m_altDown", &MouseEvent::m_altDown)
			.def_readwrite("m_metaDown", &MouseEvent::m_metaDown)
			.def_readwrite("m_wheelRotation", &MouseEvent::m_wheelRotation)
			.def_readwrite("m_wheelDelta", &MouseEvent::m_wheelDelta)
			.def_readwrite("m_linesPerAction", &MouseEvent::m_linesPerAction)
			;

		bp::def("OnInit", OnInit);
	}
