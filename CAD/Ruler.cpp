// Ruler.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Ruler.h"
#include "Gripper.h"
#include "Property.h"

void RulerMark::glCommands(double units)
{
	glDisable(GL_POLYGON_OFFSET_FILL);
	double half_width = width/2;
	double dpos = (double)pos;
	if(units > 25.0)
	{
		dpos *= 2.54; // position of the tenth of an inch, in mm
	}

	if(theApp->GetPixelScale() < 10)
	{
		// draw a line
		glBegin(GL_LINES);
		glVertex2d(dpos, 0.0);
		glVertex2d(dpos, -length);
		glEnd();
	}
	else
	{
		// draw triangles
		double p[5][3] = {
			{dpos, 0.0, 0.0},
			{dpos - half_width, -half_width, 0.0},
			{dpos - half_width, -length, 0.0},
			{dpos + half_width, -length, 0.0},
			{dpos + half_width, -half_width, 0.0}
		};

		glBegin(GL_TRIANGLES);
		glVertex3dv(p[0]);
		glVertex3dv(p[1]);
		glVertex3dv(p[4]);
		glVertex3dv(p[1]);
		glVertex3dv(p[2]);
		glVertex3dv(p[4]);
		glVertex3dv(p[2]);
		glVertex3dv(p[3]);
		glVertex3dv(p[4]);
		glEnd();
	}

	// draw text
	if(pos == 0)
	{
		std::wstring str = L"cm";
		if(units > 25.0)str = L"inches";
		glPushMatrix();
		glTranslated(dpos + half_width, -length + 2.05, 0.0);
		glColor4ub(0, 0, 0, 255);
		theApp->render_text(str.c_str(), false, 1.0, 1.0 / theApp->GetPixelScale());
		glPopMatrix();
	}
	else if(pos % 10 == 0)
	{
#if 0
		to do
		float text_width, text_height;
		std::wstring str = std::wstring::Format(L"%d", pos/10);
		if(!theApp->get_text_size(str, &text_width, &text_height))return;
		glPushMatrix();
		glTranslated(dpos - half_width - text_width, -length + 2.05, 0.0);
		theApp->render_text(str, false);
		glPopMatrix();
#endif
	}
	glEnable(GL_POLYGON_OFFSET_FILL);
}

HRuler::HRuler() : m_gl_list(0), m_select_gl_list(0)
{
	m_use_view_units = true;
	m_units = 1.0;
	m_width = 25;
	m_length = 312; // long enough for 12 inches
	m_empty_length = 3;
}

void HRuler::GetFourCorners(Point3d *point)
{
	point[0] = Point3d(-m_empty_length, -m_width, 0);
	point[1] = Point3d(m_length - m_empty_length, -m_width, 0);
	point[2] = Point3d(m_length - m_empty_length, 0, 0);
	point[3] = Point3d(-m_empty_length, 0, 0);
}

double HRuler::GetUnits()
{
	if(m_use_view_units)return theApp->m_view_units;
	return m_units;
}

void HRuler::CalculateMarks(std::list<RulerMark> &marks)
{
	if(GetUnits() > 25.0)
	{
		// inches
		int num_tenths = (int)(m_length / 2.54 - 2 * m_empty_length / 2.54 + 0.0001);

		for(int i = 0; i<= num_tenths; i++)
		{
			RulerMark mark;
			if(i % 10 == 0)
			{
				// big mark
				mark.length = 3.0; 
				mark.width = 0.1;
			}
			else if(i % 5 == 0)
			{
				// medium mark
				mark.length = 2.0; 
				mark.width = 0.1;
			}
			else
			{
				// small mark
				mark.length = 1.0; 
				mark.width = 0.1;
			}

			mark.pos = i;

			marks.push_back(mark);
		}
	}
	else
	{
		int num_mm = (int)(m_length - 2 * m_empty_length + 0.0001);

		for(int i = 0; i<= num_mm; i++)
		{
			RulerMark mark;
			if(i % 10 == 0)
			{
				// big mark
				mark.length = 3.0; 
				mark.width = 0.1;
			}
			else if(i % 5 == 0)
			{
				// medium mark
				mark.length = 2.0; 
				mark.width = 0.1;
			}
			else
			{
				// small mark
				mark.length = 1.0; 
				mark.width = 0.1;
			}

			mark.pos = i;

			marks.push_back(mark);
		}
	}
}

