// Cylinder.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Cylinder.h"
#include "Gripper.h"
#include "GripData.h"
#include "CoordinateSystem.h"
#include "PropertySolid.h"

static TopoDS_Solid MakeCylinder(const gp_Ax2& pos, double radius, double height)
{
	gp_Ax2 pos2 = pos;
	if(height<0)
	{
		pos2 = gp_Ax2(pos.Location(), -(pos.Direction()));
		height = fabs(height);
	}

	return BRepPrimAPI_MakeCylinder(pos2, radius, height);
}

CCylinder::CCylinder(const gp_Ax2& pos, double radius, double height, const wchar_t* title, const HeeksColor& col, float opacity):CSolid(MakeCylinder(pos, radius, height), title, col, opacity), m_pos(pos), m_radius(radius), m_height(height)
{
}

CCylinder::CCylinder(const TopoDS_Solid &solid, const wchar_t* title, const HeeksColor& col, float opacity):CSolid(solid, title, col, opacity), m_pos(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0)), m_radius(0.0), m_height(0.0)
{
}

const wchar_t* CCylinder::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/cyl.png";
	return iconpath.c_str();
}

HeeksObj *CCylinder::MakeACopy(void)const
{
	return new CCylinder(*this);
}

void CCylinder::MakeTransformedShape(const gp_Trsf &mat)
{
	m_pos.Transform(mat);
	double scale = gp_Vec(1, 0, 0).Transformed(mat).Magnitude();
	m_radius = fabs(m_radius * scale);
	m_height = fabs(m_height * scale);
	m_shape = MakeCylinder(m_pos, m_radius, m_height);
}

std::wstring CCylinder::StretchedName(){ return L"Stretched Cylinder";}

void CCylinder::GetProperties(std::list<Property *> *list)
{
	GetAx2Properties(list, m_pos, this);

	list->push_back(new PropertySolidLengthScaled(this, L"diameter", &m_radius, 2.0));
	list->push_back(new PropertySolidLength(this, L"height", &m_height));

	CSolid::GetProperties(list);
}

void CCylinder::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	gp_Pnt o = m_pos.Location();
	gp_Pnt px(o.XYZ() + m_pos.XDirection().XYZ() * m_radius);
	gp_Dir z_dir = m_pos.XDirection() ^ m_pos.YDirection();
	gp_Pnt pyz(o.XYZ() + m_pos.YDirection().XYZ() * m_radius + z_dir.XYZ() * m_height);
	gp_Pnt pmxz(o.XYZ() + m_pos.XDirection().XYZ() * (-m_radius) + z_dir.XYZ() * m_height);
	gp_Pnt pz(o.XYZ() + z_dir.XYZ() * m_height);
	list->push_back(GripData(GripperTypeTranslate,G2P(o),NULL));
	list->push_back(GripData(GripperTypeObjectScaleXY, G2P(px), NULL));
	list->push_back(GripData(GripperTypeRotateObject, G2P(pyz), NULL));
	list->push_back(GripData(GripperTypeRotateObject, G2P(pmxz), NULL));
	list->push_back(GripData(GripperTypeObjectScaleZ, G2P(pz), NULL));
}

void CCylinder::OnApplyPropertiesRaw()
{
	*this = CCylinder(m_pos, m_radius, m_height, m_title.c_str(), m_color, (float)m_opacity);
}

int CCylinder::GetCentrePoints(Point3d &pos, Point3d &pos2)
{
	gp_Pnt o = m_pos.Location();
	gp_Dir z_dir = m_pos.XDirection() ^ m_pos.YDirection();
	gp_Pnt pz(o.XYZ() + z_dir.XYZ() * m_height);
	pos = G2P(o);
	pos2 = G2P(pz);
	return 2;
}

bool  CCylinder::GetScaleAboutMatrix(Matrix &m)
{
	m = Matrix(G2P(m_pos.Location()), D2P(m_pos.XDirection()), D2P(m_pos.YDirection()));
	return true;
}

