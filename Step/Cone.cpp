// Cone.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Cone.h"
#include "Gripper.h"
#include "GripData.h"
#include "CoordinateSystem.h"
#include "PropertySolid.h"

CCone::CCone(const gp_Ax2& pos, double r1, double r2, double height, const wchar_t* title, const HeeksColor& col, float opacity):CSolid(BRepPrimAPI_MakeCone(pos, r1, r2, height), title, col, opacity), m_render_without_OpenCASCADE(false), m_pos(pos), m_r1(r1), m_r2(r2), m_height(height)
{
}

CCone::CCone(const TopoDS_Solid &solid, const wchar_t* title, const HeeksColor& col, float opacity):CSolid(solid, title, col, opacity), m_render_without_OpenCASCADE(false), m_pos(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0)), m_r1(0.0), m_r2(0.0), m_height(0.0)
{
}

const wchar_t* CCone::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/cone.png";
	return iconpath.c_str();
}

void CCone::glCommands(bool select, bool marked, bool no_color)
{
	if(!m_render_without_OpenCASCADE)
	{
		CSolid::glCommands(select, marked, no_color);
		return;
	}


	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	Material(m_color).glMaterial(1.0);

	int num_facets = 20;
	gp_Pnt pt_top = m_pos.Location().XYZ() + m_pos.Direction().XYZ() * m_height;

	for(int angle_pos = 0; angle_pos<num_facets; angle_pos++)
	{
		double angle0 = ((double)angle_pos)/num_facets * 6.2831853071795;
		double angle1 = ((double)(angle_pos + 1))/num_facets * 6.2831853071795;

		gp_Dir dir0(m_pos.XDirection().XYZ() * cos(angle0) + m_pos.YDirection().XYZ() * sin(angle0));
		gp_Dir dir1(m_pos.XDirection().XYZ() * cos(angle1) + m_pos.YDirection().XYZ() * sin(angle1));

		double p[4][3];
		extract(m_pos.Location().XYZ() + dir0.XYZ() * m_temp_r1, p[0]);
		extract(m_pos.Location().XYZ() + dir1.XYZ() * m_temp_r1, p[1]);
		extract(pt_top.XYZ() + dir1.XYZ() * m_temp_r2, p[2]);
		extract(pt_top.XYZ() + dir0.XYZ() * m_temp_r2, p[3]);

		gp_Dir upside0(p[3][0] - p[0][0], p[3][1] - p[0][1], p[3][2] - p[0][2]);
		gp_Dir upside1(p[2][0] - p[1][0], p[2][1] - p[1][1], p[2][2] - p[1][2]);
		gp_Dir around0( m_pos.YDirection().XYZ() * cos(angle0) - m_pos.XDirection().XYZ() * sin(angle0));
		gp_Dir around1( m_pos.YDirection().XYZ() * cos(angle1) - m_pos.XDirection().XYZ() * sin(angle1));
		gp_Dir norm0 = around0 ^ upside0;
		gp_Dir norm1 = around1 ^ upside1;

		double n0[3], n1[3];
		extract(norm0, n0);
		extract(norm1, n1);

		double up[3];
		extract(m_pos.Direction(), up);
		double down[3] = {-up[0], -up[1], -up[2]};
		double p_bot[3], p_top[3];
		extract(m_pos.Location(), p_bot);
		extract(pt_top, p_top);

		glBegin(GL_TRIANGLES);
		glNormal3dv(n0);
		glVertex3dv(p[0]);
		glNormal3dv(n1);
		glVertex3dv(p[1]);
		glNormal3dv(n1);
		glVertex3dv(p[2]);
		glNormal3dv(n0);
		glVertex3dv(p[0]);
		glNormal3dv(n1);
		glVertex3dv(p[2]);
		glNormal3dv(n0);
		glVertex3dv(p[3]);
		glNormal3dv(up);
		glVertex3dv(p[3]);
		glVertex3dv(p[2]);
		glVertex3dv(p_top);
		glNormal3dv(down);
		glVertex3dv(p_bot);
		glVertex3dv(p[1]);
		glVertex3dv(p[0]);
		glEnd();
	}

	glDisable(GL_LIGHTING);
	glShadeModel(GL_FLAT);

	glColor3ub(0, 0, 0);
	glBegin(GL_LINE_STRIP);
	for(int angle_pos = 0; angle_pos<=num_facets; angle_pos++)
	{
		double angle = ((double)angle_pos)/num_facets * 6.2831853071795;

		gp_Dir dir(m_pos.XDirection().XYZ() * cos(angle) + m_pos.YDirection().XYZ() * sin(angle));

		double p[3];
		extract(m_pos.Location().XYZ() + dir.XYZ() * m_temp_r1, p);
		glVertex3dv(p);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for(int angle_pos = 0; angle_pos<=num_facets; angle_pos++)
	{
		double angle = ((double)angle_pos)/num_facets * 6.2831853071795;

		gp_Dir dir(m_pos.XDirection().XYZ() * cos(angle) + m_pos.YDirection().XYZ() * sin(angle));

		double p[3];
		extract(pt_top.XYZ() + dir.XYZ() * m_temp_r2, p);
		glVertex3dv(p);
	}
	glEnd();
}

HeeksObj *CCone::MakeACopy(void)const
{
	return new CCone(*this);
}

void CCone::MakeTransformedShape(const gp_Trsf &mat)
{
	m_pos.Transform(mat);
	double scale = gp_Vec(1, 0, 0).Transformed(mat).Magnitude();
	m_r1 = fabs(m_r1 * scale);
	m_r2 = fabs(m_r2 * scale);
	m_height = fabs(m_height * scale);
	m_shape = BRepPrimAPI_MakeCone(m_pos, m_r1, m_r2, m_height).Shape();
}

std::wstring CCone::StretchedName(){ return L"Stretched Cone";}

void CCone::GetProperties(std::list<Property *> *list)
{
	GetAx2Properties(list, m_pos, this);

	list->push_back((Property*)(new PropertySolidLength(this, L"r1", &m_r1)));
	list->push_back((Property*)(new PropertySolidLength(this, L"r2", &m_r2)));
	list->push_back((Property*)(new PropertySolidLength(this, L"height", &m_height)));

	CSolid::GetProperties(list);
}

void CCone::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	gp_Pnt o = m_pos.Location();
	gp_Pnt px(o.XYZ() + m_pos.XDirection().XYZ() * m_r1);
	gp_Dir z_dir = m_pos.XDirection() ^ m_pos.YDirection();
	gp_Pnt py(o.XYZ() + m_pos.YDirection().XYZ() * m_r1);
	gp_Pnt pyz(o.XYZ() + m_pos.YDirection().XYZ() * m_r2 + z_dir.XYZ() * m_height);
	gp_Pnt pmx(o.XYZ() + m_pos.XDirection().XYZ() * (-m_r1));
	gp_Pnt pz(o.XYZ() + z_dir.XYZ() * m_height);
	gp_Pnt pxz(o.XYZ() + m_pos.XDirection().XYZ() * m_r2 + z_dir.XYZ() * m_height);
	list->push_back(GripData(GripperTypeTranslate,G2P(o),NULL));
	list->push_back(GripData(GripperTypeStretch, G2P(px), NULL));
	list->push_back(GripData(GripperTypeObjectScaleZ, G2P(pz), NULL));
	list->push_back(GripData(GripperTypeStretch, G2P(pxz), NULL));
	list->push_back(GripData(GripperTypeRotateObject, G2P(py), NULL));
	list->push_back(GripData(GripperTypeRotateObject, G2P(pmx), NULL));
}

