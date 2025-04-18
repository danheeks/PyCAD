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

#include "../Geom/geometry.h"
#include "../Geom/Tris.h"
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
#include "Sketch.h"
#include "HLine.h"
#include "HArc.h"
#include "HPoint.h"
#include "HText.h"
#include "HCircle.h"
#include "HILine.h"
#include "InputMode.h"
#include "MagDragWindow.h"
#include "ViewRotating.h"
#include "ViewZooming.h"
#include "ViewPanning.h"
#include "ConversionTools.h"
#include "PyWrapper.h"
#include "PyBaseObject.h"
#include "DigitizeMode.h"
#include "KeyCode.h"
#include "Gripper.h"
#include "GripperSelTransform.h"
#include "CoordinateSystem.h"
#include "Filter.h"

void OnExit()
{
	theApp->OnExit();
}

void AddPropertyToPythonList(Property* p, boost::python::list& list)
{
	if (PropertyCheck* o = dynamic_cast<PropertyCheck*>(p)){ list.append(o); return;	}
	if (PropertyChoice* o = dynamic_cast<PropertyChoice*>(p)){ list.append(o); return; }
	if (PropertyColor* o = dynamic_cast<PropertyColor*>(p)){ list.append(o); return; }
	if (PropertyDoubleScaled* o = dynamic_cast<PropertyDoubleScaled*>(p)){ list.append(o); return; }
	if (PropertyDoubleLimited* o = dynamic_cast<PropertyDoubleLimited*>(p)){ list.append(o); return; }
	if (PropertyString* o = dynamic_cast<PropertyString*>(p)){ list.append(o); return; }
	if (PropertyStringReadOnly* o = dynamic_cast<PropertyStringReadOnly*>(p)){ list.append(o); return; }
	if (PropertyFile* o = dynamic_cast<PropertyFile*>(p)){ list.append(o); return; }
	if (PropertyInt* o = dynamic_cast<PropertyInt*>(p)){ list.append(o); return; }
	if (PropertyLength* o = dynamic_cast<PropertyLength*>(p)){ list.append(o); return; }
	if (PropertyLengthWithKillGLLists* o = dynamic_cast<PropertyLengthWithKillGLLists*>(p)){ list.append(o); return; }
	if (PropertyList* o = dynamic_cast<PropertyList*>(p)){ list.append(o); return; }
	if (PropertyObjectTitle* o = dynamic_cast<PropertyObjectTitle*>(p)){ list.append(o); return; }
	if (PropertyObjectColor* o = dynamic_cast<PropertyObjectColor*>(p)){ list.append(o); return; }
}

void AddObjectToPythonList(HeeksObj* object, boost::python::list& list)
{
	switch (object->GetType())
	{
	case SketchType:
		list.append(boost::python::pointer_wrapper<CSketch*>((CSketch*)object));
		break;
	case StlSolidType:
		list.append(boost::python::pointer_wrapper<CStlSolid*>((CStlSolid*)object));
		break;
	case CircleType:
		list.append(boost::python::pointer_wrapper<HCircle*>((HCircle*)object));
		break;
	default:
		list.append(boost::python::pointer_wrapper<HeeksObj*>((HeeksObj*)object));
		break;
	}
}

class ObserverWrap : public Observer, public cad_wrapper<Observer>
{
public:
	ObserverWrap() :Observer(){}

	void OnChanged(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified)override
	{
		//		if (added && added->size() > 0)
		{
			boost::python::list added_list;
			boost::python::list removed_list;
			boost::python::list modified_list;
			if (added && added->size() > 0)
			{
				BOOST_FOREACH(HeeksObj* o, *added) {
					AddObjectToPythonList(o, added_list);
				}
				CallVoidReturn("OnAdded", added_list);
			}
			if (removed && removed->size() > 0)
			{
				BOOST_FOREACH(HeeksObj* o, *removed) {
					AddObjectToPythonList(o, removed_list);
				}
				CallVoidReturn("OnRemoved", removed_list);
			}
			if (modified && modified->size() > 0)
			{
				BOOST_FOREACH(HeeksObj* o, *modified) {
					AddObjectToPythonList(o, modified_list);
				}
				CallVoidReturn("OnModified", modified_list);
			}
		}
	}

	void WhenMarkedListChanges(bool selection_cleared, const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed)
	{
		boost::python::list added_list;
		boost::python::list removed_list;
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
		CallVoidReturn("OnSelectionChanged", added_list, removed_list);

	}

	void Clear()
	{
		CallVoidReturn("OnClear");
	}
};

std::list<Property *> *property_list = NULL;

#if 0
I am porting CInputMode to python.
This will be removed

static std::wstring str_for_input_mode;

class InputModeWrap : public CInputMode, public cad_wrapper<CInputMode>
{
public:
	InputModeWrap() :CInputMode(){}

	const wchar_t* GetTitle()override
	{
		if (boost::python::override f = this->get_override("GetTitle"))
		{
			std::string s = f();
			str_for_input_mode = Ctt(s.c_str());
			return str_for_input_mode.c_str();
		}
		return NULL;
	}

	const wchar_t* GetHelpText()override
	{
		if (boost::python::override f = this->get_override("GetHelpText"))
		{
			std::string s = f();
			str_for_input_mode = Ctt(s.c_str());
			return str_for_input_mode.c_str();
		}
		return NULL;
	}

	bool OnKeyDown(KeyCode key_code)override
	{
		if (boost::python::override f = this->get_override("OnKeyDown"))
		{
			return f(key_code);
		}
		else
			return CInputMode::OnKeyDown(key_code);
	}

	bool OnKeyUp(KeyCode key_code)override
	{
		if (boost::python::override f = this->get_override("OnKeyUp"))
		{
			return f(key_code);
		}
		else
			return CInputMode::OnKeyUp(key_code);
	}

	void GetProperties(std::list<Property *> *list)
	{
		property_list = list;
		CallVoidReturn("GetProperties");
	}

	void OnMouse(MouseEvent& event)
	{
		CallVoidReturn("OnMouse", event);
	}

	void OnModeChange(void)
	{
		CallVoidReturn("OnModeChange");
	}

	void OnFrontRender()
	{
		CallVoidReturn("OnFrontRender");
	}

	void OnRender()
	{
		CallVoidReturn("OnRender");
	}

};

#endif



void CadReset()
{
	theApp->Reset();
}

bool CadOpenFile(std::wstring fp)
{
	return theApp->OpenFile(fp.c_str(), false);
}

