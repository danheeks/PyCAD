// EndedObject.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

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

bool EndedObject::IsDifferent(HeeksObj *other)
{
	EndedObject* eobj = (EndedObject*)other;
	if(eobj->A.Dist(A) > TOLERANCE)
		return true;

	if (eobj->B.Dist(B) > TOLERANCE)
		return true;

	if(color.COLORREF_color() != eobj->color.COLORREF_color())
		return true;

	return ExtrudedObj<HeeksObj>::IsDifferent(other);
}

void EndedObject::Transform(const Matrix& m){
	A = A.Transformed(m);
	B = B.Transformed(m);
}

bool EndedObject::Stretch(const double *p, const double* shift, void* data){
	Point3d vp(p);
	Point3d vshift(shift);

	if(data == &A){
		A = vp + vshift;
	}
	else if(data == &B){
		B = vp + vshift;
	}
	return false;
}

void EndedObject::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	list->push_back(GripData(GripperTypeStretch,A.x,A.y,A.z,&A));
	list->push_back(GripData(GripperTypeStretch,B.x,B.y,B.z,&B));
}

bool EndedObject::GetStartPoint(double* pos)
{
	A.get(pos);
	return true;
}

bool EndedObject::GetEndPoint(double* pos)
{
	B.get(pos);
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
	HeeksColor c;

	// get the attributes
	int att_col;
	double x;
	if(pElem->Attribute("col", &att_col))c = HeeksColor((long)att_col);
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