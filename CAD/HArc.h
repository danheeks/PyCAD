// HArc.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "EndedObject.h"

class HArc: public EndedObject{
public:
	geoff_geometry::Point3d m_axis;
	geoff_geometry::Point3d C;
	double m_radius;

	~HArc(void);
	HArc(const geoff_geometry::Point3d &a, const geoff_geometry::Point3d &b, const geoff_geometry::Point3d &axis, const geoff_geometry::Point3d &c, const HeeksColor* col);
	HArc(const HArc &arc);

	const HArc& operator=(const HArc &b);

	bool IsIncluded(geoff_geometry::Point3d pnt);

	// HeeksObj's virtual functions
	int GetType()const{return ArcType;}
	long GetMarkingMask()const{return MARKING_FILTER_ARC;}
	int GetIDGroupType()const{return LineType;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wchar_t* GetTypeString(void)const{return L"Arc";}
	HeeksObj *MakeACopy(void)const;
	const wchar_t* GetIconFilePath();
	void Transform(const geoff_geometry::Matrix& m);
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool FindNearPoint(const geoff_geometry::Line &ray, double *point);
	bool FindPossTangentPoint(const geoff_geometry::Line &ray, double *point);
	bool Stretch(const double *p, const double* shift, void* data);
	void GetSegments(void(*callbackfunc)(const double *p, bool start), double pixels_per_mm)const;
	bool GetCentrePoint(geoff_geometry::Point3d &pos);
	void WriteXML(TiXmlNode *root);
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	bool DescendForUndo(){return false;}
	bool IsDifferent(HeeksObj* other);
	HeeksObj* MakeACopyWithID();
	void ReloadPointers();

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);

	bool Intersects(const geoff_geometry::Point3d &pnt)const;
	geoff_geometry::Point3d GetSegmentVector(double fraction)const;
	geoff_geometry::Point3d GetPointAtFraction(double fraction)const;
	static bool TangentialArc(const geoff_geometry::Point3d &p0, const geoff_geometry::Point3d &v0, const geoff_geometry::Point3d &p1, geoff_geometry::Point3d &centre, geoff_geometry::Point3d &axis);
	bool UsesID(){return true;} 
	void Reverse();
	double IncludedAngle()const;
};
