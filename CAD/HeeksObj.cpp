// HeeksObj.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "HeeksObj.h"
#include "tinyxml.h"
#include "ObjList.h"
#include "Property.h"
#include "GripData.h"
#include "strconv.h"

HeeksObj::HeeksObj(void): m_owner(NULL), m_id(0), m_visible(true), m_index(0)
{
}

HeeksObj::HeeksObj(const HeeksObj& ho): m_owner(NULL), m_id(0), m_visible(true), m_index(0)
{
	operator=(ho);
}

const HeeksObj& HeeksObj::operator=(const HeeksObj &ho)
{
	// don't copy the ID or the owner
	m_visible = ho.m_visible;
	m_id = ho.m_id;

	return *this;
}

HeeksObj::~HeeksObj()
{
	if(m_owner)m_owner->Remove(this);

	if (m_index) theApp.ReleaseIndex(m_index);
}

HeeksObj* HeeksObj::MakeACopyWithID()
{
	HeeksObj* ret = MakeACopy();
	return ret;
}

PropertyObjectTitle::PropertyObjectTitle(HeeksObj* object) :Property(object, L"object title"){ m_editable = object->CanEditString(); }
Property *PropertyObjectTitle::MakeACopy(void)const{ return new PropertyObjectTitle(*this); }
const wchar_t* PropertyObjectTitle::GetString()const{ return m_object->GetShortString(); }
void PropertyObjectTitle::Set(const wchar_t* value){ m_object->OnEditString(value); }

PropertyObjectColor::PropertyObjectColor(HeeksObj* object) :Property(object, L"color"){}
Property *PropertyObjectColor::MakeACopy(void)const{ return new PropertyObjectColor(*this); }
const HeeksColor &PropertyObjectColor::GetColor()const{ return *(m_object->GetColor()); }
void PropertyObjectColor::Set(const HeeksColor& value){ m_object->SetColor(value); }


const wchar_t* HeeksObj::GetIconFilePath()
{
	static std::wstring iconpath = theApp.GetResFolder() + L"/icons/unknown.png";
	return iconpath.c_str();
}

static bool test_bool = false;

void HeeksObj::GetProperties(std::list<Property *> *list)
{
	bool editable = CanEditString();
	list->push_back(new PropertyStringReadOnly(L"object type", GetTypeString()));
	if (GetShortString())list->push_back(new PropertyObjectTitle(this));
	if(UsesID())list->push_back(new PropertyInt(this, L"ID", (int*)(&m_id)));
	const HeeksColor* c = GetColor();
	if(c)list->push_back ( new PropertyObjectColor(this) );
	list->push_back(new PropertyCheck(this, L"visible", &m_visible));
}

bool HeeksObj::GetScaleAboutMatrix(Matrix &m)
{
#if 0
	to do
	// return the bottom left corner of the box
	CBox box;
	GetBox(box);
	if(!box.m_valid)return false;
	Matrix mat;
	mat.SetTranslationPart(Point3d(box.m_x[0], box.m_x[1], box.m_x[2]));
	extract(mat, m);
#endif
	return true;
}

void HeeksObj::GetGripperPositionsTransformed(std::list<GripData> *list, bool just_for_endof)
{
	GetGripperPositions(list,just_for_endof);
}

void HeeksObj::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	CBox box;
	GetBox(box);
	if(!box.m_valid)return;

	//TODO: This is a tab bit of a strange thing to do. Especially for planar objects like faces
	//ones that are on a plane like y-z or x-z will have all gripper merged togeather.
	list->push_back(GripData(GripperTypeTranslate,box.m_x[0],box.m_x[1],box.m_x[2],NULL));
	list->push_back(GripData(GripperTypeRotateObject,box.m_x[3],box.m_x[1],box.m_x[2],NULL));
	list->push_back(GripData(GripperTypeRotateObject,box.m_x[0],box.m_x[4],box.m_x[2],NULL));
	list->push_back(GripData(GripperTypeScale,box.m_x[3],box.m_x[4],box.m_x[2],NULL));
}

bool HeeksObj::Add(HeeksObj* object, HeeksObj* prev_object)
{
	object->m_owner = this;
	object->OnAdd();
	return true;
}

void HeeksObj::OnRemove()
{
	if(m_owner == NULL)KillGLLists();
}

void HeeksObj::SetID(int id)
{
	theApp.SetObjectID(this, id);
}

void HeeksObj::WriteXML(TiXmlNode *root)
{
	TiXmlElement *element;
	element = new TiXmlElement(Ttc(GetXMLTypeString()));
	root->LinkEndChild(element);
	WriteToXML(element);
}

void HeeksObj::WriteToXML(TiXmlElement *element)
{
	theApp.ObjectWriteToXML(this, element);
}

void HeeksObj::ReadFromXML(TiXmlElement* element)
{
	theApp.ObjectReadFromXML(this, element);
}

bool HeeksObj::OnVisibleLayer()
{
	// to do, support multiple layers.
	return true;
}

HeeksObj *HeeksObj::Find( const int type, const unsigned int id )
{
	if ((type == this->GetType()) && (this->m_id == id)) return(this);
	return(NULL);
}

#ifdef WIN32
#define snprintf _snprintf
#endif

unsigned int HeeksObj::GetIndex() {
	if (!m_index) m_index = theApp.GetIndex(this);
	return m_index;
}
