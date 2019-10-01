// OctTree.cpp

#include "OctTree.h"
#include "Tris.h"
#include "geometry.h"
#include <set>
#include <map>
#include "dxf.h"

CRay::CRay(const Point3d& p, const Point3d& v)
{
	m_p = p;
	m_v = v.Normalized();
	if (v.x > 0)
	{
		if (v.y > 0)
		{
			if (v.z > 0)
				m_type = 0;
			else
				m_type = 1;
		}
		else
		{
			if (v.z > 0)
				m_type = 2;
			else
				m_type = 3;
		}
	}
	else
	{
		if (v.y > 0)
		{
			if (v.z > 0)
				m_type = 4;
			else
				m_type = 5;
		}
		else
		{
			if (v.z > 0)
				m_type = 6;
			else
				m_type = 7;
		}
	}
}



int oct_ele_count = 0;

COctTree::COctTree(const CBox& box) :COctEle(box, 0)
{
}

void COctTree::SetInsideFlags()
{

}


void COctEle::Split()
{
	if ((m_level < MAX_OCTTREE_LEVEL))
	{
		double mid[3];
		for (int i = 0; i < 3; i++)
		{
			mid[i] = (m_box.m_x[i] + m_box.m_x[i + 3]) * 0.5;
		}

		m_children[0] = new COctEle(CBox(mid[0], mid[1], mid[2], m_box.m_x[3], m_box.m_x[4], m_box.m_x[5]), m_level + 1);
		m_children[1] = new COctEle(CBox(m_box.m_x[0], mid[1], mid[2], mid[0], m_box.m_x[4], m_box.m_x[5]), m_level + 1);
		m_children[2] = new COctEle(CBox(m_box.m_x[0], m_box.m_x[1], mid[2], mid[0], mid[1], m_box.m_x[5]), m_level + 1);
		m_children[3] = new COctEle(CBox(mid[0], m_box.m_x[1], mid[2], m_box.m_x[3], mid[1], m_box.m_x[5]), m_level + 1);
		m_children[4] = new COctEle(CBox(mid[0], mid[1], m_box.m_x[2], m_box.m_x[3], m_box.m_x[4], mid[2]), m_level + 1);
		m_children[5] = new COctEle(CBox(m_box.m_x[0], mid[1], m_box.m_x[2], mid[0], m_box.m_x[4], mid[2]), m_level + 1);
		m_children[6] = new COctEle(CBox(m_box.m_x[0], m_box.m_x[1], m_box.m_x[2], mid[0], mid[1], mid[2]), m_level + 1);
		m_children[7] = new COctEle(CBox(mid[0], m_box.m_x[1], m_box.m_x[2], m_box.m_x[3], mid[1], mid[2]), m_level + 1);
	}
}

void COctEle::MakeSphere(const Point3d& c, double r)
{
	if ((m_level < MAX_OCTTREE_LEVEL))
	{
		double mid[3];
		for (int i = 0; i < 3; i++)
		{
			mid[i] = (m_box.m_x[i] + m_box.m_x[i + 3]) * 0.5;
		}

		m_children[0] = new COctEle(CBox(mid[0], mid[1], mid[2], m_box.m_x[3], m_box.m_x[4], m_box.m_x[5]), m_level + 1);
		m_children[1] = new COctEle(CBox(m_box.m_x[0], mid[1], mid[2], mid[0], m_box.m_x[4], m_box.m_x[5]), m_level + 1);
		m_children[2] = new COctEle(CBox(m_box.m_x[0], m_box.m_x[1], mid[2], mid[0], mid[1], m_box.m_x[5]), m_level + 1);
		m_children[3] = new COctEle(CBox(mid[0], m_box.m_x[1], mid[2], m_box.m_x[3], mid[1], m_box.m_x[5]), m_level + 1);
		m_children[4] = new COctEle(CBox(mid[0], mid[1], m_box.m_x[2], m_box.m_x[3], m_box.m_x[4], mid[2]), m_level + 1);
		m_children[5] = new COctEle(CBox(m_box.m_x[0], mid[1], m_box.m_x[2], mid[0], m_box.m_x[4], mid[2]), m_level + 1);
		m_children[6] = new COctEle(CBox(m_box.m_x[0], m_box.m_x[1], m_box.m_x[2], mid[0], mid[1], mid[2]), m_level + 1);
		m_children[7] = new COctEle(CBox(mid[0], m_box.m_x[1], m_box.m_x[2], m_box.m_x[3], mid[1], mid[2]), m_level + 1);

		int inside_count = 0;
		for (int i = 0; i < 8; i++)
		{
			int touching =m_children[i]->TouchingSphere(c, r);
			m_children[i]->m_inside = (touching == 8);
			inside_count += touching;
		}
		if (inside_count == 0)
		{
			m_inside = false;
			for (int i = 0; i < 8; i++)
			{
				delete m_children[i];
				m_children[i] = NULL;
			}
		}
		else if (inside_count == 64)
		{
			m_inside = true;
			for (int i = 0; i < 8; i++)
			{
				delete m_children[i];
				m_children[i] = NULL;
			}
		}
		else
		{
			for (int i = 0; i < 8; i++)
			{
				m_children[i]->MakeSphere(c, r);
			}
			m_inside = true;
		}
	}
	else
	{
		m_inside = TouchingSphere(c, r) != 0;
	}
}

