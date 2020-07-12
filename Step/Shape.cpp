// Shape.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "Shape.h"
#include "Solid.h"
#include "Wire.h"
#include "Group.h"
#include "Face.h"
#include "Edge.h"
#include "Vertex.h"
#include "Loop.h"
#include "Cylinder.h"
#include "Cuboid.h"
#include "Sphere.h"
#include "Cone.h"
#include "strconv.h"
#include "App.h"
#include <locale.h>

// static member variable
bool CShape::m_solids_found = false;
double CShape::m_iges_sewing_tolerance = 0.001;

// static
int CShape::m_type = 0;

CShape::CShape()
:m_face_gl_list(0),
m_edge_gl_list(0),
m_select_edge_gl_list(0),
 m_opacity(1.0),
 m_volume_found(false),
 m_color(0, 0, 0),
 m_picked_face(NULL)
{
	Init();
}

CShape::CShape(const TopoDS_Shape &shape, const wchar_t* title, const HeeksColor& col, float opacity)
:IdNamedObjList(title),
 m_face_gl_list(0),
 m_edge_gl_list(0),
 m_select_edge_gl_list(0),
 m_shape(shape),
 m_opacity(opacity),
 m_volume_found(false),
 m_color(col),
 m_picked_face(NULL)
{
	Init();
}

CShape::CShape(const CShape& s)
:m_face_gl_list(0),
 m_edge_gl_list(0),
 m_select_edge_gl_list(0),
 m_volume_found(false),
 m_picked_face(NULL)
{
	// the faces, edges, vertices children are not copied, because we don't need them for copies in the undo engine
	m_faces = NULL;
	m_edges = NULL;
	m_vertices = NULL;
	operator=(s);
}

CShape::~CShape()
{
	KillGLLists();
	delete_faces_and_edges();
}

const CShape& CShape::operator=(const CShape& s)
{
    HeeksObj::operator = (s);

	// don't copy id
	delete_faces_and_edges();
	m_box = s.m_box;
	m_shape = s.m_shape;
	m_title = s.m_title;
	m_color = s.m_color;
	m_opacity = s.m_opacity;
	m_volume_found = s.m_volume_found;
	if(m_volume_found)m_volume = s.m_volume;

	KillGLLists();

	return *this;
}

void CShape::Init()
{
	m_faces = new CFaceList;
	m_edges = new CEdgeList;
	m_vertices = new CVertexList;
	Add(m_faces, NULL);
	Add(m_edges, NULL);
	Add(m_vertices, NULL);
	create_faces_and_edges();
}

void CShape::KillGLLists()
{
	if (m_face_gl_list)
	{
		glDeleteLists(m_face_gl_list, 1);
		m_face_gl_list = 0;
	}

	if (m_edge_gl_list)
	{
		glDeleteLists(m_edge_gl_list, 1);
		m_edge_gl_list = 0;
	}

	if (m_select_edge_gl_list)
	{
		glDeleteLists(m_select_edge_gl_list, 1);
		m_select_edge_gl_list = 0;
	}

	m_box = CBox();

	if(m_faces)
	{
		for(HeeksObj* object = m_faces->GetFirstChild(); object; object = m_faces->GetNextChild())
		{
			CFace* f = (CFace*)object;
			f->KillMarkingGLList();
		}
	}
}

void CShape::create_faces_and_edges()
{
	if(m_faces == NULL)
	{
		m_faces = new CFaceList;
		m_edges = new CEdgeList;
		m_vertices = new CVertexList;
		Add(m_faces, NULL);
		Add(m_edges, NULL);
		Add(m_vertices, NULL);
	}
	CreateFacesAndEdges(m_shape, m_faces, m_edges, m_vertices);
}

void CShape::delete_faces_and_edges()
{
	if(m_faces)m_faces->Clear();
	if(m_edges)m_edges->Clear();
	if(m_vertices)m_vertices->Clear();
}

void CShape::CallMesh()
{
	double pixels_per_mm = theApp->GetPixelScale();
	BRepTools::Clean(m_shape);
	BRepMesh_IncrementalMesh(m_shape, 1 / pixels_per_mm);
}

