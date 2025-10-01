// AreaCavC.cpp
#include "CavalierContours/include/cavc/polylineoffset.hpp"

// implements CArea methods using "Cavalier Contours" https://github.com/jbuckmccready/CavalierContours

#include "Area.h"

bool CArea::HolesLinked(){ return false; }

//static const double PI = 3.1415926535897932;

void CArea::Subtract(const CArea& a2)
{
}

void CArea::Intersect(const CArea& a2)
{
}

void CArea::Union(const CArea& a2)
{
}

// static
CArea CArea::UniteCurves(std::list<CCurve> &curves)
{
	CArea area;
	return area;
}

void CArea::Xor(const CArea& a2)
{
}

void CArea::Offset(double inwards_value)
{
	// input polyline
	cavc::Polyline<double> input;
	// add vertexes as (x, y, bulge)
	input.addVertex(0, 25, 1);
	input.addVertex(0, 0, 0);
	input.addVertex(2, 0, 1);
	input.addVertex(10, 0, -0.5);
	input.addVertex(8, 9, 0.374794619217547);
	input.addVertex(21, 0, 0);
	input.addVertex(23, 0, 1);
	input.addVertex(32, 0, -0.5);
	input.addVertex(28, 0, 0.5);
	input.addVertex(39, 21, 0);
	input.addVertex(28, 12, 0);
	input.isClosed() = true;

	// compute the resulting offset polylines, offset = 3
	std::vector<cavc::Polyline<double>> results = cavc::parallelOffset(input, 3.0);
}

void CArea::Thicken(double value)
{
}
