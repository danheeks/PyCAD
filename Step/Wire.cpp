// Wire.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "Wire.h"
#include "ConversionTools.h"

// static
int CWire::m_type = 0;

CWire::CWire() :CShape(){
}

CWire::CWire(const TopoDS_Wire &wire, const wchar_t* title) : CShape(wire, title, false, 1.0f){
}

CWire::~CWire(){
}

const wchar_t* CWire::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/wire.png";
	return iconpath.c_str();
}

const TopoDS_Wire &CWire::Wire()const{
	return *((TopoDS_Wire*)(&m_shape));
}

/* static */ HeeksObj *CWire::Sketch( const TopoDS_Wire & wire )
{
#if 0
    const double deviation = TOLERANCE;
    HeeksObj *sketch = new CSketch;
    for(BRepTools_WireExplorer expEdge(TopoDS::Wire(wire)); expEdge.More(); expEdge.Next())
    {
        const TopoDS_Shape &E = expEdge.Current();
        if(!ConvertEdgeToSketch2(TopoDS::Edge(E), sketch, deviation))return NULL;
    }

    return(sketch);
#else
	return NULL;
#endif
}