void CShape::glCommands(bool select, bool marked, bool no_color)
{
	bool mesh_called = false;
	bool draw_faces = (theApp->GetSolidViewMode() == SolidViewFacesAndEdges || theApp->GetSolidViewMode() == SolidViewFacesOnly);
	bool draw_edges = (theApp->GetSolidViewMode() == SolidViewFacesAndEdges || theApp->GetSolidViewMode() == SolidViewEdgesOnly);

	if(draw_faces)
	{
		for(HeeksObj* object = m_faces->GetFirstChild(); object; object = m_faces->GetNextChild())
		{
			CFace* f = (CFace*)object;
			f->MakeSureMarkingGLListExists();
		}

		if(!m_face_gl_list)
		{
			if(!mesh_called)
			{
				CallMesh();
				mesh_called = true;
			}

			// make the display list
			m_face_gl_list = glGenLists(1);
			glNewList(m_face_gl_list, GL_COMPILE);

			// render all the faces
			m_faces->glCommands(true, false, true);

			glEndList();
		}

		// update faces marking display list
		GLint currentListIndex;
		glGetIntegerv(GL_LIST_INDEX, &currentListIndex);
		if(currentListIndex == 0){
			for(HeeksObj* object = m_faces->GetFirstChild(); object; object = m_faces->GetNextChild())
			{
				CFace* f = (CFace*)object;
				f->UpdateMarkingGLList(theApp->ObjectMarked(f), no_color);
			}
		}
	}

	int *p_edge_gl_list = select ? &m_select_edge_gl_list : &m_edge_gl_list;

	if (draw_edges && !*p_edge_gl_list)
	{
		if(!mesh_called)
		{
			CallMesh();
			mesh_called = true;
		}

		// make the display list
		*p_edge_gl_list = glGenLists(1);
		glNewList(*p_edge_gl_list, GL_COMPILE);

		// render all the edges
		m_edges->glCommands(select, marked, no_color);

		// render all the vertices
		if (select)m_vertices->glCommands(true, false, false);

		glEndList();
	}

	if(draw_faces && m_face_gl_list)
	{
		// draw the face display list
		if(!select)glEnable(GL_LIGHTING);
		if (!select)glShadeModel(GL_SMOOTH);
		glCallList(m_face_gl_list);
		if (!select)glDisable(GL_LIGHTING);
		if (!select)glShadeModel(GL_FLAT);
	}

	{
		// turn off transparency
		glDisable(GL_BLEND);
		glDepthMask(1);
	}

	if (draw_edges && *p_edge_gl_list)
	{
		// draw the edge display list
		glCallList(*p_edge_gl_list);
	}
}

void CShape::GetBox(CBox &box)
{
	if(!m_box.m_valid)
	{
		if(m_faces == NULL)create_faces_and_edges();
		BRepTools::Clean(m_shape);
		BRepMesh_IncrementalMesh(m_shape, 1.0);
		if(m_faces)m_faces->GetBox(m_box);
	}

	box.Insert(m_box);
}

void CShape::MakeTransformedShape(const gp_Trsf &mat)
{
	BRepBuilderAPI_Transform myBRepTransformation(m_shape,mat);
	m_shape = myBRepTransformation.Shape();
}

std::wstring CShape::StretchedName()
{
	return L"Stretched Shape";
}

void CShape::Transform(const Matrix &m){
	gp_Trsf mat = make_matrix(m.e);

	if(IsMatrixDifferentialScale(mat))
	{
        gp_GTrsf gm(mat);
		BRepBuilderAPI_GTransform t(m_shape, gm);
        m_shape = t.Shape();
	}
	else
	{
		MakeTransformedShape(mat);
	}
	delete_faces_and_edges();
	KillGLLists();
	create_faces_and_edges();
}

// static member function
HeeksObj* CShape::MakeObject(const TopoDS_Shape &shape, const wchar_t* title, SolidTypeEnum solid_type, const HeeksColor& col, float opacity){
	if(shape.IsNull())return NULL;

	switch(shape.ShapeType()){
		case TopAbs_FACE:
			{
				return new CFace(TopoDS::Face(shape));
			}
		case TopAbs_WIRE:
			{
				return new CWire(TopoDS::Wire(shape), title);
			}
		case TopAbs_EDGE:
			{
				return new CEdge(TopoDS::Edge(shape));
			}
		case TopAbs_VERTEX:
			return NULL;
		case TopAbs_COMPOUND:
		case TopAbs_COMPSOLID:
		case TopAbs_SOLID:
		case TopAbs_SHELL:
		case TopAbs_SHAPE:
			{
				switch(solid_type)
				{
				case SOLID_TYPE_SPHERE:
					return new CSphere(*((TopoDS_Solid*)(&shape)), title, col, opacity);
				case SOLID_TYPE_CYLINDER:
					return new CCylinder(*((TopoDS_Solid*)(&shape)), title, col, opacity);
				case SOLID_TYPE_CUBOID:
					return new CCuboid(*((TopoDS_Solid*)(&shape)), title, col, opacity);
				case SOLID_TYPE_CONE:
					return new CCone(*((TopoDS_Solid*)(&shape)), title, col, opacity);
				default:
					// check there are some faces
					if(TopExp_Explorer(shape, TopAbs_FACE).More())
						return new CSolid(*((TopoDS_Solid*)(&shape)), title, col, opacity);
					return NULL;
				}
			}
	}

	return NULL;
}

static bool Cut(const std::list<TopoDS_Shape> &shapes, TopoDS_Shape& new_shape){
	if(shapes.size() < 2)return false;

	try
	{
		std::list<TopoDS_Shape>::const_iterator It = shapes.begin();
		TopoDS_Shape current_shape = *It;
		It++;
		while(It != shapes.end())
		{
			const TopoDS_Shape &cutting_shape = *It;
			current_shape = BRepAlgoAPI_Cut(current_shape, cutting_shape);
			It++;
		}

		new_shape = current_shape;
		return true;
	}
	catch (Standard_Failure) {
		Handle_Standard_Failure e = Standard_Failure::Caught();
		theApp->DoMessageBox((std::wstring(L"Error with cut operation") + L": " + Ctt(e->GetMessageString())).c_str());
		return false;
	}
}

