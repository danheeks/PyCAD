// HLine.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "../Geom/Geom.h"
#include "HLine.h"
#include "HILine.h"
#include "HCircle.h"
#include "HArc.h"
#include "Gripper.h"
#include "Sketch.h"
#include "DigitizeMode.h"
#include "HPoint.h"
#include "PropertyGeom.h"

HLine::HLine(const HLine &line):EndedObject(){
	operator=(line);
}

HLine::HLine(const Point3d &a, const Point3d &b, const HeeksColor* col):EndedObject(){
	A = a;
	B = b;
	SetColor(*col);
}

HLine::~HLine(){
}

const HLine& HLine::operator=(const HLine &b){
	EndedObject::operator=(b);
	return *this;
}

#if 0
HLine* line_for_tool = NULL;

class MakeCylinderOnLine:public Tool{
public:
	void Run(){
		Point3d v(line_for_tool->A, line_for_tool->B);
		CCylinder* new_object = new CCylinder(Point3d(line_for_tool->A, v), 1.0, v.Magnitude(), _("Cylinder"), HeeksColor(191, 191, 240), 1.0f);
		theApp->StartHistory();
		theApp->AddUndoably(new_object,NULL,NULL);
		theApp->EndHistory();
	}
	const wchar_t* GetTitle(){return _("Make Cylinder On Line");}
	std::wstring BitmapPath(){return _T("cylonlin");}
};
static MakeCylinderOnLine make_cylinder_on_line;

class MakeConeOnLine:public Tool{
public:
	void Run(){
		Point3d v(line_for_tool->A, line_for_tool->B);
		CCone* new_object = new CCone(Point3d(line_for_tool->A, v), 2.0, 1.0, v.Magnitude(), _("Cone"), HeeksColor(240, 240, 191), 1.0f);
		theApp->StartHistory();
		theApp->AddUndoably(new_object,NULL,NULL);
		theApp->EndHistory();
	}
	const wchar_t* GetTitle(){return _("Make Cone On Line");}
	std::wstring BitmapPath(){return _T("coneonlin");}
};
static MakeConeOnLine make_cone_on_line;


