// HILine.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HILine.h"
#include "HLine.h"
#include "HArc.h"
//#include "HCircle.h"
#include "HPoint.h"
#include "Gripper.h"
#include "Property.h"

HILine::HILine(const HILine &line):EndedObject(){
	operator=(line);
}

HILine::HILine(const Point3d &a, const Point3d &b, const HeeksColor* col):EndedObject(){
	A = a;
	B = b;
	SetColor(*col);
}

HILine::~HILine(){
}

const HILine& HILine::operator=(const HILine &b){
	EndedObject::operator=(b);
	return *this;
}

const wchar_t* HILine::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/iline.png";
	return iconpath.c_str();
}

void HILine::glCommands(bool select, bool marked, bool no_color)
{
	if(!no_color)
	{
		theApp->glColorEnsuringContrast(*GetColor());
	}
	GLfloat save_depth_range[2];
	if(marked)
	{
		glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
		glDepthRange(0, 0);
		glLineWidth(2);
	}

	Point3d v(A, B);
	if(v.magnitude() > 0.0000000001)
	{
		v.Normalize();

		Point3d p1 = A - v * 10000;
		Point3d p2 = A + v * 10000;

		glBegin(GL_LINES);
		glVertex3d(p1.x, p1.y, p1.z);
		glVertex3d(p2.x, p2.y, p2.z);
		glEnd();
	}

	if(marked)
	{
		glLineWidth(1);
		glDepthRange(save_depth_range[0], save_depth_range[1]);
	}
}

HeeksObj *HILine::MakeACopy(void)const{
		HILine *new_object = new HILine(*this);
		return new_object;
}

void HILine::GetBox(CBox &box){
	box.Insert(A.getBuffer());
	box.Insert(B.getBuffer());
}

void HILine::GetGripperPositions(std::list<GripData> *list, bool just_for_endof){
	if(!just_for_endof) // we don't want to snap to these for endof
	{
		EndedObject::GetGripperPositions(list,just_for_endof);
	}
}

static double length_for_properties = 0.0;

void HILine::GetProperties(std::list<Property *> *list){
	list->push_back(PropertyPnt(this, L"start", &A));
	list->push_back(PropertyPnt(this, L"end", &B));
	length_for_properties = A.Dist(B);
	list->push_back(new PropertyLength(NULL, L"Length", (const double*)&length_for_properties));

	HeeksObj::GetProperties(list);
}

bool HILine::FindNearPoint(const Point3d & ray_start, const Point3d & ray_direction, Point3d &point){
	Line ray(ray_start, ray_direction);
	Line lshort;
	double t1, t2;
	ray.Shortest(Line(A, B), lshort, t1, t2);
	point = lshort.p0 + lshort.v;
	return true;
}

bool HILine::FindPossTangentPoint(const Point3d & ray_start, const Point3d & ray_direction, Point3d &point){
	// any point on this line is a possible tangent point
	return FindNearPoint(ray_start, ray_direction, point);
}

int HILine::Intersects(const HeeksObj *object, std::list< double > *rl)const{
	int numi = 0;
#if 0
	switch(object->GetType())
	{
    case SketchType:
        return( ((CSketch *)object)->Intersects( this, rl ));

	case LineType:
		{
			Point3d pnt;
			if(intersect(GetLine(), ((HLine*)object)->GetLine(), pnt))
			{
				if(((HLine*)object)->Intersects(pnt)){
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case ILineType:
		{
			Point3d pnt;
			if(intersect(GetLine(), ((HILine*)object)->GetLine(), pnt))
			{
				if(rl)add_pnt_to_doubles(pnt, *rl);
				numi++;
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
				if(((HArc*)object)->Intersects(pnt))
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
			intersect(GetLine(), ((HCircle*)object)->GetCircle(), plist);
			if(rl)convert_pnts_to_doubles(plist, *rl);
			numi += plist.size();
		}
		break;
	}
#endif
	return numi;
}

bool HILine::GetStartPoint(Point3d &pos)
{
	pos = A;
	return true;
}

bool HILine::GetEndPoint(Point3d &pos)
{
	pos = B;
	return true;
}

Line HILine::GetLine()const{
	return Line(A, B);
}