static HeeksObj* Fuse(HeeksObj* s1, HeeksObj* s2){
	try
	{
		TopoDS_Shape sh1, sh2;
		TopoDS_Shape new_shape;
		if(CSettings::useOldFuse)new_shape = BRepAlgo_Fuse(((CShape*)s1)->Shape(), ((CShape*)s2)->Shape());
		else new_shape = BRepAlgoAPI_Fuse(((CShape*)s1)->Shape(), ((CShape*)s2)->Shape());

		HeeksObj* new_object = CShape::MakeObject(new_shape, ((CShape*)s1)->m_title_made_from_id ? std::wstring(L"Result of Fuse Operation").c_str() : ((CShape*)s1)->m_title.c_str(), SOLID_TYPE_UNKNOWN, ((CShape*)s1)->m_color, ((CShape*)s1)->GetOpacity());
		theApp->AddUndoably(new_object, NULL, NULL);
		theApp->DeleteUndoably(s1);
		theApp->DeleteUndoably(s2);
		return new_object;
	}
	catch (Standard_Failure) {
		Handle_Standard_Failure e = Standard_Failure::Caught();
		theApp->DoMessageBox((std::wstring(L"Error with fuse operation") + L": " + Ctt(e->GetMessageString())).c_str());
		return NULL;
	}
}

static HeeksObj* Common(HeeksObj* s1, HeeksObj* s2){
	if(s1 == NULL)
	{
		theApp->GetObjPointer()->Remove(s2);
		return NULL;
	}

	try
	{
		TopoDS_Shape sh1, sh2;
		TopoDS_Shape new_shape = BRepAlgoAPI_Common(((CShape*)s1)->Shape(), ((CShape*)s2)->Shape());

		HeeksObj* new_object = CShape::MakeObject(new_shape, ((CShape*)s1)->m_title_made_from_id ? std::wstring(L"Result of Common Operation").c_str() : ((CShape*)s1)->m_title.c_str(), SOLID_TYPE_UNKNOWN, ((CShape*)s1)->m_color, ((CShape*)s1)->GetOpacity());
		theApp->AddUndoably(new_object, NULL, NULL);
		theApp->DeleteUndoably(s1);
		theApp->DeleteUndoably(s2);
		return new_object;
	}
	catch (Standard_Failure) {
		Handle_Standard_Failure e = Standard_Failure::Caught();
		theApp->DoMessageBox((std::wstring(L"Error with common operation") + L": " + Ctt(e->GetMessageString())).c_str());
		return NULL;
	}
}

CFace* CShape::find(const TopoDS_Face &face)
{
	for(HeeksObj* object = m_faces->GetFirstChild(); object; object = m_faces->GetNextChild())
	{
		CFace* f = (CFace*)object;
		if(f->Face() == face)return f;
	}
	return NULL;
}

bool CShape::GetExtents(double* extents, const double* orig, const double* xdir, const double* ydir, const double* zdir)
{
	gp_Pnt p_orig(0, 0, 0);
	if(orig)p_orig = gp_Pnt(orig[0], orig[1], orig[2]);
	gp_Vec v_x(1, 0, 0);
	if(xdir)v_x = gp_Vec(xdir[0], xdir[1], xdir[2]);
	gp_Vec v_y(0, 1, 0);
	if(ydir)v_y = gp_Vec(ydir[0], ydir[1], ydir[2]);
	gp_Vec v_z(0, 0, 1);
	if(zdir)v_z = gp_Vec(zdir[0], zdir[1], zdir[2]);

	BRepPrimAPI_MakeBox cuboid_plus_x(gp_Ax2(gp_Pnt(p_orig.XYZ() + 2000000 * v_x.XYZ() + (-1000000) * v_z.XYZ() + (-1000000) * v_y.XYZ()), v_x, v_y), 1000000, 1000000, 1000000);
	BRepPrimAPI_MakeBox cuboid_minus_x(gp_Ax2(gp_Pnt(p_orig.XYZ() + (-2000000) * v_x.XYZ() + (-1000000) * v_z.XYZ() + (-1000000) * v_y.XYZ()), -v_x, v_z), 1000000, 1000000, 1000000);
	BRepPrimAPI_MakeBox cuboid_plus_y(gp_Ax2(gp_Pnt(p_orig.XYZ() + 2000000 * v_y.XYZ() + (-1000000) * v_z.XYZ() + (-1000000) * v_x.XYZ()), v_y, v_z), 1000000, 1000000, 1000000);
	BRepPrimAPI_MakeBox cuboid_minus_y(gp_Ax2(gp_Pnt(p_orig.XYZ() + (-2000000) * v_y.XYZ() + (-1000000) * v_z.XYZ() + (-1000000) * v_x.XYZ()), -v_y, v_x), 1000000, 1000000, 1000000);
	BRepPrimAPI_MakeBox cuboid_plus_z(gp_Ax2(gp_Pnt(p_orig.XYZ() + 2000000 * v_z.XYZ() + (-1000000) * v_x.XYZ() + (-1000000) * v_y.XYZ()), v_z, v_x), 1000000, 1000000, 1000000);
	BRepPrimAPI_MakeBox cuboid_minus_z(gp_Ax2(gp_Pnt(p_orig.XYZ() + (-2000000) * v_z.XYZ() + (-1000000) * v_x.XYZ() + (-1000000) * v_y.XYZ()), -v_z, v_y), 1000000, 1000000, 1000000);

	gp_Vec v_orig(p_orig.XYZ());

	TopoDS_Solid shape[6] =
	{
		cuboid_minus_x,
		cuboid_minus_y,
		cuboid_minus_z,
		cuboid_plus_x,
		cuboid_plus_y,
		cuboid_plus_z
	};

	gp_Vec vector[6] =
	{
		v_x,
		v_y,
		v_z,
		v_x,
		v_y,
		v_z
	};

	for(int i = 0; i<6; i++){
		BRepExtrema_DistShapeShape extrema(m_shape, shape[i]);
		extrema.Perform();
		gp_Pnt p = extrema.PointOnShape1(1);
		gp_Vec v(p.XYZ());
		double dp = v * vector[i];
		double dp_o = v_orig * vector[i];
		extents[i] = dp - dp_o;
	}

	return true;
}

