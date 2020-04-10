// HCircle.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "IdNamedObj.h"
#include "ExtrudedObj.h"
#include "HeeksColor.h"

class HCircle : public ExtrudedObj<IdNamedObj>{
private:
	HeeksColor color;

public:
	Point3d m_c;
	Point3d m_axis;
	double m_radius;

	~HCircle(void);
	HCircle(const Point3d &c, const Point3d& axis, double radius, const HeeksColor* col);
	HCircle(const HCircle &c);
	HCircle(){}

	const HCircle& operator=(const HCircle &c);

	// HeeksObj's virtual functions
	int GetType()const{ return CircleType; }
	long GetMarkingMask()const{ return MARKING_FILTER_CIRCLE; }
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wchar_t* GetTypeString(void)const{ return L"Circle"; }
	int GetIDGroupType()const{ return SketchType; }
	HeeksObj *MakeACopy(void)const;
	const wchar_t* GetIconFilePath();
	void Transform(const Matrix& m);
	void SetColor(const HeeksColor &col){ color = col; }
	const HeeksColor* GetColor()const{ return &color; }
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool FindNearPoint(const Line &ray, double *point);
	bool FindPossTangentPoint(const Line &ray, double *point);
	bool Stretch(const double *p, const double* shift, void* data);
	void GetSegments(void(*callbackfunc)(const double *p, bool start), double pixels_per_mm)const;
	bool GetCentrePoint(Point3d &pos);
	void WriteToXML(TiXmlElement *element);
	void ReadFromXML(TiXmlElement *element);
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	bool DescendForUndo(){ return false; }
#if 0
	static bool GetLineTangentPoints(const Circle& c1, const Circle& c2, const Point3d& a, const Point3d& b, Point3d& p1, Point3d& p2);
	static bool GetLineTangentPoint(const Circle& c, const Point3d& a, const Point3d& b, Point3d& p);
	static bool GetArcTangentPoints(const Circle& c, const gp_Lin &line, const Point3d& p, double radius, Point3d& p1, Point3d& p2, Point3d& centre, Point3d& axis);
	static bool GetArcTangentPoints(const Circle& c1, const Circle &c2, const Point3d& a, const Point3d& b, double radius, Point3d& p1, Point3d& p2, Point3d& centre, Point3d& axis);
	static bool GetArcTangentPoints(const gp_Lin& l1, const gp_Lin &l2, const Point3d& a, const Point3d& b, double radius, Point3d& p1, Point3d& p2, Point3d& centre, Point3d& axis);
	static bool GetArcTangentPoint(const gp_Lin& l, const Point3d& a, const Point3d& b, const gp_Vec *final_direction, double* radius, Point3d& p, Point3d& centre, Point3d& axis);
	static bool GetArcTangentPoint(const Circle& c, const Point3d& a, const Point3d& b, const gp_Vec *final_direction, double* radius, Point3d& p, Point3d& centre, Point3d& axis);
#endif
	void SetCircle(Circle c);
	Circle GetCircle()const;
	double GetDiameter()const;
};
