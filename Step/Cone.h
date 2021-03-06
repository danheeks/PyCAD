// Cone.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Solid.h"

class CCone: public CSolid{
protected:
	bool m_render_without_OpenCASCADE;

	// CShape's virtual functions
	void MakeTransformedShape(const gp_Trsf &mat);
	std::wstring StretchedName();

public:
	gp_Ax2 m_pos;
	double m_r1;
	double m_r2;
	double m_height;
	double m_temp_r1;
	double m_temp_r2;
	static int m_type;

	CCone(const gp_Ax2& pos, double r1, double r2, double height, const wchar_t* title, const HeeksColor& col, float opacity);
	CCone(const TopoDS_Solid &solid, const wchar_t* title, const HeeksColor& col, float opacity);

	// HeeksObj's virtual functions
	const wchar_t* GetTypeString(void)const{return L"Cone";}
	const wchar_t* GetIconFilePath();
	void glCommands(bool select, bool marked, bool no_color);
	HeeksObj *MakeACopy(void)const;
	void GetProperties(std::list<Property *> *list);
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	bool ValidateProperties();
	bool GetScaleAboutMatrix(Matrix &m);
	bool Stretch(const Point3d &p, const Point3d &shift, void* data);
	bool StretchTemporary(const Point3d &p, const Point3d &shift, void* data);
	bool DescendForUndo(){return false;}

	// CShape's virtual functions
	void SetXMLElement(TiXmlElement* element);
	void SetFromXMLElement(TiXmlElement* pElem);
	void OnApplyPropertiesRaw();

	// CSolid's virtual functions
	SolidTypeEnum GetSolidType(){return SOLID_TYPE_CONE;}

	bool Stretch2(const Point3d &p, const Point3d &shift, gp_Ax2& new_pos, double& new_r1, double& new_r2, double& new_height);
};
