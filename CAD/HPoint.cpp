// HPoint.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "HPoint.h"
#include "Gripper.h"
#include "DigitizeMode.h"
#include "Drawing.h"
#include "Property.h"

static unsigned char cross16[32] = {0x80, 0x01, 0x40, 0x02, 0x20, 0x04, 0x10, 0x08, 0x08, 0x10, 0x04, 0x20, 0x02, 0x40, 0x01, 0x80, 0x01, 0x80, 0x02, 0x40, 0x04, 0x20, 0x08, 0x10, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01};
static unsigned char cross16_selected[32] = {0xc0, 0x03, 0xe0, 0x07, 0x70, 0x0e, 0x38, 0x1c, 0x1c, 0x38, 0x0e, 0x70, 0x07, 0xe0, 0x03, 0xc0, 0x03, 0xc0, 0x07, 0xe0, 0x0e, 0x70, 0x1c, 0x38, 0x38, 0x1c, 0x70, 0x0e, 0xe0, 0x07, 0xc0, 0x03};

HPoint::~HPoint(void)
{
}

HPoint::HPoint(const Point3d &p, const HeeksColor* col)
{
	m_p = p;
	color = *col;
	m_draw_unselected=true;
}

HPoint::HPoint(const HPoint &p)
{
	operator=(p);
}

const HPoint& HPoint::operator=(const HPoint &b)
{
	HeeksObj::operator =(b);

	m_p = b.m_p;
	color = b.color;
	m_draw_unselected = b.m_draw_unselected;
	return *this;
}

const wchar_t* HPoint::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/point.png";
	return iconpath.c_str();
}

void HPoint::glCommands(bool select, bool marked, bool no_color)
{
	if(!no_color){
		theApp->glColorEnsuringContrast(color);
	}
	GLfloat save_depth_range[2];
	if(marked){
		glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
		glDepthRange(0, 0);
	}
	else if(!m_draw_unselected)
	{
		glBegin(GL_POINTS);
		glVertex3d(m_p.x, m_p.y, m_p.z);
		glEnd();
		return;
	}


	glRasterPos3d(m_p.x, m_p.y, m_p.z);
	glBitmap(16, 16, 8, 8, 10.0, 0.0, marked ? cross16_selected : cross16);
	if(marked){
		glDepthRange(save_depth_range[0], save_depth_range[1]);
	}
}

void HPoint::GetBox(CBox &box)
{
	box.Insert(m_p.x, m_p.y, m_p.z);
}

HeeksObj *HPoint::MakeACopy(void)const
{
	return new HPoint(*this);
}

void HPoint::Transform(const Matrix& m)
{
	m_p = m_p.Transformed(m);
}

void HPoint::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	if(just_for_endof)
	{
		list->push_back(GripData((EnumGripperType)0, m_p, NULL));
	}
}

void HPoint::GetProperties(std::list<Property *> *list)
{
	list->push_back( PropertyPnt(this, L"position", &m_p));

	HeeksObj::GetProperties(list);
}

bool HPoint::GetStartPoint(Point3d& pos)
{
	pos = m_p;
	return true;
}

void HPoint::SetStartPoint(const Point3d &pos)
{
	m_p = pos;
}

bool HPoint::GetEndPoint(Point3d& pos)
{
	pos = m_p;
	return true;
}

void HPoint::WriteToXML(TiXmlElement *element)
{
	element->SetAttribute("col", color.COLORREF_color());
	element->SetDoubleAttribute("x", m_p.x);
	element->SetDoubleAttribute("y", m_p.y);
	element->SetDoubleAttribute("z", m_p.z);
	IdNamedObj::WriteToXML(element);
}


//static
void HPoint::ReadFromXML(TiXmlElement *element)
{
	int int_value;
	if (element->Attribute("col", &int_value))color = HeeksColor((long)int_value);

	element->Attribute("x", &m_p.x);
	element->Attribute("y", &m_p.y);
	element->Attribute("z", &m_p.z);
	IdNamedObj::ReadFromXML(element);
}
