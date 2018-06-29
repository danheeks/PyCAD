#pragma once

#include <list>
#include <map>
#include <set>
#include <vector>
#include "geometry.h"

class CMeshFace;
class CMeshEdge;
class CMesh;
class CMeshEdgeAndDir;
class CTris;
class CTri;

class CMeshVertex
{
	public:
	float m_x[3];
	std::list<CMeshEdgeAndDir> m_e;
	std::set<CMeshFace*> m_f;

	CMeshVertex(const float *x){ m_x[0] = x[0]; m_x[1] = x[1]; m_x[2] = x[2]; }
};

class CMeshEdge
{
public:
	CMeshVertex* m_v[2]; // start then end
	CMeshFace* m_f[2]; // left then right

	CMeshEdge(){ m_v[0] = NULL; m_v[1] = NULL; m_f[0] = NULL; m_f[1] = NULL; }
};

class CMeshFace
{
public:
	std::vector<CMeshVertex*> m_vertices;
	std::vector<CMeshEdgeAndDir> m_edges;
	bool GetNormal(geoff_geometry::Vector3d &norm)const;
	void GetJoiningFaces(std::list<CMeshFace*> &joining_faces)const;
	void GetTri(CTri& tri)const;
};

class CMesh
{
	std::map<float, std::list<CMeshVertex*> > m_vertices;
	CMeshVertex* AddGetVertex(const float* x);

public:	
	std::list<CMeshFace*> m_faces;
	std::list<CMeshEdge*> m_edges;

	CMesh(){}
	CMesh(const CTris& tris);

	void AddTri(const float* x);
	CMeshEdgeAndDir AddEdge(CMeshVertex* v0, CMeshVertex* v1);
};

class CMeshEdgeAndDir
{
public:
	CMeshEdge* m_edge;
	bool m_dir;

	CMeshEdgeAndDir() : m_edge(NULL), m_dir(false){}
	CMeshEdgeAndDir(CMeshEdge* edge, bool dir) :m_edge(edge), m_dir(dir){}
	bool GetNext(std::set<CMeshEdge*> &border_edges, CMeshEdgeAndDir& next)
	{
		CMeshVertex* end_vertex = End();
		for (std::list<CMeshEdgeAndDir>::iterator It = end_vertex->m_e.begin(); It != end_vertex->m_e.end(); It++)
		{
			CMeshEdgeAndDir& edge_and_dir = *It;
			if (edge_and_dir.m_edge != m_edge && border_edges.find(edge_and_dir.m_edge) != border_edges.end())
			{
				next = edge_and_dir;
				return true;
			}
		}

		return false;
	}

	CMeshVertex* Start()
	{
		if (m_dir)return m_edge->m_v[0];
		else return m_edge->m_v[1];
	}

	CMeshVertex* End()
	{
		if (m_dir)return m_edge->m_v[1];
		else return m_edge->m_v[0];
	}

	void AddFace(CMeshFace* face);
};
