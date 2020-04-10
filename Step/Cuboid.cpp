// Cuboid.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Cuboid.h"
#include "Gripper.h"
#include "GripData.h"
#include "CoordinateSystem.h"

CCuboid::CCuboid(const gp_Ax2& pos, double x, double y, double z, const wchar_t* title, const HeeksColor& col, float opacity)
:CSolid(BRepPrimAPI_MakeBox(gp_Ax2(pos.Location().XYZ() + gp_XYZ((x < 0) ? x:0.0, (y < 0) ? y:0.0, (z < 0) ? z:0.0), pos.Direction(), pos.XDirection()), fabs(x), fabs(y), fabs(z)), title, col, opacity)
, m_pos(pos), m_x(x), m_y(y), m_z(z)
{
}

CCuboid::CCuboid(const TopoDS_Solid &solid, const wchar_t* title, const HeeksColor& col, float opacity):CSolid(solid, title, col, opacity), m_pos(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0)), m_x(0.0), m_y(0.0), m_z(0.0)
{
}

const wchar_t* CCuboid::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/cube.png";
	return iconpath.c_str();
}

HeeksObj *CCuboid::MakeACopy(void)const
{
	return new CCuboid(*this);
}

CCuboid::CCuboid( const CCuboid & rhs ) : CSolid(rhs)
{
    m_pos = rhs.m_pos;
    m_x = rhs.m_x;
    m_y = rhs.m_y;
    m_z = rhs.m_z;
}

CCuboid & CCuboid::operator= ( const CCuboid & rhs )
{
    if (this != &rhs)
    {
        m_pos = rhs.m_pos;
        m_x = rhs.m_x;
        m_y = rhs.m_y;
        m_z = rhs.m_z;

        CSolid::operator=( rhs );
    }

    return(*this);
}

void CCuboid::MakeTransformedShape(const gp_Trsf &mat)
{
	m_pos.Transform(mat);
	double scale = gp_Vec(1, 0, 0).Transformed(mat).Magnitude();
	m_x = fabs(m_x * scale);
	m_y = fabs(m_y * scale);
	m_z = fabs(m_z * scale);
	m_shape = BRepPrimAPI_MakeBox(m_pos, m_x, m_y, m_z).Shape();
}

std::wstring CCuboid::StretchedName(){ return L"Stretched Cuboid";}

void CCuboid::GetProperties(std::list<Property *> *list)
{
	GetAx2Properties(list, m_pos, this);
#if 0
	// to do
	list->push_back(new PropertyLength(this, L"width ( x )", &m_x));
	list->push_back(new PropertyLength(this, L"height( y )", &m_y));
	list->push_back(new PropertyLength(this, L"depth ( z )", &m_z));
#endif
	CSolid::GetProperties(list);
}

void CCuboid::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	gp_Pnt o = m_pos.Location();
	gp_Pnt px(o.XYZ() + m_pos.XDirection().XYZ() * m_x);
	gp_Pnt py(o.XYZ() + m_pos.YDirection().XYZ() * m_y);
	gp_Dir z_dir = m_pos.XDirection() ^ m_pos.YDirection();
	gp_Pnt pz(o.XYZ() + z_dir.XYZ() * m_z);
	gp_Pnt m2(o.XYZ() + m_pos.XDirection().XYZ() * m_x + m_pos.YDirection().XYZ() * m_y/2);
	gp_Pnt m3(o.XYZ() + m_pos.XDirection().XYZ() * m_x/2 + m_pos.YDirection().XYZ() * m_y);
	gp_Pnt m8(o.XYZ() + m_pos.YDirection().XYZ() * m_y/2 + z_dir.XYZ() * m_z);
	gp_Pnt pxy(o.XYZ() + m_pos.XDirection().XYZ() * m_x + m_pos.YDirection().XYZ() * m_y);
	gp_Pnt pxz(o.XYZ() + m_pos.XDirection().XYZ() * m_x + z_dir.XYZ() * m_z);
	gp_Pnt pyz(o.XYZ() + m_pos.YDirection().XYZ() * m_y + z_dir.XYZ() * m_z);
	gp_Pnt pxyz(o.XYZ() + m_pos.XDirection().XYZ() * m_x  + m_pos.YDirection().XYZ() * m_y + z_dir.XYZ() * m_z);
	list->push_back(GripData(GripperTypeTranslate,G2P(o),NULL));
	list->push_back(GripData(GripperTypeRotateObject, G2P(px), NULL));
	list->push_back(GripData(GripperTypeRotateObject, G2P(py), NULL));
	list->push_back(GripData(GripperTypeRotateObject, G2P(pz), NULL));
	list->push_back(GripData(GripperTypeScale, G2P(pxyz), NULL));
	list->push_back(GripData(GripperTypeRotate, G2P(pxy), NULL));
	list->push_back(GripData(GripperTypeRotate, G2P(pxz), NULL));
	list->push_back(GripData(GripperTypeRotate, G2P(pyz), NULL));
	list->push_back(GripData(GripperTypeObjectScaleX, G2P(m2), NULL));
	list->push_back(GripData(GripperTypeObjectScaleY, G2P(m3), NULL));
	list->push_back(GripData(GripperTypeObjectScaleZ, G2P(m8), NULL));
}

