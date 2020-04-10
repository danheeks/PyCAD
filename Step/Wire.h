// Wire.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "Shape.h"

class CWire:public CShape{
public:
	CWire();
	CWire(const TopoDS_Wire &shape, const wchar_t* title);
	~CWire();
	static int m_type;

	int GetType()const{ return m_type; }
	//long GetMarkingMask()const{return MARKING_FILTER_WIRE;}
	const wchar_t* GetTypeString(void)const{return L"Wire";}
	HeeksObj *MakeACopy(void)const{ return new CWire(*this);}
	const wchar_t* GetIconFilePath();

	const TopoDS_Wire &Wire()const;
	static HeeksObj *Sketch( const TopoDS_Wire & wire );
};

