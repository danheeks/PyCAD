// DigitizedPoint.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once


enum DigitizeType{
	DigitizeNoItemType,
	DigitizeEndofType,
	DigitizeIntersType,
	DigitizeMidpointType,
	DigitizeCentreType,
	DigitizeScreenType,
	DigitizeCoordsType,
	DigitizeNearestType,
	DigitizeTangentType,
	DigitizeInputType // typed into properties, for example
};

class DigitizedPoint{
public:
	geoff_geometry::Point3d m_point;
	DigitizeType m_type;
	HeeksObj* m_object1;
	HeeksObj* m_object2;

	DigitizedPoint();
	DigitizedPoint(geoff_geometry::Point3d point, DigitizeType t, HeeksObj* object1 = NULL, HeeksObj* object2 = NULL);

	int importance();

	// calculate tangent points
	static bool GetLinePoints(const DigitizedPoint& d1, const DigitizedPoint& d2, geoff_geometry::Point3d &p1, geoff_geometry::Point3d &p2);
	static bool GetArcPoints(const DigitizedPoint& d1, const geoff_geometry::Point3d *initial_direction, const DigitizedPoint& d2, geoff_geometry::Point3d &p1, geoff_geometry::Point3d &p2, geoff_geometry::Point3d &centre, geoff_geometry::Point3d &axis);
#if 0
	static bool GetTangentCircle(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, gp_Circ& c);
	static bool GetEllipse(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, gp_Elips& e);
	static bool GetCircleBetween(const DigitizedPoint& d1, const DigitizedPoint& d2, gp_Circ& c);
	static bool GetQuarticSpline(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, Handle_Geom_BSplineCurve &spline);
	static bool GetCubicSpline(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, const DigitizedPoint& d4, Handle_Geom_BSplineCurve &spline);
	static bool GetRationalSpline(std::list<DigitizedPoint> &spline_points, const DigitizedPoint& d4, Handle_Geom_BSplineCurve &spline);
#endif
};

