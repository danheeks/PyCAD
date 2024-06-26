// HILine.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "EndedObject.h"

class HILine: public EndedObject{
public:
	~HILine(void);
	HILine(const Point3d &a, const Point3d &b, const HeeksColor* col);
	HILine(const HILine &line);

	const HILine& operator=(const HILine &b);

	// HeeksObj's virtual functions
	int GetType()const{return ILineType;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wchar_t* GetTypeString(void)const{return L"Infinite Line";}
	const wchar_t* GetXMLTypeString(void)const { return L"InfiniteLine"; }
	HeeksObj *MakeACopy(void)const;
	const wchar_t* GetIconFilePath();
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool FindNearPoint(const Point3d & ray_start, const Point3d & ray_direction, Point3d &point);
	bool FindPossTangentPoint(const Point3d & ray_start, const Point3d & ray_direction, Point3d &point);
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	void CopyFrom(const HeeksObj* object){operator=(*((HILine*)object));}
	bool GetEndPoint(Point3d &pos);
	bool GetStartPoint(Point3d &pos);

	Line GetLine()const;

};