void CCone::OnApplyPropertiesRaw()
{
	bool save_visible = m_visible;
	*this = CCone(m_pos, m_r1, m_r2, m_height, m_title.c_str(), m_color, (float)m_opacity);
	m_visible = save_visible;
	this->create_faces_and_edges();
}

bool CCone::ValidateProperties()
{
	if(m_r1 <= -TOLERANCE || m_r2 <= -TOLERANCE)
	{
		theApp->DoMessageBox(L"can not enter negative value for radius");
		return false;
	}
	if(m_r1 < 0)m_r1 = 0;
	if(m_r2 < 0)m_r2 = 0;
	if(fabs(m_r1-m_r2) < TOLERANCE)
	{
		theApp->DoMessageBox(L"cone must not have both radiuses the same");
		return false;
	}
	return true;
}

bool CCone::GetScaleAboutMatrix(Matrix &m)
{
	m = Matrix(G2P(m_pos.Location()), D2P(m_pos.XDirection()), D2P(m_pos.YDirection()));
	return true;
}

bool CCone::Stretch2(const Point3d &p, const Point3d &shift, gp_Ax2& new_pos, double& new_r1, double& new_r2, double& new_height)
{
	gp_Pnt vp = make_point(&p.x);
	gp_Vec vshift = make_vector(&shift.x);

	gp_Pnt o = m_pos.Location();
	gp_Pnt px(o.XYZ() + m_pos.XDirection().XYZ() * m_r1);
	gp_Dir z_dir = m_pos.XDirection() ^ m_pos.YDirection();
	gp_Pnt py(o.XYZ() + m_pos.YDirection().XYZ() * m_r1);
	gp_Pnt pyz(o.XYZ() + m_pos.YDirection().XYZ() * m_r2 + z_dir.XYZ() * m_height);
	gp_Pnt pmxz(o.XYZ() + m_pos.XDirection().XYZ() * (-m_r2) + z_dir.XYZ() * m_height);
	gp_Pnt pz(o.XYZ() + z_dir.XYZ() * m_height);
	gp_Pnt pxz(o.XYZ() + m_pos.XDirection().XYZ() * m_r2 + z_dir.XYZ() * m_height);

	bool make_a_new_cone = false;

	if(px.IsEqual(vp, TOLERANCE)){
		px = px.XYZ() + vshift.XYZ();
		double new_x = gp_Vec(px.XYZ()) * gp_Vec(m_pos.XDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.XDirection());
		double new_y = gp_Vec(px.XYZ()) * gp_Vec(m_pos.YDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.YDirection());
		new_r1 = sqrt(new_x * new_x + new_y * new_y);
		if(fabs(new_r1 - m_r2) > 0.000000001){
			make_a_new_cone = true;
		}
	}
	else if(pxz.IsEqual(vp, TOLERANCE)){
		pxz = pxz.XYZ() + vshift.XYZ();
		double new_x = gp_Vec(pxz.XYZ()) * gp_Vec(m_pos.XDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.XDirection());
		double new_y = gp_Vec(pxz.XYZ()) * gp_Vec(m_pos.YDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.YDirection());
		new_r2 = sqrt(new_x * new_x + new_y * new_y);
		if(fabs(m_r1 - new_r2) > 0.000000001){
			make_a_new_cone = true;
		}
	}
	else if(pz.IsEqual(vp, TOLERANCE)){
		pz = pz.XYZ() + vshift.XYZ();
		new_height = gp_Vec(pz.XYZ()) * gp_Vec(z_dir) - gp_Vec(o.XYZ()) * gp_Vec(z_dir);
		if(new_height > 0){
			make_a_new_cone = true;
		}
	}

	return make_a_new_cone;
}

