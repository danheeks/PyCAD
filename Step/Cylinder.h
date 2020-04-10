// Cylinder.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Solid.h"

class CCylinder: public CSolid{
protected:
	// CShape's virtual functions
	void MakeTransformedShape(const gp_Trsf &mat);
	std::wstring StretchedName();

public:
	gp_Ax2 m_pos;
	double m_radius;
	double m_height;
	static int m_type;

	CCylinder(const gp_Ax2& pos, double radius, double height, const wchar_t* title, const HeeksColor& col, float opacity);
	CCylinder(const TopoDS_Solid &solid, const wchar_t* title, const HeeksColor& col, float opacity);

	// HeeksObj's virtual functions
	const wchar_t* GetTypeString(void)const{return L"Cylinder";}
	const wchar_t* GetIconFilePath();
	HeeksObj *MakeACopy(void)const;
	void GetProperties(std::list<Property *> *list);
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void OnApplyProperties();
	int GetCentrePoints(double* pos, double* pos2);
	bool GetScaleAboutMatrix(double *m);
	bool Stretch(const double *p, const double* shift, void* data);
	bool DescendForUndo(){return false;}

	// CShape's virtual functions
	void SetXMLElement(TiXmlElement* element);
	void SetFromXMLElement(TiXmlElement* pElem);

	// CSolid's virtual functions
	SolidTypeEnum GetSolidType(){return SOLID_TYPE_CYLINDER;}
};
