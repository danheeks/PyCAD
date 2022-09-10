// CordinateSystem.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "CoordinateSystem.h"
#include "GripData.h"
#include "Material.h"
#include "strconv.h"
#include "Property.h"

double CoordinateSystem::size = 30;
bool CoordinateSystem::size_is_pixels = true;
bool CoordinateSystem::rendering_current = false;

static unsigned char bitmapX[11] = {0x41, 0x41, 0x22, 0x14, 0x14, 0x08, 0x14, 0x14, 0x22, 0x41, 0x41};
static unsigned char bitmapY[11] = {0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x14, 0x14, 0x22, 0x22, 0x41};
static unsigned char bitmapZ[11] = {0x3f, 0x10, 0x10, 0x08, 0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x3f};

CoordinateSystem::CoordinateSystem(const std::wstring& str, const Point3d &o, const Point3d &x, const Point3d &y)
{
	m_title = str;
	m_o = o;
	m_x = x;
	m_y = y;
}



CoordinateSystem::CoordinateSystem(const CoordinateSystem &c)
{
	operator=(c);
}

const CoordinateSystem& CoordinateSystem::operator=(const CoordinateSystem &c)
{
	HeeksObj::operator=(c);
	m_o = c.m_o;
	m_x = c.m_x;
	m_y = c.m_y;
	return *this;
}

CoordinateSystem::~CoordinateSystem(void)
{
}

const wchar_t* CoordinateSystem::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/coordsys.png";
	return iconpath.c_str();
}