int COctEle::TouchingSphere(const Point3d& c, double r)
{
	Point3d p[8] =
	{
		Point3d(m_box.m_x[0], m_box.m_x[1], m_box.m_x[2]),
		Point3d(m_box.m_x[0], m_box.m_x[1], m_box.m_x[5]),
		Point3d(m_box.m_x[0], m_box.m_x[4], m_box.m_x[2]),
		Point3d(m_box.m_x[0], m_box.m_x[4], m_box.m_x[5]),
		Point3d(m_box.m_x[3], m_box.m_x[1], m_box.m_x[2]),
		Point3d(m_box.m_x[3], m_box.m_x[1], m_box.m_x[5]),
		Point3d(m_box.m_x[3], m_box.m_x[4], m_box.m_x[2]),
		Point3d(m_box.m_x[3], m_box.m_x[4], m_box.m_x[5])
	};

	int touching = 0;
	for (int i = 0; i < 8; i++)
	{
		if (p[i].Dist(c) <= r)
			touching++;
	}
	return touching;
}

//static 
int COctEle::m_child_order[8][8] = {
	{6, 2, 5, 7, 1, 3, 4, 0 },
	{ 2, 1, 3, 6, 0, 5, 7, 4 },
	{ 5, 1, 4, 6, 0, 2, 7, 3 },
	{ 1, 0, 2, 5, 3, 4, 6, 7 },
	{ 7, 3, 4, 6, 0, 2, 5, 1 },
	{ 3, 0, 2, 7, 1, 4, 6, 5 },
	{ 4, 0, 5, 7, 1, 3, 6, 2 },
	{ 0, 1, 3, 4, 2, 5, 7, 6 }
};

COctEle::COctEle(const CBox& box, int level) : m_box(box), m_level(level), m_inside(true)
{
	oct_ele_count++;
	m_children[0] = NULL; // mark the first child as NULL to show there are no children yet
}

bool COctEle::AddTri(const CTri* tri)
{
	bool added = false;

		if (tri->m_box.Intersects(m_box))
		{
	if (m_children[0])
	{
		for (int i = 0; i < 8; i++)
		{
			if (m_children[i]->AddTri(tri))added = true;
		}
	}
	else
	{
			m_tris.push_back(tri);
			SplitIfNecessary();
			added = true;
	}
		}

	return added;
}

