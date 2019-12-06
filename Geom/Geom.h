// Geom.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "geometry.h"

void intersect(const Circle& c1, const Circle& c2, std::list<Point> &list);

enum PLC_Type{
	PLC_Unknown,
	PLC_Point,
	PLC_Line,
	PLC_Circle,
	PLC_TwoCircle,
};

class PointLineOrCircle{
public:
	PLC_Type type; // PLC_Unknown, PLC_Point, PLC_Line, PLC_Circle or PLC_TwoCircle
	Point3d p;
	Line l;
	Circle c;
	Circle c2;

	PointLineOrCircle() :type(PLC_Unknown){}
};

//Point3d ClosestPointOnPlane(const Plane& pln, const Point3d &p);
Point3d ClosestPointOnLine(const Line& line, const Point3d &p);
void ClosestPointsOnLines(const Line& lin, const Line& lin2, Point3d &p1, Point3d &p2);// they might be the same point
void ClosestPointsLineAndCircle(const Line& lin, const Circle& cir, std::list<Point3d> &list);
//double GetEllipseRotation(const gp_Elips& elips);
//double DistanceToFoci(const Point3d &pnt, const gp_Elips &elips);
//void ClosestPointsLineAndEllipse(const Line& lin, const gp_Elips& elips, std::list<Point3d> &list);

// I've made all the combinations of these, 3*3*3 = 27 :), but all except 10 are just to redirect
void TangentCircles(const Point3d& p1, const Point3d& p2, const Point3d& p3, std::list<Circle>& c_list);
void TangentCircles(const Point3d& p1, const Point3d& p2, const Line& l3, std::list<Circle>& c_list);
void TangentCircles(const Point3d& p1, const Point3d& p2, const Circle& c3, std::list<Circle>& c_list);
void TangentCircles(const Point3d& p1, const Line& l2, const Point3d& p3, std::list<Circle>& c_list);
void TangentCircles(const Point3d& p1, const Line& l2, const Line& l3, std::list<Circle>& c_list);
void TangentCircles(const Point3d& p1, const Line& l2, const Circle& c3, std::list<Circle>& c_list);
void TangentCircles(const Point3d& p1, const Circle& c2, const Point3d& p3, std::list<Circle>& c_list);
void TangentCircles(const Point3d& p1, const Circle& c2, const Line& l3, std::list<Circle>& c_list);
void TangentCircles(const Point3d& p1, const Circle& c2, const Circle& c3, std::list<Circle>& c_list);
void TangentCircles(const Line& l1, const Point3d& p2, const Point3d& p3, std::list<Circle>& c_list);
void TangentCircles(const Line& l1, const Point3d& p2, const Line& l3, std::list<Circle>& c_list);
void TangentCircles(const Line& l1, const Point3d& p2, const Circle& c3, std::list<Circle>& c_list);
void TangentCircles(const Line& l1, const Line& l2, const Point3d& p3, std::list<Circle>& c_list);
void TangentCircles(const Line& l1, const Line& l2, const Line& l3, std::list<Circle>& c_list);
void TangentCircles(const Line& l1, const Line& l2, const Circle& c3, std::list<Circle>& c_list);
void TangentCircles(const Line& l1, const Circle& c2, const Point3d& p3, std::list<Circle>& c_list);
void TangentCircles(const Line& l1, const Circle& c2, const Line& l3, std::list<Circle>& c_list);
void TangentCircles(const Line& l1, const Circle& c2, const Circle& c3, std::list<Circle>& c_list);
void TangentCircles(const Circle& c1, const Point3d& p2, const Point3d& p3, std::list<Circle>& c_list);
void TangentCircles(const Circle& c1, const Point3d& p2, const Line& l3, std::list<Circle>& c_list);
void TangentCircles(const Circle& c1, const Point3d& p2, const Circle& c3, std::list<Circle>& c_list);
void TangentCircles(const Circle& c1, const Line& l2, const Point3d& p3, std::list<Circle>& c_list);
void TangentCircles(const Circle& c1, const Line& l2, const Line& l3, std::list<Circle>& c_list);
void TangentCircles(const Circle& c1, const Line& l2, const Circle& c3, std::list<Circle>& c_list);
void TangentCircles(const Circle& c1, const Circle& c2, const Point3d& p3, std::list<Circle>& c_list);
void TangentCircles(const Circle& c1, const Circle& c2, const Line& l3, std::list<Circle>& c_list);
void TangentCircles(const Circle& c1, const Circle& c2, const Circle& c3, std::list<Circle>& c_list);

void TangentCircles(const Circle& c1, const Circle& c2, const Circle& c3, std::list<Circle>& c_list);
void TangentCircles(const Circle& c1, const Line& l2, const Point3d& p3, std::list<Circle>& c_list);
void TangentCircles(const PointLineOrCircle& plc1, const PointLineOrCircle& plc2, const PointLineOrCircle& plc3, std::list<Circle>& c_list);
Circle PointToCircle(const Point3d& p);
bool LineToBigCircles(const Line& lin, const Point3d& z_axis, Circle& c1, Circle& c2);

//Point3d make_point(const double* p);
//Point3d make_vector(const Point3d &p1, const Point3d &p2);
//Point3d make_vector(const double* v);
//Line make_line(const Point3d &p1, const Point3d &p2);
//Line make_line(const Point3d &p, const Point3d &v);

//Matrix make_matrix(const double* m);
//Matrix make_matrix(const Point3d &origin, const Point3d &x_axis, const Point3d &y_axis);

//void add_pnt_to_doubles(const Point3d& pnt, std::list<double> &dlist);
//int convert_pnts_to_doubles(const std::list<Point3d> &plist, std::list<double> &dlist);
//bool make_point_from_doubles(const std::list<double> &dlist, std::list<double>::const_iterator &It, Point3d& pnt, bool four_doubles = false);
//int convert_doubles_to_pnts(const std::list<double> &dlist, std::list<Point3d> &plist, bool four_doubles = false);
//int convert_gripdata_to_pnts(const std::list<GripData> &dlist, std::list<Point3d> &plist);

//bool IsEqual(gp_Ax2 ax1, gp_Ax2 ax2);
//bool IsEqual(gp_Ax1 ax1, gp_Ax1 ax2);