bool CadImport(const std::wstring &filepath, HeeksObj* paste_into = NULL)
{
	bool result = theApp->OpenFile(filepath.c_str(), true, paste_into);
	return result;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(CadImportOverloads, CadImport, 1, 2)

BOOST_PYTHON_FUNCTION_OVERLOADS(RegisterObjectTypeOverloads, RegisterObjectType, 2, 3)

bool CadSaveFile(std::wstring fp)
{
	return theApp->SaveFile(fp.c_str());
}

bool SaveObjects(std::wstring fp, boost::python::list &list)
{
	std::list<HeeksObj*> o_list;
	for (int i = 0; i < len(list); ++i)
	{
		o_list.push_back(boost::python::extract<HeeksObj*>(list[i]));
	}
	return theApp->SaveFile(fp.c_str(), &o_list);
}

void RegisterObserver(Observer* observer)
{
	theApp->RegisterObserver(observer);
}


std::list<PyObject*>  repaint_callbacks;

void PythonOnRepaint(bool soon = false)
{
	for (std::list<PyObject*>::iterator It = repaint_callbacks.begin(); It != repaint_callbacks.end(); It++)
	{
		CallPythonCallback(*It);
	}
}

BOOST_PYTHON_FUNCTION_OVERLOADS(PythonOnRepaintOverloads, PythonOnRepaint, 0, 1)

void RegisterOnRepaint(PyObject *callback)
{
	if (!PyCallable_Check(callback))
	{
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return;
	}
	repaint_callbacks.push_back(callback);
}



std::list<PyObject*>  on_gl_commands_callbacks;

void PythonOnGLCommands()
{
	for (std::list<PyObject*>::iterator It = on_gl_commands_callbacks.begin(); It != on_gl_commands_callbacks.end(); It++)
	{
		CallPythonCallback(*It);
	}
}

void RegisterOnGLCommands(PyObject *callback)
{
	if (!PyCallable_Check(callback))
	{
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return;
	}
	on_gl_commands_callbacks.push_back(callback);
}



std::list<PyObject*>  end_xml_write_callbacks;

void PythonOnEndXmlWrite()
{
	for (std::list<PyObject*>::iterator It = end_xml_write_callbacks.begin(); It != end_xml_write_callbacks.end(); It++)
	{
		CallPythonCallback(*It);
	}
}

void RegisterOnEndXmlWrite(PyObject *callback)
{
	if (!PyCallable_Check(callback))
	{
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return;
	}
	end_xml_write_callbacks.push_back(callback);
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

std::map<std::wstring, PyObject*>  import_callbacks;
std::map<std::wstring, PyObject*>  export_callbacks;
std::wstring filepath_for_FileImportExport;

bool PythonImportFile(const wchar_t* lowercase_extension, const wchar_t* filepath)
{
	std::map<std::wstring, PyObject*>::iterator FindIt = import_callbacks.find(lowercase_extension);
	if (FindIt != import_callbacks.end())
	{
		filepath_for_FileImportExport.assign(filepath);
		CallPythonCallback(FindIt->second);
		return true; // handled
	}
	return false;
}

bool PythonExportFile(const wchar_t* lowercase_extension, const wchar_t* filepath)
{
	std::map<std::wstring, PyObject*>::iterator FindIt = export_callbacks.find(lowercase_extension);
	if (FindIt != export_callbacks.end())
	{
		filepath_for_FileImportExport.assign(filepath);
		CallPythonCallback(FindIt->second);
		return true; // handled
	}
	return false;
}

void RegisterImportFileType(const std::wstring& extension, PyObject *callback)
{
	if (!PyCallable_Check(callback))
	{
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return;
	}
	import_callbacks.insert(std::pair<std::wstring, PyObject*>(extension, callback));
}

void RegisterExportFileType(const std::wstring& extension, PyObject *callback)
{
	if (!PyCallable_Check(callback))
	{
		PyErr_SetString(PyExc_TypeError, "parameter must be callable");
		return;
	}
	export_callbacks.insert(std::pair<std::wstring, PyObject*>(extension, callback));
}

std::wstring GetFilePathForImportExport()
{
	return filepath_for_FileImportExport;
}

std::wstring GetResFolder()
{
	return theApp->m_res_folder;
}

void SetResFolder(std::wstring str)
{
	theApp->m_res_folder = str;
}

void CadMessageBox(std::wstring str)
{
	theApp->DoMessageBox(str.c_str());
}

std::wstring BaseObjectGetIconFilePath(BaseObject& object)
{
	return std::wstring(object.ObjList::GetIconFilePath());
}

std::wstring GetTitleFromHeeksObj(const HeeksObj* object)
{
	const wchar_t* s = object->GetShortStringOrTypeString();
	if (s == NULL)return L"";
	return std::wstring(s);
}

std::wstring BaseObjectGetTitle(const BaseObject& object)
{
	return object.ObjList::GetShortString();
}

HeeksColor BaseObjectGetColor(const BaseObject& object)
{
	return *(object.ObjList::GetColor());
}

int PropertyGetInt(Property& property)
{
	return property.GetInt();
}

CBox HeeksObjGetBox(HeeksObj& object)
{
    CBox box;
    object.GetBox(box);
    return box;
}

std::wstring HeeksObjGetTitle(const HeeksObj& object)
{
	return GetTitleFromHeeksObj(&object);
	//return object.GetShortString();
}

std::wstring HeeksObjGetTypeString(HeeksObj& object)
{
	return std::wstring(object.GetTypeString());
}

std::wstring HeeksObjGetIconFilePath(HeeksObj& object)
{
	return std::wstring(object.GetIconFilePath());
}

void HeeksObjReadFromXML(HeeksObj& object)
{
	object.ReadFromXML(theApp->m_cur_xml_element);
}

void HeeksObjReadObjectXml(HeeksObj& object)
{
	object.HeeksObj::ReadFromXML(theApp->m_cur_xml_element);
}

void HeeksObjWriteToXML(HeeksObj& object)
{
	object.WriteToXML(theApp->m_cur_xml_element);
}

void HeeksObjWriteObjectToXML(HeeksObj& object)
{
	object.HeeksObj::WriteToXML(theApp->m_cur_xml_element);
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

boost::python::list HeeksObjGetBaseProperties(BaseObject& object) {
	boost::python::list return_list;
	std::list<Property*> p_list;
	object.ObjList::GetProperties(&p_list);
	for (std::list<Property*>::iterator It = p_list.begin(); It != p_list.end(); It++)
	{
		Property* property = *It;
		//AddPropertyToPythonList(property, return_list);
		return_list.append(boost::python::ptr<Property*>(property));
	}
	return return_list;
}

boost::python::list BaseObjectGetChildren(BaseObject& object) {
	boost::python::list return_list;
	HeeksObj* obj = object.GetFirstChild();
	while (obj)
	{
		return_list.append(boost::python::ptr<HeeksObj*>(obj));
		obj = object.GetNextChild();
	}

	return return_list;
}


static double GetLines_pixels_per_mm = 0.0;
static boost::python::list return_list_ForGetLines;


static void CallbackForGetLines(const double *p, bool start)
{
	return_list_ForGetLines.append(boost::python::make_tuple(start, p[0], p[1], p[2]));
}

void SetGetLinesPixelsPerMm(double pixels_per_mm)
{
	GetLines_pixels_per_mm = pixels_per_mm;
}

HeeksObj* NewPoint(const Point3d& p)
{
	HPoint* point = new HPoint(p, &theApp->current_color);
	return point;
}

HeeksObj* NewLine(const Point3d& p0, const Point3d& p1)
{
	return new HLine(p0, p1, &theApp->current_color);
}

HeeksObj* NewArc(const Point3d &a, const Point3d &b, const Point3d &axis, const Point3d &c)
{
	return new HArc(a, b, axis, c, &theApp->current_color);
}

HeeksObj* NewCircle(const Point3d &c, const Point3d& axis, double radius)
{
	return new HCircle(c, axis, radius, &theApp->current_color);
}

HeeksObj* NewSketch()
{
	return new CSketch();
}

HeeksObj* NewText(const std::wstring &value)
{
	return new HText(Matrix(), value, &theApp->current_color, 0, 0);
}

HeeksObj* NewCoordinateSystem(const std::wstring &title, const Point3d& o, const Point3d& x, const Point3d& y)
{
	return new CoordinateSystem(title, o, x, y);
}

boost::python::list HeeksObjGetLines(HeeksObj& object)
{
	return_list_ForGetLines = boost::python::list();
	object.GetSegments(CallbackForGetLines, GetLines_pixels_per_mm);
	return return_list_ForGetLines;
}

Point3d HeeksObjGetStartPoint(HeeksObj& object)
{
	Point3d p(0,0,0);
	object.GetStartPoint(p);
	return p;
}

Point3d HeeksObjGetEndPoint(HeeksObj& object)
{
	Point3d p(0, 0, 0);
	object.GetEndPoint(p);
	return p;
}

Point3d HeeksObjGetCentrePoint(HeeksObj& object)
{
	Point3d p(0, 0, 0);
	object.GetCentrePoint(p);
	return p;
}

std::wstring PropertyGetShortString(Property& p)
{
	return std::wstring(p.GetShortString());
}

std::wstring PropertyGetString(Property& p)
{
	return std::wstring(p.GetString());
}

boost::python::list PropertyGetProperties(Property& p) {
	boost::python::list return_list;
	std::list<Property*> p_list;
	p.GetList(p_list);
	for (std::list<Property*>::iterator It = p_list.begin(); It != p_list.end(); It++)
	{
		Property* property = *It;
		return_list.append(boost::python::ptr<Property*>(property));
	}
	return return_list;
}

void ObjListClear(ObjList& objlist)
{
	objlist.Clear();
}

void ObjListAdd(ObjList& objlist, HeeksObj* object)
{
	objlist.Add(object, NULL);
}

void ObjAdd(HeeksObj& obj, HeeksObj* object)
{
	obj.Add(object, NULL);
}

Matrix ObjGetOrigin(HeeksObj& object)
{
	Matrix m;
	object.GetScaleAboutMatrix(m);
	return m;
}

void ObjListReadFromXML(ObjList& objlist)
{
	TiXmlElement* save_element = theApp->m_cur_xml_element;
	objlist.ObjList::ReadFromXML(theApp->m_cur_xml_element);
	theApp->m_cur_xml_element = save_element;
}

void ObjListWriteToXML(ObjList& objlist)
{
	TiXmlElement* save_element = theApp->m_cur_xml_element;
	objlist.ObjList::WriteToXML(theApp->m_cur_xml_element);
	theApp->m_cur_xml_element = save_element;
}

void ObjListCopyFrom(ObjList& objlist, HeeksObj* object)
{
	objlist.ObjList::CopyFrom(object);
}

CBox ObjListGetBox(ObjList& objlist)
{
	CBox box;
	objlist.ObjList::GetBox(box);
	return box;
}

void IdNamedObjListSetTitle(IdNamedObjList& object, const wstring& title)
{
	object.m_title_made_from_id = false;
	object.m_title = title;
}

HeeksColor PropertyGetColor(const Property& p)
{
	return p.GetColor();
}

boost::python::list PropertyGetChoices(const Property& p)
{
	boost::python::list return_list;
	std::list< std::wstring > choices;
	p.GetChoices(choices);
	for (std::list<std::wstring>::iterator It = choices.begin(); It != choices.end(); It++)
	{
		std::wstring& choice = *It;
		return_list.append(choice);
	}
	return return_list;
}

HeeksColor HeeksObjGetColor(const HeeksObj& object)
{
	return *(object.GetColor());
}

bool HeeksObjHasEdit(const HeeksObj& object)
{
	return false;
}

boost::python::tuple SketchGetStartPoint(CSketch &sketch)
{
	Point3d s(0.0, 0.0, 0.0);
	sketch.GetStartPoint(s);
	return boost::python::make_tuple(s.x, s.y, s.z);
}

boost::python::tuple SketchGetEndPoint(CSketch &sketch)
{
	Point3d s(0.0, 0.0, 0.0);
	sketch.GetEndPoint(s);
	return boost::python::make_tuple(s.x, s.y, s.z);
}

boost::python::list SketchSplit(CSketch& sketch) {
	boost::python::list olist;
	std::list<HeeksObj*> new_separate_sketches;
	sketch.ExtractSeparateSketches(new_separate_sketches, true);
	for (std::list<HeeksObj*>::iterator It = new_separate_sketches.begin(); It != new_separate_sketches.end(); It++)
	{
		HeeksObj* object = *It;
		AddObjectToPythonList(object, olist);
	}
	return olist;
}

double SketchGetCircleDiameter(CSketch& sketch)
{
	HeeksObj* span = sketch.GetFirstChild();
	if (span == NULL)
		return 0.0;

	if (span->GetType() == ArcType)
	{
		HArc* arc = (HArc*)span;
		return arc->A.Dist2D(arc->C) * 2;
	}
	else if (span->GetType() == CircleType)
	{
		HCircle* circle = (HCircle*)span;
		return circle->m_radius * 2;
	}
	return 0.0;
}

boost::python::tuple SketchGetCircleCentre(CSketch& sketch)
{
	HeeksObj* span = sketch.GetFirstChild();
	if (span == NULL)
		return boost::python::make_tuple(NULL);

	if (span->GetType() == ArcType)
	{
		HArc* arc = (HArc*)span;
		Point3d& C = arc->C;
		return boost::python::make_tuple(C.x, C.y, C.z);
	}
	else if (span->GetType() == CircleType)
	{
		HCircle* circle = (HCircle*)span;
		const Point3d& C = circle->m_c;
		return boost::python::make_tuple(C.x, C.y, C.z);
	}

	return boost::python::make_tuple(NULL);
}

void SketchWriteDXF(CSketch& sketch, std::wstring filepath)
{
	std::list<HeeksObj*> objects;
	objects.push_back(&sketch);
	theApp->SaveDXFFile(objects, filepath.c_str());
}

CCurve SketchGetCurve(CSketch& sketch)
{
	// this makes a Curve object, compatible with the geom library
	CArea area = ObjectToArea(&sketch);
	if (area.m_curves.size() == 0)
		return CCurve();
	return area.m_curves.front();
}

CArea SketchGetArea(CSketch& sketch)
{
	return ObjectToArea(&sketch);
}


CArea CircleGetArea(HCircle& circle)
{
	return ObjectToArea(&circle);
}


void BeginTriangles()
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
}

void BeginLines()
{
	if (!BaseObject::lines_begun)
	{
		if (BaseObject::triangles_begun)
		{
			glEnd();
			BaseObject::triangles_begun = false;
		}
		glBegin(GL_LINE_STRIP);
		BaseObject::lines_begun = true;
	}
}

void EndLinesOrTriangles()
{
	if (BaseObject::triangles_begun)
	{
		glEnd();
		BaseObject::triangles_begun = false;
	}
	else if (BaseObject::lines_begun)
	{
		glEnd();
		BaseObject::lines_begun = false;
	}
}

void DrawTriangle(double x0, double x1, double x2, double x3, double x4, double x5, double x6, double x7, double x8)
{
	BeginTriangles();

	Point3d p0(x0, x1, x2);
	Point3d p1(x3, x4, x5);
	Point3d p2(x6, x7, x8);
	Point3d v1(p0, p1);
	Point3d v2(p0, p2);
	try
	{
		Point3d norm = (v1 ^ v2).Normalized();
		glNormal3d(norm.x, norm.y, norm.z);
	}
	catch (...)
	{
	}
	glVertex3d(x0, x1, x2);
	glVertex3d(x3, x4, x5);
	glVertex3d(x6, x7, x8);

}

void DrawEnableLighting()
{
	glEnable(GL_LIGHTING);
}

void DrawDisableLighting()
{
	glDisable(GL_LIGHTING);
}

GLfloat save_depth_range[2];

void DrawEnableDepthTesting()
{
	glDepthRange(save_depth_range[0], save_depth_range[1]);
}

void DrawDisableDepthTesting()
{
	glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
	glDepthRange(0, 0);
}

void DrawEnableCullFace()
{
	glEnable(GL_CULL_FACE);
}

void DrawDisableCullFace()
{
	glDisable(GL_CULL_FACE);
}

void DrawLine(double x0, double x1, double x2, double x3, double x4, double x5)
{
	BeginLines();
	glVertex3d(x0, x1, x2);
	glVertex3d(x3, x4, x5);
}

void DrawColor(const HeeksColor& col)
{
	col.glColor();
}

void DrawContrastBlackOrWhite()
{
	theApp->glColorEnsuringContrast(HeeksColor(0, 0, 0));
}

void DrawTranslate(double x, double y, double z)
{
	glTranslated(x, y, z);
}

static unsigned char cross16[32] = { 0x80, 0x01, 0x40, 0x02, 0x20, 0x04, 0x10, 0x08, 0x08, 0x10, 0x04, 0x20, 0x02, 0x40, 0x01, 0x80, 0x01, 0x80, 0x02, 0x40, 0x04, 0x20, 0x08, 0x10, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01 };
static unsigned char bmp16[10] = { 0x3f, 0xfc, 0x1f, 0xf8, 0x0f, 0xf0, 0x07, 0xe0, 0x03, 0xc0 };
static unsigned char cross16_selected[32] = { 0xc0, 0x03, 0xe0, 0x07, 0x70, 0x0e, 0x38, 0x1c, 0x1c, 0x38, 0x0e, 0x70, 0x07, 0xe0, 0x03, 0xc0, 0x03, 0xc0, 0x07, 0xe0, 0x0e, 0x70, 0x1c, 0x38, 0x38, 0x1c, 0x70, 0x0e, 0xe0, 0x07, 0xc0, 0x03 };
static unsigned char cross16_wider[32] = { 0xe0, 0x07, 0xf0, 0x0f, 0xf8, 0x1f, 0x7c, 0x3e, 0x3e, 0x7c, 0x1f, 0xf8, 0x0f, 0xf0, 0x07, 0xe0, 0x07, 0xe0, 0x0f, 0xf0, 0x1f, 0xf8, 0x3e, 0x7c, 0x7c, 0x3e, 0xf8, 0x1f, 0xf0, 0x0f, 0xe0, 0x07 };

void DrawSymbol(int type, double x, double y, double z)
{
	glRasterPos3d(x, y, z);
	switch (type)
	{
	case 0:
		glBitmap(16, 16, 8, 8, 10.0, 0.0, cross16);
		break;
	case 1:
		glBitmap(16, 5, 8, 3, 10.0, 0.0, bmp16);
		break;
	case 2:
		glBitmap(16, 16, 8, 8, 10.0, 0.0, cross16_selected);
		break;
	case 3:
		glBitmap(16, 16, 8, 8, 10.0, 0.0, cross16_wider);
		break;
	}
}

void glVertexPoint3d(const Point3d& p)
{
	glVertex3dv(p.getBuffer());
}

void glVertexPoint2d(const Point& p)
{
	glVertex2dv(&p.x);
}

void GlLineWidth(int width)
{
	glLineWidth(width);
}

static unsigned int current_display_list_started = 0;

unsigned int DrawNewList()
{
	if (current_display_list_started != 0)
		return 0;
	current_display_list_started = glGenLists(1);
	glNewList(current_display_list_started, GL_COMPILE_AND_EXECUTE);
	return current_display_list_started;
}


void DrawEndList()
{
	if (current_display_list_started != 0)
		glEndList();
	current_display_list_started = 0;
}

void DrawCallList(unsigned int display_list)
{
	if (display_list != 0)
		glCallList(display_list);
}

void DrawDeleteList(unsigned int display_list)
{
	glDeleteLists(display_list, 1);
}

void DrawEnableLights()
{
	glEnable(GL_LIGHTING);
}

void DrawDisableLights()
{
	glDisable(GL_LIGHTING);
}

bool GetBufferBuffer(PyObject* bufobj, Py_buffer &view)
{

	/* Attempt to extract buffer information from it */
	if (PyObject_GetBuffer(bufobj, &view,
		PyBUF_ANY_CONTIGUOUS | PyBUF_FORMAT) == -1)
	{
		return false;
	}

	if (view.ndim != 1)
	{
		PyErr_SetString(PyExc_TypeError, "Expected a 1-dimensional array");
		PyBuffer_Release(&view);
		return false;
	}

	/* Check the type of items in the array */
	if (strcmp(view.format, "B") != 0)
	{
		PyErr_SetString(PyExc_TypeError, "Expected an array of doubles");
		PyBuffer_Release(&view);
		return false;
	}

	return true;
}

boost::python::tuple GenTexture(boost::python::object buffer, boost::python::object alphaBuffer, int imageWidth, int imageHeight, bool hasAlpha)
{
	// pass an image buffer from wx.Image.GetData

	// returns a tuple ( texture_number, newWidth )

	Py_buffer view, alphaView;
	GLubyte *bitmapData = NULL;
	GLubyte *alphaData = NULL;
	if (!GetBufferBuffer(buffer.ptr(), view))
		return boost::python::make_tuple(0, 0);
	bitmapData = (GLubyte *)view.buf;

	if (hasAlpha)
	{
		if (!GetBufferBuffer(alphaBuffer.ptr(), alphaView))
			return boost::python::make_tuple(0, 0);
		alphaData = (GLubyte *)alphaView.buf;
	}

	//unsigned char* data_buf = (unsigned char*)view.buf;
	//int size = (int)view.len;

	float power_of_two_that_gives_correct_width = std::log((float)(imageWidth)) / std::log(2.0f);
	float power_of_two_that_gives_correct_height = std::log((float)(imageHeight)) / std::log(2.0f);
	int newWidth = (int)std::pow(2.0, (int)(std::ceil(power_of_two_that_gives_correct_width)));
	{
		int newHeight = (int)std::pow(2.0, (int)(std::ceil(power_of_two_that_gives_correct_height)));
		if (newHeight > newWidth)
			newWidth = newHeight;
	}

	int bytesPerPixel = hasAlpha ? 4 : 3;

	GLuint texture_number;
	glGenTextures(1, &texture_number);
	glBindTexture(GL_TEXTURE_2D, texture_number);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	GLubyte	*imageData;
	int imageSize = newWidth * newWidth * bytesPerPixel;
	imageData = (GLubyte *)malloc(imageSize);

	int rev_val = imageHeight - 1;

	for (int y = 0; y<newWidth; y++)
	{
		for (int x = 0; x<newWidth; x++)
		{

			if (x<(imageWidth) && y<(imageHeight)){
				imageData[(x + y*newWidth)*bytesPerPixel + 0] =
					bitmapData[(x + (rev_val - y)*(imageWidth))*3 + 0];

				imageData[(x + y*newWidth)*bytesPerPixel + 1] =
					bitmapData[(x + (rev_val - y)*(imageWidth))*3 + 1];

				imageData[(x + y*newWidth)*bytesPerPixel + 2] =
					bitmapData[(x + (rev_val - y)*(imageWidth))*3 + 2];

				if (hasAlpha) imageData[(x + y*newWidth)*bytesPerPixel + 3] =
					alphaData[x + (rev_val - y)*(imageWidth)];
			}
			else
			{

				imageData[(x + y*newWidth)*bytesPerPixel + 0] = 0;
				imageData[(x + y*newWidth)*bytesPerPixel + 1] = 0;
				imageData[(x + y*newWidth)*bytesPerPixel + 2] = 0;
				if (hasAlpha) imageData[(x + y*newWidth)*bytesPerPixel + 3] = 0;
			}

		}//next
	}//next


	glTexImage2D(GL_TEXTURE_2D,
		0,
		hasAlpha ? 4 : 3,
		newWidth,
		newWidth,
		0,
		hasAlpha ? GL_RGBA : GL_RGB,
		GL_UNSIGNED_BYTE,
		imageData);

	free(imageData);

	// set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	return boost::python::make_tuple(texture_number, newWidth);
}


#define LOTS_OF_QUADS

void DrawImageQuads(int width, int height, int textureWidth, unsigned int texture_number, const Point3d &bottom_left, const Point3d &bottom_right, const Point3d &top_right, const Point3d &top_left, float opacity, bool no_color, bool marked)
{
		bool blend_enabled = false;
	if (!no_color){
		if (opacity<1) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(0);
			blend_enabled = true;
		}

		glColor4f(1.0, 1.0, 1.0, opacity);
		if (texture_number){
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texture_number);
		}
	}

	glBegin(GL_QUADS);

#ifdef LOTS_OF_QUADS
	double x_step = (double)width / 20;
	double y_step = (double)height / 20;

	for (double i = 0; i<width; i += x_step){
		for (double j = 0; j<height; j += y_step){
			double xy[4][2] = { { i, j }, { i + x_step, j }, { i + x_step, j + y_step }, { i, j + y_step } };

			for (int k = 0; k < 4; k++)
			{
				Point3d p5 = bottom_left + (bottom_right - bottom_left) * xy[k][0] / width;
				Point3d p6 = top_left + (top_right - top_left) * xy[k][0] / width;
				Point3d vt = p5 + (p6 - p5) * xy[k][1] / height;

				glTexCoord2f((float)xy[k][0] / textureWidth, (float)xy[k][1] / textureWidth);
				glVertex3dv(vt.getBuffer());
			}

		}
	}
#else
	glTexCoord2f(0, 0);
	glVertex3dv(m_x[0]);
	glTexCoord2f((float)width / textureWidth, 0);
	glVertex3dv(m_x[1]);
	glTexCoord2f((float)width / textureWidth, (float)height / textureHeight);
	glVertex3dv(m_x[2]);
	glTexCoord2f(0, (float)height / textureHeight);
	glVertex3dv(m_x[3]);
#endif

	glEnd();

	if (!no_color){
		glDisable(GL_TEXTURE_2D);
	}
	if (blend_enabled)glDisable(GL_BLEND);
}

