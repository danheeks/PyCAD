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
	geoff_geometry::Point3d m_c;
	geoff_geometry::Point3d m_axis;
	double m_radius;

	~HCircle(void);
	HCircle(const geoff_geometry::Point3d &c, const geoff_geometry::Point3d& axis, double radius, const HeeksColor* col);
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
	void Transform(const geoff_geometry::Matrix& m);
	void SetColor(const HeeksColor &col){ color = col; }
	const HeeksColor* GetColor()const{ return &color; }
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool FindNearPoint(const geoff_geometry::Line &ray, double *point);
	bool FindPossTangentPoint(const geoff_geometry::Line &ray, double *point);
	bool Stretch(const double *p, const double* shift, void* data);
	void GetSegments(void(*callbackfunc)(const double *p, bool start), double pixels_per_mm)const;
	bool GetCentrePoint(geoff_geometry::Point3d &pos);
	void WriteXML(TiXmlNode *root);
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	bool DescendForUndo(){ return false; }
	bool IsDifferent(HeeksObj* other);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
#if 0
	static bool GetLineTangentPoints(const gp_Circ& c1, const gp_Circ& c2, const geoff_geometry::Point3d& a, const geoff_geometry::Point3d& b, geoff_geometry::Point3d& p1, geoff_geometry::Point3d& p2);
	static bool GetLineTangentPoint(const gp_Circ& c, const geoff_geometry::Point3d& a, const geoff_geometry::Point3d& b, geoff_geometry::Point3d& p);
	static bool GetArcTangentPoints(const gp_Circ& c, const gp_Lin &line, const geoff_geometry::Point3d& p, double radius, geoff_geometry::Point3d& p1, geoff_geometry::Point3d& p2, geoff_geometry::Point3d& centre, geoff_geometry::Point3d& axis);
	static bool GetArcTangentPoints(const gp_Circ& c1, const gp_Circ &c2, const geoff_geometry::Point3d& a, const geoff_geometry::Point3d& b, double radius, geoff_geometry::Point3d& p1, geoff_geometry::Point3d& p2, geoff_geometry::Point3d& centre, geoff_geometry::Point3d& axis);
	static bool GetArcTangentPoints(const gp_Lin& l1, const gp_Lin &l2, const geoff_geometry::Point3d& a, const geoff_geometry::Point3d& b, double radius, geoff_geometry::Point3d& p1, geoff_geometry::Point3d& p2, geoff_geometry::Point3d& centre, geoff_geometry::Point3d& axis);
	static bool GetArcTangentPoint(const gp_Lin& l, const geoff_geometry::Point3d& a, const geoff_geometry::Point3d& b, const gp_Vec *final_direction, double* radius, geoff_geometry::Point3d& p, geoff_geometry::Point3d& centre, geoff_geometry::Point3d& axis);
	static bool GetArcTangentPoint(const gp_Circ& c, const geoff_geometry::Point3d& a, const geoff_geometry::Point3d& b, const gp_Vec *final_direction, double* radius, geoff_geometry::Point3d& p, geoff_geometry::Point3d& centre, geoff_geometry::Point3d& axis);
#endif
	double GetDiameter()const;
};