void CShape::CopyIDsFrom(const CShape* shape_from)
{
	SetID(shape_from->m_id);
	HeeksObj* face_from = shape_from->m_faces->GetFirstChild();
	for(HeeksObj* face_to = m_faces->GetFirstChild(); face_from && face_to; face_from = shape_from->m_faces->GetNextChild(), face_to = m_faces->GetNextChild())
	{
		face_to->SetID(face_from->m_id);
	}
	HeeksObj* edge_from = shape_from->m_edges->GetFirstChild();
	for(HeeksObj* edge_to = m_edges->GetFirstChild(); edge_from && edge_to; edge_from = shape_from->m_edges->GetNextChild(), edge_to = m_edges->GetNextChild())
	{
		edge_to->SetID(edge_from->m_id);
	}
	HeeksObj* v_from = shape_from->m_vertices->GetFirstChild();
	for(HeeksObj* v_to = m_vertices->GetFirstChild(); v_from && v_to; v_from = shape_from->m_vertices->GetNextChild(), v_to = m_vertices->GetNextChild())
	{
		v_to->SetID(v_from->m_id);
	}
}

HeeksObj* CShape::CutShapes(std::list<HeeksObj*> &list_in, bool dodelete)
{
	theApp->StartHistory();
	HeeksObj* return_object = NULL;

#if 0
	if(list_in.front()->GetType() == GroupType)
	{
		CGroup* group = (CGroup*)list_in.front();
		CGroup* newgroup = new CGroup();
		theApp->AddUndoably(newgroup,group->m_owner,NULL);

		std::list<HeeksObj*> children;
		HeeksObj* child = group->GetFirstChild();
		while(child)
		{
			children.push_back(child);
			child = group->GetNextChild();
		}

		std::list<HeeksObj*>::iterator iter = children.begin();
		while(iter != children.end())
		{
			std::list<HeeksObj*> newlist;
			std::list<HeeksObj*>::const_iterator it = list_in.begin();
			while(it!=list_in.end())
			{
				newlist.push_back(*it);
				++it;
			}
			newlist.pop_front();
			newlist.push_front(*iter);
			HeeksObj* newshape = CutShapes(newlist,false);
			theApp->DeleteUndoably(newshape);
			theApp->AddUndoably(newshape,newgroup, NULL);
			++iter;
		}

		theApp->DeleteUndoably(group);
		theApp->DeleteUndoably(list_in);
		return_object = newgroup;
	}
	else
#endif
	{

	// subtract from the first one in the list all the others
	std::list<TopoDS_Shape> shapes;
	std::list<HeeksObj*> delete_list;
	HeeksObj* first_solid = NULL;

	for(std::list<HeeksObj*>::const_iterator It = list_in.begin(); It != list_in.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == CSolid::m_type)
		{
			shapes.push_back(((CSolid*)object)->Shape());
			if(first_solid == NULL)first_solid = object;
			delete_list.push_back(object);
		}
		else if (object->GetType() == CFace::m_type)
		{
			shapes.push_back(((CFace*)object)->Face());
			if(first_solid == NULL)first_solid = object;
			delete_list.push_back(object);
		}
	}

	return_object = first_solid;

	TopoDS_Shape new_shape;
	if(Cut(shapes, new_shape))
	{
		HeeksObj* new_object = CShape::MakeObject(new_shape, ((CShape*)first_solid)->m_title_made_from_id ? std::wstring(L"Result of Cut Operation").c_str() : ((CShape*)first_solid)->m_title.c_str(), SOLID_TYPE_UNKNOWN, ((CShape*)first_solid)->m_color, (float)(((CShape*)first_solid)->m_opacity));
		theApp->AddUndoably(new_object, NULL, NULL);
		if(dodelete)
		{
			theApp->DeleteUndoably(delete_list);
		}
		return_object = new_object;
	}
	}

	theApp->EndHistory();
	theApp->Repaint();

	return return_object;
}

