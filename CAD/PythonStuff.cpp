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
#include "ViewPoint.h"
#include "Observer.h"
#include "strconv.h"
#include "StlSolid.h"
#include "Property.h"
#include "HXml.h"
#include "Material.h"
#include "MarkedList.h"
#include "PropertyChange.h"

namespace bp = boost::python;

bool OnInit()
{
	return theApp.OnInit();
}

void OnExit()
{
	theApp.OnExit();
}

PyObject *main_module = NULL;
PyObject *globals = NULL;

void MessageBoxPythonError()
{
	std::wstring error_string;

	PyObject *errtype, *errvalue, *traceback;
	PyErr_Fetch(&errtype, &errvalue, &traceback);
	PyErr_NormalizeException(&errtype, &errvalue, &traceback);
	if (errtype != NULL && 0)/* error type not as useful, less is more */ {
		PyObject *s = PyObject_Str(errtype);
		PyObject* pStrObj = PyUnicode_AsUTF8String(s);
		char* c = PyBytes_AsString(pStrObj);
		if (c)
		{
			wchar_t wstr[1024];
			mbstowcs(wstr, c, 1024);
			error_string.append(wstr);
			error_string.append(L"\n\n");
		}
		Py_DECREF(s);
		Py_DECREF(pStrObj);
	}
	if (errvalue != NULL) {
		PyObject *s = PyObject_Str(errvalue);
		PyObject* pStrObj = PyUnicode_AsUTF8String(s);
		char* c = PyBytes_AsString(pStrObj);
		if (c)
		{
			wchar_t wstr[1024];
			mbstowcs(wstr, c, 1024);
			error_string.append(wstr);
			error_string.append(L"\n\n\n");
		}
		Py_DECREF(s);
		Py_DECREF(pStrObj);
	}

	PyObject *pModule = PyImport_ImportModule("traceback");

	if (traceback != NULL && pModule != NULL)
	{
		PyObject* pDict = PyModule_GetDict(pModule);
		PyObject* pFunc = PyDict_GetItemString(pDict, "format_tb");
		if (pFunc && PyCallable_Check(pFunc))
		{
			PyObject* pArgs = PyTuple_New(1);
			pArgs = PyTuple_New(1);
			PyTuple_SetItem(pArgs, 0, traceback);
			PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
			if (pValue != NULL)
			{
				int len = PyList_Size(pValue);
				if (len > 0) {
					PyObject *t, *tt;
					int i;
					char *buffer;
					for (i = 0; i < len; i++) {
						tt = PyList_GetItem(pValue, i);
						t = Py_BuildValue("(O)", tt);
						if (!PyArg_ParseTuple(t, "s", &buffer)){
							return;
						}

						wchar_t wstr[1024];
						mbstowcs(wstr, buffer, 1024);
						error_string.append(wstr);
						error_string.append(L"\n");
					}
				}
			}
			Py_DECREF(pValue);
			Py_DECREF(pArgs);
		}
	}
	Py_DECREF(pModule);

	Py_XDECREF(errvalue);
	Py_XDECREF(errtype);
	Py_XDECREF(traceback);

	theApp.MessageBox(error_string.c_str());
}

static void BeforePythonCall(PyObject **main_module, PyObject **globals)
{
	if (*main_module == NULL)
	{
		*main_module = PyImport_ImportModule("__main__");
		*globals = PyModule_GetDict(*main_module);
	}

	const char* str = "import sys\nold_stdout = sys.stdout\nold_stderr = sys.stderr\nclass CatchOutErr:\n  def __init__(self):\n    self.value = ''\n  def write(self, txt):\n    self.value += txt\n  def flush(self):\n    pass\ncatchOutErr = CatchOutErr()\nsys.stdout = catchOutErr\nsys.stderr = catchOutErr"; //this is python code to redirect stdouts/stderr
	PyRun_String(str, Py_file_input, *globals, *globals); //invoke code to redirect

	if (PyErr_Occurred())
		MessageBoxPythonError();
}

static void AfterPythonCall(PyObject *main_module)
{
	PyObject *catcher = PyObject_GetAttrString(main_module, "catchOutErr"); //get our catchOutErr created above
	PyObject *output = PyObject_GetAttrString(catcher, "value"); //get the stdout and stderr from our catchOutErr object
#if PY_MAJOR_VERSION >= 3
	std::wstring s(Ctt(PyUnicode_AsUTF8(output)));
#else
	std::wstring s(Ctt(PyString_AsString(output)));
#endif
	if (s.size() > 0) {
		wprintf(s.c_str());
	}

	if (PyErr_Occurred())
		MessageBoxPythonError();

	const char* str = "sys.stdout = old_stdout\nsys.stderr = old_stderr"; 
	PyRun_String(str, Py_file_input, globals, globals); //invoke code

}

std::wstring str_for_base_object;
HeeksColor color_for_base_object;
std::list<Property *> *property_list = NULL;
HeeksObj* object_for_get_properties = NULL;


/*
This RAII structure ensures that threads created on the native C side
adhere to the laws of Python and ensure they grab the GIL lock when
calling into python
*/
struct PyLockGIL
{

	PyLockGIL()
		: gstate(PyGILState_Ensure())
	{
	}

	~PyLockGIL()
	{
		PyGILState_Release(gstate);
	}

	PyLockGIL(const PyLockGIL&) = delete;
	PyLockGIL& operator=(const PyLockGIL&) = delete;

	PyGILState_STATE gstate;
};

enum
{
	OBJECT_TYPE_UNKNOWN = 0,
	OBJECT_TYPE_SOLID,
	OBJECT_TYPE_SKETCH,
	OBJECT_TYPE_SKETCH_CONTOUR,
	OBJECT_TYPE_SKETCH_SEG,
	OBJECT_TYPE_SKETCH_LINE,
	OBJECT_TYPE_SKETCH_ARC,
	OBJECT_TYPE_CIRCLE,
};