bool CCylinder::Stretch(const Point3d &p, const Point3d &shift, void* data)
{
	gp_Pnt vp = P2G(p);
	gp_Vec vshift = P2V(shift);

	gp_Pnt o = m_pos.Location();
	gp_Pnt px(o.XYZ() + m_pos.XDirection().XYZ() * m_radius);
	gp_Dir z_dir = m_pos.XDirection() ^ m_pos.YDirection();
	gp_Pnt pz(o.XYZ() + z_dir.XYZ() * m_height);

	bool make_a_new_cylinder = false;

	if(px.IsEqual(vp, TOLERANCE)){
		px = px.XYZ() + vshift.XYZ();
		double new_x = gp_Vec(px.XYZ()) * gp_Vec(m_pos.XDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.XDirection());
		double new_y = gp_Vec(px.XYZ()) * gp_Vec(m_pos.YDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.YDirection());
		make_a_new_cylinder = true;
		m_radius = sqrt(new_x * new_x + new_y * new_y);
	}
	else if(pz.IsEqual(vp, TOLERANCE)){
		pz = pz.XYZ() + vshift.XYZ();
		double new_height = gp_Vec(pz.XYZ()) * gp_Vec(z_dir) - gp_Vec(o.XYZ()) * gp_Vec(z_dir);
		if(new_height > 0){
			make_a_new_cylinder = true;
			m_height = new_height;
		}
	}

	if(make_a_new_cylinder)
	{
		CCylinder* new_object = new CCylinder(m_pos, m_radius, m_height, NULL, m_color, (float)m_opacity);
		new_object->CopyIDsFrom(this);
		theApp->StartHistory(L"Cylinder Strech");
		theApp->DeleteUndoably(this);
		theApp->AddUndoably(new_object,m_owner,NULL);
		theApp->EndHistory();
		theApp->ClearSelection(false);
		theApp->Mark(new_object);
	}

	return true;
}

void CCylinder::SetXMLElement(TiXmlElement* element)
{
	const gp_Pnt& l = m_pos.Location();
	element->SetDoubleAttribute("lx", l.X());
	element->SetDoubleAttribute("ly", l.Y());
	element->SetDoubleAttribute("lz", l.Z());

	const gp_Dir& d = m_pos.Direction();
	element->SetDoubleAttribute("dx", d.X());
	element->SetDoubleAttribute("dy", d.Y());
	element->SetDoubleAttribute("dz", d.Z());

	const gp_Dir& x = m_pos.XDirection();
	element->SetDoubleAttribute("xx", x.X());
	element->SetDoubleAttribute("xy", x.Y());
	element->SetDoubleAttribute("xz", x.Z());

	element->SetDoubleAttribute("r", m_radius);
	element->SetDoubleAttribute("h", m_height);

	CSolid::SetXMLElement(element);
}

void CCylinder::SetFromXMLElement(TiXmlElement* pElem)
{
	// get the attributes
	double l[3] = {0, 0, 0};
	double d[3] = {0, 0, 1};
	double x[3] = {1, 0, 0};

	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "lx")	 {l[0] = a->DoubleValue();}
		else if(name == "ly"){l[1] = a->DoubleValue();}
		else if(name == "lz"){l[2] = a->DoubleValue();}

		else if(name == "dx"){d[0] = a->DoubleValue();}
		else if(name == "dy"){d[1] = a->DoubleValue();}
		else if(name == "dz"){d[2] = a->DoubleValue();}

		else if(name == "xx"){x[0] = a->DoubleValue();}
		else if(name == "xy"){x[1] = a->DoubleValue();}
		else if(name == "xz"){x[2] = a->DoubleValue();}

		else if(name == "r"){m_radius = a->DoubleValue();}
		else if(name == "h"){m_height = a->DoubleValue();}
	}

	m_pos = gp_Ax2(make_point(l), make_vector(d), make_vector(x));

	CSolid::SetFromXMLElement(pElem);
}
