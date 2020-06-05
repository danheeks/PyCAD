// HSpline.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "EndedObject.h"

class HSpline: public EndedObject{
public:
	Handle(Geom_BSplineCurve) m_spline;
	static int m_type;

	~HSpline(void);
	HSpline(const Geom_BSplineCurve &s, const HeeksColor* col);
	HSpline(const Handle_Geom_BSplineCurve s, const HeeksColor* col);
	HSpline(const std::list<gp_Pnt> &points, const HeeksColor* col);
	HSpline(const HSpline &c);

	const HSpline& operator=(const HSpline &c);

	// HeeksObj's virtual functions
	int GetType()const{ return m_type; }
	long GetMarkingMask()const{return MARKING_FILTER_CIRCLE;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wchar_t* GetTypeString(void)const{ return L"Spline"; }
	HeeksObj *MakeACopy(void)const;
	const wchar_t* GetIconFilePath();
	void Transform(const Matrix &m);
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool FindNearPoint(const double* ray_start, const double* ray_direction, double *point);
	bool FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point);
	bool Stretch(const double *p, const double* shift, void* data);
	void GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point = true)const;
	void WriteToXML(TiXmlElement *element);
	void ReadFromXML(TiXmlElement *element);
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	bool GetStartPoint(double* pos);
	bool GetEndPoint(double* pos);

	void Reverse();
};
