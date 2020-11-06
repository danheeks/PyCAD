// Solid.cpp

#include "Tris.h"
#include "geometry.h"
#include "Curve.h"
#include "dxf.h"
#include "OctTree.h"
#include "Box.h"
#include "Mesh.h"
#include "Area.h"

#include <fstream>

double LineOrPoint::MinX()const
{
	if (m_is_a_line)
	{
		if (m_line.v.x > 0)
			return m_line.p0.x;
		else
			return m_line.p0.x + m_line.v.x;
	}
	return m_p.x;
}

Point3d LineOrPoint::MostLeftPoint()const
{
	if (m_is_a_line)
	{
		if (m_line.v.x > 0)
			return m_line.p0;
		else
			return m_line.p0 + m_line.v;
	}
	return m_p;
}

void CTri::Transform(const Matrix &mat)
{
	for (int i = 0; i < 3; i++)
	{
		Point3d p(x[i][0], x[i][1], x[i][2]);
		p = p.Transformed(mat);
		x[i][0] = (float)p.x;
		x[i][1] = (float)p.y;
		x[i][2] = (float)p.z;
	}
}

void CTri::SetBox()
{
	m_box.m_valid = false;
	for (int i = 0; i < 3; i++)
		m_box.Insert(x[i][0], x[i][1], x[i][2]);
}

static void MakeTriSpan(const float* p0, const float* p1, const float* p2, Span &span)
{
	// makes a span, when we already know that p0 is below the plane and p1 and p2 are above the plane
	double fraction1 = p0[2] / (p0[2] - p1[2]);
	double fraction2 = p0[2] / (p0[2] - p2[2]);

	Point P0(p0[0], p0[1]);
	Point v1(Point(p1[0], p1[1]) - P0);
	Point v2(Point(p2[0], p2[1]) - P0);

	span.m_p = P0 + v1 * fraction1;
	span.m_v = CVertex(P0 + v2 * fraction2);
}


