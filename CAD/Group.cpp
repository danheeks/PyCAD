// Group.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Group.h"
#include "strconv.h"
#include "GripData.h"
#include "Property.h"

CGroup::CGroup()
{
	m_title = L"Group";
	m_gripper_datum_set = false;
	m_custom_grippers = false;
	m_custom_grippers_just_one_axis = true;
}

void CGroup::WriteToXML(TiXmlElement *element)
{
    element->SetAttribute("title", Ttc(m_title.c_str()));
	element->SetAttribute("custom_grippers", m_custom_grippers ? 1:0);
	element->SetAttribute("custom_grippers_one_axis", m_custom_grippers_just_one_axis ? 1:0);
	element->SetAttribute("gripper_datum_set", m_gripper_datum_set ? 1:0);
	if(m_gripper_datum_set)
	{
		element->SetDoubleAttribute("ox", m_o.x);
		element->SetDoubleAttribute("oy", m_o.y);
		element->SetDoubleAttribute("oz", m_o.z);
		element->SetDoubleAttribute("pxx", m_px.x);
		element->SetDoubleAttribute("pxy", m_px.y);
		element->SetDoubleAttribute("pxz", m_px.z);
		element->SetDoubleAttribute("pyx", m_py.x);
		element->SetDoubleAttribute("pyy", m_py.y);
		element->SetDoubleAttribute("pyz", m_py.z);
		element->SetDoubleAttribute("pzx", m_pz.x);
		element->SetDoubleAttribute("pzy", m_pz.y);
		element->SetDoubleAttribute("pzz", m_pz.z);
	}

	// instead of ObjList::WriteToXML(element), write the id of solids, or the object
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;
#if 0
		if(CShape::IsTypeAShape(object->GetType()))
		{
			TiXmlElement* solid_element = new TiXmlElement( "solid" );
			element->LinkEndChild( solid_element );
			solid_element->SetAttribute("id", object->m_id);
		}
#endif
		object->WriteXML(element);
	}
	HeeksObj::WriteToXML(element);
}

void CGroup::ReadFromXML(TiXmlElement *element)
{
	// instead of ( ObjList:: ) new_object->ReadFromXML(pElem);

	if(element->Attribute("title"))m_title = Ctt(element->Attribute("title"));
	int int_for_bool;
	if(element->Attribute("custom_grippers", &int_for_bool))m_custom_grippers = (int_for_bool != 0);
	if(element->Attribute("custom_grippers_one_axis", &int_for_bool))m_custom_grippers_just_one_axis = (int_for_bool != 0);
	if(element->Attribute("gripper_datum_set", &int_for_bool))m_gripper_datum_set = (int_for_bool != 0);
	if(m_gripper_datum_set)
	{
		double o[3], px[3], py[3], pz[3];
		element->Attribute("ox", &o[0]);
		element->Attribute("oy", &o[1]);
		element->Attribute("oz", &o[2]);
		m_o = Point3d(o);
		element->Attribute("pxx", &px[0]);
		element->Attribute("pxy", &px[1]);
		element->Attribute("pxz", &px[2]);
		m_px = Point3d(px);
		element->Attribute("pyx", &py[0]);
		element->Attribute("pyy", &py[1]);
		element->Attribute("pyz", &py[2]);
		m_py = Point3d(py);
		element->Attribute("pzx", &pz[0]);
		element->Attribute("pzy", &pz[1]);
		element->Attribute("pzz", &pz[2]);
		m_pz = Point3d(pz);
	}

	// loop through all the objects
	for(TiXmlElement* pElem = TiXmlHandle(element).FirstChildElement().Element(); pElem; pElem = pElem->NextSiblingElement())
	{
		std::string name(pElem->Value());
		if(name == "solid")
		{
			int id = 0;
			pElem->Attribute("id", &id);
			m_loaded_solid_ids.push_back(id); // solid found after load with CGroup::MoveSolidsToGroupsById
		}
		else
		{
			// load other objects normal
			HeeksObj* object = theApp->ReadXMLElement(pElem);
			if(object)Add(object, NULL);
		}
	}

	HeeksObj::ReadFromXML(element);
}