// static
void CoordinateSystem::RenderArrow()
{
	glBegin(GL_TRIANGLES);
	// bottom square
	glNormal3d(0, 0, -1);
	glVertex3d(0.1, 0, 0);
	glVertex3d(0, -0.1, 0);
	glVertex3d(0, 0.1, 0);
	glVertex3d(0, 0.1, 0);
	glVertex3d(0, -0.1, 0);
	glVertex3d(-0.1, 0, 0);
	//front right
	glNormal3d(0, -1, 0);
	glVertex3d(0, -0.1, 0);
	glNormal3d(1, 0, 0);
	glVertex3d(0.1, 0, 0);
	glNormal3d(0, -1, 0);
	glVertex3d(0, -0.1, 0.68);
	glVertex3d(0, -0.1, 0.68);
	glNormal3d(1, 0, 0);
	glVertex3d(0.1, 0, 0);
	glVertex3d(0.1, 0, 0.68);
	//back right
	glNormal3d(1, 0, 0);
	glVertex3d(0.1, 0, 0);
	glNormal3d(0, 1, 0);
	glVertex3d(0, 0.1, 0);
	glNormal3d(1, 0, 0);
	glVertex3d(0.1, 0, 0.68);
	glVertex3d(0.1, 0, 0.68);
	glNormal3d(0, 1, 0);
	glVertex3d(0, 0.1, 0);
	glVertex3d(0, 0.1, 0.68);
	//back left
	glNormal3d(0, 1, 0);
	glVertex3d(0, 0.1, 0);
	glNormal3d(-1, 0, 0);
	glVertex3d(-0.1, 0, 0);
	glNormal3d(0, 1, 0);
	glVertex3d(0, 0.1, 0.68);
	glVertex3d(0, 0.1, 0.68);
	glNormal3d(-1, 0, 0);
	glVertex3d(-0.1, 0, 0);
	glVertex3d(-0.1, 0, 0.68);
	//front left
	glNormal3d(-1, 0, 0);
	glVertex3d(-0.1, 0, 0);
	glNormal3d(0, -1, 0);
	glVertex3d(0, -0.1, 0);
	glNormal3d(-1, 0, 0);
	glVertex3d(-0.1, 0, 0.68);
	glVertex3d(-0.1, 0, 0.68);
	glNormal3d(0, -1, 0);
	glVertex3d(0, -0.1, 0);
	glVertex3d(0, -0.1, 0.68);
	//eaves
	glNormal3d(0, 0, -1);
	glVertex3d(0, -0.19, 0.68);
	glVertex3d(0, -0.1, 0.68);
	glVertex3d(0.1, 0, 0.68);
	glVertex3d(0, -0.19, 0.68);
	glVertex3d(0.1, 0, 0.68);
	glVertex3d(0.19, 0, 0.68);
	glVertex3d(0.19, 0, 0.68);
	glVertex3d(0.1, 0, 0.68);
	glVertex3d(0, 0.1, 0.68);
	glVertex3d(0.19, 0, 0.68);
	glVertex3d(0, 0.1, 0.68);
	glVertex3d(0, 0.19, 0.68);
	glVertex3d(0, 0.19, 0.68);
	glVertex3d(0, 0.1, 0.68);
	glVertex3d(-0.1, 0, 0.68);
	glVertex3d(0, 0.19, 0.68);
	glVertex3d(-0.1, 0, 0.68);
	glVertex3d(-0.19, 0, 0.68);
	glVertex3d(-0.19, 0, 0.68);
	glVertex3d(-0.1, 0, 0.68);
	glVertex3d(0, -0.1, 0.68);
	glVertex3d(-0.19, 0, 0.68);
	glVertex3d(0, -0.1, 0.68);
	glVertex3d(0, -0.19, 0.68);
	// roof
	glNormal3d(0.608, -0.608, 0.511);
	glVertex3d(0, 0, 1);
	glNormal3d(0, -0.86, 0.511);
	glVertex3d(0, -0.19, 0.68);
	glNormal3d(0.86, 0, 0.511);
	glVertex3d(0.19, 0, 0.68);
	glNormal3d(0.608, 0.608, 0.511);
	glVertex3d(0, 0, 1);
	glNormal3d(0.86, 0, 0.511);
	glVertex3d(0.19, 0, 0.68);
	glNormal3d(0, 0.86, 0.511);
	glVertex3d(0, 0.19, 0.68);
	glNormal3d(-0.608, -0.608, 0.511);
	glVertex3d(0, 0, 1);
	glNormal3d(0, 0.86, 0.511);
	glVertex3d(0, 0.19, 0.68);
	glNormal3d(-0.86, 0, 0.511);
	glVertex3d(-0.19, 0, 0.68);
	glNormal3d(-0.608, -0.608, 0.511);
	glVertex3d(0, 0, 1);
	glNormal3d(-0.86, 0, 0.511);
	glVertex3d(-0.19, 0, 0.68);
	glNormal3d(0, -0.86, 0.511);
	glVertex3d(0, -0.19, 0.68);
	glEnd();
}