void COctEle::SplitIfNecessary()
{
	if ((m_tris.size() > 2) && (m_level < MAX_OCTTREE_LEVEL))
	{
		double mid[3];
		for (int i = 0; i < 3; i++)
		{
			mid[i] = (m_box.m_x[i] + m_box.m_x[i + 3]) * 0.5;
		}

		m_children[0] = new COctEle(CBox(mid[0], mid[1], m_box.m_x[2], m_box.m_x[3], m_box.m_x[4], mid[2]), m_level + 1);
		m_children[1] = new COctEle(CBox(m_box.m_x[0], mid[1], m_box.m_x[2], mid[0], m_box.m_x[4], mid[2]), m_level + 1);
		m_children[2] = new COctEle(CBox(m_box.m_x[0], m_box.m_x[1], m_box.m_x[2], mid[0], mid[1], mid[2]), m_level + 1);
		m_children[3] = new COctEle(CBox(mid[0], m_box.m_x[1], m_box.m_x[2], m_box.m_x[3], mid[1], mid[2]), m_level + 1);
		m_children[4] = new COctEle(CBox(mid[0], mid[1], mid[2], m_box.m_x[3], m_box.m_x[4], m_box.m_x[5]), m_level + 1);
		m_children[5] = new COctEle(CBox(m_box.m_x[0], mid[1], mid[2], mid[0], m_box.m_x[4], m_box.m_x[5]), m_level + 1);
		m_children[6] = new COctEle(CBox(m_box.m_x[0], m_box.m_x[1], mid[2], mid[0], mid[1], m_box.m_x[5]), m_level + 1);
		m_children[7] = new COctEle(CBox(mid[0], m_box.m_x[1], mid[2], m_box.m_x[3], mid[1], m_box.m_x[5]), m_level + 1);

		// insert all triangles into children
		for (std::list<const CTri*>::iterator It = m_tris.begin(); It != m_tris.end(); It++)
		{
			const CTri* tri = *It;
			for (int i = 0; i < 8; i++)
			{
				m_children[i]->AddTri(tri);
			}
		}
		m_tris.clear();
	}
}

void COctTree::MakeNewListOfTriangles(const std::list<CTri> &in_tris, std::list<CTri> &tris, int keep_type)
{
	for (std::list<CTri>::const_iterator It = in_tris.begin(); It != in_tris.end(); It++)
	{
		const CTri& tri = *It;
		int in_out_type = GetInsideOutside(&tri);

		if ((in_out_type & keep_type) != 0)
			tris.push_back(tri);
	}
}

bool TriContainsMinusX(const CTri& tri)
{
	// list in order of distance from origin, only need to consider xy plane
	std::multimap<double, int> index_map;
	for (int i = 0; i < 3; i++)
	{
		index_map.insert(std::make_pair(sqrt(tri.x[i][0] * tri.x[i][0] + tri.x[i][1] * tri.x[i][1]), i));
	}

	// get the two vectors
	double angles[2];
	int j = 0;
	for (std::multimap<double, int>::iterator It = index_map.begin(); It != index_map.end(); It++)
	{
		int index = It->second;
		if (It != index_map.begin())
		{
			angles[j] = atan2(tri.x[index][1], tri.x[index][0]);
			j++;
		}
	}

	double angleminusx = PI;
	if (angles[1] < angles[0])angles[1] += 2 * PI;
	if (angles[1] - angles[0] > PI)
	{
		double temp = angles[0];
		angles[0] = angles[1];
		angles[1] = temp + 2 * PI;
	}
	if (angleminusx < angles[0])angleminusx += 2 * PI;

	bool tri_contains_minusx = (angleminusx >= angles[0] && angleminusx <= angles[1]);

	return tri_contains_minusx;
}