int HeeksTypeToObjectType(long type)
{
	switch (type)
	{
	case StlSolidType:
		return OBJECT_TYPE_SOLID;
	case SketchType:
		return OBJECT_TYPE_SKETCH;
	default:
		return OBJECT_TYPE_UNKNOWN;
	}
}

int ObjectTypeToHeeksType(long type)
{
	switch (type)
	{
	case OBJECT_TYPE_SKETCH:
		return SketchType;
	default:
		return UnknownType;
	}
}

void AddPropertyToPythonList(Property* p, boost::python::list& list)
{
	if (PropertyCheck* o = dynamic_cast<PropertyCheck*>(p)){
		list.append(boost::python::ptr<Property*>(p)); return;
	}
	if (PropertyCheckWithConfig* o = dynamic_cast<PropertyCheckWithConfig*>(p)){ list.append(o); return; }
	if (PropertyChoice* o = dynamic_cast<PropertyChoice*>(p)){ list.append(o); return; }
	if (PropertyColor* o = dynamic_cast<PropertyColor*>(p)){ list.append(o); return; }
	if (PropertyDoubleScaled* o = dynamic_cast<PropertyDoubleScaled*>(p)){ list.append(o); return; }
	if (PropertyDoubleLimited* o = dynamic_cast<PropertyDoubleLimited*>(p)){ list.append(o); return; }
	if (PropertyString* o = dynamic_cast<PropertyString*>(p)){ list.append(o); return; }
	if (PropertyStringReadOnly* o = dynamic_cast<PropertyStringReadOnly*>(p)){ list.append(o); return; }
	if (PropertyStringWithConfig* o = dynamic_cast<PropertyStringWithConfig*>(p)){ list.append(o); return; }
	if (PropertyFile* o = dynamic_cast<PropertyFile*>(p)){ list.append(o); return; }
	if (PropertyInt* o = dynamic_cast<PropertyInt*>(p)){ list.append(o); return; }
	if (PropertyLength* o = dynamic_cast<PropertyLength*>(p)){ list.append(o); return; }
	if (PropertyLengthWithConfig* o = dynamic_cast<PropertyLengthWithConfig*>(p)){ list.append(o); return; }
	if (PropertyLengthWithKillGLLists* o = dynamic_cast<PropertyLengthWithKillGLLists*>(p)){ list.append(o); return; }
	if (PropertyList* o = dynamic_cast<PropertyList*>(p)){ list.append(o); return; }
	if (PropertyObjectTitle* o = dynamic_cast<PropertyObjectTitle*>(p)){ list.append(o); return; }
	if (PropertyObjectColor* o = dynamic_cast<PropertyObjectColor*>(p)){ list.append(o); return; }
}

void AddObjectToPythonList(HeeksObj* object, boost::python::list& list)
{
	list.append(bp::ptr<HeeksObj*>(object));
#if 0
	switch (object->GetType())
	{
#if 0
	case SketchType:
		list.append(boost::python::pointer_wrapper<CSketch*>((CSketch*)object));
		break;
#endif
	case StlSolidType:
		list.append(boost::python::pointer_wrapper<CStlSolid*>((CStlSolid*)object));
		break;
#if 0
	case CircleType:
		list.append(boost::python::pointer_wrapper<HCircle*>((HCircle*)object));
		break;
#endif
	default:
		list.append(boost::python::pointer_wrapper<HeeksObj*>((HeeksObj*)object));
		break;
	}
#endif
}

bp::detail::method_result Call_Override(bp::override &f, bp::list &list)
{
	BeforePythonCall(&main_module, &globals);
	// Execute the python function
	PyLockGIL lock;
	try
	{
		bp::detail::method_result result = f(list);// , removed_list, modified_list);
		AfterPythonCall(main_module);
		return result;
	}
	catch (const bp::error_already_set&)
	{
	}
	AfterPythonCall(main_module);
}

bp::detail::method_result Call_Override(bp::override &f, bp::list &added, bp::list &removed)
{
	BeforePythonCall(&main_module, &globals);

	// Execute the python function
	PyLockGIL lock;
	try
	{
		bp::detail::method_result result = f(added, removed);
		AfterPythonCall(main_module);
		return result;
	}
	catch (const bp::error_already_set&)
	{
	}
	AfterPythonCall(main_module);
}



bp::detail::method_result Call_Override(bp::override &f)
{
	//PyObject *main_module, *globals;
	BeforePythonCall(&main_module, &globals);

	// Execute the python function
	PyLockGIL lock;
	try
	{
		bp::detail::method_result result = f();
		AfterPythonCall(main_module);
		return result;
	}
	catch (const bp::error_already_set&)
	{
	}
	AfterPythonCall(main_module);
}


bp::detail::method_result Call_Override(bp::override &f, int value)
{
	//PyObject *main_module, *globals;
	BeforePythonCall(&main_module, &globals);

	// Execute the python function
	PyLockGIL lock;
	try
	{
		bp::detail::method_result result = f(value);
		AfterPythonCall(main_module);
		return result;
	}
	catch (const bp::error_already_set&)
	{
	}
	AfterPythonCall(main_module);
}

bp::detail::method_result Call_Override(bp::override &f, bool value)
{
	//PyObject *main_module, *globals;
	BeforePythonCall(&main_module, &globals);

	// Execute the python function
	PyLockGIL lock;
	try
	{
		bp::detail::method_result result = f(value);
		AfterPythonCall(main_module);
		return result;
	}
	catch (const bp::error_already_set&)
	{
	}
	AfterPythonCall(main_module);
}