bool CCone::Stretch(const Point3d &p, const Point3d &shift, void* data)
{
	gp_Ax2 new_pos = m_pos;
	double new_r1 = m_r1;
	double new_r2 = m_r2;
	double new_height = m_height;

	bool make_a_new_cone = Stretch2(p, shift, new_pos, new_r1, new_r2, new_height);

	if(make_a_new_cone)
	{
		CCone* new_object = new CCone(new_pos, new_r1, new_r2, new_height, NULL, m_color, (float)m_opacity);
		new_object->CopyIDsFrom(this);
		theApp->StartHistory(L"Strech Cone");
		theApp->DeleteUndoably(this);
		theApp->AddUndoably(new_object,m_owner,NULL);
		theApp->EndHistory();
		theApp->ClearSelection(false);
		theApp->Mark(new_object);
		this->m_render_without_OpenCASCADE = false;
	}

	return true;
}

bool CCone::StretchTemporary(const Point3d &p, const Point3d &shift, void* data)
{
	gp_Ax2 new_pos = m_pos;
	double new_r1 = m_r1;
	double new_r2 = m_r2;
	double new_height = m_height;

	if(Stretch2(p, shift, new_pos, new_r1, new_r2, new_height))
	{
		m_render_without_OpenCASCADE = true;
		m_temp_r1 = new_r1;
		m_temp_r2 = new_r2;
		theApp->Repaint();
	}
	return false;
}

void CCone::SetXMLElement(TiXmlElement* element)
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

	element->SetDoubleAttribute("r1", m_r1);
	element->SetDoubleAttribute("r2", m_r2);
	element->SetDoubleAttribute("h", m_height);

	CSolid::SetXMLElement(element);
}

void CCone::SetFromXMLElement(TiXmlElement* pElem)
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

		else if(name == "r1"){m_r1 = a->DoubleValue();}
		else if(name == "r2"){m_r2 = a->DoubleValue();}
		else if(name == "h"){m_height = a->DoubleValue();}
	}

	m_pos = gp_Ax2(make_point(l), make_vector(d), make_vector(x));

	CSolid::SetFromXMLElement(pElem);
}
