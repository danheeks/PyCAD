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
	Point3d m_p; // end point

	CVertex3D() :m_p(Point3d(0, 0, 0)){}
	CVertex3D(const Point3d& p);

	void Transform(const Matrix& matrix);
};

class CCurve3D
{
public:
	std::list<CVertex3D> m_vertices;
	void append(const CVertex3D& vertex);

	void GetBox(CBox &box)const;
	void Reverse();
	void Transform(const Matrix& matrix);
};