HeeksObj* CShape::FuseShapes(std::list<HeeksObj*> &list_in)
{
	// fuse with the first one in the list all the others
	HeeksObj* s1 = NULL;
	std::list<HeeksObj*> list = list_in;

	for(std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++){
		HeeksObj* object = *It;
		if (object->GetType() == CSolid::m_type || object->GetType() == CFace::m_type)
		{
			if(s1 == NULL)s1 = object;
			else{
				s1 = Fuse(s1, object);
			}
		}
	}

	theApp->Repaint();

	return s1;
}

HeeksObj* CShape::CommonShapes(std::list<HeeksObj*> &list_in)
{
	// find common solid ( intersect ) with the first one in the list all the others
	HeeksObj* s1 = NULL;
	bool s1_set = false;
	std::list<HeeksObj*> list = list_in;

	for(std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++){
		HeeksObj* object = *It;
		if (object->GetType() == CSolid::m_type || object->GetType() == CFace::m_type)
		{
			if(!s1_set)
			{
				s1 = object;
				s1_set = true;
			}
			else{
				s1 = Common(s1, object);
			}
		}
	}

	theApp->Repaint();

	return s1;
}

void CShape::FilletOrChamferEdges(std::list<HeeksObj*> &list, double radius, bool chamfer_not_fillet)
{
	// make a map with a list of edges for each solid
	std::map< HeeksObj*, std::list< HeeksObj* > > solid_edge_map;

	for(std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++){
		HeeksObj* edge = *It;
		if(edge->GetType() == CEdge::m_type)
		{
			HeeksObj* solid = edge->m_owner->m_owner;
			if (solid && solid->GetType() == CSolid::m_type)
			{
				std::map< HeeksObj*, std::list< HeeksObj* > >::iterator FindIt = solid_edge_map.find(solid);
				if(FindIt == solid_edge_map.end())
				{
					std::list< HeeksObj* > empty_list;
					solid_edge_map.insert( make_pair(solid, empty_list) );
					FindIt = solid_edge_map.find(solid);
				}

				std::list< HeeksObj* > &list = FindIt->second;
				list.push_back(edge);
			}
		}
	}

	// do each solid
	for(std::map< HeeksObj*, std::list< HeeksObj* > >::iterator It = solid_edge_map.begin(); It != solid_edge_map.end(); It++)
	{
		HeeksObj* solid = It->first;
		std::list< HeeksObj* > &list = It->second;

		try{
			if(chamfer_not_fillet)
			{
				BRepFilletAPI_MakeChamfer chamfer(((CShape*)solid)->Shape());
				for(std::list< HeeksObj* >::iterator It2 = list.begin(); It2 != list.end(); It2++)
				{
					CEdge* edge = (CEdge*)(*It2);
					for(CFace* face = (CFace*)(edge->GetFirstFace()); face; face = (CFace*)(edge->GetNextFace()))
					{
						chamfer.Add(radius, TopoDS::Edge(edge->Edge()), TopoDS::Face(face->Face()));
					}
				}
				TopoDS_Shape new_shape = chamfer.Shape();
				theApp->AddUndoably(new CSolid(*((TopoDS_Solid*)(&new_shape)), L"Solid with edge blend", *(solid->GetColor()), (float)(((CShape*)solid)->m_opacity)), NULL, NULL);
				theApp->DeleteUndoably(solid);
			}
			else
			{
				BRepFilletAPI_MakeFillet fillet(((CShape*)solid)->Shape());
				for(std::list< HeeksObj* >::iterator It2 = list.begin(); It2 != list.end(); It2++)
				{
					fillet.Add(radius, TopoDS::Edge(((CEdge*)(*It2))->Edge()));
				}
				TopoDS_Shape new_shape = fillet.Shape();
				theApp->AddUndoably(new CSolid(*((TopoDS_Solid*)(&new_shape)), L"Solid with edge blend", *(solid->GetColor()), (float)(((CShape*)solid)->m_opacity)), NULL, NULL);
				theApp->DeleteUndoably(solid);
			}
		}
		catch (Standard_Failure) {
			Handle_Standard_Failure e = Standard_Failure::Caught();
			theApp->DoMessageBox((std::wstring(L"Error making fillet") + L": " + Ctt(e->GetMessageString())).c_str());
		}
		catch(...)
		{
			theApp->DoMessageBox(L"A fatal error happened during Blend");
		}
	}

	theApp->Repaint();
}

static double GetVolume(TopoDS_Shape shape)
// http://www.opencascade.org/org/forum/thread_15471/
{
GProp_GProps System;
BRepGProp::VolumeProperties(shape, System);
return System.Mass();
}

