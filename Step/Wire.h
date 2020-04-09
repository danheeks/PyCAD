// Wire.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "Shape.h"

class CWire:public CShape{
public:
	CWire(const TopoDS_Wire &shape, const wchar_t* title);
	~CWire();

	int GetType()const{return WireType;}
	long GetMarkingMask()const{return MARKING_FILTER_WIRE;}
	const wchar_t* GetTypeString(void)const{return _("Wire");}
	HeeksObj *MakeACopy(void)const{ return new CWire(*this);}
	const wchar_t* GetIconFilePath();
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);

	const TopoDS_Wire &Wire()const;
	static HeeksObj *Sketch( const TopoDS_Wire & wire );
};

