// HArc.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "EndedObject.h"

class HArc: public EndedObject{
public:
	Point3d m_axis;
	Point3d C;
	double m_radius;

	~HArc(void);
	HArc(const Point3d &a, const Point3d &b, const Point3d &axis, const Point3d &c, const HeeksColor* col);
	HArc(const HArc &arc);

	const HArc& operator=(const HArc &b);

	bool IsIncluded(Point3d pnt);

	// HeeksObj's virtual functions
	int GetType()const{return ArcType;}
	long GetMarkingMask()const{return MARKING_FILTER_ARC;}
	int GetIDGroupType()const{return LineType;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wchar_t* GetTypeString(void)const{return L"Arc";}
	HeeksObj *MakeACopy(void)const;
	const wchar_t* GetIconFilePath();
	void Transform(const Matrix& m);
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
	bool DescendForUndo(){return false;}
	bool IsDifferent(HeeksObj* other);
	HeeksObj* MakeACopyWithID();
	void ReloadPointers();

	bool Intersects(const Point3d &pnt)const;
	Point3d GetSegmentVector(double fraction)const;
	Point3d GetPointAtFraction(double fraction)const;
	static bool TangentialArc(const Point3d &p0, const Point3d &v0, const Point3d &p1, Point3d &centre, Point3d &axis);
	bool UsesID(){return true;} 
	void Reverse();
	double IncludedAngle()const;
	Circle GetCircle()const;
};