const CTri* GetClosestTri(const std::list<CIntersectPoint>& first_hits, Point3d &average_normal)
{
	// find the front-most of the multiple triangles hit and return it's normal

	// find the average normal of the triangles
	average_normal = Point3d(0.0, 0.0, 0.0);
	for (std::list<CIntersectPoint>::const_iterator It = first_hits.begin(); It != first_hits.end(); It++)
	{
		const CIntersectPoint& ip = *It;
		const CTri* tri = ip.m_tri;
		Point3d norm = Point3d(Point3d(tri->x[0][0], tri->x[0][1], tri->x[0][2]), Point3d(tri->x[1][0], tri->x[1][1], tri->x[1][2])) ^ Point3d(Point3d(tri->x[0][0], tri->x[0][1], tri->x[0][2]), Point3d(tri->x[2][0], tri->x[2][1], tri->x[2][2]));
		average_normal += norm;
	}
	average_normal.normalise();

	// for the rare case that the average normal is facing away from or toward us, then use it directly
	if (fabs(fabs(average_normal.x) - 1.0) < 0.000000001)
	{
		return NULL;
	}

	// make a matrix using x-axis as x-axis, and perp to x and average_normal as y-axis
	Point3d origin = first_hits.front().m_p;
	Point3d x_axis(1.0, 0.0, 0.0);
	Point3d y_axis = x_axis ^ average_normal;
	y_axis.normalise();
	Matrix tm(origin, x_axis, y_axis);
	Matrix invtm = tm.Inverse();

	// loop through the triangles hit, finding the one that contains -x vector
	for (std::list<CIntersectPoint>::const_iterator It = first_hits.begin(); It != first_hits.end(); It++)
	{
		const CIntersectPoint& ip = *It;
		CTri t = *(ip.m_tri);
		t.Transform(invtm);

		if (TriContainsMinusX(t))
		{
			const CTri* tri = ip.m_tri;
			return tri;
		}
	}

	return NULL; // shouldn't get here
}

int COctTree::GetInsideOutside(const Point3d& p)
{
	// make an x-ray from point to outside of model
	CXRay xray(p.y, p.z);

	std::list<CIntersectPoint> list;

	IntersectionPoints(xray, list);

	std::list<CIntersectPoint> first_hits;

	for (std::list<CIntersectPoint>::iterator It = list.begin(); It != list.end(); It++)
	{
		CIntersectPoint &ip = *It;

		if (ip.m_tri->On(p))
			return TRI_ON;

		if (fabs(ip.m_p.x - p.x) < 0.0001)
			return TRI_ON;

		if (ip.m_p.x >= p.x)
		{
			if (first_hits.size() > 0)
			{
				if (ip.m_p.x > first_hits.front().m_p.x + 0.001)
					break;
			}

			first_hits.push_back(ip);
		}
	}

	if (first_hits.size() > 0)
	{
		const CTri* tri = first_hits.front().m_tri;
		Point3d norm;
		if (first_hits.size() > 1)
		{
			tri = GetClosestTri(first_hits, norm);
		}

		if (tri)
			// use normal of the triangle hit
			norm = Point3d(Point3d(tri->x[0][0], tri->x[0][1], tri->x[0][2]), Point3d(tri->x[1][0], tri->x[1][1], tri->x[1][2])) ^ Point3d(Point3d(tri->x[0][0], tri->x[0][1], tri->x[0][2]), Point3d(tri->x[2][0], tri->x[2][1], tri->x[2][2]));

		if (norm.x > 0)
			return TRI_INSIDE;
		return TRI_OUTSIDE;
	}

	return TRI_OUTSIDE;
}

int COctTree::GetInsideOutside(const CTri* tri)
{
	int inside = 0;
	int outside = 0;
	Point3d p[4];

	for (int i = 0; i < 3; i++)
	{
		p[i] = Point3d(tri->x[i][0], tri->x[i][1], tri->x[i][2]);
	}

	Point3d v0(p[0], p[1]);
	Point3d v1(p[0], p[2]);
	p[3] = p[0] + v0 * 0.3333 + v1 * 0.3333; // mid point

	for (int i = 0; i < 4; i++)
	{
		int type = GetInsideOutside(p[i]);

		switch(type)
		{
			case TRI_INSIDE:
				inside++;
				break;
			case TRI_OUTSIDE:
				outside++;
				break;
		}
	}

	if (outside > 2)return TRI_OUTSIDE;
	if (inside > 2)return TRI_INSIDE;
	if (outside > 1)return TRI_OUTSIDE;
	if (inside > 1)return TRI_INSIDE;
	if (outside > 0)return TRI_OUTSIDE;
	if (inside > 0)return TRI_INSIDE;
	return TRI_ON;
}

bool COctEle::IntersectsBox(const CXRay& xray)
{
	if (xray.m_y < m_box.MinY() - TOLERANCE) return false;
	if (xray.m_y > m_box.MaxY() + TOLERANCE) return false;
	if (xray.m_z < m_box.MinZ() - TOLERANCE) return false;
	if (xray.m_z > m_box.MaxZ() + TOLERANCE) return false;
	return true;
}