bp::detail::method_result Call_Override(bp::override &f, double value)
{
	//PyObject *main_module, *globals;
	BeforePythonCall(&main_module, &globals);

	// Execute the python function
	PyLockGIL lock;
	try
	{
		bp::detail::method_result result = f(value);
		AfterPythonCall(main_module);
		return result;
	}
	catch (const bp::error_already_set&)
	{
	}
	AfterPythonCall(main_module);

}

bp::detail::method_result Call_Override(bp::override &f, const std::wstring& value)
{
	//PyObject *main_module, *globals;
	BeforePythonCall(&main_module, &globals);

	// Execute the python function
	PyLockGIL lock;
	try
	{
		bp::detail::method_result result = f(value);
		AfterPythonCall(main_module);
		return result;
	}
	catch (const bp::error_already_set&)
	{
	}
	AfterPythonCall(main_module);
}

class ObserverWrap : public Observer, public bp::wrapper<Observer>
{
public:
	ObserverWrap() :Observer(){}

	void OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified)override
	{
		//		if (added && added->size() > 0)
		{
			bp::list added_list;
			bp::list removed_list;
			bp::list modified_list;
			if (added && added->size() > 0)
			{
				BOOST_FOREACH(HeeksObj* o, *added) {
					AddObjectToPythonList(o, added_list);
				}
				if (bp::override f = this->get_override("OnAdded"))Call_Override(f, added_list);
			}
			if (removed && removed->size() > 0)
			{
				BOOST_FOREACH(HeeksObj* o, *removed) {
					AddObjectToPythonList(o, removed_list);
				}
				if (bp::override f = this->get_override("OnRemoved"))Call_Override(f, removed_list);
			}
			if (modified && modified->size() > 0)
			{
				BOOST_FOREACH(HeeksObj* o, *modified) {
					AddObjectToPythonList(o, modified_list);
				}
				if (bp::override f = this->get_override("OnModified"))Call_Override(f, modified_list);
			}
		}
	}

	void WhenMarkedListChanges(bool selection_cleared, const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed)
	{
		bp::list added_list;
		bp::list removed_list;
		if (added && added->size() > 0)
		{
			BOOST_FOREACH(HeeksObj* o, *added) {
				AddObjectToPythonList(o, added_list);
			}
		}
		if (removed && removed->size() > 0)
		{
			BOOST_FOREACH(HeeksObj* o, *removed) {
				AddObjectToPythonList(o, removed_list);
			}
		}
		if (bp::override f = this->get_override("OnSelectionChanged"))Call_Override(f, added_list, removed_list);

	}
};

void CadImport(std::wstring fp)
{
	theApp.OpenFile(fp.c_str(), true);
}

void RegisterObserver(Observer* observer)
{
	theApp.RegisterObserver(observer);
}


std::list<PyObject*>  repaint_callbacks;

void CallPythonCallback(PyObject* callback)
{
	//PyObject *main_module, *globals;
	BeforePythonCall(&main_module, &globals);

	// Execute the python function
	PyObject* result = PyObject_CallFunction(callback, 0);

	AfterPythonCall(main_module);

}

void PythonOnRepaint(bool soon)
{
	for (std::list<PyObject*>::iterator It = repaint_callbacks.begin(); It != repaint_callbacks.end(); It++)
	{
		CallPythonCallback(*It);
	}
}

void RegisterOnRepaint(PyObject *callback)
{
	if (!PyCallable_Check(callback))
	{
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return;
	}
	repaint_callbacks.push_back(callback);
}

PyObject* message_box_callback = NULL;

void PythonOnMessageBox(const wchar_t* message)
{
	static bool in_message_box = false;
	if (in_message_box)
		return;
	in_message_box = true;
	if (message_box_callback)
		CallPythonCallback(message_box_callback);
	in_message_box = false;
}

void RegisterMessageBoxCallback(PyObject *callback)
{
	if (!PyCallable_Check(callback))
	{
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return;
	}
	message_box_callback = callback;
}

void CadMessageBox(std::wstring str)
{
	theApp.MessageBox(str.c_str());
}

class BaseObject : public HeeksObj, public bp::wrapper<HeeksObj>
{
public:
	bool m_uses_display_list;
	int m_display_list;

	BaseObject() :HeeksObj(), m_uses_display_list(false), m_display_list(0){
	}
	bool NeverDelete(){ return true; }
	int GetType()const override
	{
		if (bp::override f = this->get_override("GetType"))
		{
			int t = f();
			return t;
		}
		return HeeksObj::GetType();
	}

	const wchar_t* GetIconFilePath() override
	{
		if (bp::override f = this->get_override("GetIconFilePath"))
		{
			std::string s = f();
			str_for_base_object = Ctt(s.c_str());
			return str_for_base_object.c_str();
		}
		return HeeksObj::GetIconFilePath();
	}

	const wchar_t* GetShortString()const override
	{
		if (bp::override f = this->get_override("GetTitle"))
		{
			std::string s = f();
			str_for_base_object = Ctt(s.c_str());
			return str_for_base_object.c_str();
		}
		return HeeksObj::GetShortString();
	}

	const wchar_t* GetTypeString()const override
	{
		if (bp::override f = this->get_override("GetTypeString"))
		{
			std::string s = f();
			str_for_base_object = Ctt(s.c_str());
			return str_for_base_object.c_str();
		}
		return HeeksObj::GetTypeString();
	}

	const HeeksColor* GetColor()const override
	{
		if (bp::override f = this->get_override("GetColor"))
		{
			color_for_base_object = f();
			return &color_for_base_object;
		}
		return HeeksObj::GetColor();
	}

	void SetColor(const HeeksColor &col) override
	{
		if (bp::override f = this->get_override("SetColor"))
		{
			f(col);
		}
	}

	static bool in_glCommands;
	static bool triangles_begun;
	static bool lines_begun;
	static TiXmlElement* m_cur_element;

