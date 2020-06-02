// Shape.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Material.h"
#include "ShapeData.h"
#include "ShapeTools.h"
#include "IdNamedObjList.h"
#include <map>

class CShape:public IdNamedObjList{
protected:
	int m_face_gl_list;
	int m_edge_gl_list;
	int m_select_edge_gl_list;
	CBox m_box;
	TopoDS_Shape m_shape;
	double m_opacity;
	bool m_volume_found;
	double m_volume;
	gp_Pnt m_centre_of_mass;

	void create_faces_and_edges();
	void delete_faces_and_edges();
	void CallMesh();
	virtual void MakeTransformedShape(const gp_Trsf &mat);
	virtual std::wstring StretchedName();

public:
	static bool m_solids_found; // a flag for xml writing
	CFaceList* m_faces;
	CEdgeList* m_edges;
	CVertexList* m_vertices;
	HeeksColor m_color;
	CFace* m_picked_face;
	static int m_type;
	static double m_iges_sewing_tolerance;

	CShape();
	CShape(const TopoDS_Shape &shape, const wchar_t* title, const HeeksColor& col, float opacity);
	CShape(const CShape& s);
	~CShape();

	virtual const CShape& operator=(const CShape& s);

	// HeeksObj's virtual functions
	int GetType()const{return m_type;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	void KillGLLists(void);
	void Transform(const Matrix &m);
	void GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal = true);
	double Area()const;
	void CopyFrom(const HeeksObj* object);
	void WriteXML(TiXmlNode *root);
	bool UsesID(){return true;}
	bool CanAddTo(HeeksObj* owner){ return ((owner != NULL) && (owner->GetType() == GroupType || owner->GetType() == DocumentType)); }
	bool DescendForUndo(){return false;}
	bool DrawAfterOthers(){return m_opacity < 0.9999;}
	void GetProperties(std::list<Property *> *list);

	const TopoDS_Shape &Shape(){return m_shape;}
	const TopoDS_Shape *GetShape(){return &m_shape;}

	CFace* find(const TopoDS_Face &face);
	bool GetExtents(double* extents, const double* orig = NULL, const double* xdir = NULL, const double* ydir = NULL, const double* zdir = NULL);
	void CopyIDsFrom(const CShape* shape_from);
	float GetOpacity();
	void SetOpacity(float opacity);
	void CalculateVolumeAndCentre();

	static HeeksObj* CutShapes(std::list<HeeksObj*> &list,bool dodelete=true);
	static HeeksObj* FuseShapes(std::list<HeeksObj*> &list);
	static HeeksObj* CommonShapes(std::list<HeeksObj*> &list);
	static void FilletOrChamferEdges(std::list<HeeksObj*> &list, double radius, bool chamfer_not_fillet = false);
	static bool ImportSolidsFile(const wchar_t* filepath, bool undoably,std::map<int, CShapeData> *index_map = NULL, HeeksObj* paste_into = NULL);
	static bool ExportSolidsFile(const std::list<HeeksObj*>& objects, const wchar_t* filepath, std::map<int, CShapeData> *index_map = NULL);
	static HeeksObj* MakeObject(const TopoDS_Shape &shape, const wchar_t* title, SolidTypeEnum solid_type, const HeeksColor& col, float opacity);
	static bool IsTypeAShape(int t);
	static bool IsMatrixDifferentialScale(const gp_Trsf& trsf);

	virtual void SetXMLElement(TiXmlElement* element){}
	virtual void SetFromXMLElement(TiXmlElement* pElem){}

	void Init();
};