void HRuler::glCommands(bool select, bool marked, bool no_color)
{
	double m[16];
	m_trsf.GetTransposed(m);
	glPushMatrix();
	glMultMatrixd(m);

	int* plist = select ? &m_select_gl_list : &m_gl_list;
	if (*plist)
	{
		glCallList(*plist);
	}
	else{
		*plist = glGenLists(1);
		glNewList(*plist, GL_COMPILE_AND_EXECUTE);

		// draw a filled white rectangle
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 0.0);
		if(!no_color)glColor4ub(255, 255, 255, 120); // white
		if (!select)theApp->EnableBlend();
		glDepthMask(0);
		Point3d point[4];
		GetFourCorners(point);
		glBegin(GL_TRIANGLES);
		glVertex3d(point[0].x, point[0].y, point[0].z);
		glVertex3d(point[1].x, point[1].y, point[1].z);
		glVertex3d(point[2].x, point[2].y, point[2].z);
		glVertex3d(point[0].x, point[0].y, point[0].z);
		glVertex3d(point[2].x, point[2].y, point[2].z);
		glVertex3d(point[3].x, point[3].y, point[3].z);
		glEnd();
		if(select)theApp->DisableBlend();
		glDepthMask(1);

		if (!no_color)
		{
			// draw a black rectangle border
			glColor4ub(0, 0, 0, 255); // black

			glBegin(GL_LINE_STRIP);
			glVertex3d(point[0].x, point[0].y, point[0].z);
			glVertex3d(point[1].x, point[1].y, point[1].z);
			glVertex3d(point[2].x, point[2].y, point[2].z);
			glVertex3d(point[3].x, point[3].y, point[3].z);
			glVertex3d(point[0].x, point[0].y, point[0].z);
			glEnd();

			// draw the marks ( with their numbers )
			std::list<RulerMark> marks;
			CalculateMarks(marks);
			for (std::list<RulerMark>::iterator It = marks.begin(); It != marks.end(); It++)
			{
				RulerMark& mark = *It;
				mark.glCommands(GetUnits());
			}
		}
		glEnable(GL_POLYGON_OFFSET_FILL);

		glEndList();
	}

	glPopMatrix();
}

void HRuler::KillGLLists()
{
	if (m_gl_list)
	{
		glDeleteLists(m_gl_list, 1);
		m_gl_list = 0;
	}
	if (m_select_gl_list)
	{
		glDeleteLists(m_select_gl_list, 1);
		m_select_gl_list = 0;
	}
}

void HRuler::GetBox(CBox &box)
{
	Point3d point[4];
	GetFourCorners(point);

	for(int i = 0; i<4; i++)
	{
		point[i] = point[i].Transformed(m_trsf);
		box.Insert(point[i].x, point[i].y, point[i].z);
	}
}

HeeksObj *HRuler::MakeACopy(void)const{ return new HRuler(*this);}

void HRuler::Transform(const Matrix& m)
{
	m_trsf = m * m_trsf;
}

void HRuler::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	Point3d point[4];
	GetFourCorners(point);

	for(int i = 0; i<4; i++)point[i] = point[i].Transformed(m_trsf);

	list->push_back(GripData(GripperTypeRotateObject,point[0],NULL));
	list->push_back(GripData(GripperTypeRotateObject,point[2],NULL));
}

class PropertyUseViewUnits :public PropertyCheck
{
public:
	PropertyUseViewUnits(HeeksObj* object) :PropertyCheck(object, L"use view units", &((HRuler*)object)->m_use_view_units){}
	Property *MakeACopy(void)const{ return new PropertyUseViewUnits(*this); }
	void Set(bool value)
	{
		PropertyCheck::Set(value);
		m_object->KillGLLists();
	}
};

class PropertyRulerUnits : public Property
{
public:
	PropertyRulerUnits(HeeksObj* object) : Property(object, L"units"){}
	Property *MakeACopy(void)const{ return new PropertyRulerUnits(*this); }
	virtual void GetChoices(std::list< std::wstring > &choices)const{
		choices.push_back ( std::wstring ( L"mm") );
		choices.push_back ( std::wstring ( L"inch" ) );
	}
	virtual int get_property_type(){ return ChoicePropertyType; }
	void Set(int value)
	{
		((HRuler*)m_object)->m_units = (value == 0) ? 1.0 : 25.4;
		m_object->KillGLLists();
	}
	int GetInt()const
	{
		if (((HRuler*)m_object)->m_units > 25.0)return 1;
		return 0;
	}
};

void HRuler::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyUseViewUnits(this));
	if(!m_use_view_units){
		list->push_back(new PropertyRulerUnits(this));
	}
	list->push_back(new PropertyLengthWithKillGLLists(this, L"width", &m_width ));
	list->push_back(new PropertyLengthWithKillGLLists(this, L"length", &m_length ));
	list->push_back(new PropertyLengthWithKillGLLists(this, L"empty_length", &m_empty_length ));

	HeeksObj::GetProperties(list);
}

bool HRuler::GetScaleAboutMatrix(Matrix &m)
{
	m_trsf = m;
	return true;
}
