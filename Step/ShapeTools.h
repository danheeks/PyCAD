// ShapeTools.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class CFace;
class CEdge;

class CFaceList: public ObjList{
public:
	const wchar_t* GetTypeString(void)const{return L"Faces";}
	HeeksObj *MakeACopy(void)const{ return new CFaceList(*this);}
	const wchar_t* GetIconFilePath();
	bool DescendForUndo(){return false;}
	long GetMarkingMask()const{return 0;}// not pickable
};

class CEdgeList: public ObjList{
public:
	const wchar_t* GetTypeString(void)const{return L"Edges";}
	HeeksObj *MakeACopy(void)const{ return new CEdgeList(*this);}
	const wchar_t* GetIconFilePath();
	bool DescendForUndo(){return false;}
	long GetMarkingMask()const{return 0;}// not pickable
};

class CVertexList: public ObjList{
public:
	const wchar_t* GetTypeString(void)const{return L"Vertices";}
	HeeksObj *MakeACopy(void)const{ return new CVertexList(*this);}
	const wchar_t* GetIconFilePath();
	bool DescendForUndo(){return false;}
	long GetMarkingMask()const{return 0;}// not pickable
};

void CreateFacesAndEdges(TopoDS_Shape shape, CFaceList* faces, CEdgeList* edges, CVertexList* vertices);

