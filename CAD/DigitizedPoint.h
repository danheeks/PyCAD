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
	Point3d m_point;
	DigitizeType m_type;
	HeeksObj* m_object1;
	HeeksObj* m_object2;

	DigitizedPoint();
	DigitizedPoint(Point3d point, DigitizeType t, HeeksObj* object1 = NULL, HeeksObj* object2 = NULL);

	int importance();

	// calculate tangent points
	static bool GetLinePoints(const DigitizedPoint& d1, const DigitizedPoint& d2, Point3d &p1, Point3d &p2);
	static bool GetArcPoints(const DigitizedPoint& d1, const Point3d *initial_direction, const DigitizedPoint& d2, Point3d &p1, Point3d &p2, Point3d &centre, Point3d &axis);
	static bool GetTangentCircle(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, Circle& c);
#if 0
	static bool GetEllipse(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, gp_Elips& e);
#endif
	static bool GetCircleBetween(const DigitizedPoint& d1, const DigitizedPoint& d2, Circle& c);
#if 0
	static bool GetQuarticSpline(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, Handle_Geom_BSplineCurve &spline);
	static bool GetCubicSpline(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, const DigitizedPoint& d4, Handle_Geom_BSplineCurve &spline);
	static bool GetRationalSpline(std::list<DigitizedPoint> &spline_points, const DigitizedPoint& d4, Handle_Geom_BSplineCurve &spline);
#endif
};

