#include "Mesh.h"
#include "Tris.h"

int face_id = 1;

CMesh::CMesh(const CTris& tris)
{
	face_id = 1;
	for (std::list<CTri>::const_iterator It = tris.m_tris.begin(); It != tris.m_tris.end(); It++)
	{
		const CTri& tri = *It;
		AddTri(tri.x[0]);
	}
}

CMeshVertex* CMesh::AddGetVertex(const float* x)
{
	CMeshVertex* v = NULL;
	std::map<float, std::list<CMeshVertex*> >::iterator FindIt = m_vertices.find(x[0]);
	if (FindIt == m_vertices.end())
	{
		std::list<CMeshVertex*> empty_list;
		FindIt = m_vertices.insert(std::make_pair(x[0], empty_list)).first;
		v = new CMeshVertex(x);
		FindIt->second.push_back(v);
	}
	else
	{
		std::list<CMeshVertex*>& list = FindIt->second;
		for (std::list<CMeshVertex*>::iterator It = list.begin(); It != list.end(); It++)
		{
			v = *It;
			if (v->m_x[1] == x[1] && v->m_x[2] == x[2])
				return v;
		}
		v = new CMeshVertex(x);
		list.push_back(v);
	}

	return v;
}

void CMesh::AddTri(const float* x)
{
	CMeshFace* face = new CMeshFace();
	face->m_id = face_id;
	face_id++;
	m_faces.push_back(face);

	CMeshVertex* vertices[3];
	for (int i = 0; i < 3; i++)
	{
		CMeshVertex* v = AddGetVertex(&x[3 * i]);
		face->m_vertices.push_back(v);
		vertices[i] = v;
		vertices[i]->m_f.insert(face);
	}

	CMeshEdgeAndDir edges[3];
	edges[0] = AddEdge(vertices[0], vertices[1]);
	edges[1] = AddEdge(vertices[1], vertices[2]);
	edges[2] = AddEdge(vertices[2], vertices[0]);

	for (int i = 0; i < 3; i++)
	{
		face->m_edges.push_back(edges[i]);
		edges[i].AddFace(face);
	}
}

void CMeshEdgeAndDir::AddFace(CMeshFace* face)
{
	if (m_dir)
		m_edge->m_f[0] = face;
	else
		m_edge->m_f[1] = face;
}

bool CMeshFace::GetNormal(Point3d &norm)const
{
	 if (m_vertices.size() < 3)
		 return false;

	Point3d p[3];
	for (int i = 0; i < 3; i++)
	{
		p[i] = Point3d(m_vertices[i]->m_x[0], m_vertices[i]->m_x[1], m_vertices[i]->m_x[2]);
	}

	norm = Point3d(p[0], p[1]) ^ Point3d(p[0], p[2]);
	norm.normalise();

	return true;
}

void CMeshFace::GetJoiningFaces(std::list<CMeshFace*> &joining_faces)const
{
	for (std::vector<CMeshEdgeAndDir>::const_iterator It = m_edges.begin(); It != m_edges.end(); It++)
	{
		const CMeshEdgeAndDir& e = *It;
		for (int i = 0; i < 2; i++)
		{
			if (e.m_edge->m_f[i] != this)joining_faces.push_back(e.m_edge->m_f[i]);
		}
	}
}

void CMeshFace::GetTri(CTri& tri)const
{
	for (unsigned int i = 0; i < m_vertices.size(); i++)
	{
		tri.x[i][0] = m_vertices[i]->m_x[0];
		tri.x[i][1] = m_vertices[i]->m_x[1];
		tri.x[i][2] = m_vertices[i]->m_x[2];
	}
}

CMeshEdgeAndDir CMesh::AddEdge(CMeshVertex* v0, CMeshVertex* v1)
{
	for (std::list<CMeshEdgeAndDir>::iterator It = v0->m_e.begin(); It != v0->m_e.end(); It++)
	{
		CMeshEdgeAndDir& edge_and_dir = *It;
		if (edge_and_dir.End() == v1)
			return edge_and_dir;
	}

	CMeshEdge* e = new CMeshEdge;
	m_edges.push_back(e);
	e->m_v[0] = v0;
	e->m_v[1] = v1;
	v0->m_e.push_back(CMeshEdgeAndDir(e, true));
	v1->m_e.push_back(CMeshEdgeAndDir(e, false));
	return CMeshEdgeAndDir(e, true);
}
