// HPoint.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "IdNamedObj.h"
#include "HeeksColor.h"

class HPoint: public IdNamedObj{
private:
	HeeksColor color;

public:
	geoff_geometry::Point3d m_p;
	bool m_draw_unselected;
	double mx,my;

	~HPoint(void);
	HPoint(const geoff_geometry::Point3d &p, const HeeksColor* col);
	HPoint(const HPoint &p);

	const HPoint& operator=(const HPoint &b);

	// HeeksObj's virtual functions
	int GetType()const{return PointType;}
	long GetMarkingMask()const{return MARKING_FILTER_POINT;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wchar_t* GetTypeString(void)const{return L"Point";}
	HeeksObj *MakeACopy(void)const;
	const wchar_t* GetIconFilePath();
	void Transform(const geoff_geometry::Matrix& m);
	void SetColor(const HeeksColor &col){color = col;}
	const HeeksColor* GetColor()const{return &color;}
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool GetStartPoint(geoff_geometry::Point3d& pos);
	bool GetEndPoint(geoff_geometry::Point3d& pos);
	void CopyFrom(const HeeksObj* object){operator=(*((HPoint*)object));}
	void WriteXML(TiXmlNode *root);
	bool IsDifferent(HeeksObj* other);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
};
