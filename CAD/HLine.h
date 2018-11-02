// HLine.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "EndedObject.h"

class HLine: public EndedObject{
public:
	~HLine(void);
	HLine(const geoff_geometry::Point3d &a, const geoff_geometry::Point3d &b, const HeeksColor* col);
	HLine(const HLine &line);

	const HLine& operator=(const HLine &b);

	// HeeksObj's virtual functions
	int GetType()const{return LineType;}
	long GetMarkingMask()const{return MARKING_FILTER_LINE;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wchar_t* GetTypeString(void)const{return L"Line";}
	HeeksObj *MakeACopy(void)const;
	const wchar_t* GetIconFilePath();
	bool GetMidPoint(geoff_geometry::Point3d &pos);
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool FindNearPoint(const geoff_geometry::Point3d & ray_start, const geoff_geometry::Point3d & ray_direction, geoff_geometry::Point3d &point);
	bool FindPossTangentPoint(const geoff_geometry::Point3d & ray_start, const geoff_geometry::Point3d & ray_direction, geoff_geometry::Point3d &point);
	void GetSegments(void(*callbackfunc)(const double *p, bool start), double pixels_per_mm)const;
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	void CopyFrom(const HeeksObj* object){operator=(*((HLine*)object));}
	void WriteXML(TiXmlNode *root);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
    bool UsesID(){return true;} 
	bool Intersects(const geoff_geometry::Point3d &pnt)const;
	geoff_geometry::Point3d GetSegmentVector(double fraction);
	void Reverse();

	geoff_geometry::Line GetLine()const;
};