bool CShape::ImportSolidsFile(const wchar_t* filepath, bool undoably, std::map<int, CShapeData> *index_map, HeeksObj* paste_into)
{
	// returns true, if suffix handled
	std::wstring wf(filepath);

	HeeksObj* add_to = theApp->GetObjPointer();
	if(paste_into)add_to = paste_into;

	if(endsWith(wf, L".stp") || endsWith(wf,L".STP") || endsWith(wf,L".step") || endsWith(wf,L".STEP"))
	{
		char oldlocale[1000];
		strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

		Standard_CString aFileName = (Standard_CString) (Ttc(filepath));
		STEPControl_Reader Reader;
		int status = Reader.ReadFile( aFileName );

		if ( status == IFSelect_RetDone )
		{
			int num = Reader.NbRootsForTransfer();
			for(int i = 1; i<=num; i++)
			{
				Handle_Standard_Transient root = Reader.RootForTransfer(i);
				Reader.TransferEntity(root);
				TopoDS_Shape rShape = Reader.Shape(i);
				if(index_map)
				{
					// change the id ( and any other data ), to the one in the step file index
					std::map<int, CShapeData>::iterator FindIt = index_map->find(i);
					if(FindIt != index_map->end())
					{
						CShapeData& shape_data = FindIt->second;
						HeeksObj* new_object = MakeObject(rShape, L"STEP solid", shape_data.m_solid_type, HeeksColor(191, 191, 191), 1.0f);
						if(new_object)
						{
							if(undoably)theApp->AddUndoably(new_object, add_to, NULL);
							else add_to->Add(new_object, NULL);
							shape_data.SetShape((CShape*)new_object/*, !theApp->m_inPaste*/, true);
						}
					}
				}
				else
				{
					HeeksObj* new_object = MakeObject(rShape, L"STEP solid", SOLID_TYPE_UNKNOWN, HeeksColor(191, 191, 191), 1.0f);
					if(undoably)theApp->AddUndoably(new_object, add_to, NULL);
					else add_to->Add(new_object, NULL);
				}
			}
		}
		else{
			theApp->DoMessageBox(L"STEP import not done!");
		}

		setlocale(LC_NUMERIC, oldlocale);

		return true;
	}
	else if(endsWith(wf, L".igs") || endsWith(wf,L".IGS") || endsWith(wf,L".iges") || endsWith(wf,L".IGES"))
	{
		char oldlocale[1000];
		strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

		Standard_CString aFileName = (Standard_CString) (Ttc(filepath));

		IGESControl_Reader Reader;
		int status = Reader.ReadFile( aFileName );

		if ( status == IFSelect_RetDone )
		{
			int num = Reader.NbRootsForTransfer();
			int shapes_added_for_sewing = 0;
			BRepOffsetAPI_Sewing face_sewing(m_iges_sewing_tolerance);
			std::list<TopoDS_Shape> shapes_readed;

			for(int i = 1; i<=num; i++)
			{
				Handle_Standard_Transient root = Reader.RootForTransfer(i);
				Reader.TransferEntity(root);
				TopoDS_Shape rShape = Reader.Shape(i);

				TopAbs_ShapeEnum shape_type = rShape.ShapeType();

				if (shape_type == TopAbs_COMPOUND)
				{
					for (TopExp_Explorer explorer(rShape, TopAbs_SOLID); explorer.More(); explorer.Next())
					{
						TopoDS_Shape shape = explorer.Current();
						TopAbs_ShapeEnum shape_type2 = shape.ShapeType();
						shape_type = shape_type2;
					}
				}

				shapes_readed.push_back(rShape);

				for (TopExp_Explorer explorer(rShape, TopAbs_FACE); explorer.More(); explorer.Next())
				{
					face_sewing.Add (explorer.Current());
					shapes_added_for_sewing++;
				}
			}

			bool sewed_shape_added = false;
			std::list<TopoDS_Edge> free_edges;
			std::list<TopoDS_Edge> too_many_face_edges;

			try{
				if(shapes_added_for_sewing > 0)
				{
					face_sewing.Perform();
					TopoDS_Shape sewed_shape = face_sewing.SewedShape();

					if (!sewed_shape.IsNull())
					{
						std::list<TopoDS_Shell> shells;

						TopAbs_ShapeEnum sewed_shape_type = sewed_shape.ShapeType();
						if (sewed_shape_type == TopAbs_SHELL)
						{
							shells.push_back(TopoDS::Shell(sewed_shape));
						}
						else if (sewed_shape_type == TopAbs_COMPOUND)
						{
							for (TopExp_Explorer explorer(sewed_shape, TopAbs_SHELL); explorer.More(); explorer.Next())
							{
								shells.push_back(TopoDS::Shell(explorer.Current()));
							}
						}

						BRepBuilderAPI_MakeSolid solid_maker;

						for (std::list<TopoDS_Shell>::iterator It = shells.begin(); It != shells.end(); It++)
						{
							TopoDS_Shell shell = *It;
							bool shell_added = false;
							try{
								solid_maker.Add(shell);
								TopoDS_Shape solid = solid_maker.Solid();

								if (GetVolume(solid) < 0.0) solid.Reverse();

								HeeksObj* new_object = MakeObject(solid, L"sewed IGES solid", SOLID_TYPE_UNKNOWN, HeeksColor(191, 191, 191), 1.0f);
								if (undoably)theApp->AddUndoably(new_object, add_to, NULL);
								else add_to->Add(new_object, NULL);
								shell_added = true;
							}
							catch (Standard_Failure) {
								Handle_Standard_Failure e = Standard_Failure::Caught();
								theApp->DoMessageBox((std::wstring(L"Error making solid from sewn shell") + L": " + Ctt(e->GetMessageString())).c_str());
							}
							catch (...)
							{
							}

							if (!shell_added)
							{
								HeeksObj* new_object = MakeObject(shell, L"sew failed IGES shell", SOLID_TYPE_UNKNOWN, HeeksColor(191, 128, 128), 1.0f);
								if (undoably)theApp->AddUndoably(new_object, add_to, NULL);
								else add_to->Add(new_object, NULL);
							}
							sewed_shape_added = true;
					}

					}

					if (face_sewing.NbFreeEdges() > 0)
					{
						bool m_create_disconnected_edges = true;
						if (m_create_disconnected_edges)
						{
							for (int i = 0; i < face_sewing.NbFreeEdges(); i++)
							{
								HeeksObj* new_object = MakeObject(face_sewing.FreeEdge(i + 1), L"free edge from IGES import", SOLID_TYPE_UNKNOWN, HeeksColor(255, 0, 0), 1.0f);
								if (undoably)theApp->AddUndoably(new_object, add_to, NULL);
								else add_to->Add(new_object, NULL);
							}
						}
					}
					if (face_sewing.NbMultipleEdges() > 0)
					{
						bool m_create_more_than_twice_edges = true;
						if (m_create_more_than_twice_edges)
						{
							for (int i = 0; i < face_sewing.NbMultipleEdges(); i++)
							{
								HeeksObj* new_object = MakeObject(face_sewing.MultipleEdge(i + 1), L"multiple edge from IGES import", SOLID_TYPE_UNKNOWN, HeeksColor(255, 128, 0), 1.0f);
								if (undoably)theApp->AddUndoably(new_object, add_to, NULL);
								else add_to->Add(new_object, NULL);
							}
						}
					}
					if (face_sewing.NbDegeneratedShapes() > 0)
					{
						bool m_create_degenerate_shapes = true;
						if (m_create_degenerate_shapes)
						{
							for (int i = 0; i < face_sewing.NbDegeneratedShapes(); i++)
							{
								HeeksObj* new_object = MakeObject(face_sewing.DegeneratedShape(i + 1), L"degenerate shape from IGES import", SOLID_TYPE_UNKNOWN, HeeksColor(255, 128, 0), 1.0f);
								if (undoably)theApp->AddUndoably(new_object, add_to, NULL);
								else add_to->Add(new_object, NULL);
							}
						}
					}

				}
			}
			catch (Standard_Failure) {
				Handle_Standard_Failure e = Standard_Failure::Caught();
				theApp->DoMessageBox((std::wstring(L"Error sewing") + L": " + Ctt(e->GetMessageString())).c_str());
			}
			catch (...)
			{
			}
			if(!sewed_shape_added)
			{
				// add the originals
				for(std::list<TopoDS_Shape>::iterator It = shapes_readed.begin(); It != shapes_readed.end(); It++)
				{
					TopoDS_Shape rShape = *It;
					HeeksObj* new_object = MakeObject(rShape, L"IGES shape", SOLID_TYPE_UNKNOWN, HeeksColor(191, 191, 191), 1.0f);
					if(undoably)theApp->AddUndoably(new_object, add_to, NULL);
					else add_to->Add(new_object, NULL);
				}
			}

		}
		else{
			theApp->DoMessageBox(L"IGES import not done!");
		}

		setlocale(LC_NUMERIC, oldlocale);

		return true;
	}
	else if(endsWith(wf,L".brep") || endsWith(wf,L".BREP"))
	{
		char oldlocale[1000];
		strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

		TopoDS_Shape shape;
		BRep_Builder builder;
		Standard_Boolean result = BRepTools::Read(  shape,(char *) Ttc(filepath), builder );

		if(result)
		{
			HeeksObj* new_object = MakeObject(shape, L"BREP solid", SOLID_TYPE_UNKNOWN, HeeksColor(191, 191, 191), 1.0f);
			if (undoably)theApp->AddUndoably(new_object, add_to, NULL);
			else add_to->Add(new_object, NULL);
		}
		else{
			theApp->DoMessageBox(L"STEP import not done!");
		}

		setlocale(LC_NUMERIC, oldlocale);

		return true;
	}
	return false;
}

