// CoordinateSystem.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "HeeksObj.h"

class CoordinateSystem: public HeeksObj
{
public:
	geoff_geometry::Point3d m_o;
	geoff_geometry::Point3d m_x;
	geoff_geometry::Point3d m_y;
	std::wstring m_title;

	static double size;
	static bool size_is_pixels; // false for mm
	static bool rendering_current;

	CoordinateSystem(const std::wstring& str, const geoff_geometry::Point3d &o, const geoff_geometry::Point3d &x, const geoff_geometry::Point3d &y);
	CoordinateSystem(const CoordinateSystem &c);
	~CoordinateSystem(void);

	const CoordinateSystem& operator=(const CoordinateSystem &c);

	// HeeksObj's virtual functions
	int GetType()const{return CoordinateSystemType;}
	long GetMarkingMask()const{return MARKING_FILTER_COORDINATE_SYSTEM;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wchar_t* GetTypeString(void)const{return L"Coordinate System";}
	const wchar_t* GetShortString(void)const{return m_title.c_str();}
	bool CanEditString(void)const{return true;}
	void OnEditString(const wchar_t* str);
	HeeksObj *MakeACopy(void)const;
	const wchar_t* GetIconFilePath();
	void Transform(const geoff_geometry::Matrix &m);
	void GetProperties(std::list<Property *> *list);
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	bool GetScaleAboutMatrix(geoff_geometry::Matrix &m);
	void WriteXML(TiXmlNode *root);

	geoff_geometry::Matrix GetMatrix();
	void ApplyMatrix();

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
	static void RenderArrow();
	static void RenderDatum(bool bright, bool solid); // render a coordinate system at 0, 0, 0
	static void AxesToAngles(const geoff_geometry::Point3d &x, const geoff_geometry::Point3d &y, double &v_angle, double &h_angle, double &t_angle);
	static void AnglesToAxes(const double &v_angle, const double &h_angle, const double &t_angle, geoff_geometry::Point3d &x, geoff_geometry::Point3d &y);
	//static void GetAx2Properties(std::list<Property *> *list, gp_Ax2& a, HeeksObj* object);
	bool PickFrom3Points();
	bool PickFrom1Point();
};