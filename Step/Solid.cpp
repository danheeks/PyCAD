// Solid.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "Solid.h"
#include "MarkedList.h"

// static
int CSolid::m_type = 0;

CSolid::CSolid(const TopoDS_Solid &solid, const wchar_t* title, const HeeksColor& col, float opacity):CShape(solid, title, col, opacity)
{
}

CSolid::CSolid( const CSolid & rhs ) : CShape(rhs)
{
    *this = rhs;    // Call the assignment operator.
}

CSolid::~CSolid()
{
}

HeeksObj *CSolid::MakeACopy(void)const
{
	return new CSolid(*this);
}

void CSolid::SetXMLElement(TiXmlElement* element)
{
	element->SetAttribute("col", m_color.COLORREF_color());
	if(m_opacity < 0.9999)element->SetDoubleAttribute("opacity", m_opacity);

}

void CSolid::SetFromXMLElement(TiXmlElement* pElem)
{
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){m_color = HeeksColor((long)(a->IntValue()));}
		else if(name == "opacity"){m_opacity = (float)(a->DoubleValue());}
	}
}

const wchar_t* CSolid::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/solid.png";
	return iconpath.c_str();
}

void CSolid::SetColor(const HeeksColor &col)
{
	if (m_faces)
	{
		for (HeeksObj* object = m_faces->GetFirstChild(); object; object = m_faces->GetNextChild())
		{
			object->SetColor(col);
		}
	}
}

const HeeksColor* CSolid::GetColor()const
{
	// faces have color now, so just return first one
	if (m_faces)
	{
		HeeksObj* object = m_faces->GetFirstChild();
		if (object)return object->GetColor();
	}
	return NULL;
}

void CSolid::MakeTransformedShape(const gp_Trsf &mat)
{
	BRepBuilderAPI_Transform myBRepTransformation(m_shape,mat);
	m_shape = myBRepTransformation.Shape();
}

void CSolid::OnApplyPropertiesRaw()
{
	*this = CSolid(*((TopoDS_Solid*)(&m_shape)), m_title.c_str(), m_color, (float)m_opacity);
}