void DrawTris(const CTris& tris, bool with_normals)
{
	glBegin(GL_TRIANGLES);
	for (std::list<CTri>::const_iterator It = tris.m_tris.begin(); It != tris.m_tris.end(); It++)
	{
		const CTri& tri = *It;
		if (with_normals)
		{
			Point3d p0 = Point3d(tri.x[0][0], tri.x[0][1], tri.x[0][2]);
			Point3d p1 = Point3d(tri.x[1][0], tri.x[1][1], tri.x[1][2]);
			Point3d p2 = Point3d(tri.x[2][0], tri.x[2][1], tri.x[2][2]);
			Point3d v0 = p1 - p0;
			Point3d v1 = p2 - p0;
			Point3d norm = v0^v1;
			norm.normalise();
			glNormal3dv(norm.getBuffer());
		}
		glVertex3fv(tri.x[0]);
		glVertex3fv(tri.x[1]);
		glVertex3fv(tri.x[2]);
	}
	glEnd();
}

void DrawMultMatrix(const Matrix& mat)
{
	double m[16];
	mat.GetTransposed(m);
	glMultMatrixd(m);
}

void DrawPushMatrix()
{
	glPushMatrix();
}

void DrawPopMatrix()
{
	glPopMatrix();
}

void DrawObjectsOnFront(boost::python::list &list, bool do_depth_testing)
{
	std::list<HeeksObj*> o_list;
	for (int i = 0; i < len(list); ++i)
	{
		o_list.push_back(boost::python::extract<HeeksObj*>(list[i]));
	}
	return theApp->DrawObjectsOnFront(o_list, do_depth_testing);
}

void RenderScreenText(const std::wstring &str1, double scale)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	theApp->m_current_viewport->SetIdentityProjection();
	theApp->background_color[0].best_black_or_white().glColor();
	int w, h;
	theApp->m_current_viewport->GetViewportSize(&w, &h);
	glTranslated(2.0, h - 1.0, 0.0);

	theApp->render_screen_text2(str1.c_str(), scale);

	//Even though this is in reverse order, the different matrices have different stacks, and we want to exit here in the modelview
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void RenderScreenTextAt(const std::wstring &str1, double scale, double x, double y, double theta)
{
	theApp->render_screen_text_at(str1.c_str(), scale, x, y, theta);
}

boost::python::object GetObjectFromId(int type, int id) {
	boost::python::list olist;
	HeeksObj* object = theApp->GetIDObject(type, id);
	if (object != NULL)
	{
		AddObjectToPythonList(object, olist);
		if (boost::python::len(olist) > 0)
		{
			return olist[0];
		}
	}

	return boost::python::object(); // None
}

void SetXmlValue(const std::wstring &name, PyObject* value)
{
	const char* sname = Ttc(name.c_str());
	if (PyLong_Check(value))
	{
		theApp->m_cur_xml_element->SetAttribute(sname, PyLong_AsLong(value));
	}
	else if (PyFloat_Check(value))
	{
		theApp->m_cur_xml_element->SetDoubleAttribute(sname, PyFloat_AsDouble(value));
	}
	else if (PyBool_Check(value))
	{
		theApp->m_cur_xml_element->SetAttribute(sname, PyObject_IsTrue(value) ? 1:0);
	}
	else if (PyBytes_Check(value))
	{
		theApp->m_cur_xml_element->SetAttribute(sname, PyBytes_AsString(value));
	}
	else if (PyUnicode_Check(value))
	{
		theApp->m_cur_xml_element->SetAttribute(sname, PyBytes_AsString(PyUnicode_AsASCIIString(value)));
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "invalid value type");
		boost::python::throw_error_already_set();
	}
}

void BeginXmlChild(const std::wstring &child_name)
{
	std::string svalue(Ttc(child_name.c_str()));

	TiXmlElement *element = new TiXmlElement(Ttc(child_name.c_str()));
	theApp->m_cur_xml_element->LinkEndChild(element);
	theApp->m_cur_xml_element = element;
}

void EndXmlChild()
{
	if (theApp->m_cur_xml_element)theApp->m_cur_xml_element = theApp->m_cur_xml_element->Parent()->ToElement();
}

boost::python::object GetXmlObject() {
	if (theApp->m_cur_xml_element)
	{
		HeeksObj* object = theApp->ReadXMLElement(theApp->m_cur_xml_element);
		if (object != NULL)
		{
			boost::python::list olist;
			AddObjectToPythonList(object, olist);
			if (boost::python::len(olist) > 0)
			{
				return olist[0];
			}
		}
	}

	return boost::python::object(); // None
}