void CCuboid::OnApplyProperties()
{
	*this = CCuboid(m_pos, m_x, m_y, m_z, m_title.c_str(), m_color,(float) m_opacity);
	this->create_faces_and_edges();
	theApp->Repaint();
}


bool CCuboid::GetScaleAboutMatrix(double *m)
{
	gp_Trsf mat = make_matrix(m_pos.Location(), m_pos.XDirection(), m_pos.YDirection());
	extract(mat, m);
	return true;
}

bool CCuboid::Stretch(const double *p, const double* shift, void* data)
{
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	gp_Pnt o = m_pos.Location();
	gp_Dir z_dir = m_pos.XDirection() ^ m_pos.YDirection();
	gp_Pnt m2(o.XYZ() + m_pos.XDirection().XYZ() * m_x + m_pos.YDirection().XYZ() * m_y/2);
	gp_Pnt m3(o.XYZ() + m_pos.XDirection().XYZ() * m_x/2 + m_pos.YDirection().XYZ() * m_y);
	gp_Pnt m8(o.XYZ() + m_pos.YDirection().XYZ() * m_y/2 + z_dir.XYZ() * m_z);

	bool make_a_new_cuboid = false;

	if(m2.IsEqual(vp, TOLERANCE)){
		m2 = m2.XYZ() + vshift.XYZ();
		double new_x = gp_Vec(m2.XYZ()) * gp_Vec(m_pos.XDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.XDirection());
		if(new_x > 0){
			make_a_new_cuboid = true;
			m_x = new_x;
		}
	}
	else if(m3.IsEqual(vp, TOLERANCE)){
		m3 = m3.XYZ() + vshift.XYZ();
		double new_y = gp_Vec(m3.XYZ()) * gp_Vec(m_pos.YDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.YDirection());
		if(new_y > 0){
			make_a_new_cuboid = true;
			m_y = new_y;
		}
	}
	else if(m8.IsEqual(vp, TOLERANCE)){
		m8 = m8.XYZ() + vshift.XYZ();
		double new_z = gp_Vec(m8.XYZ()) * gp_Vec(z_dir) - gp_Vec(o.XYZ()) * gp_Vec(z_dir);
		if(new_z > 0){
			make_a_new_cuboid = true;
			m_z = new_z;
		}
	}

	if(make_a_new_cuboid)
	{
		CCuboid* new_object = new CCuboid(m_pos, m_x, m_y, m_z, NULL, m_color, (float)m_opacity);
		new_object->CopyIDsFrom(this);
		theApp->StartHistory();
		theApp->DeleteUndoably(this);
		theApp->AddUndoably(new_object,m_owner,NULL);
		theApp->EndHistory();
		theApp->ClearSelection(false);
		theApp->Mark(new_object);
	}

	return true;
}

void CCuboid::SetXMLElement(TiXmlElement* element)
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

	element->SetDoubleAttribute("wx", m_x);
	element->SetDoubleAttribute("wy", m_y);
	element->SetDoubleAttribute("wz", m_z);

	CSolid::SetXMLElement(element);
}

void CCuboid::SetFromXMLElement(TiXmlElement* pElem)
{
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

		else if(name == "wx"){m_x = a->DoubleValue();}
		else if(name == "wy"){m_y = a->DoubleValue();}
		else if(name == "wz"){m_z = a->DoubleValue();}
	}

	m_pos = gp_Ax2(make_point(l), make_vector(d), make_vector(x));

	CSolid::SetFromXMLElement(pElem);
}

void CCuboid::GetOnEdit(bool(**callback)(HeeksObj*, std::list<HeeksObj*> *))
{
	// to do, dialogs for drawing objects
	//*callback = OnEdit;
}
