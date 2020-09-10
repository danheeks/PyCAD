// Sphere.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Sphere.h"
#include "Gripper.h"
#include "MarkedList.h"
#include "OCCProperty.h"
#include "PropertySolid.h"

CSphere::CSphere(const gp_Pnt& pos, double radius, const wchar_t* title, const HeeksColor& col, float opacity):CSolid(BRepPrimAPI_MakeSphere(pos, radius), title, col, opacity), m_pos(pos), m_radius(radius)
{
}

CSphere::CSphere(const TopoDS_Solid &solid, const wchar_t* title, const HeeksColor& col, float opacity):CSolid(solid, title, col, opacity), m_pos(0, 0, 0), m_radius(0.0)
{
}

CSphere::CSphere( const CSphere & rhs ) : CSolid(rhs)
{
    m_pos = rhs.m_pos;
    m_radius = rhs.m_radius;
}

CSphere & CSphere::operator= ( const CSphere & rhs )
{
    if (this != &rhs)
    {
        m_pos = rhs.m_pos;
        m_radius = rhs.m_radius;

        CSolid::operator=( rhs );
    }

    return(*this);
}

HeeksObj *CSphere::MakeACopy(void)const
{
	return new CSphere(*this);
}

const wchar_t* CSphere::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/sphere.png";
	return iconpath.c_str();
}

void CSphere::MakeTransformedShape(const gp_Trsf &mat)
{
	m_pos.Transform(mat);
	double scale = gp_Vec(1, 0, 0).Transformed(mat).Magnitude();
	m_radius = fabs(m_radius * scale);
	m_shape = BRepPrimAPI_MakeSphere(m_pos, m_radius).Shape();
}

std::wstring CSphere::StretchedName(){ return L"Ellipsoid";}

void CSphere::GetProperties(std::list<Property *> *list)
{
	list->push_back(PropertyGp<gp_Pnt>(this, L"centre", &m_pos));
	list->push_back(new PropertySolidLength(this, L"radius", &m_radius));

	CSolid::GetProperties(list);
}

void CSphere::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	list->push_back(GripData(GripperTypeTranslate,G2P(m_pos),NULL));
	list->push_back(GripData(GripperTypeScale,Point3d(m_pos.X() + m_radius,m_pos.Y(),m_pos.Z()),NULL));
}

void CSphere::OnApplyPropertiesRaw()
{
	*this = CSphere(m_pos, m_radius, m_title.c_str(), m_color, (float)m_opacity);
}

bool CSphere::GetCentrePoint(Point3d &pos)
{
	pos = G2P(m_pos);
	return true;
}

bool CSphere::GetScaleAboutMatrix(Matrix &m)
{
	m = Matrix();
	m.Translate(G2P(m_pos));
	return true;
}

void CSphere::SetXMLElement(TiXmlElement* element)
{
	element->SetDoubleAttribute("px", m_pos.X());
	element->SetDoubleAttribute("py", m_pos.Y());
	element->SetDoubleAttribute("pz", m_pos.Z());
	element->SetDoubleAttribute("r", m_radius);

	CSolid::SetXMLElement(element);
}

void CSphere::SetFromXMLElement(TiXmlElement* pElem)
{
	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "px"){m_pos.SetX(a->DoubleValue());}
		else if(name == "py"){m_pos.SetY(a->DoubleValue());}
		else if(name == "pz"){m_pos.SetZ(a->DoubleValue());}
		else if(name == "r"){m_radius = a->DoubleValue();}
	}

	CSolid::SetFromXMLElement(pElem);
}
