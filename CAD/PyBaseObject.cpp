#include "stdafx.h"

#include <Python.h>

#include <boost/progress.hpp>
#include <boost/timer.hpp>
#include <boost/foreach.hpp>
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/class.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/python/call.hpp>

#include "PyBaseObject.h"
#include "strconv.h"







std::wstring str_for_base_object;
HeeksColor color_for_base_object;


bool BaseObject::NeverDelete(){ return true; }

int BaseObject::GetType()const{
	return m_type;
	//std::pair<bool, int> result = CallReturnInt("GetType");
	//if (result.first)
	//	return result.second;
	//return 0;
}

const wchar_t* BaseObject::GetIconFilePath()
{
	std::pair<bool, std::string> result = CallReturnString("GetIconFilePath");
	if (result.first)
		return Ctt(result.second.c_str());
	return ObjList::GetIconFilePath();
}

const wchar_t* BaseObject::GetShortString()const
{
	std::pair<bool, std::string> result = CallReturnString("GetTitle");
	if (result.first)
		return Ctt(result.second.c_str());
	return ObjList::GetShortString();
}

const wchar_t* BaseObject::GetTypeString()const
{
	std::pair<bool, std::string> result = CallReturnString("GetTypeString");
	if (result.first)
		return Ctt(result.second.c_str());
	return ObjList::GetTypeString();
}

const HeeksColor* BaseObject::GetColor()const
{
	std::pair<bool, bool> result = CallReturnBool("HasColor");
	if (result.first && result.second)
	{
		// HasColor exists and HasColor returns True
		std::pair<bool, HeeksColor> result2 = CallReturnColor("GetColor");
		if (result2.first)
		{
			color_for_base_object = result2.second;
			return &color_for_base_object;
		}
	}
	return ObjList::GetColor();
}

void BaseObject::SetColor(const HeeksColor &col)
{
	CallVoidReturn("SetColor", col);
}

void BaseObject::glCommands(bool select, bool marked, bool no_color)
{
	m_no_colour = no_color;
	m_marked = marked;
	m_select = select;

	if (!CallVoidReturn("OnGlCommands", select, marked, no_color))
		ObjList::glCommands(select, marked, no_color);
}

extern std::list<Property *> *property_list;

void BaseObject::GetProperties(std::list<Property *> *list)
{
	property_list = list;
	if (!CallVoidReturn("GetProperties"))
		ObjList::GetProperties(list);
}


void BaseObject::GetBox(CBox &box)
{
	if (!CallVoidReturn("GetBox", box))
		ObjList::GetBox(box);
}

void BaseObject::KillGLLists()
{
	if (!CallVoidReturn("KillGLLists"))
		ObjList::KillGLLists();
}

void BaseObject::WriteToXML(TiXmlElement *element)
{
	BaseObject::m_cur_element = element;
	CallVoidReturn("WriteXml");
}

void BaseObject::ReadFromXML(TiXmlElement *element)
{
	BaseObject::m_cur_element = element;
	CallVoidReturn("ReadXml");
}

HeeksObj* BaseObject::MakeACopy()const
{
	std::pair<bool, HeeksObj*> result = CallReturnHeeksObj("MakeACopy");
	if (result.first)
		return result.second;
	return NULL;
}

void BaseObject::CopyFrom(const HeeksObj* object)
{
	copy_from_object = object;
	CallVoidReturn("CopyFrom", object);
}

void BaseObject::ReloadPointers()
{
	if (!CallVoidReturn("ReloadPointers"))
		ObjList::ReloadPointers();
}

bool BaseObject::OneOfAKind()
{
	std::pair<bool, bool> result = CallReturnBool("OneOfAKind");
	if (result.first)
		return result.second;
	return ObjList::OneOfAKind();
}

bool BaseObject::OneOfAKind_default(){
	return ObjList::OneOfAKind();
}

void BaseObject::OnAdd()
{
	CallVoidReturn("OnAdd");
}

void BaseObject::OnRemove()
{
	CallVoidReturn("OnRemove");
}

bool BaseObject::SetClickMarkPoint(const Point3d &ray_start, const Point3d &ray_direction)
{
	std::pair<bool, bool> result = CallReturnBool("SetClickMarkPoint", ray_start, ray_direction);
	if (result.first)
		return result.second;
	return ObjList::SetClickMarkPoint(ray_start, ray_direction);
}

static void(*callback_for_GetTriangles)(const double*, const double*) = NULL;
static double cusp_for_GetTriangles = 0.0;
static bool just_one_average_normal_for_GetTriangles = false;

void BaseObject::GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal)
{
	callback_for_GetTriangles = callbackfunc;
	cusp_for_GetTriangles = cusp;
	just_one_average_normal_for_GetTriangles = just_one_average_normal;
	CallVoidReturn("GetTriangles");
}

void BaseObject::AddTriangle(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2)
{
	static double x[9];
	static double n[9];

	x[0] = x0;
	x[1] = y0;
	x[2] = z0;
	x[3] = x1;
	x[4] = y1;
	x[5] = z1;
	x[6] = x2;
	x[7] = y2;
	x[8] = z2;
	Point3d p0(&x[0]);
	Point3d p1(&x[1]);
	Point3d p2(&x[2]);
	Point3d v1(p0, p1);
	Point3d v2(p0, p2);
	try
	{
		Point3d norm = (v1 ^ v2).Normalized();
		n[0] = norm.x;
		n[1] = norm.y;
		n[2] = norm.z;
		if (!just_one_average_normal_for_GetTriangles)
		{
			n[3] = n[0];
			n[4] = n[1];
			n[5] = n[2];
			n[6] = n[0];
			n[7] = n[1];
			n[8] = n[2];
		}
		(*callback_for_GetTriangles)(x, n);
	}
	catch (...)
	{
	}
}


boost::python::override BaseObject::get_override(char const* name) const
{
	boost::python::override result = boost::python::wrapper<ObjList>::get_override(name);
	if (result != NULL)
		return result;
	PyErr_Clear();
	return result;
}


// static definitions
bool BaseObject::in_glCommands = false;
bool BaseObject::triangles_begun = false;
bool BaseObject::lines_begun = false;
TiXmlElement* BaseObject::m_cur_element = NULL;
bool BaseObject::m_no_colour = false;
bool BaseObject::m_marked = false;
bool BaseObject::m_select = false;
const HeeksObj* BaseObject::copy_from_object = NULL;

