// Cuboid.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Solid.h"

class CCuboid: public CSolid{
protected:
	// CShape's virtual functions
	void MakeTransformedShape(const gp_Trsf &mat);
	std::wstring StretchedName();

public:
	gp_Ax2 m_pos; // coordinate system defining position and orientation
	double m_x; // width
	double m_y; // height
	double m_z; // depth
	static int m_type;

	CCuboid(const gp_Ax2& pos, double x, double y, double z, const wchar_t* title, const HeeksColor& col, float opacity);
	CCuboid(const TopoDS_Solid &solid, const wchar_t* title, const HeeksColor& col, float opacity);
	CCuboid(const CCuboid & rhs);
	CCuboid & operator= ( const CCuboid &rhs );

	// HeeksObj's virtual functions
	const wchar_t* GetTypeString(void)const{return L"Cuboid";}
	const wchar_t* GetIconFilePath();
	HeeksObj *MakeACopy(void)const;
	void GetProperties(std::list<Property *> *list);
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void OnApplyProperties();
	bool GetScaleAboutMatrix(double *m);
	bool Stretch(const double *p, const double* shift, void* data);
	bool DescendForUndo(){return false;}
	void GetOnEdit(bool(**callback)(HeeksObj*, std::list<HeeksObj*> *));

	// CShape's virtual functions
	void SetXMLElement(TiXmlElement* element);
	void SetFromXMLElement(TiXmlElement* pElem);

	// CSolid's virtual functions
	SolidTypeEnum GetSolidType(){return SOLID_TYPE_CUBOID;}
};
