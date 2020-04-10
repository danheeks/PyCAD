// Vertex.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "HeeksObj.h"

class CFace;
class CEdge;
class CShape;

class HVertex:public HeeksObj{
private:
	TopoDS_Vertex m_topods_vertex;
	std::list<CEdge*>::iterator m_edgeIt;

	void FindEdges();

public:
	std::list<CEdge*> m_edges;
	std::list<CFace*> m_faces;
	double m_point[3];
	static int m_type;

	HVertex();
	HVertex(const TopoDS_Vertex &vertex);
	~HVertex();

	int GetType()const{return m_type;}
//	long GetMarkingMask()const{return MARKING_FILTER_VERTEX;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	HeeksObj *MakeACopy(void)const{ return new HVertex(*this);}
	const wchar_t* GetIconFilePath();
	const wchar_t* GetTypeString(void)const{return L"Vertex";}
	bool UsesID(){return true;}
	void ModifyByMatrix(const double* m);

	const TopoDS_Vertex &Vertex(){ return m_topods_vertex; }
	CEdge* GetFirstEdge();
	CEdge* GetNextEdge();
	CShape* GetParentBody();
};

