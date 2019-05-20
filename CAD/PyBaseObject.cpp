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
std::list<Property *> *property_list = NULL;
HeeksObj* object_for_get_properties = NULL;


void HandlePythonCallError();

bool BaseObject::NeverDelete(){ return true; }

int BaseObject::GetType()const{
	std::pair<bool, int> result = CallReturnInt("GetType");
	if (result.first)
		return result.second;
	return 0;
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

	if (!CallVoidReturn("OnGlCommands"))
		ObjList::glCommands(select, marked, no_color);
}

void BaseObject::GetProperties(std::list<Property *> *list)
{
	property_list = list;
	object_for_get_properties = this;
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


void AddProperty(Property* property)
{
	if (property_list)
		property_list->push_back(property);
}
