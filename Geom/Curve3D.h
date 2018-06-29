// Curve3D.h
// Copyright 2016, Dan Heeks

#pragma once

#include <vector>
#include <list>
#include <math.h>
#include "geometry.h"
#include "Box.h"

class CVertex3D
{
public:
	geoff_geometry::Point3d m_p; // end point

	CVertex3D() :m_p(geoff_geometry::Point3d(0, 0, 0)){}
	CVertex3D(const geoff_geometry::Point3d& p);

	void Transform(const geoff_geometry::Matrix& matrix);
};

class CCurve3D
{
public:
	std::list<CVertex3D> m_vertices;
	void append(const CVertex3D& vertex);

	void GetBox(CBox &box)const;
	void Reverse();
	void Transform(const geoff_geometry::Matrix& matrix);
};
