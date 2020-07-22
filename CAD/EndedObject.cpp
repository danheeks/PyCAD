// EndedObject.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include <stdafx.h>

#include "EndedObject.h"
#include "HPoint.h"
#include "MarkedList.h"
#include "GripData.h"

EndedObject::EndedObject(void){
}

EndedObject::EndedObject(const EndedObject& e)
{
	operator=(e);
}

EndedObject::~EndedObject(){
}

const EndedObject& EndedObject::operator=(const EndedObject &b){
	ExtrudedObj<HeeksObj>::operator = (b);
	A = b.A;
	B = b.B;
	color = b.color;
	return *this;
}

HeeksObj* EndedObject::MakeACopyWithID()
{
	EndedObject* pnew = (EndedObject*)ExtrudedObj<HeeksObj>::MakeACopyWithID();
	return pnew;
}

void EndedObject::Transform(const Matrix& m){
	A = A.Transformed(m);
	B = B.Transformed(m);
}

bool EndedObject::Stretch(const Point3d &p, const Point3d &shift, void* data){
	if(data == &A){
		A = p + shift;
	}
	else if(data == &B){
		B = p + shift;
	}
	return false;
}

void EndedObject::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	list->push_back(GripData(GripperTypeStretch,A,&A));
	list->push_back(GripData(GripperTypeStretch,B,&B));
}

bool EndedObject::GetStartPoint(Point3d &pos)
{
	pos = A;
	return true;
}

bool EndedObject::GetEndPoint(Point3d &pos)
{
	pos = B;
	return true;
}

void EndedObject::WriteToXML(TiXmlElement *element)
{
	element->SetAttribute("col", color.COLORREF_color());
	element->SetDoubleAttribute("sx", A.x);
	element->SetDoubleAttribute("sy", A.y);
	element->SetDoubleAttribute("sz", A.z);
	element->SetDoubleAttribute("ex", B.x);
	element->SetDoubleAttribute("ey", B.y);
	element->SetDoubleAttribute("ez", B.z);

	ExtrudedObj<HeeksObj>::WriteToXML(element);
}

void EndedObject::ReadFromXML(TiXmlElement* pElem)
{
	// get the attributes
	int att_col;
	double x;
	if(pElem->Attribute("col", &att_col))color = HeeksColor((long)att_col);
	if(pElem->Attribute("sx", &x))A.x = x;
	if(pElem->Attribute("sy", &x))A.y = x;
	if(pElem->Attribute("sz", &x))A.z = x;
	if(pElem->Attribute("ex", &x))B.x = x;
	if(pElem->Attribute("ey", &x))B.y = x;
	if(pElem->Attribute("ez", &x))B.z = x;

	else
	{
		// try the version where the points were children
		bool a_found = false;
		for(TiXmlElement* pElem2 = TiXmlHandle(pElem).FirstChildElement().Element(); pElem2;	pElem2 = pElem2->NextSiblingElement())
		{
			HeeksObj* object = theApp->ReadXMLElement(pElem2);
			if(object->GetType() == PointType)
			{
				if(!a_found)
				{
					A = ((HPoint*)object)->m_p;
					a_found = true;
				}
				else
				{
					B = ((HPoint*)object)->m_p;
					delete object;
					break;
				}
			}
			delete object;
		}
	}

	ExtrudedObj<HeeksObj>::ReadFromXML(pElem);
}