std::multimap<double, CIntersectPoint>* map_for_IntersectionPoints = NULL;
const CXRay* x_ray_for_IntersectionPoints = NULL;
std::set<const CTri*>* tris_checked_for_IntersectionPoints = NULL;

bool CXRay::Intersects(const CTri* tri, LineOrPoint& intof)const
{
	Line line(Point3d(-1000.0, m_y, m_z), Point3d(1000.0, m_y, m_z));
	return tri->Intof(line, intof);
}

void COctEle::IntersectionPoints()
{
	if (!IntersectsBox(*x_ray_for_IntersectionPoints))
		return;

	for (std::list<const CTri*>::iterator It = m_tris.begin(); It != m_tris.end(); It++)
	{
		const CTri* tri = *It;
		if (tris_checked_for_IntersectionPoints->find(tri) == tris_checked_for_IntersectionPoints->end())
		{
			LineOrPoint line_or_point;
			if (x_ray_for_IntersectionPoints->Intersects(tri, line_or_point))
			{
				Point3d tri_int_point;
				if (line_or_point.m_is_a_line)
				{
					if (line_or_point.m_line.v.x < 0.0)
						tri_int_point = line_or_point.m_line.p0 + line_or_point.m_line.v;
					else
						tri_int_point = line_or_point.m_line.p0;
				}
				else
					tri_int_point = line_or_point.m_p;
				map_for_IntersectionPoints->insert(std::make_pair(tri_int_point.x, CIntersectPoint(tri_int_point, tri)));
			}
			tris_checked_for_IntersectionPoints->insert(tri);
		}
	}

	if (m_children[0])
	{
		for (int i = 0; i < 8; i++)
			m_children[i]->IntersectionPoints();
	}

}

void COctTree::IntersectionPoints(const CXRay& xray, std::list<CIntersectPoint> &list)
{
	std::multimap<double, CIntersectPoint> map;
	std::set<const CTri*> tris_checked;

	map_for_IntersectionPoints = &map;
	x_ray_for_IntersectionPoints = &xray;
	tris_checked_for_IntersectionPoints = &tris_checked;

	COctEle::IntersectionPoints();

	for (std::multimap<double, CIntersectPoint>::iterator It = map.begin(); It != map.end(); It++)
	{
		list.push_back(It->second);
	}
}

void GetTriTrisIntersections(const CTri& tri, const std::set<const CTri*> &set, std::list<Point3d> &intof_list)
{
	Triangle3d t = tri.GeoffTri();

	std::list<Point3d> list;

	for (std::set<const CTri*>::const_iterator It = set.begin(); It != set.end(); It++)
	{
		const CTri* tri2 = *It;
		Triangle3d t2 = tri2->GeoffTri();

		std::list<Point3d> ilist;
		t.Intof(t2, ilist);
		for (std::list<Point3d>::iterator It2 = ilist.begin(); It2 != ilist.end(); It2++)
		{
			Point3d& p = *It2;
			bool unique = true;
			for (std::list<Point3d>::iterator It3 = list.begin(); It3 != list.end(); It3++)
			{
				Point3d& p2 = *It3;
				if (p == p2)
				{
					unique = false;
					break;
				}
			}
			if (unique)list.push_back(p);
		}
	}

	for (std::list<Point3d>::iterator It = list.begin(); It != list.end(); It++)
	{
		Point3d& p = *It;
		intof_list.push_back(p);
	}
}

void GetTriTrisIntersections(const CTri& tri, const std::set<const CTri*> &set, std::list<Line> &intof_list)
{
	Triangle3d t = tri.GeoffTri();

	for (std::set<const CTri*>::const_iterator It = set.begin(); It != set.end(); It++)
	{
		const CTri* tri2 = *It;
		Triangle3d t2 = tri2->GeoffTri();

		std::list<Point3d> ilist;
		t.Intof(t2, ilist);
		if (ilist.size() == 2)
		{
			intof_list.push_back(Line(ilist.front(), ilist.back()));
		}
	}
}

