// Face.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "HeeksObj.h"
#include "HeeksColor.h"

class CEdge;
class CLoop;
class CShape;
class CNurbSurfaceParams;

class CFace:public HeeksObj{
private:
	TopoDS_Face m_topods_face;
#if _DEBUG
	double m_pos_x;
	double m_pos_y;
	double m_pos_z;
	double m_normal_x;
	double m_normal_y;
	double m_normal_z;
	bool m_orientation;
#endif
	int m_marking_gl_list; // simply has material commands, inserted in the parent body's display list
	HeeksColor m_color;

public:
	CBox m_box;
	int m_temp_attr; // not saved with the model
	std::list<CEdge*>::iterator m_edgeIt;
	std::list<CEdge*> m_edges;
	std::list<CLoop*>::iterator m_loopIt;
	std::list<CLoop*> m_loops;
	static int m_type;
	static HeeksColor face_selection_color;

	CFace();
	CFace(const TopoDS_Face &face);
	~CFace();

	int GetType()const{return m_type;}
//	long GetMarkingMask()const{return MARKING_FILTER_FACE;}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wchar_t* GetIconFilePath();
	HeeksObj *MakeACopy(void)const{ return new CFace(*this);}
	const wchar_t* GetTypeString(void)const{return L"Face";}
	void SetColor(const HeeksColor &col){ m_color = col; }
	const HeeksColor* GetColor()const{ return &m_color; }
	void GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal = false);
	double Area()const;
	void Transform(const Matrix &m);
	void WriteXML(TiXmlNode *root);
	void GetProperties(std::list<Property *> *list);
	bool UsesID(){return true;}
	void GetGripperPositionsTransformed(std::list<GripData> *list, bool just_for_endof);

	const TopoDS_Face &Face(){return m_topods_face;}
	gp_Dir GetMiddleNormal(gp_Pnt *pos = NULL)const;
	gp_Dir GetNormalAtUV(double u, double v, gp_Pnt *pos = NULL)const;
	bool GetUVAtPoint(const gp_Pnt &pos, double *u, double *v)const;
	bool GetClosestPoint(const gp_Pnt &pos, gp_Pnt &closest_pnt)const;
	bool GetClosestSurfacePoint(const gp_Pnt &pos, gp_Pnt &closest_pnt)const;
	void GetPlaneParams(gp_Pln &p);
	void GetCylinderParams(gp_Cylinder &c);
	void GetSphereParams(gp_Sphere &s);
	void GetConeParams(gp_Cone &c);
	void GetTorusParams(gp_Torus &t);
	bool GetNurbSurfaceParams(CNurbSurfaceParams* params);
	int GetSurfaceType();
	bool IsAPlane(gp_Pln *returned_plane)const;
	std::wstring GetSurfaceTypeStr();
	CEdge* GetFirstEdge();
	CEdge* GetNextEdge();
	CLoop* GetFirstLoop();
	CLoop* GetNextLoop();
	bool Orientation();
	void GetUVBox(double *uv_box)const;
	void GetSurfaceUVPeriod(double *uv, bool *isUPeriodic, bool *isVPeriodic);
	CShape* GetParentBody();
	void MakeSureMarkingGLListExists();
	void KillMarkingGLList();
	void UpdateMarkingGLList(bool marked, bool no_color);
	void RenderNormalArrow();
};

#if 0
class FaceToSketchTool:public Tool
{
public:
	const wchar_t* GetTitle(){return L"Make a sketch from face";}
	std::wstring BitmapPath(){return L"face2sketch";}
	void Run();

	static double deviation;
};

#endif
