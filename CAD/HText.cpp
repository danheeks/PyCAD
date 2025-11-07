// HText.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "HText.h"
#include "PropertyGeom.h"
#include "Gripper.h"
#include "GripData.h"
#include "strconv.h"

HText::HText(const Matrix &trsf, const std::wstring &text, const HeeksColor* col,
			 int hj, int vj):m_color(*col),  m_trsf(trsf), m_text(text),
			 m_h_justification(hj), m_v_justification(vj)
{
}

HText::HText(const HText &b)
{
	operator=(b);
}

HText::~HText(void)
{
}

const HText& HText::operator=(const HText &b)
{
    if (this != &b)
    {
        ObjList::operator=(b);
        m_trsf = b.m_trsf;
        m_text = b.m_text;
        m_color = b.m_color;
		m_v_justification = b.m_v_justification;
		m_h_justification = b.m_h_justification;
    }

	return *this;
}

const wchar_t* HText::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/text.png";
	return iconpath.c_str();
}

double HText::GetScale() const
{
	double sx;
	m_trsf.GetScale(sx);
	return sx;
}

void HText::GetOffset(double& x, double& y)const
{
	x = 0.0;
	y = 0.0;
	float width, height;
	if (GetTextSize(m_text, &width, &height))
	{
		switch (m_h_justification)
		{
		case 0:// Left
			break;
		case 1:// Center
			x = -width * 0.5;
			break;
		case 2:// Right
			x = -width;
			break;
		default:
			break;
		}
		switch (m_v_justification)//0 = Baseline; 1 = Bottom; 2 = Middle; 3 = Top
		{
		case 0:// Baseline
			y = height * 1.52;
			break;
		case 1:// Bottom
			y = height * (-0.2);
			break;
		case 2:// Middle
			y = height * 0.5;
			break;
		case 3:// Top
			break;
		default:
			break;
		}
	}
}

void HText::glCommands(bool select, bool marked, bool no_color)
{
	glPushMatrix();
	double m[16];
	m_trsf.GetTransposed(m);
	glMultMatrixd(m);
	double sx;
	m_trsf.GetScale(sx);

	if(!no_color)theApp->glColorEnsuringContrast(m_color);

	// We're using the internal font
	double x, y;
	GetOffset(x, y);
	glTranslated(x, y, 0);
	theApp->render_text(m_text.c_str(), select, 1.0, 0.2 /(theApp->GetPixelScale() * sx));
	glPopMatrix();

	ObjList::glCommands(select, marked, no_color);
}

bool HText::GetTextSize( const std::wstring & text, float *pWidth, float *pHeight ) const
{
	// We're using the internal font.
	double sx;
	m_trsf.GetScale(sx);
	if(!theApp->get_text_size(text.c_str(), pWidth, pHeight))return(false);
	*pWidth *= (float)sx;
	*pHeight *= (float)sx;
	return(true);
} // End GetTextSize() method

void HText::GetBoxPoints(std::list<Point3d> &pnts)
{
	Point3d vt(0, 0, 0);
	vt.Transform(m_trsf);

	float width, height;
	if (! GetTextSize( m_text, &width, &height ))
	{
		pnts.push_back(vt);
		return;
	}

	Point3d point[4];
	point[0] = Point3d(0, 0, 0);
	point[1] = Point3d(width, 0, 0);
	point[2] = Point3d(0, height, 0);
	point[3] = Point3d(width, height, 0);

	double x = 0;
	double y = 0;

	switch(m_h_justification)
	{
	case 0:// Left
		break;
	case 1:// Center
		x = -width * 0.5;
		break;
	case 2:// Right
		x = -width;
		break;
	default:
		break;
	}
	switch(m_v_justification)//0 = Baseline; 1 = Bottom; 2 = Middle; 3 = Top
	{
	case 0:// Baseline
		y = height * 0.0;
		break;
	case 1:// Bottom
		y = height;
		break;
	case 2:// Middle
		y= height * 0.5;
		break;
	case 3:// Top
		break;
	default:
		break;
	}

	Matrix shift;
	shift.Translate(Point3d(x, y, 0));

	for(int i = 0; i<4; i++)
	{
		//point[i].Transform(shift);
		point[i].Transform(m_trsf);
		pnts.push_back(point[i]);
	}
}

void HText::GetBox(CBox &box)
{
	std::list<Point3d> pnts;
	GetBoxPoints(pnts);

	for(std::list<Point3d>::iterator It = pnts.begin(); It != pnts.end(); It++)
	{
		Point3d &p = *It;
		box.Insert(p.x, p.y, p.z);
	}
}

HeeksObj *HText::MakeACopy(void)const
{
	return new HText(*this);
}

