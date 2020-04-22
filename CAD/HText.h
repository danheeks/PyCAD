// HText.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "HeeksObj.h"
#include "HeeksColor.h"

class HText: public ObjList {
private:
	HeeksColor m_color;

	void GetBoxPoints(std::list<Point3d> &pnts);

public:
	Matrix m_trsf; // matrix defining position, orientation, scale, compared with default text size
	std::wstring m_text;
	int m_h_justification;//0 = Left;1= Center; 2 = Right
	int m_v_justification;//0 = Baseline; 1 = Bottom; 2 = Middle; 3 = Top

	HText(const Matrix &trsf, const std::wstring &text, const HeeksColor* col, int hj, int vj );
	HText(const HText &b);
	~HText(void);

	const HText& operator=(const HText &b);

	// HeeksObj's virtual functions
	int GetType()const{return TextType;}
	long GetMarkingMask()const{return MARKING_FILTER_TEXT;}
	void glCommands(bool select, bool marked, bool no_color);
	bool DrawAfterOthers(){return true;}
	void GetBox(CBox &box);
	const wchar_t* GetTypeString(void)const{return L"Text";}
	HeeksObj *MakeACopy(void)const;
	const wchar_t* GetIconFilePath();
	void Transform(const Matrix& m);
	void SetColor(const HeeksColor &col){m_color = col;}
	const HeeksColor* GetColor()const{return &m_color;}
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool Stretch(const double *p, const double* shift, void* data);
	void CopyFrom(const HeeksObj* object){operator=(*((HText*)object));}
	void WriteToXML(TiXmlElement *element);
	void ReadFromXML(TiXmlElement *element);
	const wchar_t* GetShortString(void)const{ return m_text.c_str(); }
	bool CanEditString(void)const{return true;}
	void OnEditString(const wchar_t* str);
	bool CanAdd(HeeksObj* object);

	bool GetTextSize( const std::wstring & text, float *pWidth, float *pHeight ) const;
};