class ClickMidpointOnLine:public Tool{
public:
	void Run(){
		Point3d midpoint((line_for_tool->A + line_for_tool->B) /2);

		theApp->m_digitizing->digitized_point = DigitizedPoint(midpoint, DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(theApp->input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}
	const wchar_t* GetTitle(){return _("Click Midpoint On Line");}
	std::wstring BitmapPath(){return _T("click_line_midpoint");}
};
static ClickMidpointOnLine click_midpoint_on_line;


class ClickStartPointOnLine:public Tool{
public:
	void Run(){
		theApp->m_digitizing->digitized_point = DigitizedPoint(line_for_tool->A, DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(theApp->input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}
	const wchar_t* GetTitle(){return _("Click Start Of Line");}
	std::wstring BitmapPath(){return _T("click_line_end_one");}
};
static ClickStartPointOnLine click_start_point_on_line;


class ClickEndPointOnLine:public Tool{
public:
	void Run(){
		theApp->m_digitizing->digitized_point = DigitizedPoint(line_for_tool->B, DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(theApp->input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}
	const wchar_t* GetTitle(){return _("Click End Of Line");}
	std::wstring BitmapPath(){return _T("click_line_end_two");}
};
static ClickEndPointOnLine click_end_point_on_line;

#endif

const wchar_t* HLine::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/line.png";
	return iconpath.c_str();
}

bool HLine::GetStartPoint(Point3d &pos)
{
	pos = A;
	return true;
}

bool HLine::GetEndPoint(Point3d &pos)
{
	pos = B;
	return true;
}

bool HLine::GetMidPoint(Point3d &pos)
{
	pos = (A + B) * 0.5;
	return true;
}

#if 0
void HLine::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	line_for_tool = this;
	t_list->push_back(&make_cylinder_on_line);
	t_list->push_back(&make_cone_on_line);

	Drawing *pDrawingMode = dynamic_cast<Drawing *>(theApp->input_mode_object);
	if (pDrawingMode != NULL)
	{
		t_list->push_back(&click_start_point_on_line);
		t_list->push_back(&click_midpoint_on_line);
		t_list->push_back(&click_end_point_on_line);
	}
}
#endif

void HLine::glCommands(bool select, bool marked, bool no_color){
	if(!no_color){
		theApp->glColorEnsuringContrast(color);
	}
	GLfloat save_depth_range[2];
	if(marked){
		glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
		glDepthRange(0, 0);
		glLineWidth(2);
	}
	glBegin(GL_LINES);
	glVertex3d(A.x, A.y, A.z);
	glVertex3d(B.x, B.y, B.z);
	if (m_thickness != 0.0)
	{
		Point3d Ve(m_extrusion_vector[0], m_extrusion_vector[1], m_extrusion_vector[2]);
		Point3d Ae = A + Ve * m_thickness;
		Point3d Be = B + Ve * m_thickness;
		glVertex3d(B.x, B.y, B.z);
		glVertex3d(Be.x, Be.y, Be.z);
		glVertex3d(Be.x, Be.y, Be.z);
		glVertex3d(Ae.x, Ae.y, Ae.z);
		glVertex3d(Ae.x, Ae.y, Ae.z);
		glVertex3d(A.x, A.y, A.z);
	}
	glEnd();
	if(marked){
		glLineWidth(1);
		glDepthRange(save_depth_range[0], save_depth_range[1]);
	}

	EndedObject::glCommands(select,marked,no_color);
}

HeeksObj *HLine::MakeACopy(void)const{
	HLine *new_object = new HLine(*this);
	return new_object;
}

void HLine::GetBox(CBox &box){
	box.Insert(A.getBuffer());
	box.Insert(B.getBuffer());
}

void HLine::GetGripperPositions(std::list<GripData> *list, bool just_for_endof){
	EndedObject::GetGripperPositions(list,just_for_endof);
}

static double length_for_properties = 0.0;

void HLine::GetProperties(std::list<Property *> *list){
	list->push_back(PropertyPnt(this, L"start", &A));
	list->push_back(PropertyPnt(this, L"end", &B));
	length_for_properties = A.Dist(B);
	list->push_back(new PropertyLength(NULL, L"Length", (const double*)&length_for_properties));

	HeeksObj::GetProperties(list);
}

bool HLine::FindNearPoint(const Point3d & ray_start, const Point3d & ray_direction, Point3d &point){
	// The OpenCascade libraries throw an exception when one tries to
	// create a gp_Lin() object using a vector that doesn't point
	// anywhere.  If this is a zero-length line then we're in
	// trouble.  Don't bother with it.
	if ((A.x == B.x) &&
	    (A.y == B.y) &&
	    (A.z == B.z)) return(false);

	Line line(ray_start, ray_direction);
	Line lshort;
	double t1, t2;
	line.Shortest(GetLine(), lshort, t1, t2);
	Point3d p1 = lshort.p0;

	if(!Intersects(p1))
		return false;

	point = p1;
	return true;
}

bool HLine::FindPossTangentPoint(const Point3d & ray_start, const Point3d & ray_direction, Point3d &point){
	// any point on this line is a possible tangent point
	return FindNearPoint(ray_start, ray_direction, point);
}

Line HLine::GetLine()const{
	return Line(A, B);
}

int HLine::Intersects(const HeeksObj *object, std::list< double > *rl)const{
	int numi = 0;

	switch(object->GetType())
	{
    case SketchType:
        return( ((CSketch *)object)->Intersects( this, rl ));

	case LineType:
		{
			// The OpenCascade libraries throw an exception when one tries to
			// create a gp_Lin() object using a vector that doesn't point
			// anywhere.  If this is a zero-length line then we're in
			// trouble.  Don't bother with it.
			if ((A.x == B.x) &&
			    (A.y == B.y) &&
			    (A.z == B.z)) break;

			Point3d pnt;
			if(Intof(GetLine(), ((HLine*)object)->GetLine(), pnt))
			{
				if(Intersects(pnt) && ((HLine*)object)->Intersects(pnt)){
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case ILineType:
		{
			Point3d pnt;
			if (Intof(GetLine(), ((HILine*)object)->GetLine(), pnt))
			{
				if(Intersects(pnt)){
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case ArcType:
		{
			std::list<Point3d> plist;
			intersect(GetLine(), ((HArc*)object)->GetCircle(), plist);
			for(std::list<Point3d>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				Point3d& pnt = *It;
				if(Intersects(pnt) && ((HArc*)object)->Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case CircleType:
		{
			std::list<Point3d> plist;
			Line line2d(Point3d(A.x, A.y, 0.0), Point3d(B.x, B.y, 0.0));
			intersect(line2d, ((HCircle*)object)->GetCircle(), plist);

			for (std::list<Point3d>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				Point3d& pnt = *It;
				if(rl)add_pnt_to_doubles(pnt, *rl);
				numi++;
			}
		}
		break;
	}

	return numi;
}

bool HLine::Intersects(const Point3d &pnt)const
{
	Line this_line = GetLine();
	if (!this_line.Intof(pnt))return false;

	// check it lies between A and B
	Point3d v = this_line.v;
	v.Normalize();
	double dpA = Point3d(A) * v;
	double dpB = Point3d(B) * v;
	double dp = Point3d(pnt) * v;
	return dp >= dpA - TOLERANCE && dp <= dpB + TOLERANCE;
}

void HLine::GetSegments(void(*callbackfunc)(const double *p, bool start), double pixels_per_mm)const{
	(*callbackfunc)(A.getBuffer(), true);
	(*callbackfunc)(B.getBuffer(), false);
}

Point3d HLine::GetSegmentVector(double fraction)
{
	Point3d line_vector(A, B);
	if(line_vector.magnitude() < 0.000000001)return Point3d(0, 0, 0);
	return Point3d(A, B).Normalized();
}

void HLine::Reverse()
{
	Point3d temp = A;
	A = B;
	B = temp;
}

