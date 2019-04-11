// Group.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "ObjList.h"

class CGroup: public ObjList{
	std::list<int> m_loaded_solid_ids; // only used during loading xml file

public:
	std::wstring m_title;
	bool m_gripper_datum_set;
	bool m_custom_grippers;
	bool m_custom_grippers_just_one_axis;
	Point3d m_o;
	Point3d m_px;
	Point3d m_py;
	Point3d m_pz;

	CGroup();

	const wchar_t* GetTypeString(void)const{return L"Group";}
	int GetType()const{return GroupType;}
	HeeksObj *MakeACopy(void)const{ return new CGroup(*this);}
	const wchar_t* GetIconFilePath();
	void WriteToXML(TiXmlElement *element);
	void ReadFromXML(TiXmlElement* element);
	bool UsesID(){return true;}
	const wchar_t* GetShortString(void)const{return m_title.c_str();}
	bool CanEditString(void)const{return true;}
	void OnEditString(const wchar_t* str);
	bool Stretch(const double *p, const double* shift, void* data);
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool GetScaleAboutMatrix(Matrix &m);
	void Transform(const Matrix& m);

	Matrix GetMatrix();
};