bool CTri::IntersectsXY(Span &span)
{
	if (x[0][2] > 0.000000001)
	{
		if (x[1][2] > 0.000000001)
		{
			if (x[2][2] < -0.000000001)
			{
				MakeTriSpan(x[2], x[0], x[1], span);
				return true;
			}
			else
			{
				return false;
			}
		}
		else if (x[1][2] < -0.000000001)
		{
			if (x[2][2] < -0.000000001)
			{
				MakeTriSpan(x[0], x[1], x[2], span);
				return true;
			}
			else
			{
				MakeTriSpan(x[1], x[0], x[2], span);
				return true;
			}
		}
		else
		{
			if (x[2][2] > 0.000000001)
			{
				return false;
			}
			else if (x[2][2] < -0.000000001)
			{
				MakeTriSpan(x[2], x[0], x[1], span);
				return true;
			}
			else
			{
				span.m_p = Point(x[1][0], x[1][1]);
				span.m_v = CVertex(Point(x[2][0], x[2][1]));
				return true;
			}
		}
	}
	else if (x[0][2] < -0.000000001)
	{
		if (x[1][2] > 0.000000001)
		{
			if (x[2][2] < -0.000000001)
			{
				MakeTriSpan(x[1], x[0], x[2], span);
				return true;
			}
			else
			{
				MakeTriSpan(x[0], x[1], x[2], span);
				return true;
			}
		}
		else if (x[1][2] < -0.000000001)
		{
			if (x[2][2] > 0.000000001)
			{
				MakeTriSpan(x[2], x[0], x[1], span);
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			if (x[2][2] > 0.000000001)
			{
				MakeTriSpan(x[2], x[0], x[1], span);
				return true;
			}
			else if (x[2][2] < -0.000000001)
			{
				return false;
			}
			else
			{
				span.m_p = Point(x[1][0], x[1][1]);
				span.m_v = CVertex(Point(x[2][0], x[2][1]));
				return true;
			}
		}
	}
	else
	{
		if (x[1][2] > 0.000000001)
		{
			if (x[2][2] > 0.000000001)
			{
				return false;
			}
			else if (x[2][2] < -0.000000001)
			{
				MakeTriSpan(x[2], x[0], x[1], span);
				return true;
			}
			else
			{
				span.m_p = Point(x[0][0], x[0][1]);
				span.m_v = CVertex(Point(x[2][0], x[2][1]));
				return true;
			}
		}
		else if (x[1][2] < -0.000000001)
		{
			if (x[2][2] > 0.000000001)
			{
				MakeTriSpan(x[1], x[0], x[2], span);
				return true;
			}
			else if (x[2][2] < -0.000000001)
			{
				return false;
			}
			else
			{
				span.m_p = Point(x[0][0], x[0][1]);
				span.m_v = CVertex(Point(x[2][0], x[2][1]));
				return true;
			}
		}
		else
		{
			if (x[2][2] > 0.000000001)
			{
				span.m_p = Point(x[0][0], x[0][1]);
				span.m_v = CVertex(Point(x[1][0], x[1][1]));
				return true;
			}
			else if (x[2][2] < -0.000000001)
			{
				span.m_p = Point(x[0][0], x[0][1]);
				span.m_v = CVertex(Point(x[1][0], x[1][1]));
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}

Triangle3d CTri::GeoffTri()const
{
	return Triangle3d(Point3d(x[0][0], x[0][1], x[0][2]), Point3d(x[1][0], x[1][1], x[1][2]), Point3d(x[2][0], x[2][1], x[2][2]));
}

bool CTri::On(const Point3d &p)const
{
	Triangle3d t = GeoffTri();
	Plane pl(t.getVert1(), t.getVert2(), t.getVert3());
	Point3d near = pl.Near(p);
	if (near.Dist(p) < 0.001)
	{
		if (t.Inside(p))
			return true;
	}
	return false;
}

bool ClosestIntof(const Line& l, const Point3d& v0, const Point3d& v1, Point3d& intof)
{
	if (l.Intof(v0) && l.Intof(v1))
	{
		double d0 = l.v * Point3d(v0);
		double d1 = l.v * Point3d(v1);
		if (d0 < d1)intof = v0;
		else intof = v1;
		return true;
	}
	return false;
}

static void AddPointToEdgeIntofs(const Point3d& p, std::list<Point3d> &edge_intofs)
{
	if (edge_intofs.size() < 2)
		edge_intofs.push_back(p);
	else
	{
		double d01 = edge_intofs.front().Dist(edge_intofs.back());
		double d02 = edge_intofs.front().Dist(p);
		double d12 = edge_intofs.back().Dist(p);

		int best_pair = 0;

		if (d01 > d02)
		{
			if (d12 > d01)best_pair = 2;
			else best_pair = 0;
		}
		else
		{
			if (d12 > d02)best_pair = 2;
			else best_pair = 1;
		}

		switch (best_pair)
		{
		case 0:
			break;
		case 1:
			edge_intofs.back() = p;
			break;
		case 2:
			edge_intofs.front() = p;
			break;
		}
	}
}

bool CTri::Intof(const Line& l, LineOrPoint& intof)const
{
	Triangle3d t = GeoffTri();
	Point3d p;

	//Plane pl(t.getVert1(), t.getVert2(), t.getVert3());
	//if (!(pl.On(l.p0) && pl.On(l.p0 + l.v)))
	//{
	//	return false;
	//}

	std::list<Point3d> edge_intofs;
	Point3d other_p;

	Line edge1(t.getVert1(), t.getVert2());

	int edge1_num_ints = edge1.IntofFinite(l, p, other_p);
	if (edge1_num_ints == 2)
	{
		intof = LineOrPoint(Line(p, other_p));
		return true;
	}
	else if ((edge1_num_ints == 1) && (p != t.getVert1()))
		AddPointToEdgeIntofs(p, edge_intofs);

	Line edge2(t.getVert2(), t.getVert3());

	int edge2_num_ints = edge2.IntofFinite(l, p, other_p);
	if (edge2_num_ints == 2)
	{
		intof = LineOrPoint(Line(p, other_p));
		return true;
	}
	else if ((edge2_num_ints == 1) && (p != t.getVert2()))
		AddPointToEdgeIntofs(p, edge_intofs);

	Line edge3(t.getVert3(), t.getVert1());

	int edge3_num_ints = edge3.IntofFinite(l, p, other_p);
	if (edge3_num_ints == 2)
	{
		intof = LineOrPoint(Line(p, other_p));
		return true;
	}
	else if ((edge3_num_ints == 1) && (p != t.getVert3()))
		AddPointToEdgeIntofs(p, edge_intofs);

	if (edge_intofs.size() == 0)
	{
		bool result = t.Intof(l, p);
		if (result)
		{
			intof = LineOrPoint(p);
			return true;
		}

		if (t.Inside(l.p0))edge_intofs.push_back(l.p0);
		if (t.Inside(l.p0 + l.v))edge_intofs.push_back(l.p0 + l.v);
	}

	if (edge_intofs.size() == 1)
	{
		if (edge_intofs.front() != l.p0 && this->On(l.p0))edge_intofs.push_back(l.p0);
		else if (edge_intofs.front() != (l.p0 + l.v) && this->On(l.p0 + l.v))edge_intofs.push_back(l.p0 + l.v);
	}

	if (edge_intofs.size() == 1)
	{
		intof = LineOrPoint(Point3d(edge_intofs.front()));
		return true;
	}
	else if (edge_intofs.size() == 2)
	{
		intof = LineOrPoint(Line(edge_intofs.front(), edge_intofs.back()));
		return true;
	}

	return false;
}

bool CTri::SplitAtZ(double z, CTris &new_tris_above, CTris &new_tris_below)const
{
	// returns false if triangle was left alone
	// returns true if the triangle was split, or it was above z, in which case the triangle should be deleted.
	bool above[3] = { false, false, false };
	int num_above = 0;
	for (unsigned int i = 0; i < 3; i++)
	{
		if (x[i][2] > z + TOLERANCE)
		{
			above[i] = true;
			num_above++;
		}
	}

	if (num_above == 0)
		return false;

	if (num_above == 3)
	{
		new_tris_above.AddTri(this->x[0]);
	}
	else
	{
		CTris *p_new_tris_above = &new_tris_above;
		CTris *p_new_tris_below = &new_tris_below;

		// written for (num_above == 1) use the same code swapped for (num_above == 2)
		if (num_above == 2)
		{
			for (unsigned int i = 0; i < 3; i++)
				above[i] = !above[i];

			p_new_tris_above = &new_tris_below;
			p_new_tris_below = &new_tris_above;
		}

		unsigned int above_index, below_index1, below_index2;
		if (above[1]){ above_index = 1; below_index1 = 2; below_index2 = 0; }
		else if (above[2]){ above_index = 2; below_index1 = 0; below_index2 = 1; }
		else { above_index = 0; below_index1 = 1; below_index2 = 2; }

		Point3d p0(x[above_index][0], x[above_index][1], x[above_index][2]);
		Point3d p1(x[below_index1][0], x[below_index1][1], x[below_index1][2]);
		Point3d p2(x[below_index2][0], x[below_index2][1], x[below_index2][2]);
		double fraction1 = (z - p0.z) / (p1.z - p0.z);
		Point3d m1 = p0 + (p1 - p0) * fraction1;
		double fraction2 = (z - p0.z) / (p2.z - p0.z);
		Point3d m2 = p0 + (p2 - p0) * fraction2;

		float f0[9] = { (float)(p0.x), (float)(p0.y), (float)(p0.z), (float)(m1.x), (float)(m1.y), (float)z, (float)(m2.x), (float)(m2.y), (float)z };
		p_new_tris_above->AddTri(f0);

		float f1[9] = { (float)(m1.x), (float)(m1.y), (float)z, (float)(p1.x), (float)(p1.y), (float)(p1.z), (float)(m2.x), (float)(m2.y), (float)z };
		p_new_tris_below->AddTri(f1);

		float f2[9] = { (float)(p1.x), (float)(p1.y), (float)(p1.z), (float)(p2.x), (float)(p2.y), (float)(p2.z), (float)(m2.x), (float)(m2.y), (float)z };
		p_new_tris_below->AddTri(f2);
	}

	return true;
}


void CTris::read_from_file(const std::wstring& stl_file_path)
{
	// read the stl file
	char fp[1024];
	wcstombs(fp, stl_file_path.c_str(), 1024);
	std::ifstream ifs(fp, std::ios::binary);
	if (!ifs)return;

	char solid_string[6] = "aaaaa";
	ifs.read(solid_string, 5);
	if (ifs.eof())return;
	if (strcmp(solid_string, "solid"))
	{
		// try binary file read

		// read the header
		char header[81];
		header[80] = 0;
		memcpy(header, solid_string, 5);
		ifs.read(&header[5], 75);

		unsigned int num_facets = 0;
		ifs.read((char*)(&num_facets), 4);

		for (unsigned int i = 0; i<num_facets; i++)
		{
			float n[3];
			ifs.read((char*)(n), 12);
			float x[3][3];
			ifs.read((char*)(x[0]), 36);
			short attr;
			ifs.read((char*)(&attr), 2);
			AddTri(x[0]);
		}
	}
	else
	{
		// "solid" already found
		char str[1024] = "solid";
		ifs.getline(&str[5], 1024);
		char title[1024];
		if (sscanf(str, "solid %s", title) == 1)
		{
			wchar_t wtitle[1024];
			mbstowcs(wtitle, title, 1024);
			m_title.assign(wtitle);
		}

		float x[3][3];
		char five_chars[6] = "aaaaa";

		int vertex = 0;

		while (!ifs.eof())
		{
			ifs.getline(str, 1024);

			int i = 0, j = 0;
			for (; i<5; i++, j++)
			{
				if (str[j] == 0)break;
				while (str[j] == ' ' || str[j] == '\t')j++;
				five_chars[i] = str[j];
			}
			if (i == 5)
			{
				if (!strcmp(five_chars, "verte"))
				{
					sscanf(str, " vertex %f %f %f", &(x[vertex][0]), &(x[vertex][1]), &(x[vertex][2]));
					vertex++;
					if (vertex > 2)vertex = 2;
				}
				else if (!strcmp(five_chars, "facet"))
				{
					vertex = 0;
				}
				else if (!strcmp(five_chars, "endfa"))
				{
					if (vertex == 2)
					{
						AddTri(x[0]);
					}
				}
			}
		}
	}
}

void CTris::MakeSection(const Point& s, const Point& e, const std::string& dxf_file_path)
{
	if (s == e)
		return;

	Point3d origin(s.x, s.y, 0.0);
	Point3d end(e.x, e.y, 0.0);

	Point3d x_axis(origin, end);
	Point3d y_axis(0.0, 0.0, 1.0);

	Point3d plane_normal = x_axis ^ Point3d(0.0, 0.0, 1.0);

	Plane(origin, plane_normal);

	Matrix tm(origin, x_axis, y_axis);
	Matrix inv_tm = tm;
	inv_tm = inv_tm.Inverse(); // this will transform things from our plane to the world coords

	// make a list of triangles transformed with our plane now being the flat z = 0 plane
	Span span;
	std::list<Span> spans;

	for (std::list<CTri >::iterator It = m_tris.begin(); It != m_tris.end(); It++)
	{
		CTri t = *It; // copy of the triangle
		t.Transform(inv_tm);

		if (t.IntersectsXY(span))
			spans.push_back(span);
	}

	// write a dxf file
	{
		CDxfWrite dxf_writer(dxf_file_path.c_str());
		for (std::list<Span>::iterator It = spans.begin(); It != spans.end(); It++)
		{
			Span& span = *It;
			Point3d s(span.m_p.x, span.m_p.y, 0.0);
			Point3d e(span.m_v.m_p.x, span.m_v.m_p.y, 0.0);
			//s = s.Transform(tm);
			//e = e.Transform(tm);
			dxf_writer.WriteLine(s.getBuffer(), e.getBuffer(), "0");
		}
	}
}

//static
CTris* CTris::DoBoolean(const CTris& solid1, const CTris& solid2, SOLID_BOOLEAN_TYPE boolean_type)
{

	COctTree octtree1(solid1.m_box);
	COctTree octtree2(solid2.m_box);

	for (std::list<CTri>::const_iterator It = solid1.m_tris.begin(); It != solid1.m_tris.end(); It++)
	{
		const CTri& tri = *It;
		octtree1.AddTri(&tri);
	}

	for (std::list<CTri>::const_iterator It = solid2.m_tris.begin(); It != solid2.m_tris.end(); It++)
	{
		const CTri& tri = *It;
		octtree2.AddTri(&tri);
	}

	CTris* new_solid = new CTris();

	CTris copy1 = solid1;
	CTris copy2 = solid2;

	octtree2.SplitTriangles(copy1.m_tris);
	octtree1.SplitTriangles(copy2.m_tris);

	//*new_solid = copy2;
	//return new_solid;

	switch (boolean_type)
	{
	case SOLID_BOOLEAN_UNION:
		octtree2.MakeNewListOfTriangles(copy1.m_tris, new_solid->m_tris, TRI_OUTSIDE | TRI_ON);
		octtree1.MakeNewListOfTriangles(copy2.m_tris, new_solid->m_tris, TRI_OUTSIDE);
		break;
	case SOLID_BOOLEAN_CUT:
		octtree2.MakeNewListOfTriangles(copy1.m_tris, new_solid->m_tris, TRI_OUTSIDE);
		octtree1.MakeNewListOfTriangles(copy2.m_tris, new_solid->m_tris, TRI_INSIDE);
		break;
	case SOLID_BOOLEAN_COMMON:
		octtree2.MakeNewListOfTriangles(copy1.m_tris, new_solid->m_tris, TRI_INSIDE | TRI_ON);
		octtree1.MakeNewListOfTriangles(copy2.m_tris, new_solid->m_tris, TRI_INSIDE);
		break;
	case SOLID_BOOLEAN_XOR:
		octtree2.MakeNewListOfTriangles(copy1.m_tris, new_solid->m_tris, TRI_OUTSIDE);
		octtree1.MakeNewListOfTriangles(copy2.m_tris, new_solid->m_tris, TRI_OUTSIDE);
		break;
	}

	return new_solid;
}

CTris* CTris::BooleanCut(const CTris& solid)const
{
	CTris* new_solid = DoBoolean(*this, solid, SOLID_BOOLEAN_CUT);
	return new_solid;
}

CTris* CTris::BooleanUnion(const CTris& solid)const
{
	CTris* new_solid = DoBoolean(*this, solid, SOLID_BOOLEAN_UNION);
	return new_solid;
}

CTris* CTris::BooleanCommon(const CTris& solid)const
{
	CTris* new_solid = DoBoolean(*this, solid, SOLID_BOOLEAN_COMMON);
	return new_solid;
}

void CTris::AddTri(const float* x)
{
	m_tris.push_back(CTri());
	memcpy(m_tris.back().x[0], x, 9 * sizeof(float));
	m_tris.back().SetBox();
	m_box.Insert(m_tris.back().m_box);
}

void CTris::WriteStl(const std::string& stl_file_path)
{
	std::ofstream ofs(stl_file_path.c_str());
	if (!ofs)
	{
		return;
	}
	ofs.imbue(std::locale("C"));

	ofs << "solid" << endl;

	// write all the objects

	for (std::list<CTri>::const_iterator It = m_tris.begin(); It != m_tris.end(); It++)
	{
		const CTri& tri = *It;
		const float* x = tri.x[0];

		ofs << " facet normal " << 0.0 << " " << 0.0 << " " << 0.0 << endl;
		ofs << "   outer loop" << endl;

		ofs << "     vertex " << x[0] << " " << x[1] << " " << x[2] << endl;
		ofs << "     vertex " << x[3] << " " << x[4] << " " << x[5] << endl;
		ofs << "     vertex " << x[6] << " " << x[7] << " " << x[8] << endl;
		ofs << "   endloop" << endl;
		ofs << " endfacet" << endl;


	}

	ofs << "endsolid" << endl;
}

void CTris::SplitTriangles(const CTris& solid)
{
	COctTree octtree(solid.m_box);

	for (std::list<CTri>::const_iterator It = solid.m_tris.begin(); It != solid.m_tris.end(); It++)
	{
		const CTri& tri = *It;
		octtree.AddTri(&tri);
	}

	octtree.SplitTriangles(m_tris);
}

void GetEdgeLoop(std::set<CMeshEdge*> &border_edges, std::list<CMeshEdgeAndDir> &edge_loop)
{
	CMeshEdge* edge = *(border_edges.begin());
	CMeshEdgeAndDir edge_and_dir(edge, true);

	while (border_edges.size() > 0)
	{
		edge_loop.push_back(edge_and_dir);
		border_edges.erase(edge_and_dir.m_edge);
		if (!edge_and_dir.GetNext(border_edges, edge_and_dir))
			break;
	}
}

void EdgeLoopToCurve(std::list<CMeshEdgeAndDir> &edge_loop, CCurve &curve)
{
	for (std::list<CMeshEdgeAndDir>::iterator It = edge_loop.begin(); It != edge_loop.end(); It++)
	{
		CMeshEdgeAndDir& e = *It;
		if (It == edge_loop.begin())
		{
			CMeshVertex* vs = e.Start();
			curve.m_vertices.push_back(Point(vs->m_x[0], vs->m_x[1]));
		}
		CMeshVertex* ve = e.End();
		curve.m_vertices.push_back(Point(ve->m_x[0], ve->m_x[1]));
	}
}

CArea CTris::Shadow()const
{
	Point3d axis(0, 0, 1);

	CMesh mesh(*this);

	// list all the edges that are cross over edges
	std::set<CMeshEdge*> border_edges;

	for (std::list<CMeshEdge*>::iterator It = mesh.m_edges.begin(); It != mesh.m_edges.end(); It++)
	{
		CMeshEdge* edge = *It;

		if (edge->m_f[0] == NULL || edge->m_f[1] == NULL)
			border_edges.insert(edge);
		else
		{
			Point3d n0, n1;
			if (!edge->m_f[0]->GetNormal(n0) || !edge->m_f[1]->GetNormal(n1))
				border_edges.insert(edge);
			else
			{
				double dp0 = n0 * axis;
				double dp1 = n1 * axis;
				bool sign0 = dp0 > 0;
				bool sign1 = dp1 > 0;
				if (sign0 != sign1)
					border_edges.insert(edge);
			}
		}
	}

	CArea area;

	std::list<Line> lines_for_dxf;

	while (border_edges.size() > 0)
	{
		std::list<CMeshEdgeAndDir> edge_loop;
		GetEdgeLoop(border_edges, edge_loop);
		CCurve curve;
		EdgeLoopToCurve(edge_loop, curve);
		area.m_curves.push_back(curve);
	}

	return area;
}

CArea CTris::Shadow2(bool just_up_allowed)const
{
	Matrix tm;
	return Shadow2Mat(tm);
}

CArea CTris::Shadow2Mat(const Matrix &tm, bool just_up_allowed)const
{
	Matrix inv_tm = tm.Inverse();

	Point3d axis(0, 0, 1);

	CArea::m_fit_arcs = false;

	std::list<CCurve> curves;

	for (std::list<CTri>::const_iterator It = m_tris.begin(); It != m_tris.end(); It++)
	{
		CTri t = (*It);
		t.Transform(inv_tm);
		CTri* tri = &t;
		Point3d norm = Point3d(Point3d(tri->x[0][0], tri->x[0][1], tri->x[0][2]), Point3d(tri->x[1][0], tri->x[1][1], tri->x[1][2])) ^ Point3d(Point3d(tri->x[0][0], tri->x[0][1], tri->x[0][2]), Point3d(tri->x[2][0], tri->x[2][1], tri->x[2][2]));
		if ((just_up_allowed == false) || (norm * axis > 0.0))
		{
			CArea tri_area;
			CCurve curve;
			Point p0(tri->x[0][0], tri->x[0][1]);
			Point p1(tri->x[1][0], tri->x[1][1]);
			Point p2(tri->x[2][0], tri->x[2][1]);
			curve.m_vertices.push_back(p0);
			curve.m_vertices.push_back(p1);
			curve.m_vertices.push_back(p2);
			curve.m_vertices.push_back(p0);
			if (curve.GetArea() < 0)
			{
				curves.push_back(curve);
			}
		}
	}

	CArea area = CArea::UniteCurves(curves);
	for (std::list<CCurve>::iterator It = area.m_curves.begin(); It != area.m_curves.end(); It++)
	{
		CCurve& curve = *It;
		curve.Reverse();
	}

	return area;
}

void CTris::ProjectSpan(const Span& span, std::list<Line>& lines)const
{
	Point s = span.m_p;
	Point e = span.m_v.m_p;

	Point3d origin(s.x, s.y, 0.0);
	Point3d end(e.x, e.y, 0.0);

	Point3d x_axis(origin, end);
	Point3d y_axis(0.0, 0.0, 1.0);

	Point3d plane_normal = x_axis ^ Point3d(0.0, 0.0, 1.0);

	Plane(origin, plane_normal);

	Matrix tm(origin, x_axis, y_axis);
	Matrix inv_tm = tm;
	inv_tm = inv_tm.Inverse(); // this will transform things from our plane to the world coords

	// make a list of triangles transformed with our plane now being the flat z = 0 plane
	Span rspan;
	std::list<Span> spans;
	CBox2D box2;
	span.GetBox(box2);
	CBox span_box(box2.MinX(), box2.MinY(), m_box.MinZ(), box2.MaxX(), box2.MaxY(), m_box.MaxZ());

	rspan = Span(Point(0, 0), Point(span.Length(), 0));
	spans.push_back(rspan);

	for (std::list<CTri >::const_iterator It = m_tris.begin(); It != m_tris.end(); It++)
	{

		CTri t = *It; // copy of the triangle
		t.SetBox();
		if (t.m_box.Intersects(span_box))
		{
			t.Transform(inv_tm);
			if (t.IntersectsXY(rspan))
				spans.push_back(rspan);
		}
	}

	ClipSpans(spans, span.Length());

	// add the spans transformed back to the input span's plane
	for (std::list<Span>::iterator It = spans.begin(); It != spans.end(); It++)
	{
		Span& span = *It;
		Point3d s(span.m_p.x, span.m_p.y, 0.0);
		Point3d e(span.m_v.m_p.x, span.m_v.m_p.y, 0.0);
		s = s.Transformed(tm);
		e = e.Transformed(tm);
		lines.push_back(Line(s, e));
	}
}

void CTris::Project(const CArea& a, std::list<Line>& lines)const
{
	for (std::list<CCurve>::const_iterator It = a.m_curves.begin(); It != a.m_curves.end(); It++)
	{
		CCurve curve = *It;
		curve.UnFitArcs();
		std::list<Span> spans;
		curve.GetSpans(spans);
		for (std::list<Span>::iterator It2 = spans.begin(); It2 != spans.end(); It2++)
		{
			// make a plane for the span
			Span& span = *It2;
			ProjectSpan(span, lines);
		}
	}
}

class SpanClipper
{
	std::map<double, Span> m_map;
	double m_length;

	Point PointOnSpan(const Span& span, double x)
	{
		double width = (span.m_v.m_p.x - span.m_p.x);
		if (fabs(width) < 0.0000001)
		{
			double y = span.m_v.m_p.y;
			if (span.m_p.y > y)y = span.m_p.y;
			return Point(x, y);
		}
		double rise = (span.m_v.m_p.y - span.m_p.y);
		double gradient = rise / width;

		double y = span.m_p.y + (x - span.m_p.x) * gradient;

		return Point(x, y);
	}

public:
	SpanClipper(double length){ m_length = length; }

	void Insert2(Span span)
	{
		m_map.erase(span.m_p.x);
		m_map.insert(std::make_pair(span.m_p.x, span));
	}

	void MergeSpan(const Span &span)
	{
		// span won't cross the ends of existing spans, but may be shorter

		if (m_map.size() == 0)
		{
			m_map.erase(span.m_p.x);
			m_map.insert(std::make_pair(span.m_p.x, span));
		}
		else
		{
			std::map<double, Span>::iterator FindIt = m_map.upper_bound(span.m_p.x);
			FindIt--;
			Span existing_span = FindIt->second;
			Span* current_span = &FindIt->second;

			if (span.m_p.x < existing_span.m_p.x)
			{
				m_map.erase(span.m_p.x);
				m_map.insert(std::make_pair(span.m_p.x, span));
				return;
			}

			if ((span.m_p.x - existing_span.m_v.m_p.x) > -0.0000001)
			{
				// start of span is on or after the end of existing span
				m_map.erase(span.m_p.x);
				m_map.insert(std::make_pair(span.m_p.x, span));
				return;
			}

			bool start_above = false;

				Point existing_span_point = PointOnSpan(existing_span, span.m_p.x);
				if (span.m_p.y > existing_span_point.y)
				{
					start_above = true;
					if (fabs(span.m_p.x - existing_span.m_p.x) > 0.0000001)
					{
						// start not on start, insert new span
							current_span->m_v.m_p = existing_span_point;
							m_map.erase(span.m_p.x);
							current_span = &(m_map.insert(std::make_pair(span.m_p.x, span)).first->second);
					}
					else
					{
						// start on start
						if (fabs(span.m_v.m_p.x - existing_span.m_p.x) < 0.0000001)
						{
							// end on start too, so add a point ( no, throw away )
							return;
						}

						// end not on start, so remove existing span
						m_map.erase(FindIt);

						// add span
						m_map.erase(span.m_p.x);
						current_span = &(m_map.insert(std::make_pair(span.m_p.x, span)).first->second);
					}
				}

			if ((existing_span.m_v.m_p.x - span.m_v.m_p.x) > 0.0000001)
			{
				// end not on end, insert original span
				Point existing_span_point = PointOnSpan(existing_span, span.m_v.m_p.x);
				bool end_above = span.m_v.m_p.y > existing_span_point.y;
				if (end_above != start_above)
				{
					// span crosses, so find intersection
					std::list<Point> pts;
					span.Intersect(existing_span, pts);
					if (pts.size() > 0)
					{
						Point ip = pts.front();
						if (start_above)
						{
							// end below, so cut short the current span
							current_span->m_v.m_p = ip;

							// add what's left of the existing span
							Span new_span = existing_span;
							new_span.m_p = ip;
							m_map.erase(new_span.m_p.x);
							current_span = &(m_map.insert(std::make_pair(new_span.m_p.x, new_span)).first->second);
						}
						else
						{
							// end above, so add the span with start changed
							current_span->m_v.m_p = ip;
							Span new_span = span;
							new_span.m_p = ip;
							m_map.erase(new_span.m_p.x);
							current_span = &(m_map.insert(std::make_pair(new_span.m_p.x, new_span)).first->second);

							// add what's lefts of the existing span
							new_span = existing_span;
							new_span.m_p = PointOnSpan(existing_span, current_span->m_v.m_p.x);
							m_map.erase(new_span.m_p.x);
							current_span = &(m_map.insert(std::make_pair(new_span.m_p.x, new_span)).first->second);
						}
					}
				}
				else
				{
					// start and end on same side
					if (end_above)
					{
						// add what's lefts of the existing span
						Span new_span = existing_span;
						new_span.m_p = PointOnSpan(existing_span, current_span->m_v.m_p.x);
						m_map.erase(new_span.m_p.x);
						current_span = &(m_map.insert(std::make_pair(new_span.m_p.x, new_span)).first->second);
					}
				}
			}
		}
	}

	void SplitSpan(const Span& span, std::list<Span> &split_spans)
	{
		std::map<double, Span>::iterator StartIt = m_map.lower_bound(span.m_p.x);

		if (StartIt == m_map.end())
		{
			split_spans.push_back(span);
			return;
		}

		std::map<double, Span>::iterator FinishIt = m_map.upper_bound(span.m_v.m_p.x);
		std::map<double, Span>::iterator LastIt = FinishIt;
		LastIt--;

		Point cur_pt = span.m_p;

		for (std::map<double, Span>::iterator It = StartIt; It != FinishIt; It++)
		{
			// existing span will be either exactly starting on this, or starting after it
			double x = It->first;
			Span& existing_span = It->second;

			if (existing_span.m_p.x > span.m_p.x && existing_span.m_p.x < span.m_v.m_p.x)
			{
				Point pt = PointOnSpan(span, existing_span.m_p.x);
				split_spans.push_back(Span(cur_pt, pt));
				cur_pt = pt;
			}

			if (existing_span.m_v.m_p.x > span.m_p.x && existing_span.m_v.m_p.x < span.m_v.m_p.x)
			{
				Point pt = PointOnSpan(span, existing_span.m_v.m_p.x);
				split_spans.push_back(Span(cur_pt, pt));
				cur_pt = pt;
			}
		}

		split_spans.push_back(Span(cur_pt, span.m_v));
	}

	void Insert(Span span)
	{
		if (span.Length() < 0.0000001)
			return;

		if (span.m_v.m_p.x < span.m_p.x)
			span.Reverse();

		if (span.m_v.m_p.x < 0.0)
			return;

		if (span.m_p.x > m_length)
			return;

		if (span.m_p.x < 0.0)
			span.m_p = PointOnSpan(span, 0.0);

		if (span.m_v.m_p.x > m_length)
			span.m_v.m_p = PointOnSpan(span, m_length);

		std::list<Span> split_spans;
		SplitSpan(span, split_spans);

		for (std::list<Span>::iterator It = split_spans.begin(); It != split_spans.end(); It++)
		{
			Span& s = *It;
			if (s.Length() > 0.00000000000001)
			{
				if ((fabs(s.m_p.x - 10.0) < 0.001) && (fabs(s.m_v.m_p.x - 15.0) < 0.001))
				{
					MergeSpan(s);
				}
				else
				{
					MergeSpan(s);
				}
			}
		}
	}

	void GetSpans(std::list<Span>& spans)
	{
		for (std::map<double, Span>::iterator It = m_map.begin(); It != m_map.end(); It++)
		{
			if (It->second.Length() > 0.0000001)
				spans.push_back(It->second);
		}
	}
};


void CTris::ClipSpans(std::list<Span>& spans, double length)const
{
	// clip the spans to the range 0>x>length
	// just keep the top spans

	// the spans must only be lines

	// make an ordered set of Spans
	SpanClipper span_clipper(length);

	for (std::list<Span>::iterator It = spans.begin(); It != spans.end(); It++)
	{
		Span& span = *It;
		span_clipper.Insert(span);
	}

	spans.clear();
	span_clipper.GetSpans(spans);
}

void CTris::Transform(const Matrix& tm)
{
	for (std::list<CTri>::iterator It = m_tris.begin(); It != m_tris.end(); It++)
	{
		CTri* tri = &(*It);
		tri->Transform(tm);
	}
}

void CTris::GetBox(CBox& box)const
{
	box.Insert(this->m_box);
}

const CTris& CTris::operator+=(const CTris &t)
{
	for (std::list<CTri>::const_iterator It = t.m_tris.begin(); It != t.m_tris.end(); It++)
	{
		const CTri& tri = *It;
		m_tris.push_back(tri);
	}

	m_box.Insert(t.m_box);

	return *this;
}

FaceFlatType GetFaceFlatType(CMeshFace* face)
{
	Point3d norm;
	if(!face->GetNormal(norm))
		return FaceFlatTypeDown;

	if (norm.z > 0.999999)
		return FaceFlatTypeFlat;
	if (norm.z > 0.000001)
		return  FaceFlatTypeUpButNotFlat;
	return FaceFlatTypeDown;
}

class CFaceGroup
{
public:
	FaceFlatType m_face_type;
	CTris m_tris;
};

void WalkFacesMakingAFaceGroup(CMeshFace* face, std::list<CFaceGroup> &face_groups, std::set<CMeshFace*> &unvisited_faces)
{
	// make a face group containing face and all joining faces which have the same flat face type
	// face is assumed to be in unvisitied_faces

	std::list<CMeshFace*> to_add;
	to_add.push_back(face);
	CFaceGroup face_group;
	face_group.m_face_type = GetFaceFlatType(face);

	while (to_add.size() > 0)
	{
		CMeshFace* face = to_add.front();
		unvisited_faces.erase(face);
		CTri tri;
		face->GetTri(tri);
		face_group.m_tris.AddTri(tri.x[0]);
		to_add.pop_front();
		std::list<CMeshFace*> joining_faces;
		face->GetJoiningFaces(joining_faces);
		for (std::list<CMeshFace*>::iterator It = joining_faces.begin(); It != joining_faces.end(); It++)
		{
			CMeshFace* joining_face = *It;
			if (unvisited_faces.find(joining_face) != unvisited_faces.end())
			{
				FaceFlatType face_type = GetFaceFlatType(joining_face);
				if (face_type == face_group.m_face_type)
				{
					to_add.push_back(joining_face);
				}
			}
		}
	}

	face_groups.push_back(face_group);
}

void CTris::GetMachiningAreas(std::list<CMachiningArea>& areas)const
{
	CMesh mesh(*this);

	std::set<CMeshFace*> unvisited_faces;
	for (std::list<CMeshFace*>::iterator It = mesh.m_faces.begin(); It != mesh.m_faces.end(); It++)
	{
		unvisited_faces.insert(*It);
	}

	std::list<CFaceGroup> face_groups;

	while (unvisited_faces.size() > 0)
	{
		CMeshFace* face = *(unvisited_faces.begin());
		WalkFacesMakingAFaceGroup(face, face_groups, unvisited_faces);
	}

	// make a list of flat planes to split non flat areas
	std::set<double> plane_heights;
	for (std::list<CFaceGroup>::iterator It = face_groups.begin(); It != face_groups.end(); It++)
	{
		CFaceGroup& face_group = *It;
		if (face_group.m_face_type == FaceFlatTypeFlat)
		{
			// just insert the z-value of the first point of the first triangle
			plane_heights.insert(face_group.m_tris.m_tris.front().x[0][2]);
		}
	}

	std::list<CFaceGroup> new_face_groups;
	for (std::list<CFaceGroup>::iterator It = face_groups.begin(); It != face_groups.end(); It++)
	{
		CFaceGroup& face_group = *It;
		if (face_group.m_face_type == FaceFlatTypeUpButNotFlat)
		{
			for (std::set<double>::reverse_iterator RIt = plane_heights.rbegin(); RIt != plane_heights.rend(); RIt++)
			{
				double z = *RIt;
				CFaceGroup new_face_group;
				if (face_group.m_tris.SplitAtZ(z, new_face_group.m_tris))
				{
					// add the old, modified face_group
					//new_face_groups.push_back(face_group);

					// add the new face group
					new_face_group.m_face_type = FaceFlatTypeUpButNotFlat;
					new_face_groups.push_back(new_face_group);
				}
			}
		}
	}

	// add the new face groups
	for (std::list<CFaceGroup>::iterator It = new_face_groups.begin(); It != new_face_groups.end(); It++)
	{
		CFaceGroup &face_group = *It;
		face_groups.push_back(face_group);
	}

	std::multimap<double, CMachiningArea> height_sorted_areas;

	for (std::list<CFaceGroup>::iterator It = face_groups.begin(); It != face_groups.end(); It++)
	{
		CFaceGroup& face_group = *It;
		CMachiningArea a;
		a.m_face_type = face_group.m_face_type;
		a.m_area = face_group.m_tris.Shadow2();
		a.m_top = face_group.m_tris.m_box.MaxZ();
		a.m_bottom = face_group.m_tris.m_box.MinZ();
		height_sorted_areas.insert(std::make_pair(a.m_top, a));
	}

	for (std::multimap<double, CMachiningArea>::iterator It = height_sorted_areas.begin(); It != height_sorted_areas.end(); It++)
	{
		CMachiningArea& a = It->second;
		areas.push_front(a);
	}
}

static bool GetFlattenedEdgeTm(CMeshEdgeAndDir* axle_edge, Matrix &tm, std::map<CMeshEdge*, Span>& edge_span_map)
{
	// find the Span which is storing the new position of the edge
	std::map<CMeshEdge*, Span>::iterator FindIt = edge_span_map.find(axle_edge->m_edge);
	if (FindIt == edge_span_map.end())// not found
		return false;

	Span& span = FindIt->second;

	Point o = span.m_p;
	Point vx = span.m_v.m_p - span.m_p;
	Point vy = ~vx;
	Point3d go(o.x, o.y, 0.0);
	Point3d gvx(vx.x, vx.y, 0.0);
	Point3d gvy(vy.x, vy.y, 0.0);

	tm = Matrix(go, gvx, gvy);

	return true;
}
static void AddEdgesToSpanMap(CMeshFace* face, const CTri &tri, std::map<CMeshEdge*, Span>& edge_span_map)
{
	for (int i = 0; i < 3; i++)
	{
		CMeshEdge* edge = face->m_edges[i].m_edge;
		bool dir = face->m_edges[i].m_dir;
		std::map<CMeshEdge*, Span>::iterator FindIt = edge_span_map.find(edge);
		if (FindIt == edge_span_map.end())// not found
		{
			int i0 = i;
			int i1 = i + 1;
			if (i1 == 3)i1 = 0;
			
			Span span(Point(tri.x[i0][0], tri.x[i0][1]), Point(tri.x[i1][0], tri.x[i1][1]));
			edge_span_map.insert(std::make_pair(edge, span));
		}
	}
}

static void AddFlattenedFace(CMeshFace* face, CMeshEdgeAndDir* axle_edge, CTris& new_tris, std::set<CMeshFace*>& faces_done, std::map<CMeshEdge*, Span>& edge_span_map, std::list<CMeshEdgeAndDir*>& edges_to_do)
{
	if (face == NULL) // unconnected edge of the surface
		return;

	if (faces_done.find(face) != faces_done.end())// already done yet
		return;

	for (int i = 0; i < 3; i++)
	{
		if (&(face->m_edges[i]) != axle_edge)edges_to_do.push_back(&(face->m_edges[i]));
	}

	faces_done.insert(face);

	// flatten the triangle by rotating around the axle edge and moving down in z

	if (axle_edge == NULL)// if no axle given
		axle_edge = &(face->m_edges[0]); // just use the first edge

	CTri tri;
	face->GetTri(tri);

	Point3d edge_p0(axle_edge->m_edge->m_v[0]->m_x[0], axle_edge->m_edge->m_v[0]->m_x[1], axle_edge->m_edge->m_v[0]->m_x[2]);
	Point3d edge_p1(axle_edge->m_edge->m_v[1]->m_x[0], axle_edge->m_edge->m_v[1]->m_x[1], axle_edge->m_edge->m_v[1]->m_x[2]);
	Point3d vx(edge_p0, edge_p1);

	Point3d normal;
	tri.GeoffTri().getNormal(&normal);

	if (normal.magnitude() < 0.5)
		return; // no size triangle!

	if (!(axle_edge->m_dir))
	{
		vx = -vx;
		edge_p0 = edge_p1;
	}
	Point3d vy = (normal ^ vx.Normalized()).Normalized();

	Matrix tm(edge_p0, vx, vy);
	Matrix inv_tm = tm.Inverse();

	tri.Transform(inv_tm);

	if (GetFlattenedEdgeTm(axle_edge, tm, edge_span_map))
		tri.Transform(tm);

	new_tris.AddTri(tri.x[0]);

	// add spans to edge span map
	AddEdgesToSpanMap(face, tri, edge_span_map);
}

CTris* CTris::GetFlattenedSurface()const
{
	if (this->m_tris.size() == 0)return NULL;
	CMesh mesh(*this);
	std::set<CMeshFace*> faces_done;
	std::list<CMeshEdgeAndDir*> edges_to_do;
	std::map<CMeshEdge*, Span> edge_span_map;

	CTris* new_solid = new CTris();

	CMeshFace* first_face = *(mesh.m_faces.begin());

	AddFlattenedFace(first_face, NULL, *new_solid, faces_done, edge_span_map, edges_to_do);

	while (edges_to_do.size() > 0)
	{
		// the face on the left of the edge will have already been done, so do the face on the right of the edge
		CMeshEdgeAndDir* edge_to_do = edges_to_do.front();
		edges_to_do.pop_front();
		CMeshFace* face = edge_to_do->m_dir ? edge_to_do->m_edge->m_f[1] : edge_to_do->m_edge->m_f[0];

		AddFlattenedFace(face, edge_to_do, *new_solid, faces_done, edge_span_map, edges_to_do);
	}

	return new_solid;
}

bool CTris::SplitAtZ(double z, CTris& new_tris)
{
	// put any parts of the triangle that are above z into new_tris.
	// leave any parts of the trialge that are bloew z in m_tris
	// return true if there are any triangles in new_tris

	if (z >= m_box.MaxZ())
		return false;

	if (z <= m_box.MinZ())
		return false;

	bool split_done = false;

	CTris new_tris_below;

	for (std::list<CTri>::iterator It = m_tris.begin(); It != m_tris.end(); It++)
	{
		CTri& tri = *It;
		if (tri.SplitAtZ(z, new_tris, new_tris_below))
			split_done = true;
		else
			new_tris_below.AddTri(tri.x[0]);
	}

	if (split_done)
	{
		*this = new_tris_below;
	}

	return split_done;
}
