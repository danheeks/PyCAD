// Sphere.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Solid.h"

class CSphere: public CSolid{
protected:
	// CShape's virtual functions
	void MakeTransformedShape(const gp_Trsf &mat);
	std::wstring StretchedName();

public:
	gp_Pnt m_pos;
	double m_radius;

	CSphere(const gp_Pnt& pos, double radius, const wchar_t* title, const HeeksColor& col, float opacity);
	CSphere(const TopoDS_Solid &solid, const wchar_t* title, const HeeksColor& col, float opacity);
	CSphere(const CSphere & rhs);
	CSphere & operator= ( const CSphere &rhs );

	// HeeksObj's virtual functions
	const wchar_t* GetTypeString(void)const{return L"Sphere";}
	const wchar_t* GetIconFilePath();
	HeeksObj *MakeACopy(void)const;
	void GetProperties(std::list<Property *> *list);
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	bool GetCentrePoint(Point3d &pos);
	bool GetScaleAboutMatrix(Matrix &m);
	bool DescendForUndo(){ return false; }

	// CShape's virtual functions
	void SetXMLElement(TiXmlElement* element);
	void SetFromXMLElement(TiXmlElement* pElem);
	void OnApplyPropertiesRaw();

	// CSolid's virtual functions
	SolidTypeEnum GetSolidType(){return SOLID_TYPE_SPHERE;}
};