	void glCommands(bool select, bool marked, bool no_color) override
	{
		if (in_glCommands)
			return; // shouldn't be needed

		if (!select)
		{
			glEnable(GL_LIGHTING);
			if (!no_color)
			{
				const HeeksColor* c = this->GetColor();
				if (c)Material(*c).glMaterial(1.0);
			}
		}

		bool display_list_started = false;
		bool do_render_commands = true;
		if (m_uses_display_list)
		{
			if (m_display_list)
			{
				glCallList(m_display_list);
				do_render_commands = false;
			}
			else{
				m_display_list = glGenLists(1);
				glNewList(m_display_list, GL_COMPILE_AND_EXECUTE);
				display_list_started = true;
			}
		}

		if (do_render_commands)
		{
			if (bp::override f = this->get_override("OnRenderTriangles"))
			{
				in_glCommands = true;

				Call_Override(f);

				if (triangles_begun)
				{
					glEnd();
					triangles_begun = false;
				}

				if (lines_begun)
				{
					glEnd();
					lines_begun = false;
				}
				in_glCommands = false;
			}
		}

		if (display_list_started)
		{
			glEndList();
		}

		if (!select)glDisable(GL_LIGHTING);

	}

	void GetProperties(std::list<Property *> *list) override
	{
		if (bp::override f = this->get_override("GetProperties"))
		{
			property_list = list;
			object_for_get_properties = this;
			Property* p = Call_Override(f);
		}
		HeeksObj::GetProperties(list);
	}

	void GetBox(CBox &box) override
	{
		if (bp::override f = this->get_override("GetBox"))
		{
			bp::tuple tuple = Call_Override(f);

			if (bp::len(tuple) == 6)
			{
				double xmin = bp::extract<double>(tuple[0]);
				double ymin = bp::extract<double>(tuple[1]);
				double zmin = bp::extract<double>(tuple[2]);
				double xmax = bp::extract<double>(tuple[3]);
				double ymax = bp::extract<double>(tuple[4]);
				double zmax = bp::extract<double>(tuple[5]);
				box.Insert(CBox(xmin, ymin, zmin, xmax, ymax, zmax));
			}
			else
			{
				PyErr_SetString(PyExc_RuntimeError, "GetBox takes exactly 6 parameters!");
			}
		}
	}

	void KillGLLists() override
	{
		if (m_uses_display_list && m_display_list)
		{
			glDeleteLists(m_display_list, 1);
			m_display_list = 0;
		}
	}

	void WriteXML(TiXmlNode *root)override
	{
		BaseObject::m_cur_element = new TiXmlElement(Ttc(this->GetTypeString()));
		root->LinkEndChild(BaseObject::m_cur_element);

		if (bp::override f = this->get_override("WriteXML"))
		{
			Call_Override(f);
		}
		WriteBaseXML(BaseObject::m_cur_element);
	}

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem)
	{
#if 0
		to do
		geoff_geometry::Point3d p;
		HeeksColor c;

		// get the attributes
		for (TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
		{
			std::string name(a->Name());
			if (name == "col"){ c = HeeksColor((long)(a->IntValue())); }
			else if (name == "x"){ p.SetX(a->DoubleValue()); }
			else if (name == "y"){ p.SetY(a->DoubleValue()); }
			else if (name == "z"){ p.SetZ(a->DoubleValue()); }
		}

		HPoint* new_object = new HPoint(p, &c);
		new_object->ReadBaseXML(pElem);

		return new_object;
#else
		return NULL;
#endif
	}
};


// static definitions
bool BaseObject::in_glCommands = false;
bool BaseObject::triangles_begun = false;
bool BaseObject::lines_begun = false;
TiXmlElement* BaseObject::m_cur_element = NULL;


int GetTypeFromHeeksObj(const HeeksObj* object)
{
	switch (object->GetType())
	{
	case SketchType:
		return (int)OBJECT_TYPE_SKETCH;
	case CircleType:
		return (int)OBJECT_TYPE_CIRCLE;
	case StlSolidType:
		return (int)OBJECT_TYPE_SOLID;
	default:
		return object->GetType();
	}
}

int BaseObjectGetType(const HeeksObj& object)
{
	return GetTypeFromHeeksObj(&object);
	//return object.GetType();
}

std::wstring BaseObjectGetIconFilePath(BaseObject& object)
{
	return std::wstring(object.GetIconFilePath());
}

std::wstring GetTitleFromHeeksObj(const HeeksObj* object)
{
	const wchar_t* s = object->GetShortString();
	if (s == NULL)return L"";
	return std::wstring(s);
}

std::wstring BaseObjectGetTitle(const HeeksObj& object)
{
	return GetTitleFromHeeksObj(&object);
	//return object.GetShortString();
}

unsigned int BaseObjectGetID(BaseObject& object)
{
	return object.GetID();
}

void BaseObjectSetUsesGLList(BaseObject& object, bool on)
{
	object.m_uses_display_list = on;
}

HeeksColor BaseObjectGetColor(const BaseObject& object)
{
	return *(object.GetColor());
}

int PropertyGetInt(Property& property)
{
	return property.GetInt();
}


int HeeksObjGetType(const HeeksObj& object)
{
	return GetTypeFromHeeksObj(&object);
	//return object.GetType();
}

std::wstring HeeksObjGetTitle(const HeeksObj& object)
{
	return GetTitleFromHeeksObj(&object);
	//return object.GetShortString();
}

std::wstring HeeksObjGetIconFilePath(HeeksObj& object)
{
	return std::wstring(object.GetIconFilePath());
}