std::wstring GetXmlValue(const std::wstring &name, const std::wstring &default_value = L"")
{
	if (theApp->m_cur_xml_element != NULL)
	{
		const char* value = theApp->m_cur_xml_element->Attribute(Ttc(name.c_str()));
		if (value != NULL)
			return std::wstring(Ctt(value));
	}
	return default_value;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(GetXmlValueOverloads, GetXmlValue, 1, 2)

std::wstring GetXmlText()
{
	if (theApp->m_cur_xml_element == NULL)return L"";
	const char* text = theApp->m_cur_xml_element->GetText();
	if (text == NULL)return L"";
	return Ctt(text);
}

void SetXmlText(const std::wstring& str)
{
	if (theApp->m_cur_xml_element == NULL)return;

	// add actual text as a child object
	TiXmlText* text = new TiXmlText(Ttc(str.c_str()));
	theApp->m_cur_xml_element->LinkEndChild(text);
}

bool GetXmlBool(const std::wstring &name, bool default_value = false)
{
	if (theApp->m_cur_xml_element == NULL)
		return default_value;

	const char* cname = Ttc(name.c_str());
	if (theApp->m_cur_xml_element->Attribute(cname) == NULL)
		return default_value;
	int value;
	theApp->m_cur_xml_element->Attribute(cname, &value);
	return value != 0;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(GetXmlBoolOverloads, GetXmlBool, 1, 2)

int GetXmlInt(const std::wstring &name, int default_value = 0)
{
	if (theApp->m_cur_xml_element == NULL)
		return default_value;

	const char* cname = Ttc(name.c_str());
	if (theApp->m_cur_xml_element->Attribute(cname) == NULL)
		return default_value;
	int value;
	theApp->m_cur_xml_element->Attribute(cname, &value);
	return value;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(GetXmlIntOverloads, GetXmlInt, 1, 2)

double GetXmlFloat(const std::wstring &name, double default_value = 0)
{
	if (theApp->m_cur_xml_element == NULL)
		return default_value;

	const char* cname = Ttc(name.c_str());
	if (theApp->m_cur_xml_element->Attribute(cname) == NULL)
		return default_value;
	double value;
	theApp->m_cur_xml_element->Attribute(cname, &value);
	return value;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(GetXmlFloatOverloads, GetXmlFloat, 1, 2)

Matrix GetXmlMatrix(const std::wstring &name, const Matrix* default_value = NULL)
{
	if (theApp->m_cur_xml_element == NULL)
	{
		if (default_value)return *default_value;
		return Matrix();
	}

	const char* cname = Ttc(name.c_str());
	size_t length = strlen(cname);
	char* elename = (char*)malloc(length + 3);
	memcpy(elename, cname, length);
	char* e0 = &elename[length];
	char* e1 = &elename[length + 1];
	elename[length + 2] = 0;

	Matrix return_matrix;
	double* ele = return_matrix.e;

	for (int j = 0; j < 4; j++)
	{
		*e0 = '0' + j;
		for (int i = 0; i < 4; i++)
		{
			*e1 = '0' + i;
			if (theApp->m_cur_xml_element->Attribute(elename, ele) == NULL)
			{
				if (default_value)return *default_value;
				return Matrix();
			}
			ele++;
		}
	}

	return_matrix.IsUnit(); // set flags
	return_matrix.IsMirrored();

	return return_matrix;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(GetXmlMatrixOverloads, GetXmlMatrix, 1, 2)


void SetXmlMatrix(const std::wstring &name, const Matrix& m)
{
	const char* cname = Ttc(name.c_str());
	size_t length = strlen(cname);
	char* elename = (char*)malloc(length + 3);
	memcpy(elename, cname, length);
	char* e0 = &elename[length];
	char* e1 = &elename[length + 1];
	elename[length + 2] = 0;
	const double* ele = m.e;

	for (int j = 0; j < 4; j++)
	{
		*e0 = '0' + j;
		for (int i = 0; i < 4; i++)
		{
			*e1 = '0' + i;
			theApp->m_cur_xml_element->SetDoubleAttribute(elename, *ele);
			ele++;
		}
	}
}



void ReturnFromXmlChild()
{
	if(theApp->m_cur_xml_element)theApp->m_cur_xml_element = theApp->m_cur_xml_element->Parent()->ToElement();
}

boost::python::object GetFirstXmlChild()
{
	if (theApp->m_cur_xml_element)
	{
		TiXmlElement* first_child = theApp->m_cur_xml_element->FirstChildElement();
		if (first_child == NULL)
		{
			// leave current object as it is, but return None
			return boost::python::object(); // None
		}
		else
		{
			// set current to the first child and return it
			theApp->m_cur_xml_element = first_child;
			return boost::python::object(std::wstring(Ctt(theApp->m_cur_xml_element->Value())));
		}
	}
	return boost::python::object(); // None
}

boost::python::object GetNextXmlChild()
{
	if (theApp->m_cur_xml_element)
	{
		TiXmlElement* next_sibling = theApp->m_cur_xml_element->NextSiblingElement();
		if (next_sibling == NULL)
		{
			// set current element to be the parent, but return None
			theApp->m_cur_xml_element = theApp->m_cur_xml_element->Parent()->ToElement();
			return boost::python::object(); // None
		}
		else
		{
			// set current to the next sibling and return it
			theApp->m_cur_xml_element = next_sibling;
			return boost::python::object(std::wstring(Ctt(theApp->m_cur_xml_element->Value())));
		}
	}
	return boost::python::object(); // None
}

void OpenXmlFile(const std::wstring &filepath, HeeksObj* paste_into = NULL, HeeksObj* paste_before = NULL)
{
	theApp->OpenXMLFile(filepath.c_str(), paste_into, paste_before, true);
}

BOOST_PYTHON_FUNCTION_OVERLOADS(OpenXMLFileOverloads, OpenXmlFile, 1, 3)

class PropertyWrap : public Property, public cad_wrapper<Property>
{
	int m_type;
public:
	PropertyWrap() :Property(), m_type(InvalidPropertyType){}
	PropertyWrap(int type, const std::wstring& title, HeeksObj* object) :Property(object, title.c_str()), m_type(type){ m_editable = true; }
	int get_property_type()const{ return m_type; }
	int GetInt()const override
	{
		std::pair<bool, int> result = CallReturnInt("GetInt");
		if (result.first)
			return result.second;
		return Property::GetInt();
	}
	bool GetBool()const override
	{
		std::pair<bool, bool> result = CallReturnBool("GetBool");
		if (result.first)
			return result.second;
		return Property::GetBool();
	}
	double GetDouble()const override
	{
		std::pair<bool, double> result = CallReturnDouble("GetFloat");
		if (result.first)
			return result.second;
		return Property::GetDouble();
	}
	const wchar_t* GetString()const override
	{
		std::pair<bool, std::wstring> result = CallReturnWString("GetString");
		if (result.first)
			return result.second.c_str();
		return Property::GetString();
	}
	void Set(bool value)override
	{
		CallVoidReturn("SetBool", value);
	}
	void Set(int value)override
	{
		CallVoidReturn("SetInt", value);
	}
	void Set(double value)override
	{
		CallVoidReturn("SetFloat", value);
	}
	void Set(const wchar_t* value)override
	{
		CallVoidReturn("SetString", std::wstring(value));
	}
	Property *MakeACopy(void)const{ return new PropertyWrap(*this); }
};


int PropertyWrapGetInt(PropertyWrap& property)
{
	return property.GetInt();
}


class UndoableWrap : public Undoable, public cad_wrapper<Undoable>
{
public:
	void Run(bool redo) override
	{
		CallVoidReturn("Run", redo);
	}
	const wchar_t* GetTitle() override
	{
		std::pair<bool, std::string> result = CallReturnString("GetTitle");
		if (result.first)
			return Ctt(result.second.c_str());
		return L"";
	}
	void RollBack()
	{
		CallVoidReturn("RollBack");
	}

	bool CanBeDeleted()
	{
		return false;
	}
};



void StlSolidWriteSTL(CStlSolid& solid, double tolerance, std::wstring filepath)
{
	std::list<HeeksObj*> list;
	list.push_back(&solid);
	theApp->SaveSTLFileAscii(list, filepath.c_str(), tolerance);
}

int StlSolidNumTriangles(CStlSolid& solid)
{
	return solid.m_list.size();
}

int ViewportGetHeight(CViewport& viewport)
{
	return viewport.GetViewportSize().GetHeight();
}

int ViewportGetWidth(CViewport& viewport)
{
	return viewport.GetViewportSize().GetWidth();
}

void ViewportSetView(CViewport& viewport)
{
	viewport.SetViewport();
	viewport.m_view_point.SetProjection(false);
	viewport.m_view_point.SetModelview();
}

static boost::shared_ptr<CStlSolid> initStlSolid(const std::wstring& title, const HeeksColor* color)
{
	return boost::shared_ptr<CStlSolid>(new CStlSolid(title.c_str(), color));
}

static boost::shared_ptr<PropertyStringReadOnly> initPropertyStringReadOnly(const std::wstring& title, const std::wstring& value)
{
	return boost::shared_ptr<PropertyStringReadOnly>(new PropertyStringReadOnly(title.c_str(), value.c_str()));
}

static boost::shared_ptr<HPoint> initHPoint(const Point3d& p)
{
	return boost::shared_ptr<HPoint>(new HPoint(p, &theApp->current_color));
}

static boost::shared_ptr<HILine> initHILine(const Point3d &a, const Point3d &b)
{
	return boost::shared_ptr<HILine>(new HILine(a, b, &theApp->current_color));
}

static boost::shared_ptr<HCircle> initHCircle(const Point3d& c, const Point3d& a, double r)
{
	return boost::shared_ptr<HCircle>(new HCircle(c, a, r, &theApp->current_color));
}

static boost::shared_ptr<GripData> initGripData(const Point3d& p, EnumGripperType t, int alternative_icon)
{
	return boost::shared_ptr<GripData>(new GripData(t, p, NULL, false, alternative_icon));
}

boost::python::list GetSelectedObjects() {
	boost::python::list slist;
	for (std::list<HeeksObj *>::iterator It = theApp->m_marked_list->list().begin(); It != theApp->m_marked_list->list().end(); It++)
	{
		HeeksObj* object = *It;
		AddObjectToPythonList(object, slist);
	}
	return slist;
}

static CFilter selection_types_found(true);

CFilter* GetSelectionTypes()
{
	selection_types_found.Clear();
	for (std::list<HeeksObj *>::iterator It = theApp->m_marked_list->list().begin(); It != theApp->m_marked_list->list().end(); It++)
	{
		HeeksObj* object = *It;
		selection_types_found.AddType(object->GetType());
	}
	return &selection_types_found;
}

boost::python::list GetSelectionProperties()
{
	std::list<Property *> list;
	theApp->m_marked_list->GetProperties(&list);
	boost::python::list return_list;
	for (std::list<Property *>::iterator It = list.begin(); It != list.end(); It++)
	{
		Property* prop = *It;
		return_list.append(boost::python::ptr<Property*>(prop));
	}
	return return_list;
}

void SetMarkNewlyAddedObjects(bool value)
{
	theApp->m_mark_newly_added_objects = value;
}

bool GetMarkNewlyAddedObjects(void)
{
	return theApp->m_mark_newly_added_objects;
}

unsigned int GetNumSelected()
{
	return theApp->m_marked_list->list().size();
}

boost::python::list GetObjects() {
	boost::python::list olist;
	for (HeeksObj *object = theApp->GetFirstChild(); object; object = theApp->GetNextChild())
	{
		AddObjectToPythonList(object, olist);
	}
	return olist;
}

bool ObjectMarked(HeeksObj* object)
{
	return theApp->m_marked_list->ObjectMarked(object);
}

void Select(HeeksObj* object, bool call_OnChanged = true)
{
	theApp->m_marked_list->Add(object, call_OnChanged);
}

BOOST_PYTHON_FUNCTION_OVERLOADS(SelectOverloads, Select, 1, 2)

void Unselect(HeeksObj* object, bool call_OnChanged)
{
	theApp->m_marked_list->Remove(object, call_OnChanged);
}

void ClearSelection(bool call_OnChanged = false)
{
	theApp->m_marked_list->Clear(call_OnChanged);
}

BOOST_PYTHON_FUNCTION_OVERLOADS(ClearSelectionOverloads, ClearSelection, 0, 1)

double GetViewUnits()
{
	return theApp->m_view_units;
}

void SetViewUnits(double units)
{
	theApp->m_view_units = units;
}

void StartHistory(const std::wstring& title, bool freeze_observers = true)
{
	theApp->StartHistory(title.c_str(), freeze_observers);
}

BOOST_PYTHON_FUNCTION_OVERLOADS(StartHistoryOverloads, StartHistory, 1, 2)

void EndHistory()
{
	theApp->EndHistory();
}

void ClearHistory()
{
	theApp->ClearHistory();
}

int GetHistoryLevel()
{
	return theApp->GetHistoryLevel();
}

bool IsModified()
{
	return theApp->IsModified();
}

void SetLikeNewFile()
{
	theApp->SetLikeNewFile();
}

void RollBack()
{
	theApp->RollBack();
}

void RollForward()
{
	theApp->RollForward();
}

std::wstring GetUndoTitle()
{
	const wchar_t* s = theApp->GetUndoTitle();
	if (s == NULL)
		return std::wstring(L"");
	return std::wstring(s);
}

std::wstring GetRedoTitle()
{
	const wchar_t* s = theApp->GetRedoTitle();
	if (s == NULL)
		return std::wstring(L"");
	return std::wstring(s);
}

void DeleteUndoably(HeeksObj *object)
{
	theApp->DeleteUndoably(object);
}

void DeleteObjectsUndoably(boost::python::list &list)
{
	std::list<HeeksObj*> o_list;
	for (int i = 0; i < len(list); ++i)
	{
		o_list.push_back(boost::python::extract<HeeksObj*>(list[i]));
	}
	return theApp->DeleteUndoably(o_list);
}

void DeleteInternalObject(HeeksObj* object)
{
	delete object;
}

Matrix* GetDrawMatrix(bool get_the_appropriate_orthogonal)
{
	return theApp->GetDrawMatrix(get_the_appropriate_orthogonal);
}

void CopyUndoably(HeeksObj* object, HeeksObj* copy_object)
{
	theApp->CopyUndoably(object, copy_object);
}

void AddUndoably(HeeksObj *object, HeeksObj* owner = NULL, HeeksObj* add_before = NULL)
{
	theApp->AddUndoably(object, owner, add_before);
}

BOOST_PYTHON_FUNCTION_OVERLOADS(AddUndoablyOverloads, AddUndoably, 1, 3)


void TransformUndoably(HeeksObj *object, const Matrix &m)
{
	theApp->TransformUndoably(object, m);
}


void DoUndoable(Undoable* undoable)
{
	theApp->DoUndoable(undoable);
}

void WasModified(HeeksObj *object)
{
	theApp->WasModified(object);
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
	std::list<HeeksObj*> &marked = theApp->m_marked_list->list();
	HeeksObj* recently_marked_sibling = NULL;
	for (std::list<HeeksObj*>::reverse_iterator It = marked.rbegin(); It != marked.rend(); It++)
	{
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
			theApp->m_marked_list->Clear(false);
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

		theApp->m_marked_list->Add(list_to_mark, true);
	}
	else
	{
		if (control_down)
		{
			if (theApp->m_marked_list->ObjectMarked(object))
			{
				theApp->m_marked_list->Remove(object, true);
			}
			else{
				theApp->m_marked_list->Add(object, true);
			}
		}
		else
		{
			if (theApp->m_marked_list->ObjectMarked(object))
			{
				waiting_until_left_up = true;
			}
			else
			{
				theApp->m_marked_list->Clear(false);
				theApp->m_marked_list->Add(object, true);
			}
		}
	}

	return waiting_until_left_up;
}

void ChangePropertyString(const std::wstring& value, Property* property)
{
	theApp->DoUndoable(new PropertyChangeString(value, property));
}

void ChangePropertyDouble(const double& value, Property* property)
{
	theApp->DoUndoable(new PropertyChangeDouble(value, property));
}

void ChangePropertyLength(const double& value, Property* property)
{
	theApp->DoUndoable(new PropertyChangeLength(value, property));
}

void ChangePropertyInt(const int& value, Property* property)
{
	theApp->DoUndoable(new PropertyChangeInt(value, property));
}

void ChangePropertyChoice(const int& value, Property* property)
{
	theApp->DoUndoable(new PropertyChangeChoice(value, property));
}

void ChangePropertyColor(const HeeksColor& value, Property* property)
{
	theApp->DoUndoable(new PropertyChangeColor(value, property));
}

void ChangePropertyCheck(const bool& value, Property* property)
{
	theApp->DoUndoable(new PropertyChangeCheck(value, property));
}

CApp* GetApp()
{
	return theApp;
}

HeeksObj* GetFirstChild()
{
	return theApp->GetFirstChild();
}

HeeksObj* GetNextChild()
{
	return theApp->GetNextChild();
}

HeeksObj* ObjectGetOwner(HeeksObj* object)
{
	return object->m_owner;
}

void ObjectSetOwner(HeeksObj* object, HeeksObj* new_owner)
{
	object->m_owner = new_owner;
}

double GetUnits()
{
	return theApp->m_view_units;
}

void PyIncref(PyObject* object)
{
	Py_IncRef(object);
}

int GetNextID(int id_group_type)
{
	return theApp->GetNextID(id_group_type);
}

bool GetDrawSelect()
{
	return BaseObject::m_select;
}

bool GetDrawMarked()
{
	return BaseObject::m_marked;
}

bool CanUndo()
{
	return theApp->CanUndo();
}

bool CanRedo()
{
	return theApp->CanRedo();
}

boost::python::list ObjectsUnderWindow(IRect window, bool only_if_fully_in, bool one_of_each, const CFilter &filter, bool just_top_level)
{
	window.MakePositive();

	boost::python::list olist;
	std::list<HeeksObj*> objects;
	theApp->GetObjectsInWindow(window, only_if_fully_in, one_of_each, filter, objects, just_top_level);
	for (std::list<HeeksObj*>::iterator It = objects.begin(); It != objects.end(); It++)
	{
		HeeksObj* object = *It;
		AddObjectToPythonList(object, olist);
	}
	return olist;
}

int BaseObjectGetIndex(BaseObject& object)
{
	return object.GetIndex();
}

int HeeksObjGetIndex(HeeksObj& object)
{
	return object.GetIndex();
}

static GripperSelTransform drag_object_gripper(GripData(GripperTypeTranslate, Point3d(0, 0, 0)), NULL);

Gripper* GetDragGripper(){ return &drag_object_gripper; }

bool GetDigitizeEnd(){ return theApp->digitize_end; }
void SetDigitizeEnd(bool value){ theApp->digitize_end = value; }
bool GetDigitizeInters(){ return theApp->digitize_inters; }
void SetDigitizeInters(bool value){ theApp->digitize_inters = value; }
bool GetDigitizeCentre(){ return theApp->digitize_centre; }
void SetDigitizeCentre(bool value){ theApp->digitize_centre = value; }
bool GetDigitizeMidpoint(){ return theApp->digitize_midpoint; }
void SetDigitizeMidpoint(bool value){ theApp->digitize_midpoint = value; }
bool GetDigitizeNearest(){ return theApp->digitize_nearest; }
void SetDigitizeNearest(bool value){ theApp->digitize_nearest = value; }
bool GetDigitizeCoords(){ return theApp->digitize_coords; }
void SetDigitizeCoords(bool value){ theApp->digitize_coords = value; }
bool GetDigitizeTangent(){ return theApp->digitize_tangent; }
void SetDigitizeTangent(bool value){ theApp->digitize_tangent = value; }
bool GetDigitizeSnapToGrid(){ return theApp->draw_to_grid; }
void SetDigitizeSnapToGrid(bool value){ theApp->draw_to_grid = value; }
double GetDigitizeGridSize(){ return theApp->digitizing_grid; }
void SetDigitizeGridSize(double value){ theApp->digitizing_grid = value; }

HeeksColor GetBackgroundColor(int index)
{
	if (index < 0 || index >= NUM_BACKGROUND_COLORS)
		return HeeksColor(0,0,0);
	return theApp->background_color[index];
}

void SetBackgroundColor(int index, const HeeksColor& color)
{
	if (index >= 0 && index < NUM_BACKGROUND_COLORS)
		theApp->background_color[index] = color;
}

BackgroundMode GetBackgroundMode()
{
	return theApp->m_background_mode;
}

void SetBackgroundMode(BackgroundMode mode)
{
	theApp->m_background_mode = mode;
}

bool GetRotateUpright()
{
	return theApp->m_rotate_mode == 0;
}

void SetRotateUpright(bool upright)
{
	theApp->m_rotate_mode = upright ? 0:1;
}

bool GetShowDatum()
{
	return theApp->m_show_datum_coords_system;
}

void SetShowDatum(bool show)
{
	theApp->m_show_datum_coords_system = show;
}

bool GetDatumSolid()
{
	return theApp->m_datum_coords_system_solid_arrows;
}

void SetDatumSolid(bool solid)
{
	theApp->m_datum_coords_system_solid_arrows = solid;
}

double GetDatumSize()
{
	return CoordinateSystem::size;
}

void SetDatumSize(double size)
{
	CoordinateSystem::size = size;
}

bool GetDatumSizeIsPixelsNotMm()
{
	return CoordinateSystem::size_is_pixels;
}

void SetDatumSizeIsPixelsNotMm(bool p)
{
	CoordinateSystem::size_is_pixels = p;
}

bool GetShowRuler()
{
	return theApp->m_show_ruler;
}

void SetShowRuler(bool show)
{
	theApp->m_show_ruler = show;
}

int GetGridMode()
{
	return theApp->grid_mode;
}

void SetGridMode(int mode)
{
	theApp->grid_mode = mode;
}

bool GetPerspective()
{
	return theApp->m_current_viewport->m_view_point.GetPerspective();
}

void SetPerspective(bool p)
{
	theApp->m_current_viewport->m_view_point.SetPerspective(p);
}


HeeksColor GetCurrentColor()
{
	return theApp->current_color;
}

void SetCurrentColor(const HeeksColor& color)
{
	theApp->current_color = color;
}

bool HeeksObjGetVisible(const HeeksObj& object)
{
	return object.m_visible;
}

void HeeksObjSetVisible(HeeksObj& object, bool visible)
{
	object.m_visible = visible;
}

CTris tris_for_HeeksObjGetTris;

static void GetTrisForHeeksObjGetTris(const double* x, const double* n)
{
	CTri tri;
	tri.x[0][0] = x[0];
	tri.x[0][1] = x[1];
	tri.x[0][2] = x[2];
	tri.x[1][0] = x[3];
	tri.x[1][1] = x[4];
	tri.x[1][2] = x[5];
	tri.x[2][0] = x[6];
	tri.x[2][1] = x[7];
	tri.x[2][2] = x[8];
	tris_for_HeeksObjGetTris.m_tris.push_back(tri);
}

CTris HeeksObjGetTris(HeeksObj& object, double cusp)
{
	tris_for_HeeksObjGetTris.m_tris.clear();
	object.GetTriangles(GetTrisForHeeksObjGetTris, cusp);
	return tris_for_HeeksObjGetTris;
}

CSketch* NewSketchFromCurve(const CCurve& curve)
{
	CSketch* new_sketch = new CSketch();

	std::list<Span> spans;
	curve.GetSpans(spans);

	for (std::list<Span>::iterator It = spans.begin(); It != spans.end(); It++)
	{
		Span &span = *It;
		HeeksObj* new_span = NULL;
		if (span.m_v.m_type == 0)
		{
			new_span = new HLine(Point3d(span.m_p.x, span.m_p.y, 0), Point3d(span.m_v.m_p.x, span.m_v.m_p.y, 0), &theApp->current_color);
		}
		else
		{
			new_span = new HArc(Point3d(span.m_p.x, span.m_p.y, 0), Point3d(span.m_v.m_p.x, span.m_v.m_p.y, 0), Point3d(0, 0, (span.m_v.m_type > 0)? 1:-1), Point3d(span.m_v.m_c.x, span.m_v.m_c.y, 0), &theApp->current_color);
		}

		new_sketch->Add(new_span, NULL);
	}

	return new_sketch;
}


CSketch* NewSketchFromArea(const CArea& area)
{
	CSketch* new_sketch = new CSketch();

	for (std::list<CCurve>::const_iterator CIt = area.m_curves.begin(); CIt != area.m_curves.end(); CIt++)
	{
		const CCurve& curve = *CIt;
		std::list<Span> spans;
		curve.GetSpans(spans);

		for (std::list<Span>::iterator It = spans.begin(); It != spans.end(); It++)
		{
			Span &span = *It;
			HeeksObj* new_span = NULL;
			if (span.m_v.m_type == 0)
			{
				new_span = new HLine(Point3d(span.m_p.x, span.m_p.y, 0), Point3d(span.m_v.m_p.x, span.m_v.m_p.y, 0), &theApp->current_color);
			}
			else
			{
				new_span = new HArc(Point3d(span.m_p.x, span.m_p.y, 0), Point3d(span.m_v.m_p.x, span.m_v.m_p.y, 0), Point3d(0, 0, (span.m_v.m_type > 0) ? 1 : -1), Point3d(span.m_v.m_c.x, span.m_v.m_c.y, 0), &theApp->current_color);
			}

			new_sketch->Add(new_span, NULL);
		}
	}

	return new_sketch;
}

CStlSolid* NewStlSolidFromStl(const CTris& tris)
{
	CStlSolid* new_solid = new CStlSolid();

	for (std::list<CTri>::const_iterator It = tris.m_tris.begin(); It != tris.m_tris.end(); It++)
	{
		const CTri& tri = *It;
		new_solid->AddTriangle(tri.x[0]);
	}

	return new_solid;
}

void RenderSketchAsExtrusion(CSketch& sketch, double start_depth, double final_depth)
{
	CArea area = SketchGetArea(sketch);
	area.Reorder();
	for (std::list<CCurve>::iterator It = area.m_curves.begin(); It != area.m_curves.end(); It++)
	{
		CCurve& curve = *It;
		curve.UnFitArcs();
	}

	std::list<CTris> tri_list;
	area.GetTriangles(tri_list);

	glBegin(GL_TRIANGLES);

	// walls
	for (std::list<CCurve>::iterator It = area.m_curves.begin(); It != area.m_curves.end(); It++)
	{
		CCurve& curve = *It;
		CVertex* prev_vt = NULL;
		for (std::list<CVertex>::iterator VIt = curve.m_vertices.begin(); VIt != curve.m_vertices.end(); VIt++)
		{
			CVertex& vt = *VIt;
			if (prev_vt)
			{
				Point norm = ~(prev_vt->m_p - vt.m_p);
				glNormal3d(norm.x, norm.y, 0);
				glVertex3f(prev_vt->m_p.x, prev_vt->m_p.y, final_depth);
				glVertex3f(vt.m_p.x, vt.m_p.y, final_depth);
				glVertex3f(vt.m_p.x, vt.m_p.y, start_depth);
				glVertex3f(prev_vt->m_p.x, prev_vt->m_p.y, final_depth);
				glVertex3f(vt.m_p.x, vt.m_p.y, start_depth);
				glVertex3f(prev_vt->m_p.x, prev_vt->m_p.y, start_depth);
			}
			prev_vt = &vt;
		}
	}

	for (std::list<CTris>::iterator It = tri_list.begin(); It != tri_list.end(); It++)
	{
		CTris &tris = *It;

		// top layer
		glNormal3f(0.0f, 0.0f, 1.0f);
		for (std::list<CTri>::iterator It2 = tris.m_tris.begin(); It2 != tris.m_tris.end(); It2++)
		{
			CTri& tri = *It2;
			glVertex3f(tri.x[0][0], tri.x[0][1], start_depth);
			glVertex3f(tri.x[1][0], tri.x[1][1], start_depth);
			glVertex3f(tri.x[2][0], tri.x[2][1], start_depth);
		}

		// bottom layer
		glNormal3f(0.0f, 0.0f, -1.0f);
		for (std::list<CTri>::iterator It2 = tris.m_tris.begin(); It2 != tris.m_tris.end(); It2++)
		{
			CTri& tri = *It2;
			glVertex3f(tri.x[0][0], tri.x[0][1], final_depth);
			glVertex3f(tri.x[2][0], tri.x[2][1], final_depth);
			glVertex3f(tri.x[1][0], tri.x[1][1], final_depth);
		}
	}

	glEnd();
}

bool GetAntialiasing(void)
{
	return theApp->m_antialiasing;
}

void SetAntialiasing(bool value)
{
	theApp->m_antialiasing = value;
}

boost::python::tuple TangentialArc(const Point3d &p0, const Point3d &v0, const Point3d &p1)
{
	Point3d centre(0, 0, 0);
	Point3d axis(0, 0, 0);
	bool arc_found = HArc::TangentialArc(p0, v0, p1, centre, axis);

	return boost::python::make_tuple(arc_found, centre, axis);
}

boost::python::tuple GetTangentCirclePy(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3)
{
	Circle c;
	bool found = GetTangentCircle(d1, d2, d3, c);
	return boost::python::make_tuple(found, c);
}

boost::python::tuple GetCircleBetweenPy(const DigitizedPoint& d1, const DigitizedPoint& d2)
{
	Circle c;
	bool found = GetCircleBetween(d1, d2, c);
	return boost::python::make_tuple(found, c);
}

static std::string Filter__str__(const CFilter& self) {
	std::ostringstream ss;
	ss << self;
	return ss.str();
}

HeeksObj* DigitizedPointGetObject1(const DigitizedPoint& d)
{
	return d.m_object1;
}

BOOST_PYTHON_MODULE(cad) {

	boost::python::docstring_options local_docstring_options(true, true, false); // This will enable user-defined docstrings and python signatures, while disabling the C++ signatures

	boost::python::class_<BaseObject, boost::noncopyable >("BaseObject", "derive your custom CAD objects from this")
		.def(boost::python::init<int>())
		.def("GetIDGroupType", &BaseObject::GetIDGroupType)
		.def("GetIconFilePath", &BaseObjectGetIconFilePath)
		.def("GetTitle", &BaseObjectGetTitle)
		.def("GetID", &BaseObject::GetID)
		.def("SetID", &BaseObject::SetID)
		.def("GetIndex", &BaseObjectGetIndex)
		.def("GetColor", &BaseObjectGetColor)
		.def("AutoExpand", &BaseObject::AutoExpand)
		.def("GetNumChildren", &BaseObject::GetNumChildren)
		.def("GetOwner", &ObjectGetOwner, boost::python::return_value_policy<boost::python::reference_existing_object>())
		.def("SetOwner", &ObjectSetOwner)
		.def("GetFirstChild", &BaseObject::GetFirstChild, boost::python::return_value_policy<boost::python::reference_existing_object>())
		.def("GetNextChild", &BaseObject::GetNextChild, boost::python::return_value_policy<boost::python::reference_existing_object>())
		.def("GetChildren", &BaseObjectGetChildren)
		.def("Clear", static_cast< void (BaseObject::*)(void) >(&BaseObject::Clear))
		.def("CanAdd", &BaseObject::CanAdd)
		.def("CanAddTo", &BaseObject::CanAddTo)
		.def("CanBeDeleted", &HeeksObj::CanBeRemoved)
		.def("CanBeCopied", &HeeksObj::CanBeCopied)
		.def("OneOfAKind", &BaseObject::OneOfAKind_default)
		.def("CopyFrom", &ObjListCopyFrom)
		.def("GetProperties", &HeeksObjGetProperties)
		.def("GetBaseProperties", &HeeksObjGetBaseProperties)
		.def("GetChildrensBox", &ObjListGetBox)
		.def("ReadObjectXml", &HeeksObjReadObjectXml)
		.def("Clear", &ObjListClear)
		.def("Add", &ObjListAdd)
		.def("GetCopyFromObject", &BaseObject::GetCopyFromObject, boost::python::return_value_policy<boost::python::reference_existing_object>())
		.def("AddTriangle", &BaseObject::AddTriangle)
		.def("GetVisible", &HeeksObjGetVisible)
		;

	boost::python::class_<HeeksObj, boost::noncopyable>("Object")
		.def(boost::python::init<HeeksObj>())
		.def("GetType", &HeeksObj::GetType)
		.def("GetIDGroupType", &HeeksObj::GetIDGroupType)
		.def("GetTypeString", HeeksObjGetTypeString)
		.def("GetIconFilePath", &HeeksObjGetIconFilePath)
		.def("GetID", &HeeksObj::GetID)
		.def("SetID", &HeeksObj::SetID)
		.def("GetIndex", &HeeksObjGetIndex)
		.def("KillGLLists", &HeeksObj::KillGLLists)
		.def("GetColor", &HeeksObjGetColor)
		.def("SetColor", &HeeksObj::SetColor)
		.def("HasEdit", &HeeksObjHasEdit)
		.def("GetTitle", &HeeksObjGetTitle)
		.def("GetBox", &HeeksObjGetBox)
		.def("OnGlCommands", &HeeksObj::glCommands)
		.def("AutoExpand", &HeeksObj::AutoExpand)
		.def("GetNumChildren", &HeeksObj::GetNumChildren)
		.def("GetOwner", &ObjectGetOwner, boost::python::return_value_policy<boost::python::reference_existing_object>())
		.def("SetOwner", &ObjectSetOwner)
		.def("GetFirstChild", &HeeksObj::GetFirstChild, boost::python::return_value_policy<boost::python::reference_existing_object>())
		.def("GetNextChild", &HeeksObj::GetNextChild, boost::python::return_value_policy<boost::python::reference_existing_object>())
		.def("CanAdd", &HeeksObj::CanAdd)
		.def("CanAddTo", &HeeksObj::CanAddTo)
		.def("CanBeDeleted", &HeeksObj::CanBeRemoved)
		.def("OneOfAKind", &HeeksObj::OneOfAKind)
		.def("CanBeCopied", &HeeksObj::CanBeCopied)
		.def("CopyFrom", &HeeksObj::CopyFrom)
		.def("ReadXml", &HeeksObjReadFromXML)
		.def("ReadObjectXml", &HeeksObjReadObjectXml)
		.def("WriteXml", &HeeksObjWriteToXML)
		.def("WriteObjectXml", &HeeksObjWriteObjectToXML)
		.def("GetProperties", &HeeksObjGetProperties)
		.def("GetLines", &HeeksObjGetLines)
		.def("SetStartPoint", &HeeksObj::SetStartPoint)
		.def("GetStartPoint", &HeeksObjGetStartPoint)
		.def("GetEndPoint", &HeeksObjGetEndPoint)
		.def("SetEndPoint", &HeeksObj::SetEndPoint)
		.def("GetCentrePoint", &HeeksObjGetCentrePoint)
		.def("SetCentrePoint", &HeeksObj::SetCentrePoint)
		.def("MakeACopy", &HeeksObj::MakeACopy, boost::python::return_value_policy<boost::python::reference_existing_object>())
		.def("Clear", &HeeksObj::Clear)
		.def("Add", &ObjAdd)
		.def("GetOrigin", &ObjGetOrigin)
		.def("Transform", &HeeksObj::Transform)
		.def("GetVisible", &HeeksObjGetVisible)
		.def("SetVisible", &HeeksObjSetVisible)
		.def("GetTris", HeeksObjGetTris)
		;

	boost::python::class_<Gripper, boost::python::bases<HeeksObj>, boost::noncopyable>("Gripper")
		.def(boost::python::init<Gripper>())
		.def("OnGripperGrabbed", &Gripper::OnGripperGrabbed)
		.def("OnGripperMoved", &Gripper::OnGripperMoved)
		.def("OnGripperReleased", &Gripper::OnGripperReleased)
		;

	boost::python::class_<GripData>("GripData", boost::python::no_init)
		.def("__init__", boost::python::make_constructor(&initGripData))
		;

	boost::python::enum_<EnumGripperType>("GripperType")
		.value("Translate", GripperTypeTranslate)
		.value("Rotate", GripperTypeRotate)
		.value("RotateObject", GripperTypeRotateObject)
		.value("RotateObjectXY", GripperTypeRotateObjectXY)
		.value("RotateObjectXZ", GripperTypeRotateObjectXZ)
		.value("RotateObjectYZ", GripperTypeRotateObjectYZ)
		.value("Scale", GripperTypeScale)
		.value("ObjectScaleX", GripperTypeObjectScaleX)
		.value("ObjectScaleY", GripperTypeObjectScaleY)
		.value("ObjectScaleZ", GripperTypeObjectScaleZ)
		.value("ObjectScaleXY", GripperTypeObjectScaleXY)
		.value("Angle", GripperTypeAngle)
		.value("Stretch", GripperTypeStretch)
		;

	boost::python::class_<HeeksColor>("Color")
		.def(boost::python::init<HeeksColor>())
		.def(boost::python::init<unsigned char, unsigned char, unsigned char>())
		.def(boost::python::init<long>())
		.def_readwrite("red", &HeeksColor::red)
		.def_readwrite("green", &HeeksColor::green)
		.def_readwrite("blue", &HeeksColor::blue)
		.def("ref", &HeeksColor::COLORREF_color)
		.def("SetGlColor", &HeeksColor::glColor)
		;

	boost::python::class_<Material>("Material")
		.def(boost::python::init<Material>())
		.def(boost::python::init<const HeeksColor&>())
		.def("glMaterial", &Material::glMaterial)
		;

	boost::python::class_<PropertyWrap, boost::noncopyable >("Property")
		.def(boost::python::init<int, std::wstring, HeeksObj*>())
		.def("GetType", &Property::get_property_type)
		.def("GetTitle", &PropertyGetShortString)
		.def("GetString", &PropertyGetString)
		.def("GetDouble", &Property::GetDouble)
		.def("GetInt", &Property::GetInt)
		.def("GetBool", &Property::GetBool)
		.def("GetColor", &PropertyGetColor)
		.def("GetChoices", &PropertyGetChoices)
		.def_readwrite("editable", &PropertyWrap::m_editable)
		.def_readwrite("object", &PropertyWrap::m_object)
		.def("GetProperties", &PropertyGetProperties)
		;

	boost::python::class_<ObjList, boost::python::bases<HeeksObj>, boost::noncopyable>("ObjList")
		.def(boost::python::init<ObjList>())
		.def("ReadXml", &ObjListReadFromXML)
		.def("WriteXml", &ObjListWriteToXML)
		.def("CopyFrom", &ObjListCopyFrom)
		.def("GetChildrensBox", &ObjListGetBox)
		;

	boost::python::class_<IdNamedObj, boost::python::bases<HeeksObj>, boost::noncopyable>("IdNamedObj")
		.def(boost::python::init<IdNamedObj>())
		;

	boost::python::class_<IdNamedObjList, boost::python::bases<ObjList>, boost::noncopyable>("IdNamedObjList")
		.def(boost::python::init<IdNamedObjList>())
		.def_readwrite("title", &IdNamedObjList::m_title)
		.def_readwrite("title_made_from_id", &IdNamedObjList::m_title_made_from_id)
		.def("SetTitle", &IdNamedObjListSetTitle)
		;

	boost::python::class_<CSketch, boost::python::bases<IdNamedObjList>, boost::noncopyable>("Sketch")
		.def(boost::python::init<CSketch>())
		.def("GetStartPoint", &SketchGetStartPoint)
		.def("GetEndPoint", &SketchGetEndPoint)
		.def("IsCircle", &CSketch::IsCircle)
		.def("IsClosed", &CSketch::IsClosed)
		.def("HasMultipleSketches", &CSketch::HasMultipleSketches)
		.def("Split", &SketchSplit)
		.def("GetCircleDiameter", &SketchGetCircleDiameter)
		.def("GetCircleCentre", &SketchGetCircleCentre)
		.def("WriteDxf", &SketchWriteDXF)
		.def("GetSketchOrder", &CSketch::GetSketchOrder)
		.def("ReOrderSketch", &CSketch::ReOrderSketch)
		.def("GetCurve", &SketchGetCurve)
		.def("GetArea", &SketchGetArea)
		.def("RenderAsExtrusion", &RenderSketchAsExtrusion)
		;

	boost::python::class_<HCircle, boost::python::bases<IdNamedObj> >("Circle", boost::python::no_init)
		.def("__init__", boost::python::make_constructor(&initHCircle))
		.def("GetArea", &CircleGetArea)
		.def("SetCircle", &HCircle::SetCircle)
		.def_readwrite("C", &HCircle::m_c)
		.def_readwrite("radius", &HCircle::m_radius)
		.def_readwrite("axis", &HCircle::m_axis)
		;

	boost::python::class_<HPoint, boost::python::bases<IdNamedObj> >("Point", boost::python::no_init)
		.def("__init__", boost::python::make_constructor(&initHPoint))
//			.def_readwrite("p", &HPoint::m_p)
		;

	boost::python::class_<EndedObject, boost::python::bases<HeeksObj> >("EndedObject", boost::python::no_init)
		.def_readwrite("A", &EndedObject::A)
		.def_readwrite("B", &EndedObject::B)
		;

	boost::python::class_<HLine, boost::python::bases<EndedObject> >("Line", boost::python::no_init)
		;

	boost::python::class_<HILine, boost::python::bases<EndedObject> >("ILine", boost::python::no_init)
		.def("__init__", boost::python::make_constructor(&initHILine))
		;

	boost::python::class_<HArc, boost::python::bases<EndedObject> >("Arc", boost::python::no_init)
		.def_readwrite("C", &HArc::C)
		.def_readwrite("axis", &HArc::m_axis)
		;

	boost::python::class_<CStlSolid, boost::python::bases<HeeksObj> >("StlSolid")
		.def(boost::python::init<CStlSolid>())
		.def("__init__", boost::python::make_constructor(&initStlSolid))
		.def(boost::python::init<const std::wstring&>())// load a stl solid from a filepath
		.def("WriteSTL", &StlSolidWriteSTL) ///function WriteSTL///params float tolerance, string filepath///writes an STL file for the body to the given tolerance
		.def("NumTriangles", StlSolidNumTriangles)
		;

	boost::python::class_<CoordinateSystem, boost::python::bases<HeeksObj> >("CoordinateSystem")
		.def(boost::python::init<CoordinateSystem>())
		.def(boost::python::init<const std::wstring&, const Point3d &, const Point3d &, const Point3d &>()) //, "create with name, origin, x-axis and y-axis"
		.def_readwrite("o", &CoordinateSystem::m_o)
		.def_readwrite("x", &CoordinateSystem::m_x)
		.def_readwrite("y", &CoordinateSystem::m_y)
		.def_readwrite("title", &CoordinateSystem::m_title)
	;

	boost::python::class_<PropertyCheck, boost::noncopyable, boost::python::bases<Property> >("PropertyCheck", boost::python::no_init);
	boost::python::class_<PropertyChoice, boost::python::bases<Property> >("PropertyChoice", boost::python::no_init);
	boost::python::class_<PropertyColor, boost::python::bases<Property> >("PropertyColor", boost::python::no_init);
	boost::python::class_<PropertyDouble, boost::python::bases<Property> >("PropertyDouble", boost::python::no_init);
	boost::python::class_<PropertyDoubleScaled, boost::python::bases<Property> >("PropertyDoubleScaled", boost::python::no_init);
	boost::python::class_<PropertyLengthScaled, boost::python::bases<PropertyDoubleScaled> >("PropertyLengthScaled", boost::python::no_init);
	boost::python::class_<PropertyDoubleLimited, boost::python::bases<PropertyDouble> >("PropertyDoubleLimited", boost::python::no_init);
	boost::python::class_<PropertyString, boost::python::bases<Property> >("PropertyString", boost::python::no_init);
	boost::python::class_<PropertyStringReadOnly, boost::python::bases<Property> >("PropertyStringReadOnly", boost::python::no_init)
		.def("__init__", boost::python::make_constructor(&initPropertyStringReadOnly))
		;
	boost::python::class_<PropertyFile, boost::python::bases<PropertyString> >("PropertyFile", boost::python::no_init);
	boost::python::class_<PropertyInt, boost::python::bases<Property> >("PropertyInt", boost::python::no_init);
	boost::python::class_<PropertyLength, boost::python::bases<PropertyDouble> >("PropertyLength", boost::python::no_init);
	boost::python::class_<PropertyLengthWithKillGLLists, boost::python::bases<PropertyLength> >("PropertyLengthWithKillGLLists", boost::python::no_init);
	boost::python::class_<PropertyList, boost::python::bases<Property> >("PropertyList", boost::python::no_init);
	boost::python::class_<PropertyObjectTitle, boost::python::bases<Property> >("PropertyObjectTitle", boost::python::no_init);
	boost::python::class_<PropertyObjectColor, boost::python::bases<Property> >("PropertyObjectColor", boost::python::no_init);

	boost::python::class_<Undoable, boost::noncopyable>("Undoable", boost::python::no_init);

	boost::python::class_<UndoableWrap, boost::noncopyable >("BaseUndoable");

	boost::python::class_<PropertyChangeString, boost::python::bases<Undoable> >("PropertyChangeString", boost::python::no_init).def(boost::python::init<const std::wstring&, Property*>());
	boost::python::class_<PropertyChangeDouble, boost::python::bases<Undoable> >("PropertyChangeDouble", boost::python::no_init).def(boost::python::init<const double&, Property*>());
	boost::python::class_<PropertyChangeLength, boost::python::bases<Undoable> >("PropertyChangeLength", boost::python::no_init).def(boost::python::init<const double&, Property*>());
	boost::python::class_<PropertyChangeInt, boost::python::bases<Undoable> >("PropertyChangeInt", boost::python::no_init).def(boost::python::init<const int&, Property*>());
	boost::python::class_<PropertyChangeColor, boost::python::bases<Undoable> >("PropertyChangeColor", boost::python::no_init).def(boost::python::init<const HeeksColor&, Property*>());
	boost::python::class_<PropertyChangeChoice, boost::python::bases<Undoable> >("PropertyChangeChoice", boost::python::no_init).def(boost::python::init<const int&, Property*>());
	boost::python::class_<PropertyChangeCheck, boost::python::bases<Undoable> >("PropertyChangeCheck", boost::python::no_init).def(boost::python::init<const bool&, Property*>());

	boost::python::class_<CApp, boost::python::bases<ObjList>, boost::noncopyable>("App", boost::python::no_init);

	boost::python::class_<CViewPoint>("ViewPoint", boost::python::no_init)
		.def("SetView", &CViewPoint::SetView)
		.def("Unproject", &CViewPoint::glUnproject)
		.def("ShiftI", &CViewPoint::ShiftI)
		.def("TurnI", &CViewPoint::TurnI)
		.def("TurnVerticalI", &CViewPoint::TurnVerticalI)
		.def("Rightwards", &CViewPoint::rightwards_vector)
		.def("Forwards", &CViewPoint::forwards_vector)
		.def_readwrite("lens_point", &CViewPoint::m_lens_point)
		.def_readwrite("target_point", &CViewPoint::m_target_point)
		.def_readwrite("vertical", &CViewPoint::m_vertical)
		;

	boost::python::class_<CViewport>("Viewport")
		.def(boost::python::init<int, int>())
		.def("glCommands", &CViewport::glCommands)
		.def("WidthAndHeightChanged", &CViewport::WidthAndHeightChanged)
		.def("OnMagExtents", &CViewport::OnMagExtents)
		.def("RestorePreviousViewPoint", &CViewport::RestorePreviousViewPoint)
		.def("ClearViewpoints", &CViewport::ClearViewpoints)
		.def("StoreViewPoint", &CViewport::StoreViewPoint)
		.def("GetHeight", &ViewportGetHeight)
		.def("GetWidth", &ViewportGetWidth)
		.def("SetView", &ViewportSetView)
		.def("SetXOR", &CViewport::SetXOR)
		.def("EndXOR", &CViewport::EndXOR)
		.def("DrawWindow", &CViewport::DrawWindow)
		.def("OnWheelRotation", &CViewport::OnWheelRotation)
		.def_readwrite("orthogonal", &CViewport::m_orthogonal)
		.def_readwrite("view_point", &CViewport::m_view_point)
		;

	boost::python::enum_<MouseEventType>("MouseEventType")
	.value("MouseEventNull", MouseEventNull)
	.value("MouseEventLeftDown", MouseEventLeftDown)
	.value("MouseEventLeftUp", MouseEventLeftUp)
	.value("MouseEventLeftDClick", MouseEventLeftDClick)
	.value("MouseEventRightDown", MouseEventRightDown)
	.value("MouseEventRightUp", MouseEventRightUp)
	.value("MouseEventMiddleDown", MouseEventMiddleDown)
	.value("MouseEventMiddleUp", MouseEventMiddleUp)
	.value("MouseEventMovingOrDragging", MouseEventMovingOrDragging)
	.value("MouseEventWheelRotation", MouseEventWheelRotation)
		;

	boost::python::class_<MouseEvent>("MouseEvent")
		.def(boost::python::init<MouseEvent>())
		.def("LeftDown", &MouseEvent::LeftDown)
		.def("LeftUp", &MouseEvent::LeftUp)
		.def("LeftDClick", &MouseEvent::LeftDClick)
		.def("RightDown", &MouseEvent::RightDown)
		.def("RightUp", &MouseEvent::RightUp)
		.def("MiddleDown", &MouseEvent::MiddleDown)
		.def("MiddleUp", &MouseEvent::MiddleUp)
		.def("Moving", &MouseEvent::Moving)
		.def("GetWheelRotation", &MouseEvent::GetWheelRotation)
		.def_readwrite("event_type", &MouseEvent::m_event_type)
		.def_readwrite("x", &MouseEvent::m_x)
		.def_readwrite("y", &MouseEvent::m_y)
		.def_readwrite("leftDown", &MouseEvent::m_leftDown)
		.def_readwrite("middleDown", &MouseEvent::m_middleDown)
		.def_readwrite("rightDown", &MouseEvent::m_rightDown)
		.def_readwrite("controlDown", &MouseEvent::m_controlDown)
		.def_readwrite("shiftDown", &MouseEvent::m_shiftDown)
		.def_readwrite("altDown", &MouseEvent::m_altDown)
		.def_readwrite("metaDown", &MouseEvent::m_metaDown)
		.def_readwrite("wheelRotation", &MouseEvent::m_wheelRotation)
		.def_readwrite("wheelDelta", &MouseEvent::m_wheelDelta)
		.def_readwrite("linesPerAction", &MouseEvent::m_linesPerAction)
		;

	boost::python::class_<IPoint>("IPoint")
		.def(boost::python::init<int, int>())
		.def_readwrite("x", &IPoint::x)
		.def_readwrite("y", &IPoint::y)
		;
	boost::python::class_<IRect>("IRect")
		.def(boost::python::init<int, int, int, int>())
		.def(boost::python::init<int, int>())
		.def_readwrite("x", &IRect::x)
		.def_readwrite("y", &IRect::y)
		.def_readwrite("width", &IRect::width)
		.def_readwrite("height", &IRect::height)
		;

	boost::python::class_<CFilter>("Filter")
		.def(boost::python::init<int>())
		.def("Clear", &CFilter::Clear)
		.def("AddType", &CFilter::AddType)
		.def("IsTypeInFilter", &CFilter::IsTypeInFilter)
		.def("Size", &CFilter::Size)
		.def_readwrite("empty_means_none", &CFilter::m_empty_means_none)
		.def("__str__", Filter__str__);
	;

	boost::python::class_<ObserverWrap, boost::noncopyable >("Observer")
		.def(boost::python::init<ObserverWrap>())
		;

	boost::python::enum_<KeyCode>("KeyCode")
		.value("None", K_NONE)
		.value("ControlA", K_CONTROL_A)
		.value("ControlB", K_CONTROL_B)
		.value("ControlC", K_CONTROL_C)
		.value("ControlD", K_CONTROL_D)
		.value("ControlE", K_CONTROL_E)
		.value("ControlF", K_CONTROL_F)
		.value("ControlG", K_CONTROL_G)
		.value("ControlH", K_CONTROL_H)
		.value("ControlI", K_CONTROL_I)
		.value("ControlJ", K_CONTROL_J)
		.value("ControlK", K_CONTROL_K)
		.value("ControlL", K_CONTROL_L)
		.value("ControlM", K_CONTROL_M)
		.value("ControlN", K_CONTROL_N)
		.value("ControlO", K_CONTROL_O)
		.value("ControlP", K_CONTROL_P)
		.value("ControlQ", K_CONTROL_Q)
		.value("ControlR", K_CONTROL_R)
		.value("ControlS", K_CONTROL_S)
		.value("ControlT", K_CONTROL_T)
		.value("ControlU", K_CONTROL_U)
		.value("ControlV", K_CONTROL_V)
		.value("ControlW", K_CONTROL_W)
		.value("ControlX", K_CONTROL_X)
		.value("ControlY", K_CONTROL_Y)
		.value("ControlZ", K_CONTROL_Z)
		.value("Back", K_BACK)
		.value("Tab", K_TAB)
		.value("Return", K_RETURN)
		.value("Escape", K_ESCAPE)
		.value("Space", K_SPACE)
		.value("Delete", K_DELETE)
		.value("Start", K_START)
		.value("LButton", K_LBUTTON)
		.value("RButton", K_RBUTTON)
		.value("Cancel", K_CANCEL)
		.value("MButton", K_MBUTTON)
		.value("Clear", K_CLEAR)
		.value("Shift", K_SHIFT)
		.value("Alt", K_ALT)
		.value("Control", K_CONTROL)
		.value("Menu", K_MENU)
		.value("Pause", K_PAUSE)
		.value("Capital", K_CAPITAL)
		.value("End", K_END)
		.value("Home", K_HOME)
		.value("Left", K_LEFT)
		.value("Up", K_UP)
		.value("Right", K_RIGHT)
		.value("Down", K_DOWN)
		.value("Select", K_SELECT)
		.value("Print", K_PRINT)
		.value("Execute", K_EXECUTE)
		.value("Sanpshot", K_SNAPSHOT)
		.value("Insert", K_INSERT)
		.value("Help", K_HELP)
		.value("Numpad0", K_NUMPAD0)
		.value("Numpad1", K_NUMPAD1)
		.value("Numpad2", K_NUMPAD2)
		.value("Numpad3", K_NUMPAD3)
		.value("Numpad4", K_NUMPAD4)
		.value("Numpad5", K_NUMPAD5)
		.value("Numpad6", K_NUMPAD6)
		.value("Numpad7", K_NUMPAD7)
		.value("Numpad8", K_NUMPAD8)
		.value("Numpad9", K_NUMPAD9)
		.value("Multiply", K_MULTIPLY)
		.value("Add", K_ADD)
		.value("Separator", K_SEPARATOR)
		.value("Subtract", K_SUBTRACT)
		.value("Decimal", K_DECIMAL)
		.value("Divide", K_DIVIDE)
		.value("F1", K_F1)
		.value("F2", K_F2)
		.value("F3", K_F3)
		.value("F4", K_F4)
		.value("F5", K_F5)
		.value("F6", K_F6)
		.value("F7", K_F7)
		.value("F8", K_F8)
		.value("F9", K_F9)
		.value("F10", K_F10)
		.value("F11", K_F11)
		.value("F12", K_F12)
		.value("F13", K_F13)
		.value("F14", K_F14)
		.value("F15", K_F15)
		.value("F16", K_F16)
		.value("F17", K_F17)
		.value("F18", K_F18)
		.value("F19", K_F19)
		.value("F20", K_F20)
		.value("F21", K_F21)
		.value("F22", K_F22)
		.value("F23", K_F23)
		.value("F24", K_F24)
		.value("Numlock", K_NUMLOCK)
		.value("Scroll", K_SCROLL)
		.value("PageUp", K_PAGEUP)
		.value("PageDown", K_PAGEDOWN)
		.value("NumpadSpace", K_NUMPAD_SPACE)
		.value("NumpadTab", K_NUMPAD_TAB)
		.value("NumpadEnter", K_NUMPAD_ENTER)
		.value("NumpadF1", K_NUMPAD_F1)
		.value("NumpadF2", K_NUMPAD_F2)
		.value("NumpadF3", K_NUMPAD_F3)
		.value("NumpadF4", K_NUMPAD_F4)
		.value("NumpadHome", K_NUMPAD_HOME)
		.value("NumpadLeft", K_NUMPAD_LEFT)
		.value("NumpadUp", K_NUMPAD_UP)
		.value("NumpadRight", K_NUMPAD_RIGHT)
		.value("NumpadDown", K_NUMPAD_DOWN)
		.value("NumpadPageUp", K_NUMPAD_PAGEUP)
		.value("NumpadPageDown", K_NUMPAD_PAGEDOWN)
		.value("NumpadEnd", K_NUMPAD_END)
		.value("NumpadBegin", K_NUMPAD_BEGIN)
		.value("NumpadInsert", K_NUMPAD_INSERT)
		.value("NumpadDelete", K_NUMPAD_DELETE)
		.value("NumpadEqual", K_NUMPAD_EQUAL)
		.value("NumpadMultiply", K_NUMPAD_MULTIPLY)
		.value("NumpadAdd", K_NUMPAD_ADD)
		.value("NumpadSeparator", K_NUMPAD_SEPARATOR)
		.value("NumpadSubtract", K_NUMPAD_SUBTRACT)
		.value("NumpadDecimal", K_NUMPAD_DECIMAL)
		.value("NumpadDivide", K_NUMPAD_DIVIDE)
		.value("WindowsLeft", K_WINDOWS_LEFT)
		.value("WindowsRight", K_WINDOWS_RIGHT)
		.value("WindowsMenu", K_WINDOWS_MENU)
		.value("RawControl", K_RAW_CONTROL)
		.value("Command", K_COMMAND)
		.value("Special1", K_SPECIAL1)
		.value("Special2", K_SPECIAL2)
		.value("Special3", K_SPECIAL3)
		.value("Special4", K_SPECIAL4)
		.value("Special5", K_SPECIAL5)
		.value("Special6", K_SPECIAL6)
		.value("Special7", K_SPECIAL7)
		.value("Special8", K_SPECIAL8)
		.value("Special9", K_SPECIAL9)
		.value("Special10", K_SPECIAL10)
		.value("Special11", K_SPECIAL11)
		.value("Special12", K_SPECIAL12)
		.value("Special13", K_SPECIAL13)
		.value("Special14", K_SPECIAL14)
		.value("Special15", K_SPECIAL15)
		.value("Special16", K_SPECIAL16)
		.value("Special17", K_SPECIAL17)
		.value("Special18", K_SPECIAL18)
		.value("Special19", K_SPECIAL19)
		.value("Special20", K_SPECIAL20)
		;

	boost::python::enum_<DigitizeType>("DigitizeType")
		.value("DIGITIZE_NO_ITEM_TYPE", DigitizeNoItemType)
		.value("DIGITIZE_ENDOF_TYPE", DigitizeEndofType)
		.value("DIGITIZE_INTER_TYPE", DigitizeIntersType)
		.value("DIGITIZE_MIDPOINT_TYPE", DigitizeMidpointType)
		.value("DIGITIZE_CENTRE_TYPE", DigitizeCentreType)
		.value("DIGITIZE_SCREEN_TYPE", DigitizeScreenType)
		.value("DIGITIZE_COORDS_TYPE", DigitizeCoordsType)
		.value("DIGITIZE_NEAREST_TYPE", DigitizeNearestType)
		.value("DIGITIZE_TANGENT_TYPE", DigitizeTangentType)
		.value("DIGITIZE_INPUT_TYPE", DigitizeInputType)
		;

	boost::python::enum_<SketchOrderType>("SketchOrderType")
		.value("SketchOrderTypeUnknown", SketchOrderTypeUnknown)
		.value("SketchOrderTypeEmpty", SketchOrderTypeEmpty)
		.value("SketchOrderTypeOpen", SketchOrderTypeOpen)
		.value("SketchOrderTypeReverse", SketchOrderTypeReverse)
		.value("SketchOrderTypeBad", SketchOrderTypeBad)
		.value("SketchOrderTypeReOrder", SketchOrderTypeReOrder)
		.value("SketchOrderTypeCloseCW", SketchOrderTypeCloseCW)
		.value("SketchOrderTypeCloseCCW", SketchOrderTypeCloseCCW)
		.value("SketchOrderTypeMultipleCurves", SketchOrderTypeMultipleCurves)
		.value("SketchOrderHasCircles", SketchOrderHasCircles)
		.value("MaxSketchOrderTypes", MaxSketchOrderTypes)
		;

	boost::python::enum_<BackgroundMode>("BackgroundMode")
		.value("OneColor", BackgroundModeOneColor)
		.value("TwoColors", BackgroundModeTwoColors)
		.value("TwoColorsLeftToRight", BackgroundModeTwoColorsLeftToRight)
		.value("FourColors", BackgroundModeFourColors)
		.value("SkyDome", BackgroundModeSkyDome)
		;

	boost::python::class_<DigitizedPoint>("DigitizedPoint")
		.def(boost::python::init<Point3d, DigitizeType>())
		.def(boost::python::init<DigitizedPoint>())
		.def_readwrite("point", &DigitizedPoint::m_point)
		.def_readwrite("type", &DigitizedPoint::m_type)
		.def("GetObject1", &DigitizedPointGetObject1, boost::python::return_value_policy<boost::python::reference_existing_object>())
		;

	boost::python::def("OnExit", OnExit);
	boost::python::def("Reset", CadReset);
	boost::python::def("OpenFile", CadOpenFile);
	boost::python::def("Import", &CadImport, CadImportOverloads((boost::python::arg("filepath"), boost::python::arg("paste_into") = NULL)));
	boost::python::def("SaveFile", CadSaveFile);
	boost::python::def("SaveObjects", SaveObjects);
	boost::python::def("DrawTriangle", &DrawTriangle);
	boost::python::def("DrawEnableLighting", &DrawEnableLighting);
	boost::python::def("DrawDisableLighting", &DrawDisableLighting);
	boost::python::def("DrawEnableDepthTesting", &DrawEnableDepthTesting);
	boost::python::def("DrawDisableDepthTesting", &DrawDisableDepthTesting);
	boost::python::def("DrawEnableCullFace", &DrawEnableCullFace);
	boost::python::def("DrawDisableCullFace", &DrawDisableCullFace);
	boost::python::def("DrawLine", &DrawLine);
	boost::python::def("DrawColor", &DrawColor);
	boost::python::def("DrawContrastBlackOrWhite", &DrawContrastBlackOrWhite);
	boost::python::def("DrawTranslate", &DrawTranslate);
	boost::python::def("DrawSymbol", &DrawSymbol, "Use glBitmap to draw a symbol of a limit collection of types at the given position");
	boost::python::def("BeginTriangles", &BeginTriangles);
	boost::python::def("BeginLines", &BeginLines);
	boost::python::def("EndLinesOrTriangles", &EndLinesOrTriangles);
	boost::python::def("GlVertex", &glVertexPoint3d);
	boost::python::def("GlVertex2D", &glVertexPoint2d);
	boost::python::def("GlLineWidth", &GlLineWidth);
	boost::python::def("DrawNewList", &DrawNewList);
	boost::python::def("DrawEndList", &DrawEndList);
	boost::python::def("DrawCallList", &DrawCallList);
	boost::python::def("DrawDeleteList", &DrawDeleteList);
	boost::python::def("DrawEnableLights", &DrawEnableLights);
	boost::python::def("DrawDisableLights", &DrawDisableLights);
	boost::python::def("GenTexture", &GenTexture);
	boost::python::def("DrawImageQuads", DrawImageQuads);
	boost::python::def("DrawTris", DrawTris);
	boost::python::def("DrawMultMatrix", DrawMultMatrix);
	boost::python::def("DrawPushMatrix", DrawPushMatrix);
	boost::python::def("DrawPopMatrix", DrawPopMatrix);
	boost::python::def("DrawObjectsOnFront", DrawObjectsOnFront);	
	boost::python::def("RenderScreenTextAt", &RenderScreenTextAt);
	boost::python::def("RenderScreenText", &RenderScreenText);
	boost::python::def("GetObjectFromId", &GetObjectFromId, boost::python::args("type", "id"), "returns the object of given type with given id, or None");
	boost::python::def("RegisterObjectType", &RegisterObjectType, RegisterObjectTypeOverloads((boost::python::arg("name"), boost::python::arg("callback"), boost::python::arg("add_to_filter") = true)));
	boost::python::def("GetObjectNamesAndTypes", GetObjectNamesAndTypes);
	boost::python::def("SetXmlValue", SetXmlValue);
	boost::python::def("BeginXmlChild", BeginXmlChild);
	boost::python::def("EndXmlChild", EndXmlChild);
	boost::python::def("GetXmlObject", &GetXmlObject);
	boost::python::def("GetXmlValue", &GetXmlValue, GetXmlValueOverloads((boost::python::arg("name"), boost::python::arg("default_value") = std::wstring(L""))));
	boost::python::def("GetXmlText", &GetXmlText);
	boost::python::def("SetXmlText", &SetXmlText);
	boost::python::def("GetXmlBool", &GetXmlBool, GetXmlBoolOverloads((boost::python::arg("name"), boost::python::arg("default_value") = false)));
	boost::python::def("GetXmlInt", &GetXmlInt, GetXmlIntOverloads((boost::python::arg("name"), boost::python::arg("default_value") = 0)));
	boost::python::def("GetXmlFloat", &GetXmlFloat, GetXmlFloatOverloads((boost::python::arg("name"), boost::python::arg("default_value") = 0.0)));
	boost::python::def("GetXmlMatrix", &GetXmlMatrix, GetXmlMatrixOverloads((boost::python::arg("name"), boost::python::arg("default_value") = NULL)));
	boost::python::def("SetXmlMatrix", SetXmlMatrix);
	boost::python::def("ReturnFromXmlChild", ReturnFromXmlChild);
	boost::python::def("GetFirstXmlChild", GetFirstXmlChild);
	boost::python::def("GetNextXmlChild", GetNextXmlChild);
	boost::python::def("OpenXmlFile", &OpenXmlFile, OpenXMLFileOverloads((boost::python::arg("filepath"), boost::python::arg("paste_into") = NULL, boost::python::arg("paste_before") = NULL)));
	boost::python::def("RegisterOnEndXmlWrite", RegisterOnEndXmlWrite);
	boost::python::def("RegisterObserver", RegisterObserver);
	boost::python::def("RegisterOnRepaint", RegisterOnRepaint);
	boost::python::def("RegisterOnGLCommands", RegisterOnGLCommands);
	boost::python::def("Repaint", &PythonOnRepaint, PythonOnRepaintOverloads((boost::python::arg("soon") = false)));
	boost::python::def("RegisterMessageBoxCallback", RegisterMessageBoxCallback);
	boost::python::def("RegisterImportFileType", RegisterImportFileType);
	boost::python::def("RegisterExportFileType", RegisterExportFileType);
	boost::python::def("GetFilePathForImportExport", GetFilePathForImportExport);
	boost::python::def("GetResFolder", GetResFolder);
	boost::python::def("SetResFolder", SetResFolder);
	boost::python::def("MessageBox", CadMessageBox);
	boost::python::def("GetSelectedObjects", GetSelectedObjects);
	boost::python::def("GetSelectionTypes", GetSelectionTypes, boost::python::return_value_policy<boost::python::reference_existing_object>());
	boost::python::def("GetNumSelected", GetNumSelected);
	boost::python::def("GetObjects", GetObjects);
	boost::python::def("ObjectMarked", ObjectMarked);
	boost::python::def("Select", &Select, SelectOverloads(	(boost::python::arg("object"),	boost::python::arg("CallOnChanged") = NULL)));
	boost::python::def("Unselect", Unselect);
	boost::python::def("ClearSelection", &ClearSelection, ClearSelectionOverloads(boost::python::arg("CallOnChanged") = false));
	boost::python::def("GetSelectionProperties", GetSelectionProperties);
	boost::python::def("SetMarkNewlyAddedObjects", SetMarkNewlyAddedObjects);
	boost::python::def("GetMarkNewlyAddedObjects", GetMarkNewlyAddedObjects);
	boost::python::def("GetViewUnits", GetViewUnits);
	boost::python::def("SetViewUnits", SetViewUnits);
	boost::python::def("GetApp", GetApp, boost::python::return_value_policy<boost::python::reference_existing_object>());
	boost::python::def("StartHistory", &StartHistory, StartHistoryOverloads(boost::python::arg("freeze_observers")));
	boost::python::def("EndHistory", EndHistory);
	boost::python::def("ClearHistory", ClearHistory);
	boost::python::def("GetHistoryLevel", GetHistoryLevel);
	boost::python::def("IsModified", IsModified);
	boost::python::def("SetLikeNewFile", SetLikeNewFile);
	boost::python::def("RollBack", RollBack);
	boost::python::def("RollForward", RollForward);
	boost::python::def("GetUndoTitle", GetUndoTitle);
	boost::python::def("GetRedoTitle", GetRedoTitle);
	boost::python::def("DeleteUndoably", DeleteUndoably);
	boost::python::def("DeleteObjectsUndoably", DeleteObjectsUndoably);
	boost::python::def("DeleteInternalObject", DeleteInternalObject);
	boost::python::def("GetDrawMatrix", GetDrawMatrix, boost::python::return_value_policy<boost::python::reference_existing_object>());
	boost::python::def("AddUndoably", &AddUndoably, AddUndoablyOverloads((boost::python::arg("object"), boost::python::arg("owner") = NULL, boost::python::arg("add_before") = NULL)));
	boost::python::def("CopyUndoably", CopyUndoably);
	boost::python::def("TransformUndoably", TransformUndoably);
	boost::python::def("DoUndoable", DoUndoable);
	boost::python::def("WasModified", WasModified);
	boost::python::def("ShiftSelect", ShiftSelect);
	boost::python::def("ChangePropertyString", ChangePropertyString);
	boost::python::def("ChangePropertyDouble", ChangePropertyDouble);
	boost::python::def("ChangePropertyLength", ChangePropertyLength);
	boost::python::def("ChangePropertyInt", ChangePropertyInt);
	boost::python::def("ChangePropertyChoice", ChangePropertyChoice);
	boost::python::def("ChangePropertyColor", ChangePropertyColor);
	boost::python::def("ChangePropertyCheck", ChangePropertyCheck);
	boost::python::def("GetUnits", GetUnits);
	boost::python::def("SetGetLinesPixelsPerMm", SetGetLinesPixelsPerMm);
	boost::python::def("GetDragGripper", GetDragGripper, boost::python::return_value_policy<boost::python::reference_existing_object>());
	boost::python::def("AddGripper", AddGripper);
	boost::python::def("NewPoint", NewPoint, boost::python::return_value_policy<boost::python::reference_existing_object>());
	boost::python::def("NewLine", NewLine, boost::python::return_value_policy<boost::python::reference_existing_object>());
	boost::python::def("NewArc", NewArc, boost::python::return_value_policy<boost::python::reference_existing_object>());
	boost::python::def("NewCircle", NewCircle, boost::python::return_value_policy<boost::python::reference_existing_object>());
	boost::python::def("NewSketch", NewSketch, boost::python::return_value_policy<boost::python::reference_existing_object>());
	boost::python::def("NewText", NewText, boost::python::return_value_policy<boost::python::reference_existing_object>());
	boost::python::def("NewCoordinateSystem", NewCoordinateSystem, boost::python::return_value_policy<boost::python::reference_existing_object>());
	boost::python::def("PyIncref", PyIncref);
	boost::python::def("GetNextID", GetNextID);
	boost::python::def("GetDrawSelect", GetDrawSelect);
	boost::python::def("GetDrawMarked", GetDrawMarked);
	boost::python::def("CanUndo", CanUndo);
	boost::python::def("CanRedo", CanRedo);
	boost::python::def("ObjectsUnderWindow", ObjectsUnderWindow);
	boost::python::def("Digitize", digitize1);
	boost::python::def("GetTangentCircle", GetTangentCirclePy);
	boost::python::def("GetCircleBetween", GetCircleBetweenPy);
	boost::python::def("GetDigitizeEnd", GetDigitizeEnd);
	boost::python::def("SetDigitizeEnd", SetDigitizeEnd);
	boost::python::def("GetDigitizeInters", GetDigitizeInters);
	boost::python::def("SetDigitizeInters", SetDigitizeInters);
	boost::python::def("GetDigitizeCentre", GetDigitizeCentre);
	boost::python::def("SetDigitizeCentre", SetDigitizeCentre);
	boost::python::def("GetDigitizeMidpoint", GetDigitizeMidpoint);
	boost::python::def("SetDigitizeMidpoint", SetDigitizeMidpoint);
	boost::python::def("GetDigitizeNearest", GetDigitizeNearest);
	boost::python::def("SetDigitizeNearest", SetDigitizeNearest);
	boost::python::def("GetDigitizeCoords", GetDigitizeCoords);
	boost::python::def("SetDigitizeCoords", SetDigitizeCoords);
	boost::python::def("GetDigitizeTangent", GetDigitizeTangent);
	boost::python::def("SetDigitizeTangent", SetDigitizeTangent);
	boost::python::def("GetDigitizeSnapToGrid", GetDigitizeSnapToGrid);
	boost::python::def("SetDigitizeSnapToGrid", SetDigitizeSnapToGrid);
	boost::python::def("GetDigitizeGridSize", GetDigitizeGridSize);
	boost::python::def("SetDigitizeGridSize", SetDigitizeGridSize);
	boost::python::def("GetBackgroundColor", GetBackgroundColor);
	boost::python::def("SetBackgroundColor", SetBackgroundColor);
	boost::python::def("GetBackgroundMode", GetBackgroundMode);
	boost::python::def("SetBackgroundMode", SetBackgroundMode);
	boost::python::def("GetRotateUpright", GetRotateUpright);
	boost::python::def("SetRotateUpright", SetRotateUpright);
	boost::python::def("GetShowDatum", GetShowDatum);
	boost::python::def("SetShowDatum", SetShowDatum);
	boost::python::def("GetDatumSolid", GetDatumSolid);
	boost::python::def("SetDatumSolid", SetDatumSolid);
	boost::python::def("GetDatumSize", GetDatumSize);
	boost::python::def("SetDatumSize", SetDatumSize);
	boost::python::def("GetDatumSizeIsPixelsNotMm", GetDatumSizeIsPixelsNotMm);
	boost::python::def("SetDatumSizeIsPixelsNotMm", SetDatumSizeIsPixelsNotMm);
	boost::python::def("GetShowRuler", GetShowRuler);
	boost::python::def("SetShowRuler", SetShowRuler);
	boost::python::def("GetGridMode", GetGridMode);
	boost::python::def("SetGridMode", SetGridMode);
	boost::python::def("GetPerspective", GetPerspective);
	boost::python::def("SetPerspective", SetPerspective);
	boost::python::def("GetCurrentColor", GetCurrentColor);
	boost::python::def("SetCurrentColor", SetCurrentColor);
	boost::python::def("NewSketchFromCurve", NewSketchFromCurve, boost::python::return_value_policy<boost::python::reference_existing_object>());
	boost::python::def("NewSketchFromArea", NewSketchFromArea, boost::python::return_value_policy<boost::python::reference_existing_object>());
	boost::python::def("NewStlSolidFromStl", NewStlSolidFromStl, boost::python::return_value_policy<boost::python::reference_existing_object>());
	boost::python::def("CombineSelectedSketches", CombineSelectedSketches);
	boost::python::def("GetStretchPoint", GetStretchPoint);
	boost::python::def("GetStretchShift", GetStretchShift);
	boost::python::def("GetAntialiasing", GetAntialiasing);
	boost::python::def("SetAntialiasing", SetAntialiasing);
	boost::python::def("TangentialArc", TangentialArc);
	boost::python::scope().attr("OBJECT_TYPE_UNKNOWN") = (int)UnknownType;
	boost::python::scope().attr("OBJECT_TYPE_SKETCH") = (int)SketchType;
	boost::python::scope().attr("OBJECT_TYPE_SKETCH_LINE") = (int)LineType;
	boost::python::scope().attr("OBJECT_TYPE_SKETCH_ARC") = (int)ArcType;
	boost::python::scope().attr("OBJECT_TYPE_ILINE") = (int)ILineType;
	boost::python::scope().attr("OBJECT_TYPE_CIRCLE") = (int)CircleType;
	boost::python::scope().attr("OBJECT_TYPE_GRIPPER") = (int)GripperType;
	boost::python::scope().attr("OBJECT_TYPE_POINT") = (int)PointType;
	boost::python::scope().attr("OBJECT_TYPE_STL_SOLID") = (int)StlSolidType;
	boost::python::scope().attr("OBJECT_TYPE_COORD_SYS") = (int)CoordinateSystemType;
	boost::python::scope().attr("PROPERTY_TYPE_INVALID") = (int)InvalidPropertyType;
	boost::python::scope().attr("PROPERTY_TYPE_STRING") = (int)StringPropertyType;
	boost::python::scope().attr("PROPERTY_TYPE_LONG_STRING") = (int)LongStringPropertyType;
	boost::python::scope().attr("PROPERTY_TYPE_DOUBLE") = (int)DoublePropertyType;
	boost::python::scope().attr("PROPERTY_TYPE_LENGTH") = (int)LengthPropertyType;
	boost::python::scope().attr("PROPERTY_TYPE_INT") = (int)IntPropertyType;
	boost::python::scope().attr("PROPERTY_TYPE_CHOICE") = (int)ChoicePropertyType;
	boost::python::scope().attr("PROPERTY_TYPE_COLOR") = (int)ColorPropertyType;
	boost::python::scope().attr("PROPERTY_TYPE_CHECK") = (int)CheckPropertyType;
	boost::python::scope().attr("PROPERTY_TYPE_LIST") = (int)ListOfPropertyType;
	boost::python::scope().attr("PROPERTY_TYPE_FILE") = (int)FilePropertyType;
}