void SplitTriWithPoint(const CTri& tri, const Point3d &p, std::list<CTri> &new_tris)
{
	Triangle3d t = tri.GeoffTri();

	if (p == t.getVert1() || p == t.getVert2() || p == t.getVert3())
	{
		// if the point is on a corner, just add the original
		new_tris.push_back(tri);
		return;
	}

	if (!t.Inside(p))
	{
		// if the point is not in the triangle, just add the original
		new_tris.push_back(tri);
		return;
	}

	float x[3] = { (float)(p.x), (float)(p.y), (float)(p.z) };

	Line edge1(t.getVert1(), t.getVert2());
	if (!edge1.Intof(p))
	{
		CTri new_tri(tri);
		memcpy(new_tri.x[2], x, 3 * sizeof(float));
		new_tri.SetBox();
		new_tris.push_back(new_tri);
	}

	Line edge2(t.getVert2(), t.getVert3());
	if (!edge2.Intof(p))
	{
		CTri new_tri(tri);
		memcpy(new_tri.x[0], x, 3 * sizeof(float));
		new_tri.SetBox();
		new_tris.push_back(new_tri);
	}

	Line edge3(t.getVert3(), t.getVert1());
	if (!edge3.Intof(p))
	{
		CTri new_tri(tri);
		memcpy(new_tri.x[1], x, 3 * sizeof(float));
		new_tri.SetBox();
		new_tris.push_back(new_tri);
	}
}
//
//class CLineOrPointAndTri
//{
//public:
//	LineOrPoint m_line_or_point;
//	const CTri* m_tri;
//
//	CLineOrPointAndTri(const LineOrPoint& line_or_point, const CTri* m_tri) :m_line_or_point(line_or_point), m_tri(tri){}
//};

void SplitTriWithLineSegs(const CTri& tri, const std::list<Line> &intof_line_list, std::list<CTri> &new_tris)
{
	std::list<CTri> local_new_tris;
	local_new_tris.push_back(tri);

	for (std::list<Line>::const_iterator It = intof_line_list.begin(); It != intof_line_list.end(); It++)
	{
		const Line& line = *It;
		std::list<CTri> temp_new_tris;

		for (std::list<CTri>::iterator It3 = local_new_tris.begin(); It3 != local_new_tris.end();)
		{
			CTri& tri3 = *It3;
			LineOrPoint intof;
			if (tri3.Intof(line, intof))
			{
				if (intof.m_is_a_line)
				{
					std::list<CTri> temp_temp_new_tris;
					SplitTriWithPoint(tri3, intof.m_line.p0, temp_temp_new_tris);
					for (std::list<CTri>::iterator It4 = temp_temp_new_tris.begin(); It4 != temp_temp_new_tris.end();It4++)
					{
						CTri& tri4 = *It4;
						SplitTriWithPoint(tri4, intof.m_line.p0 + intof.m_line.v, temp_new_tris);
					}
				}
				else
					SplitTriWithPoint(tri3, intof.m_p, temp_new_tris);
				It3 = local_new_tris.erase(It3);
			}
			else
				It3++;
		}

		for (std::list<CTri>::iterator It2 = temp_new_tris.begin(); It2 != temp_new_tris.end(); It2++)
		{
			local_new_tris.push_back(*It2);
		}
	}

	for (std::list<CTri>::iterator It = local_new_tris.begin(); It != local_new_tris.end(); It++)
	{
		new_tris.push_back(*It);
	}
}

void COctTree::SplitTriangles(std::list<CTri> &tris)
{
	size_t size = tris.size();
	unsigned int i = 0;
	for (std::list<CTri>::iterator It = tris.begin(); i < size; i++)
	{
		CTri& tri = *It;
		std::set<const CTri*> set;
		GetTrianglesInBox(tri.m_box, set);

		std::list<Line> intof_list;
		GetTriTrisIntersections(tri, set, intof_list);

		//WriteDxfFile(L"c:\\temp\\tri_intersections.dxf", intof_list);
		SplitTriWithLineSegs(tri, intof_list, tris);

		It = tris.erase(It);
	}
}

std::set<const CTri*>* set_for_GetTrianglesInBox = NULL;
const CBox* box_for_GetTrianglesInBox = NULL;