boost::python::list HeeksObjGetProperties(HeeksObj& object) {
	boost::python::list return_list;
	std::list<Property*> p_list;
	object.GetProperties(&p_list);
	for (std::list<Property*>::iterator It = p_list.begin(); It != p_list.end(); It++)
	{
		Property* property = *It;
		//AddPropertyToPythonList(property, return_list);
		return_list.append(boost::python::ptr<Property*>(property));
	}
	return return_list;
}

std::wstring PropertyGetShortString(Property& p)
{
	return std::wstring(p.GetShortString());
}

std::wstring PropertyGetString(Property& p)
{
	return std::wstring(p.GetString());
}

HeeksColor PropertyGetColor(const Property& p)
{
	return p.GetColor();
}

HeeksColor HeeksObjGetColor(const HeeksObj& object)
{
	return *(object.GetColor());
}


void DrawTriangle(double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7, double x8)
{
	if (!BaseObject::triangles_begun)
	{
		if (BaseObject::lines_begun)
		{
			glEnd();
			BaseObject::lines_begun = false;
		}
		glBegin(GL_TRIANGLES);
		BaseObject::triangles_begun = true;
	}

	geoff_geometry::Point3d p0(x0, x1, x2);
	geoff_geometry::Point3d p1(x3, x4, x5);
	geoff_geometry::Point3d p2(x6, x7, x8);
	geoff_geometry::Point3d v1(p0, p1);
	geoff_geometry::Point3d v2(p0, p2);
	try
	{
		geoff_geometry::Point3d norm = (v1 ^ v2).Normalized();
		glNormal3d(norm.x, norm.y, norm.z);
	}
	catch (...)
	{
	}
	glVertex3d(x0, x1, x2);
	glVertex3d(x3, x4, x5);
	glVertex3d(x6, x7, x8);

}

void DrawLine(double x0, double x1, double x2, double x3, double x4, double x5)
{
	if (!BaseObject::lines_begun)
	{
		if (BaseObject::triangles_begun)
		{
			glEnd();
			BaseObject::triangles_begun = false;
		}
		glBegin(GL_LINES);
		BaseObject::lines_begun = true;
	}
	glVertex3d(x0, x1, x2);
	glVertex3d(x3, x4, x5);
}

void AddProperty(Property* property)
{
	property_list->push_back(property);
}

std::wstring GetFileFullPath()
{
	const wchar_t* fp = theApp.GetFileFullPath();
	if (fp)return (fp);
	else return L"";
}

bp::object GetObjectFromId(int type, int id) {
	// to do
	// this returns a list with the object in, because that works
	// but it should just return an object
	boost::python::list olist;
	HeeksObj* object = theApp.GetIDObject(ObjectTypeToHeeksType(type), id);
	if (object != NULL)
	{
		AddObjectToPythonList(object, olist);
		if (bp::len(olist) > 0)
		{
			return olist[0];
		}
	}

	return boost::python::object(); // None
}

static std::map<std::string, PyObject*> xml_read_callbacks;

HeeksObj* ReadPyObjectFromXMLElement(TiXmlElement* pElem);

HeeksObj* ReadPyObjectFromXMLElementWithName(const std::string& name, TiXmlElement* pElem)
{
	std::map< std::string, PyObject* >::iterator FindIt = xml_read_callbacks.find(name);
	HeeksObj* object = NULL;
	if (FindIt != xml_read_callbacks.end())
	{
		PyObject* python_callback = FindIt->second;

		BaseObject::m_cur_element = pElem;

		//PyObject *main_module, *globals;
		BeforePythonCall(&main_module, &globals);

		// Execute the python function
		PyObject* result = PyObject_CallFunction(python_callback, 0);
		if (result)
		{
			object = bp::extract<HeeksObj*>(result);
			object->ReadBaseXML(pElem);
		}

		AfterPythonCall(main_module);
	}
	else
	{
		object = HXml::ReadFromXMLElement(pElem);
	}

	return object;
}

void RegisterXMLRead(std::wstring name, PyObject *callback)
{
	if (!PyCallable_Check(callback))
	{
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return;
	}

	const char* name_c = Ttc(name.c_str());

	// add an entry in map from name to 
	xml_read_callbacks.insert(std::make_pair(name_c, callback));

	// tell HeeksCAD that it's a python object callback by registering it's dummy callback.
	// HeeksCAD will spot this and call ReadPyObjectFromXMLElementWithName
	theApp.RegisterReadXMLfunction(name_c, ReadPyObjectFromXMLElement);
}

void SetXmlValue(const std::wstring &name, const std::wstring &value)
{
	std::string svalue(Ttc(value.c_str()));
	BaseObject::m_cur_element->SetAttribute(Ttc(name.c_str()), svalue.c_str());
}

std::wstring GetXmlValue(const std::wstring &name)
{
	if (BaseObject::m_cur_element != NULL)
	{
		const char* value = BaseObject::m_cur_element->Attribute(Ttc(name.c_str()));
		if (value != NULL)
			return std::wstring(Ctt(value));
	}
	return L"";
}

class PropertyWrap : public Property, public bp::wrapper<Property>
{
	int m_type;
public:
	PropertyWrap() :Property(), m_type(InvalidPropertyType){}
	PropertyWrap(int type, const std::wstring& title, HeeksObj* object) :Property(object, title.c_str()), m_type(type){ m_editable = true; }
	int get_property_type()const{ return m_type; }
	int GetInt()const override
	{
		if (bp::override f = this->get_override("GetInt"))return Call_Override(f);
		return Property::GetInt();
	}
	bool GetBool()const override
	{
		if (bp::override f = this->get_override("GetBool"))return Call_Override(f);
		return Property::GetBool();
	}
	double GetDouble()const override
	{
		if (bp::override f = this->get_override("GetFloat"))return Call_Override(f);
		return Property::GetDouble();
	}
	const wchar_t* GetString()const override
	{
		if (bp::override f = this->get_override("GetStr"))return Call_Override(f);
		return Property::GetString();
	}
	void Set(bool value)override
	{
		if (bp::override f = this->get_override("SetBool"))Call_Override(f, value);

	}
	void Set(int value)override
	{
		if (bp::override f = this->get_override("SetInt"))Call_Override(f, value);
	}
	void Set(double value)override
	{
		if (bp::override f = this->get_override("SetFloat"))Call_Override(f, value);
	}
	void Set(const wchar_t* value)override
	{
		if (bp::override f = this->get_override("SetStr"))Call_Override(f, value);
	}
	Property *MakeACopy(void)const{ return new PropertyWrap(*this); }
};


