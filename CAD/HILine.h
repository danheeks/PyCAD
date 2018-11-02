// HILine.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "EndedObject.h"

class HILine: public EndedObject{
public:
	~HILine(void);
	HILine(const geoff_geometry::Point3d &a, const geoff_geometry::Point3d &b, const HeeksColor* col);
	HILine(const HILine &line);

	const HILine& operator=(const HILine &b);

	// HeeksObj's virtual functions
	int GetType()const{return ILineType;}
	long GetMarkingMask()const{return MARKING_FILTER_ILINE;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wchar_t* GetTypeString(void)const{return L"Infinite Line";}
	HeeksObj *MakeACopy(void)const;
	const wchar_t* GetIconFilePath();
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool FindNearPoint(const geoff_geometry::Point3d & ray_start, const geoff_geometry::Point3d & ray_direction, geoff_geometry::Point3d &point);
	bool FindPossTangentPoint(const geoff_geometry::Point3d & ray_start, const geoff_geometry::Point3d & ray_direction, geoff_geometry::Point3d &point);
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	void CopyFrom(const HeeksObj* object){operator=(*((HILine*)object));}
	void WriteXML(TiXmlNode *root);
	bool GetEndPoint(geoff_geometry::Point3d &pos);
	bool GetStartPoint(geoff_geometry::Point3d &pos);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
};