static void WriteShapeOrGroup(STEPControl_Writer &writer, HeeksObj* object, std::map<int, CShapeData> *index_map, int &i)
{
	if(CShape::IsTypeAShape(object->GetType())){

		if(index_map)index_map->insert( std::pair<int, CShapeData>(i, CShapeData((CShape*)object)) );
		i++;
		writer.Transfer(((CSolid*)object)->Shape(), STEPControl_AsIs);
	}

	if(object->GetType() == GroupType)
	{
		for(HeeksObj* o = object->GetFirstChild(); o; o = object->GetNextChild())
		{
			WriteShapeOrGroup(writer, o, index_map, i);
		}
	}
}

bool CShape::ExportSolidsFile(const std::list<HeeksObj*>& objects, const wchar_t* filepath, std::map<int, CShapeData> *index_map)
{
	// returns true, if suffix handled
	std::wstring wf(filepath);
	lowerCase(wf);

	if(endsWith(wf,L".stp") || endsWith(wf,L".step"))
	{
		char oldlocale[1000];
		strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

		STEPControl_Writer writer;
		// add all the solids
		int i = 1;
		for(std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
		{
			HeeksObj* object = *It;
			WriteShapeOrGroup(writer, object, index_map, i);
		}
		Standard_CString aFileName = (Standard_CString)(Ttc(filepath));
		writer.Write(aFileName);

		setlocale(LC_NUMERIC, oldlocale);

		return true;
	}
	else if(endsWith(wf,L".igs") || endsWith(wf,L".iges"))
	{
		char oldlocale[1000];
		strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

		Standard_CString aFileName = (Standard_CString) (Ttc(filepath));

		IGESControl_Controller::Init();
		IGESControl_Writer writer;

		// add all the solids
		for(std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
		{
			HeeksObj* object = *It;
			if(CShape::IsTypeAShape(object->GetType())){
				writer.AddShape(((CShape*)object)->Shape());
			}
			else if(object->GetType() == CWire::m_type){
				writer.AddShape(((CWire*)object)->Shape());
			}
		}
		writer.Write(aFileName);

		setlocale(LC_NUMERIC, oldlocale);

		return true;
	}
	else if(endsWith(wf,L".brep"))
	{
#ifdef __WXMSW__
		ofstream ofs(filepath);
#else
		ofstream ofs(Ttc(filepath));
#endif
		for(std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
		{
			HeeksObj* object = *It;
			if(CShape::IsTypeAShape(object->GetType())){
				BRepTools::Write(((CShape*)object)->Shape(), ofs);
			}
		}
	}

	return false;
}

void CShape::GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal){
	BRepTools::Clean(m_shape);
	BRepMesh_IncrementalMesh(m_shape, cusp);

	return IdNamedObjList::GetTriangles(callbackfunc, cusp, just_one_average_normal);
}

double CShape::Area()const{
	double area = 0.0;

	for(HeeksObj* object = m_faces->GetFirstChild(); object; object = m_faces->GetNextChild())
	{
		CFace* f = (CFace*)object;
		area += f->Area();
	}

	return area;
}

// static member function
bool CShape::IsTypeAShape(int t){
	return t == CSolid::m_type || t == CWire::m_type;
}

// static
bool CShape::IsMatrixDifferentialScale(const gp_Trsf& trsf)
{
	double scalex = gp_Vec(1, 0, 0).Transformed(trsf).Magnitude();
	double scaley = gp_Vec(0, 1, 0).Transformed(trsf).Magnitude();
	double scalez = gp_Vec(0, 0, 1).Transformed(trsf).Magnitude();

	if(fabs(scalex - scaley) > 0.000000000001)return true;
	if(fabs(scalex - scalez) > 0.000000000001)return true;
	return false;
}

void CShape::CopyFrom(const HeeksObj* object)
{
	*this = *((CShape*)object);
}

void CShape::WriteXML(TiXmlNode *root)
{
	CShape::m_solids_found = true;
}

float CShape::GetOpacity()
{
	return (float)m_opacity;
}

void CShape::SetOpacity(float opacity)
{
	m_opacity = opacity;
	if(m_opacity < 0.0)m_opacity = 0.0f;
	if(m_opacity > 1.0)m_opacity = 1.0f;
}

void CShape::CalculateVolumeAndCentre()
{
	GProp_GProps System;
    BRepGProp::VolumeProperties(m_shape, System);
    m_volume = System.Mass();
	m_centre_of_mass = System.CentreOfMass();
	m_volume_found = true;
}

static double volume_for_properties = 0.0;
static double centre_of_mass[3] = { 0.0, 0.0, 0.0 };

#if 0
class CalculateVolumeProperty :public Property{
public:
	CalculateVolumeProperty(HeeksObj* object) :Property(object, L"calculate volume"){}

	// Property's virtual functions
	int get_property_type(){ return CheckPropertyType; }
	Property *MakeACopy(void)const{	return new CalculateVolumeProperty(*this);}
	void Set(bool value){
		((CShape*)m_object)->CalculateVolumeAndCentre();
		theApp->m_frame->RefreshProperties();
	}
	bool GetBool(void)const{ return false; }
};
#endif

void CShape::GetProperties(std::list<Property *> *list)
{
#if 0
	list->push_back(new PropertyDoubleLimited(this, L"opacity", &m_opacity, true, 0.0, true, 1.0));

	if(m_volume_found)
	{
		volume_for_properties = this->m_volume;
		volume_for_properties /= (pow(theApp->m_view_units, 3)); // convert volume to cubic units
		list->push_back(new PropertyDouble(this, L"volume", (const double*)(&volume_for_properties)));

		extract(m_centre_of_mass, centre_of_mass);

		list->push_back(PropertyVertex(this, L"centre of gravity", (const double*)centre_of_mass));
	}
	else
	{
		list->push_back(new CalculateVolumeProperty(this));
	}
#endif
	IdNamedObjList::GetProperties(list);
}