int PropertyWrapGetInt(PropertyWrap& property)
{
	return property.GetInt();
}



void StlSolidWriteSTL(CStlSolid& solid, double tolerance, std::wstring filepath)
{
	std::list<HeeksObj*> list;
	list.push_back(&solid);
	theApp.SaveSTLFileAscii(list, filepath.c_str(), tolerance);
}

static boost::shared_ptr<CStlSolid> initStlSolid(const std::wstring& title, const HeeksColor* color)
{
	return boost::shared_ptr<CStlSolid>(new CStlSolid(title.c_str(), color));
}

boost::python::list GetSelectedObjects() {
	boost::python::list slist;
	for (std::list<HeeksObj *>::iterator It = theApp.m_marked_list->list().begin(); It != theApp.m_marked_list->list().end(); It++)
	{
		HeeksObj* object = *It;
		AddObjectToPythonList(object, slist);
	}
	return slist;
}

boost::python::list GetObjects() {
	boost::python::list olist;
	for (HeeksObj *object = theApp.GetFirstChild(); object; object = theApp.GetNextChild())
	{
		AddObjectToPythonList(object, olist);
	}
	return olist;
}

bool ObjectMarked(HeeksObj* object)
{
	return theApp.m_marked_list->ObjectMarked(object);
}

void Select(HeeksObj* object, bool call_OnChanged)
{
	theApp.m_marked_list->Add(object, call_OnChanged);
}

void Unselect(HeeksObj* object, bool call_OnChanged)
{
	theApp.m_marked_list->Remove(object, call_OnChanged);
}

void ClearSelection(bool call_OnChanged)
{
	theApp.m_marked_list->Clear(call_OnChanged);
}

double GetViewUnits()
{
	return theApp.m_view_units;
}

void SetViewUnits(double units)
{
	theApp.m_view_units = units;
}

void StartHistory()
{
	theApp.StartHistory();
}

void EndHistory()
{
	theApp.EndHistory();
}

void RollBack()
{
	theApp.RollBack();
}

void RollForward()
{
	theApp.RollForward();
}

void DeleteUndoably(HeeksObj *object)
{
	theApp.DeleteUndoably(object);
}

void AddUndoably(HeeksObj *object, HeeksObj* owner, HeeksObj* prev_object)
{
	theApp.AddUndoably(object, owner, prev_object);
}

void DoUndoable(Undoable* undoable)
{
	theApp.DoUndoable(undoable);
}

bool ShiftSelect(HeeksObj *object, bool control_down)
{
	// mark a list of siblings

	bool waiting_until_left_up = false;
	HeeksObj* parent = object->m_owner;
	std::set<HeeksObj*> sibling_set;
	std::list<HeeksObj*> sibling_list;
	for (HeeksObj* sibling = parent->GetFirstChild(); sibling; sibling = parent->GetNextChild())
	{
		sibling_set.insert(sibling);
		sibling_list.push_back(sibling);
	}
	// find most recently marked sibling
	std::list<HeeksObj*> &marked = theApp.m_marked_list->list();
	HeeksObj* recently_marked_sibling = NULL;
	bool recent_first = false;
	for (std::list<HeeksObj*>::reverse_iterator It = marked.rbegin(); It != marked.rend(); It++)
	{
		if (*It == object)recent_first = true;
		if (sibling_set.find(*It) != sibling_set.end())
		{
			recently_marked_sibling = *It;
			break;
		}
	}

	if (recently_marked_sibling)
	{
		if (!control_down)
		{
			theApp.m_marked_list->Clear(false);
		}

		bool marking = false;
		std::list<HeeksObj*> list_to_mark;
		bool finish_marking = false;
		for (std::list<HeeksObj*>::iterator It = sibling_list.begin(); !finish_marking && It != sibling_list.end(); It++)
		{
			HeeksObj* sibling = *It;
			if (sibling == object || sibling == recently_marked_sibling)
			{
				if (marking)finish_marking = true;
				else marking = true;
			}

			if (marking)
			{
				list_to_mark.push_back(sibling);
			}
		}

		theApp.m_marked_list->Add(list_to_mark, true);
	}
	else
	{
		if (control_down)
		{
			if (theApp.m_marked_list->ObjectMarked(object))
			{
				theApp.m_marked_list->Remove(object, true);
			}
			else{
				theApp.m_marked_list->Add(object, true);
			}
		}
		else
		{
			if (theApp.m_marked_list->ObjectMarked(object))
			{
				waiting_until_left_up = true;
			}
			else
			{
				theApp.m_marked_list->Clear(false);
				theApp.m_marked_list->Add(object, true);
			}
		}
	}

	return waiting_until_left_up;
}

void ChangePropertyString(const wstring& value, Property* property)
{
	theApp.DoUndoable(new PropertyChangeString(value, property));
}

void ChangePropertyDouble(const double& value, Property* property)
{
	theApp.DoUndoable(new PropertyChangeDouble(value, property));
}

void ChangePropertyLength(const double& value, Property* property)
{
	theApp.DoUndoable(new PropertyChangeLength(value, property));
}

void ChangePropertyInt(const int& value, Property* property)
{
	theApp.DoUndoable(new PropertyChangeInt(value, property));
}