// static
void CoordinateSystem::RenderDatum(bool bright, bool solid, bool no_color)
{
	double s = size;
	if(size_is_pixels)s /= theApp->GetPixelScale();

	if(solid)
	{
		// render an arrow, like I saw on a commercial CAD system
		if (!no_color)
		{
			glEnable(GL_LIGHTING);
			glShadeModel(GL_SMOOTH);
		}
		glPushMatrix();
		glScaled(s, s, s);
		if (!no_color)
		{
			if (bright)Material(HeeksColor(0, 0, 255)).glMaterial(1.0);
			else Material(HeeksColor(64, 64, 128)).glMaterial(1.0);
		}
		RenderArrow();
		if (!no_color)
		{
			if (bright)Material(HeeksColor(255, 0, 0)).glMaterial(1.0);
			else Material(HeeksColor(128, 64, 64)).glMaterial(1.0);
		}
		glRotated(90, 0, 1, 0);
		RenderArrow();
		if (!no_color)
		{
			if (bright)Material(HeeksColor(0, 255, 0)).glMaterial(1.0);
			else Material(HeeksColor(64, 128, 64)).glMaterial(1.0);
		}
		glRotated(90, -1, 0, 0);
		RenderArrow();
		glPopMatrix();
		if (!no_color)
		{
			glShadeModel(GL_FLAT);
			glDisable(GL_LIGHTING);
		}
	}
	else
	{
		// red, green, blue for x, y, z, like I saw on Open Arena
		glBegin(GL_LINES);
		if (!no_color)
		{
			if (bright)glColor3ub(255, 0, 0);
			else glColor3ub(128, 64, 64);
		}
		glVertex3d(0, 0, 0);
		glVertex3d(s, 0, 0);
		if (!no_color)
		{
			if (bright)glColor3ub(0, 255, 0);
			else glColor3ub(64, 128, 64);
		}
		glVertex3d(0, 0, 0);
		glVertex3d(0, s, 0);
		if (!no_color)
		{
			if (bright)glColor3ub(0, 0, 255);
			else glColor3ub(64, 64, 128);
		}
		glVertex3d(0, 0, 0);
		glVertex3d(0, 0, s);
		glEnd();
	}

	if(bright && !no_color)
	{
		// render X, Y, Z text
		double extra_pixels_out = 10.0;
		double s2 = s + extra_pixels_out /theApp->GetPixelScale();
		glColor3ub(255, 0, 0);
		glRasterPos3d(s2, 0, 0);
		glBitmap(8, 11, 3, 5, 0.0, 0.0, bitmapX);
		glColor3ub(0, 255, 0);
		glRasterPos3d(0, s2, 0);
		glBitmap(8, 11, 3, 5, 0.0, 0.0, bitmapY);
		glColor3ub(0, 0, 255);
		glRasterPos3d(0, 0, s2);
		glBitmap(8, 11, 3, 5, 0.0, 0.0, bitmapZ);
	}
}

void CoordinateSystem::ApplyMatrix()
{
	double m[16];
	GetMatrix().GetTransposed(m);
	glMultMatrixd(m);
}

void CoordinateSystem::glCommands(bool select, bool marked, bool no_color)
{
	if(!select && !rendering_current && this == theApp->m_current_coordinate_system)return; // will get rendered in HeeksCADapp::glCommandsAll
	if(marked)glLineWidth(2);
	glPushMatrix();
	double m[16];
	GetMatrix().GetTransposed(m);
	glMultMatrixd(m);

	bool bright = rendering_current;

	RenderDatum(bright, theApp->m_datum_coords_system_solid_arrows, no_color);

	glPopMatrix();
	glLineWidth(1);
}

void CoordinateSystem::GetBox(CBox &box)
{
	Point3d vt(0, 0, 0);
	vt = vt.Transformed(GetMatrix());
	double p[3];
	vt.get(p);
	box.Insert(p);
}

void CoordinateSystem::OnEditString(const wchar_t* str){
	m_title.assign(str);
}

HeeksObj *CoordinateSystem::MakeACopy(void)const
{
	return new CoordinateSystem(*this);
}

void CoordinateSystem::Transform(const Matrix &m)
{
	m_o = m_o.Transformed(m);
	m_x = m_x.TransformedOnlyRotation(m);
	m_y = m_y.TransformedOnlyRotation(m);
}


void CoordinateSystem::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyString(this, L"title", &m_title));
	list->push_back(PropertyPnt(this, L"position", &m_o));
	list->push_back(PropertyPnt(this, L"x-axis", &m_x));
	list->push_back(PropertyPnt(this, L"y-axis", &m_y));

	HeeksObj::GetProperties(list);
}

void CoordinateSystem::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	Matrix mat = GetMatrix();
	double s = size;
	if(size_is_pixels)s /= theApp->GetPixelScale();

	Point3d px(m_o + m_x * s);
	Point3d py(m_o + m_y * s);
	Point3d vz = Point3d(0, 0, 1).Transformed(mat) - m_o;
	Point3d pz(m_o + vz * s);
	list->push_back(GripData(GripperTypeTranslate,m_o,NULL));
	list->push_back(GripData(GripperTypeRotateObject,px,NULL));
	list->push_back(GripData(GripperTypeRotateObject,py,NULL));
	list->push_back(GripData(GripperTypeRotateObject,pz,NULL));
}