void HText::Transform(const Matrix& m)
{
	m_trsf = m_trsf * m;
}

void HText::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	std::list<Point3d> pnts;
	GetBoxPoints(pnts);

	EnumGripperType gripper_types[4] = {GripperTypeTranslate, GripperTypeRotateObject, GripperTypeRotateObject, GripperTypeScale};

	int i = 0;
	for(std::list<Point3d>::iterator It = pnts.begin(); It != pnts.end(); It++, i++)
	{
		Point3d &point = *It;
		list->push_back(GripData(gripper_types[i],point));
	}
}

void HText::GetProperties(std::list<Property *> *list)
{
	list->push_back(PropertyTrsf(this, L"orientation", &m_trsf));
	{
		std::list< std::wstring > choices;
		choices.push_back ( std::wstring ( L"left" ) );
		choices.push_back ( std::wstring ( L"center" ) );
		choices.push_back ( std::wstring ( L"right" ) );
		list->push_back(new PropertyChoice(this, L"horizontal justification", choices, &m_h_justification ) );
	}
	{
		std::list< std::wstring > choices;
		choices.push_back ( std::wstring ( L"baseline" ) );
		choices.push_back ( std::wstring ( L"bottom" ) );
		choices.push_back ( std::wstring ( L"middle" ) );
		choices.push_back ( std::wstring ( L"top" ) );
		list->push_back(new PropertyChoice(this, L"vertical justification", choices, &m_v_justification ) );
	}

	ObjList::GetProperties(list);
}

bool HText::Stretch(const double *p, const double* shift, void* data)
{
	return false;
}

void HText::OnEditString(const wchar_t* str){
	m_text.assign(str);
	// to do, use undoable property changes
}

void HText::WriteToXML(TiXmlElement *element)
{
	element->SetAttribute("text", Ttc(m_text.c_str()) );

	element->SetAttribute("col", m_color.COLORREF_color());
	element->SetDoubleAttribute("m0", m_trsf.e[0]);
	element->SetDoubleAttribute("m1", m_trsf.e[1]);
	element->SetDoubleAttribute("m2", m_trsf.e[2]);
	element->SetDoubleAttribute("m3", m_trsf.e[3]);
	element->SetDoubleAttribute("m4", m_trsf.e[4]);
	element->SetDoubleAttribute("m5", m_trsf.e[5]);
	element->SetDoubleAttribute("m6", m_trsf.e[6]);
	element->SetDoubleAttribute("m7", m_trsf.e[7]);
	element->SetDoubleAttribute("m8", m_trsf.e[8]);
	element->SetDoubleAttribute("m9", m_trsf.e[9]);
	element->SetDoubleAttribute("ma", m_trsf.e[10]);
	element->SetDoubleAttribute("mb", m_trsf.e[11]);

	element->SetAttribute("vj",m_v_justification);
	element->SetAttribute("hj",m_h_justification);

	ObjList::WriteToXML(element);
}

void HText::ReadFromXML(TiXmlElement* pElem)
{
	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "col"){m_color = HeeksColor((long)(a->IntValue()));}
		else if(name == "text")	{ m_text.assign( Ctt(a->Value()) ); }
		else if(name == "m0"){ m_trsf.e[0] = a->DoubleValue();}
		else if (name == "m1"){ m_trsf.e[1] = a->DoubleValue(); }
		else if (name == "m2"){ m_trsf.e[2] = a->DoubleValue(); }
		else if (name == "m3"){ m_trsf.e[3] = a->DoubleValue(); }
		else if (name == "m4"){ m_trsf.e[4] = a->DoubleValue(); }
		else if (name == "m5"){ m_trsf.e[5] = a->DoubleValue(); }
		else if (name == "m6"){ m_trsf.e[6] = a->DoubleValue(); }
		else if (name == "m7"){ m_trsf.e[7] = a->DoubleValue(); }
		else if (name == "m8"){ m_trsf.e[8] = a->DoubleValue(); }
		else if (name == "m9"){ m_trsf.e[9] = a->DoubleValue(); }
		else if (name == "ma"){ m_trsf.e[10] = a->DoubleValue(); }
		else if (name == "mb"){ m_trsf.e[11] = a->DoubleValue(); }
	}

	// test and set matrix values
	m_trsf.IsUnit();
	m_trsf.IsMirrored();

	pElem->Attribute("vj", &m_v_justification);
	pElem->Attribute("hj",&m_h_justification);

	ObjList::ReadFromXML(pElem);
}

bool HText::CanAdd(HeeksObj* object)
{
	if (object == NULL) return(false);
	if (GetNumChildren() > 0)
	{
		theApp->DoMessageBox(L"Only a single orientation modifier is supported");
		return(false);
	}

	return(false);
}