void ChangePropertyChoice(const int& value, Property* property)
{
	theApp.DoUndoable(new PropertyChangeChoice(value, property));
}

void ChangePropertyColor(const HeeksColor& value, Property* property)
{
	theApp.DoUndoable(new PropertyChangeColor(value, property));
}

void ChangePropertyCheck(const bool& value, Property* property)
{
	theApp.DoUndoable(new PropertyChangeCheck(value, property));
}

CApp* GetApp()
{
	return &theApp;
}

HeeksObj* GetFirstChild()
{
	return theApp.GetFirstChild();
}

HeeksObj* GetNextChild()
{
	return theApp.GetNextChild();
}

HeeksObj* ObjectGetOwner(HeeksObj* object)
{
	return object->m_owner;
}


	BOOST_PYTHON_MODULE(cad) {
		bp::class_<BaseObject, boost::noncopyable >("BaseObject")
			.def("GetType", &BaseObjectGetType)
			.def("GetIconFilePath", &BaseObjectGetIconFilePath)
			.def("GetTitle", &BaseObjectGetTitle)
			.def("GetID", &BaseObjectGetID)
			.def("KillGLLists", &BaseObject::KillGLLists)
			.def("SetUsesGLList", &BaseObjectSetUsesGLList)
			.def("GetColor", &BaseObjectGetColor)
			.def("AutoExpand", &BaseObject::AutoExpand)
			.def("GetNumChildren", &BaseObject::GetNumChildren)
			;

		bp::class_<HeeksObj, boost::noncopyable>("Object")
			.def(bp::init<HeeksObj>())
			.def("GetType", &HeeksObjGetType)
			.def("GetIconFilePath", &HeeksObjGetIconFilePath)
			.def("GetID", &HeeksObj::GetID)
			.def("GetIndex", &HeeksObj::GetIndex)
			.def("GetColor", &HeeksObjGetColor)
			.def("GetTitle", &HeeksObjGetTitle)
			.def("AutoExpand", &HeeksObj::AutoExpand)
			.def("GetNumChildren", &HeeksObj::GetNumChildren)
			.def("GetOwner", &ObjectGetOwner, bp::return_value_policy<bp::reference_existing_object>())
			.def("GetFirstChild", &HeeksObj::GetFirstChild, bp::return_value_policy<bp::reference_existing_object>())
			.def("GetNextChild", &HeeksObj::GetNextChild, bp::return_value_policy<bp::reference_existing_object>())
			.def("CanAdd", &HeeksObj::CanAdd)
			.def("CanAddTo", &HeeksObj::CanAddTo)
			.def("OneOfAKind", &HeeksObj::OneOfAKind)
			.def("CopyFrom", &HeeksObj::CopyFrom)
			.def("GetProperties", &HeeksObjGetProperties)
			;

		bp::class_<HeeksColor>("Color")
			.def(bp::init<HeeksColor>())
			.def(bp::init<unsigned char, unsigned char, unsigned char>())
			.def(bp::init<long>())
			.def_readwrite("red", &HeeksColor::red)
			.def_readwrite("green", &HeeksColor::green)
			.def_readwrite("blue", &HeeksColor::blue)
			.def("ref", &HeeksColor::COLORREF_color)
			;

		bp::class_<PropertyWrap, boost::noncopyable >("PropertyBase")
			.def(bp::init<int, std::wstring, HeeksObj*>())
			;

		//boost::python::register_ptr_to_python<boost::shared_ptr<Property> >();
		//boost::python::register_ptr_to_python<boost::shared_ptr<PropertyObjectTitle> >();

		bp::class_<Property, boost::noncopyable>("Property")
			.def(bp::init<Property>())
			.def("GetType", &Property::get_property_type)
			.def("GetTitle", &PropertyGetShortString)
			.def("GetString", &PropertyGetString)
			.def("GetDouble", &Property::GetDouble)
			.def("GetInt", &Property::GetInt)
			.def("GetBool", &Property::GetBool)
			.def("GetColor", &PropertyGetColor)
			.def_readwrite("editable", &PropertyWrap::m_editable)
			.def_readwrite("object", &PropertyWrap::m_object)
			;

		bp::class_<PropertyCheck, boost::noncopyable, bp::bases<Property>>("PropertyCheck", boost::python::no_init);
		bp::class_<PropertyCheckWithConfig, bp::bases<PropertyCheck>>("PropertyCheckWithConfig", boost::python::no_init);
		bp::class_<PropertyChoice, bp::bases<Property>>("PropertyChoice", boost::python::no_init);
		bp::class_<PropertyColor, bp::bases<Property>>("PropertyColor", boost::python::no_init);
		bp::class_<PropertyDouble, bp::bases<Property>>("PropertyDouble", boost::python::no_init);
		bp::class_<PropertyDoubleScaled, bp::bases<Property>>("PropertyDoubleScaled", boost::python::no_init);
		bp::class_<PropertyLengthScaled, bp::bases<PropertyDoubleScaled>>("PropertyLengthScaled", boost::python::no_init);
		bp::class_<PropertyDoubleLimited, bp::bases<PropertyDouble>>("PropertyDoubleLimited", boost::python::no_init);
		bp::class_<PropertyString, bp::bases<Property>>("PropertyString", boost::python::no_init);
		bp::class_<PropertyStringReadOnly, bp::bases<Property>>("PropertyStringReadOnly", boost::python::no_init);
		bp::class_<PropertyStringWithConfig, bp::bases<PropertyString>>("PropertyStringWithConfig", boost::python::no_init);
		bp::class_<PropertyFile, bp::bases<PropertyString>>("PropertyFile", boost::python::no_init);
		bp::class_<PropertyInt, bp::bases<Property>>("PropertyInt", boost::python::no_init);
		bp::class_<PropertyLength, bp::bases<PropertyDouble>>("PropertyLength", boost::python::no_init);
		bp::class_<PropertyLengthWithConfig, bp::bases<PropertyLength>>("PropertyLengthWithConfig", boost::python::no_init);
		bp::class_<PropertyLengthWithKillGLLists, bp::bases<PropertyLength>>("PropertyLengthWithKillGLLists", boost::python::no_init);
		bp::class_<PropertyList, bp::bases<Property>>("PropertyList", boost::python::no_init);
		bp::class_<PropertyObjectTitle, bp::bases<Property>>("PropertyObjectTitle", boost::python::no_init);
		bp::class_<PropertyObjectColor, bp::bases<Property>>("PropertyObjectColor", boost::python::no_init);

		bp::class_<Undoable, boost::noncopyable>("Undoable", boost::python::no_init);
		bp::class_<PropertyChangeString, bp::bases<Undoable>>("PropertyChangeString", boost::python::no_init).def(bp::init<const std::wstring&, Property*>());
		bp::class_<PropertyChangeDouble, bp::bases<Undoable>>("PropertyChangeDouble", boost::python::no_init).def(bp::init<const double&, Property*>());
		bp::class_<PropertyChangeLength, bp::bases<Undoable>>("PropertyChangeLength", boost::python::no_init).def(bp::init<const double&, Property*>());
		bp::class_<PropertyChangeInt, bp::bases<Undoable>>("PropertyChangeInt", boost::python::no_init).def(bp::init<const int&, Property*>());
		bp::class_<PropertyChangeColor, bp::bases<Undoable>>("PropertyChangeColor", boost::python::no_init).def(bp::init<const HeeksColor&, Property*>());
		bp::class_<PropertyChangeChoice, bp::bases<Undoable>>("PropertyChangeChoice", boost::python::no_init).def(bp::init<const int&, Property*>());
		bp::class_<PropertyChangeCheck, bp::bases<Undoable>>("PropertyChangeCheck", boost::python::no_init).def(bp::init<const bool&, Property*>());

		bp::class_<CStlSolid, bp::bases<HeeksObj>>("StlSolid")
			.def(bp::init<CStlSolid>())
			.def("__init__", bp::make_constructor(&initStlSolid))
			.def(bp::init<const std::wstring&>())// load a stl solid from a filepath
			.def("WriteSTL", &StlSolidWriteSTL) ///function WriteSTL///params float tolerance, string filepath///writes an STL file for the body to the given tolerance
			;

		bp::class_<ObjList, bp::bases<HeeksObj>>("ObjList")
			.def(bp::init<ObjList>())
			.def("Clear", &ObjList::ClearUndoably)
			;

		bp::class_<CApp, bp::bases<ObjList>>("App")
			.def(bp::init<CApp>())
			;

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

		bp::class_<ObserverWrap, boost::noncopyable >("Observer")
			.def(bp::init<ObserverWrap>())
			;

		bp::def("OnInit", OnInit);
		bp::def("OnExit", OnExit);
		bp::def("Import", CadImport);
		bp::def("RegisterObserver", RegisterObserver);
		bp::def("RegisterOnRepaint", RegisterOnRepaint);
		bp::def("RegisterMessageBoxCallback", RegisterMessageBoxCallback);
		bp::def("MessageBox", CadMessageBox);
		bp::def("GetSelectedObjects", GetSelectedObjects);
		bp::def("GetObjects", GetObjects);
		bp::def("ObjectMarked", ObjectMarked);
		bp::def("Select", Select);
		bp::def("Unselect", Unselect);
		bp::def("ClearSelection", ClearSelection);
		bp::def("GetViewUnits", GetViewUnits);
		bp::def("SetViewUnits", SetViewUnits);
		bp::def("GetApp", GetApp, bp::return_value_policy<bp::reference_existing_object>());
		bp::def("StartHistory", StartHistory);
		bp::def("EndHistory", EndHistory);
		bp::def("RollBack", RollBack);
		bp::def("RollForward", RollForward);
		bp::def("DeleteUndoably", DeleteUndoably);
		bp::def("AddUndoably", AddUndoably);
		bp::def("DoUndoable", DoUndoable);
		bp::def("ShiftSelect", ShiftSelect);
		bp::def("ChangePropertyString", ChangePropertyString);
		bp::def("ChangePropertyDouble", ChangePropertyDouble);
		bp::def("ChangePropertyLength", ChangePropertyLength);
		bp::def("ChangePropertyInt", ChangePropertyInt);
		bp::def("ChangePropertyChoice", ChangePropertyChoice);
		bp::def("ChangePropertyColor", ChangePropertyColor);
		bp::def("ChangePropertyCheck", ChangePropertyCheck);

		bp::scope().attr("PROPERTY_TYPE_INVALID") = (int)InvalidPropertyType;
		bp::scope().attr("PROPERTY_TYPE_STRING") = (int)StringPropertyType;
		bp::scope().attr("PROPERTY_TYPE_DOUBLE") = (int)DoublePropertyType;
		bp::scope().attr("PROPERTY_TYPE_LENGTH") = (int)LengthPropertyType;
		bp::scope().attr("PROPERTY_TYPE_INT") = (int)IntPropertyType;
		bp::scope().attr("PROPERTY_TYPE_CHOICE") = (int)ChoicePropertyType;
		bp::scope().attr("PROPERTY_TYPE_COLOR") = (int)ColorPropertyType;
		bp::scope().attr("PROPERTY_TYPE_CHECK") = (int)CheckPropertyType;
		bp::scope().attr("PROPERTY_TYPE_LIST") = (int)ListOfPropertyType;
		bp::scope().attr("PROPERTY_TYPE_FILE") = (int)FilePropertyType;

	}
