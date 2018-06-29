// OctTree.h

#pragma once

#include <fstream>
#include <list>
#include <set>
#include "Box.h"
#include "geometry.h"

class CTri;
class LineOrPoint;

#define MAX_OCTTREE_LEVEL 8

class CXRay
{
	// an infinite line parallel to the x-axis, with given y and z values
public:
	double m_y;
	double m_z;

	CXRay(double y, double z) :m_y(y), m_z(z){}
	bool Intersects(const CTri* tri, LineOrPoint& intof)const;
};

class CRay
{
public:
	geoff_geometry::Point3d m_p;
	geoff_geometry::Vector3d m_v;
	int m_type;//  0 +x+y+z  1 +x+y-z  2 +x-y+z  3 +x-y-z  4 -x+y+z  5 -x+y-z  6 -x-y+z  7 -x-y-z

	CRay(const geoff_geometry::Point3d& p, const geoff_geometry::Vector3d& v);
};


class COctEle
{
public:
	enum BooleanType
	{
		OCTELE_BOOLEAN_UNION,
		OCTELE_BOOLEAN_CUT,
		OCTELE_BOOLEAN_COMMON,
		OCTELE_BOOLEAN_XOR,
	};

	CBox m_box;
	int m_level;
	std::list<const CTri*> m_tris;
	COctEle* m_children[8];  // (1, 1,-1), (-1, 1,-1), (-1,-1,-1), (1,-1,-1), (1, 1, 1), (-1, 1, 1), (-1,-1, 1), (1,-1, 1)  
	bool m_inside; // valid if there are no children
	static int m_child_order[8][8]; // [ray_type][child]

	COctEle(const CBox& box, int level);

	bool AddTri(const CTri* tri);
	void SplitIfNecessary();
	void MakeNewListOfTriangles();
	bool IntersectsBox(const CXRay& xray);
	void IntersectionPoints();
	void GetTrianglesInBox();
	bool GetRayColor(const CRay& ray, unsigned char* col4, bool bright_spot);
	void Split();
	void MakeSphere(const geoff_geometry::Point3d& c, double r);
	int TouchingSphere(const geoff_geometry::Point3d& c, double r);
};

class CIntersectPoint
{
public:
	geoff_geometry::Point3d m_p;
	const CTri* m_tri;

	CIntersectPoint(const geoff_geometry::Point3d &p, const CTri* tri) :m_p(p), m_tri(tri){}
};

static const int TRI_INSIDE = 1;
static const int TRI_OUTSIDE = 2;
static const int TRI_ON = 4;

class COctTree: public COctEle
{
public:
	COctTree(const CBox& box);

	void SetInsideFlags();
	void MakeNewListOfTriangles(const std::list<CTri> &in_tris, std::list<CTri> &tris, int keep_type);
	int GetInsideOutside(const geoff_geometry::Point3d& p);
	int GetInsideOutside(const CTri* tri);
	void IntersectionPoints(const CXRay& xray, std::list<CIntersectPoint> &list);
	void SplitTriangles(std::list<CTri> &tris);
	void GetTrianglesInBox(const CBox& box, std::set<const CTri*> &list);
};