const wchar_t* CGroup::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/group.png";
	return iconpath.c_str();
}

void CGroup::OnEditString(const wchar_t* str){
	m_title.assign(str);
}

bool CGroup::Stretch(const double *p, const double* shift, void* data){
	Point3d vp(p);
	Point3d vshift(shift);
	m_px = vp + vshift;
	return false;
}

void CGroup::GetGripperPositions(std::list<GripData> *list, bool just_for_endof){
	if(m_custom_grippers)
	{
		if(!m_gripper_datum_set)
		{
			CBox box;
			GetBox(box);
			if(!box.m_valid)return;
			m_o = Point3d((box.MinX() + box.MaxX())/2, (box.MinY() + box.MaxY())/2, (box.MinZ() + box.MaxZ())/2);
			m_px = Point3d(box.MaxX(), (box.MinY() + box.MaxY())/2, (box.MinZ() + box.MaxZ())/2);
			m_py = Point3d((box.MinX() + box.MaxX())/2, box.MaxY(), (box.MinZ() + box.MaxZ())/2);
			m_pz = Point3d((box.MinX() + box.MaxX())/2, (box.MinY() + box.MaxY())/2, box.MaxZ());
			m_gripper_datum_set = true;
		}

		list->push_back(GripData(GripperTypeTranslate, m_o.x, m_o.y, m_o.z, NULL));
		if(m_custom_grippers_just_one_axis)
		{
			list->push_back(GripData(GripperTypeStretch, m_px.x, m_px.y, m_px.z, NULL));
			list->push_back(GripData(GripperTypeRotateObjectYZ, m_py.x, m_py.y, m_py.z, NULL));
			list->push_back(GripData(GripperTypeRotateObjectYZ, m_pz.x, m_pz.y, m_pz.z, NULL));
		}
		else
		{
			list->push_back(GripData(GripperTypeRotateObjectXY, m_px.x, m_px.y, m_px.z, NULL));
			list->push_back(GripData(GripperTypeRotateObjectYZ, m_py.x, m_py.y, m_py.z, NULL));
			list->push_back(GripData(GripperTypeRotateObjectXZ, m_pz.x, m_pz.y, m_pz.z, NULL));
		}
	}
	else
	{
		ObjList::GetGripperPositions(list, just_for_endof);
	}
}

void CGroup::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyCheck(this, L"custom grippers", &m_custom_grippers));

	if(m_custom_grippers)
	{
		list->push_back ( new PropertyCheck(this,  L"custom grippers just one axis", &m_custom_grippers_just_one_axis) );
		list->push_back(PropertyPnt(this, L"datum", &m_o));
		list->push_back(PropertyPnt(this, L"x", &m_px));
		list->push_back(PropertyPnt(this, L"y", &m_py));
		list->push_back(PropertyPnt(this, L"z", &m_pz));
	}
	ObjList::GetProperties(list);
}

static CGroup* object_for_tools = NULL;
static Point3d* vertex_for_pick_pos = NULL;
static void(*callback_for_pick_pos)(const double*) = NULL;

static void on_set_o(const double* pos)
{
	Point3d shift(object_for_tools->m_o, Point3d(pos));
	object_for_tools->m_o = Point3d(pos);
	object_for_tools->m_px = object_for_tools->m_px + shift;
	object_for_tools->m_py = object_for_tools->m_py + shift;
	object_for_tools->m_pz = object_for_tools->m_pz + shift;
	theApp->Repaint();
}

Matrix CGroup::GetMatrix()
{
	return Matrix(m_o, Point3d(m_o, m_px), Point3d(m_o, m_py));
}

bool CGroup::GetScaleAboutMatrix(Matrix &m)
{
	if(m_custom_grippers)
	{
		m = GetMatrix();
		return true;
	}
	
	return ObjList::GetScaleAboutMatrix(m);
}

void CGroup::Transform(const Matrix& m)
{
	ObjList::Transform(m);

	if(m_gripper_datum_set)
	{
		Matrix mat = Matrix(m);
		m_o = m_o.Transformed(mat);
		m_px = m_px.Transformed(mat);
		m_py = m_py.Transformed(mat);
		m_pz = m_pz.Transformed(mat);
	}
}
