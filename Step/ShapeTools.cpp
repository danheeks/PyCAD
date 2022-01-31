// ShapeTools.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "ShapeTools.h"
#include "Vertex.h"
#include "Edge.h"
#include "Face.h"
#include "Loop.h"

const wchar_t* CFaceList::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/faces.png";
	return iconpath.c_str();
}

const wchar_t* CEdgeList::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/edges.png";
	return iconpath.c_str();
}

const wchar_t* CVertexList::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/vertices.png";
	return iconpath.c_str();
}

void CreateFacesAndEdges(TopoDS_Shape shape, CFaceList* faces, CEdgeList* edges, CVertexList* vertices)
{

}