bool CoordinateSystem::GetScaleAboutMatrix(Matrix &m)
{
	m = GetMatrix();
	return true;
}

void CoordinateSystem::WriteToXML(TiXmlElement *element)
{
	element->SetAttribute("title", Ttc(m_title.c_str()) );
	element->SetDoubleAttribute("ox", m_o.x);
	element->SetDoubleAttribute("oy", m_o.y);
	element->SetDoubleAttribute("oz", m_o.z);
	element->SetDoubleAttribute("xx", m_x.x);
	element->SetDoubleAttribute("xy", m_x.y);
	element->SetDoubleAttribute("xz", m_x.z);
	element->SetDoubleAttribute("yx", m_y.x);
	element->SetDoubleAttribute("yy", m_y.y);
	element->SetDoubleAttribute("yz", m_y.z);
	HeeksObj::WriteToXML(element);
}

Matrix CoordinateSystem::GetMatrix()
{
	return Matrix(m_o, m_x, m_y);
}

void CoordinateSystem::ReadFromXML(TiXmlElement *element)
{
	element->Attribute("ox", &m_o.x);
	element->Attribute("oy", &m_o.y);
	element->Attribute("oz", &m_o.z);
	element->Attribute("xx", &m_x.x);
	element->Attribute("xy", &m_x.y);
	element->Attribute("xz", &m_x.z);
	element->Attribute("yx", &m_y.x);
	element->Attribute("yy", &m_y.y);
	element->Attribute("yz", &m_y.z);
	const char* title = element->Attribute("title");
	if (title)m_title.assign(Ctt(title));
	HeeksObj::ReadFromXML(element);
}

#if 0
static CoordinateSystem* coordinate_system_for_PickFrom3Points = NULL;
static Point3d y_for_PickFrom3Points(0, 1, 0);
static Point3d z_for_PickFrom3Points(0, 0, 1);
static const double unit_vec_tol = 0.0000000001;


static void OnGlCommandsForPickFrom3Points()
{
	CoordinateSystem::rendering_current = true;
	coordinate_system_for_PickFrom3Points->glCommands(false, true, false);
	CoordinateSystem::rendering_current = false;
}
#endif

bool CoordinateSystem::PickFrom3Points()
{
#if 0
	CoordinateSystem temp = *this;
	coordinate_system_for_PickFrom3Points = &temp;
	y_for_PickFrom3Points = m_y;
	z_for_PickFrom3Points = m_x ^ m_y;
	m_visible = false;
	theApp->RegisterOnGLCommands(OnGlCommandsForPickFrom3Points);

	double pos[3];

	bool result = false;

	if(theApp->PickPosition(_("Pick the location"), pos, on_set_origin))
	{
		if(theApp->PickPosition(_("Pick a point on the x-axis"), pos, on_set_x))
		{
			result = theApp->PickPosition(_("Pick a point where y > 0"), pos, on_set_y);
		}
	}

	*this = temp;
	theApp->RemoveOnGLCommands(OnGlCommandsForPickFrom3Points);

	theApp->Repaint();

	return result;
#endif
	return true;
}

bool CoordinateSystem::PickFrom1Point()
{
#if 0
	CoordinateSystem temp = *this;
	coordinate_system_for_PickFrom3Points = &temp;
	y_for_PickFrom3Points = m_y;
	z_for_PickFrom3Points = m_x ^ m_y;
	m_visible = false;
	theApp->RegisterOnGLCommands(OnGlCommandsForPickFrom3Points);

	double pos[3];

	bool result = theApp->PickPosition(_("Pick the location"), pos, on_set_origin);
	
	*this = temp;
	theApp->RemoveOnGLCommands(OnGlCommandsForPickFrom3Points);

	theApp->Repaint();

	return result;
#endif
	return false;
}