void COctEle::GetTrianglesInBox()
{
	if (box_for_GetTrianglesInBox->Intersects(m_box))
	{
		for (std::list<const CTri*>::iterator It = m_tris.begin(); It != m_tris.end(); It++)
		{
			const CTri* tri = *It;
			set_for_GetTrianglesInBox->insert(tri);
		}

		if (m_children[0])
		{
			for (int i = 0; i < 8; i++)
				m_children[i]->GetTrianglesInBox();
		}
	}
}

/*
Fast Ray-Box Intersection
by Andrew Woo
from "Graphics Gems", Academic Press, 1990
*/

#define NUMDIM	3
#define RIGHT	0
#define LEFT	1
#define MIDDLE	2
#define TRUE 1
#define FALSE 0

char HitBoundingBox(const double* minB, const double* maxB, const double* origin, const double* dir, double* coord)
{
	char inside = TRUE;
	char quadrant[NUMDIM];
	register int i;
	int whichPlane;
	double maxT[NUMDIM];
	double candidatePlane[NUMDIM];

	/* Find candidate planes; this loop can be avoided if
	rays cast all from the eye(assume perpsective view) */
	for (i = 0; i<NUMDIM; i++)
		if (origin[i] < minB[i]) {
			quadrant[i] = LEFT;
			candidatePlane[i] = minB[i];
			inside = FALSE;
		}
		else if (origin[i] > maxB[i]) {
			quadrant[i] = RIGHT;
			candidatePlane[i] = maxB[i];
			inside = FALSE;
		}
		else	{
			quadrant[i] = MIDDLE;
		}

		/* Ray origin inside bounding box */
		if (inside)	{
			memcpy(coord, origin, 3*sizeof(double));
			return (TRUE);
		}


		/* Calculate T distances to candidate planes */
		for (i = 0; i < NUMDIM; i++)
			if (quadrant[i] != MIDDLE && dir[i] != 0.)
				maxT[i] = (candidatePlane[i] - origin[i]) / dir[i];
			else
				maxT[i] = -1.;

		/* Get largest of the maxT's for final choice of intersection */
		whichPlane = 0;
		for (i = 1; i < NUMDIM; i++)
			if (maxT[whichPlane] < maxT[i])
				whichPlane = i;

		/* Check final candidate actually inside box */
		if (maxT[whichPlane] < 0.) return (FALSE);
		for (i = 0; i < NUMDIM; i++)
			if (whichPlane != i) {
				coord[i] = origin[i] + maxT[whichPlane] * dir[i];
				if (coord[i] < minB[i] || coord[i] > maxB[i])
					return (FALSE);
			}
			else {
				coord[i] = candidatePlane[i];
			}
			return TRUE;				/* ray hits box */
}


bool COctEle::GetRayColor(const CRay& ray, unsigned char* col4, bool bright_spot)
{
	double coord[3];
	char result = HitBoundingBox(m_box.m_x, &m_box.m_x[3], ray.m_p.getBuffer(), ray.m_v.getBuffer(), coord);

	if (result == 0)
		return false;

	if (m_children[0])
	{
		for (int i = 0; i < 8; i++)
		{
			if (m_children[m_child_order[ray.m_type][i]]->GetRayColor(ray, col4, bright_spot))
				return true;
		}
	}
	else
	{
		if (m_inside)
		{
			unsigned char c = 0xff;
			if (coord[0] == m_box.m_x[3])c = 0xc0;
			else if (coord[0] == m_box.m_x[0])c = 0xb0;
			else if (coord[1] == m_box.m_x[4])c = 0xa0;
			else if (coord[1] == m_box.m_x[1])c = 0x90;
			else if (coord[2] == m_box.m_x[5])c = 0x80;
			else if (coord[2] == m_box.m_x[2])c = 0x70;
			*col4 = c;
			col4++;
			*col4 = c;
			col4++;
			*col4 = c;
			return true;
		}
	}
	return false;
}

void COctTree::GetTrianglesInBox(const CBox& box, std::set<const CTri*> &set)
{
	set_for_GetTrianglesInBox = &set;
	box_for_GetTrianglesInBox = &box;

	COctEle::GetTrianglesInBox();
}
