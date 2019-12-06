// Geom.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "Geom.h"

#define GEOM_TOL TOLERANCE


void ClosestPointsLineAndCircle(const Line& line, const Circle& circle, std::list<Point3d> &list)
{
	// just do 2D equations, I can't work out the 3D case

	// the points will always be somewhere on the circle

	if(fabs(line.v.Normalized().z) > 0.4){
		// line is a bit perpendicular to plane of circle
		// just consider the point where the line passes through the plane of the circle

		Plane pl(Point3d(circle.pc), Point3d(0,0,1));
		Point3d plane_point;
		double t;
		if (!pl.Intof(line, plane_point, t))return;

		double dist_to_centre = plane_point.Dist(Point3d(circle.pc));

		if(dist_to_centre < GEOM_TOL)return;

		// just use the closest point in the direction of the closest point on line
		Point3d p(Point3d(circle.pc) + (plane_point - Point3d(circle.pc)).Normalized() * circle.radius);
		list.push_back(p);
	}
	else{
		// line is mostly flat in the plane of the circle

		Point3d to_centre = ClosestPointOnLine(line, Point3d(circle.pc));
		double dist_to_centre = to_centre.Dist(Point3d(circle.pc));
		if(dist_to_centre > circle.radius - GEOM_TOL)
		{
			// just use the closest point in the direction of the closest point on line
			Point3d p(Point3d(circle.pc) + (to_centre - Point3d(circle.pc)).Normalized() * circle.radius);
			list.push_back(p);
		}
		else
		{

			// from geoff's geometry

			// solving	x = x0 + dx * t			x = y0 + dy * t
			//			x = xc + R * cos(a)		y = yc + R * sin(a)		for t
			// gives :-  t� (dx� + dy�) + 2t(dx*dx0 + dy*dy0) + (x0-xc)� + (y0-yc)� - R� = 0

			Point3d lv = line.v.Normalized();

			// do the equation in the plane of the circle with it's centre being x0, y0
			Point3d local_line_location(line.p0 - Point3d(circle.pc));
			double x0 = local_line_location.x;
			double y0 = local_line_location.y;

			// flatten line direction
			lv.z=0.0;
			lv.Normalize();

			double dx = lv.x;
			double dy = lv.y;
			double R = circle.radius;

			double a = dx * dx + dy * dy;
			double b = 2 * (dx* x0 + dy * y0);
			double c = x0 * x0 + y0 * y0 - R * R;

			// t = (-b +- sqrt(b*b - 4*a*c))/(2*a)
			double sq = sqrt(b*b - 4*a*c);
			double t1 = (-b + sq)/(2*a);
			double t2 = (-b - sq)/(2*a);

			Point3d p1(circle.pc.x + x0 + t1 * dx, circle.pc.y + y0 + t1 * dy, 0);
			Point3d p2(circle.pc.x + x0 + t2 * dx, circle.pc.y + y0 + t2 * dy, 0);

			list.push_back(p1);
			list.push_back(p2);
		}
	}
}

void TangentCircles(const Point3d& p1, const Point3d& p2, const Point3d& p3, std::list<Circle>& c_list)
{
	Circle c1 = PointToCircle(p1);
	Circle c2 = PointToCircle(p2);
	Circle c3 = PointToCircle(p3);

	TangentCircles(c1, c2, c3, c_list);
}

void TangentCircles(const Point3d& p1, const Point3d& p2, const Line& l3, std::list<Circle>& c_list)
{
	Circle c1 = PointToCircle(p1);
	Circle c2 = PointToCircle(p2);
	Circle c3, c3b;
	if(!LineToBigCircles(l3, Point3d(0, 0, 1), c3, c3b))return;
	TangentCircles(c1, c2, c3, c_list);
	TangentCircles(c1, c2, c3b, c_list);
}

void TangentCircles(const Point3d& p1, const Point3d& p2, const Circle& c3, std::list<Circle>& c_list)
{
	Circle c1 = PointToCircle(p1);
	Circle c2 = PointToCircle(p2);

	TangentCircles(c1, c2, c3, c_list);
}

void TangentCircles(const Point3d& p1, const Line& l2, const Point3d& p3, std::list<Circle>& c_list)
{
	TangentCircles(p1, p3, l2, c_list);
}

void TangentCircles(const Point3d& p1, const Line& l2, const Line& l3, std::list<Circle>& c_list)
{
	Circle c1 = PointToCircle(p1);
	Circle c2, c2b;
	if(!LineToBigCircles(l2, Point3d(0, 0, 1), c2, c2b))return;
	Circle c3, c3b;
	if(!LineToBigCircles(l3, Point3d(0, 0, 1), c3, c3b))return;

	TangentCircles(c1, c2, c3, c_list);
	TangentCircles(c1, c2, c3b, c_list);
	TangentCircles(c1, c2b, c3, c_list);
	TangentCircles(c1, c2b, c3b, c_list);
}

void TangentCircles(const Point3d& p1, const Line& l2, const Circle& c3, std::list<Circle>& c_list)
{
	TangentCircles(c3, l2, p1, c_list);
}

void TangentCircles(const Point3d& p1, const Circle& c2, const Point3d& p3, std::list<Circle>& c_list)
{
	TangentCircles(p1, p3, c2, c_list);
}

void TangentCircles(const Point3d& p1, const Circle& c2, const Line& l3, std::list<Circle>& c_list)
{
	TangentCircles(c2, l3, p1, c_list);
}

void TangentCircles(const Point3d& p1, const Circle& c2, const Circle& c3, std::list<Circle>& c_list)
{
	Circle c1 = PointToCircle(p1);

	TangentCircles(c1, c2, c3, c_list);
}

void TangentCircles(const Line& l1, const Point3d& p2, const Point3d& p3, std::list<Circle>& c_list)
{
	TangentCircles(p2, p3, l1, c_list);
}

void TangentCircles(const Line& l1, const Point3d& p2, const Line& l3, std::list<Circle>& c_list)
{
	TangentCircles(p2, l1, l3, c_list);
}

void TangentCircles(const Line& l1, const Point3d& p2, const Circle& c3, std::list<Circle>& c_list)
{
	TangentCircles(c3, l1, p2, c_list);
}

void TangentCircles(const Line& l1, const Line& l2, const Point3d& p3, std::list<Circle>& c_list)
{
	TangentCircles(p3, l1, l2, c_list);
}

void TangentCircles(const Line& l1, const Line& l2, const Line& l3, std::list<Circle>& c_list)
{
	Circle c1, c1b;
	if(!LineToBigCircles(l1, Point3d(0, 0, 1), c1, c1b))return;
	Circle c2, c2b;
	if(!LineToBigCircles(l2, Point3d(0, 0, 1), c2, c2b))return;
	Circle c3, c3b;
	if(!LineToBigCircles(l3, Point3d(0, 0, 1), c3, c3b))return;

	TangentCircles(c1, c2, c3, c_list);
	TangentCircles(c1, c2, c3b, c_list);
	TangentCircles(c1, c2b, c3, c_list);
	TangentCircles(c1, c2b, c3b, c_list);
	TangentCircles(c1b, c2, c3, c_list);
	TangentCircles(c1b, c2, c3b, c_list);
	TangentCircles(c1b, c2b, c3, c_list);
	TangentCircles(c1b, c2b, c3b, c_list);
}

void TangentCircles(const Line& l1, const Line& l2, const Circle& c3, std::list<Circle>& c_list)
{
	Circle c1, c1b;
	if(!LineToBigCircles(l1, Point3d(0, 0, 1), c1, c1b))return;
	Circle c2, c2b;
	if(!LineToBigCircles(l2, Point3d(0, 0, 1), c2, c2b))return;

	TangentCircles(c1, c2, c3, c_list);
	TangentCircles(c1, c2b, c3, c_list);
	TangentCircles(c1b, c2, c3, c_list);
	TangentCircles(c1b, c2b, c3, c_list);
}

void TangentCircles(const Line& l1, const Circle& c2, const Point3d& p3, std::list<Circle>& c_list)
{
	TangentCircles(c2, l1, p3, c_list);
}

void TangentCircles(const Line& l1, const Circle& c2, const Line& l3, std::list<Circle>& c_list)
{
	TangentCircles(l1, l3, c2, c_list);
}

void TangentCircles(const Line& l1, const Circle& c2, const Circle& c3, std::list<Circle>& c_list)
{
	Circle c1, c1b;
	if(!LineToBigCircles(l1, Point3d(0, 0, 1), c1, c1b))return;

	TangentCircles(c1, c2, c3, c_list);
	TangentCircles(c1b, c2, c3, c_list);
}

void TangentCircles(const Circle& c1, const Point3d& p2, const Point3d& p3, std::list<Circle>& c_list)
{
	TangentCircles(p2, p3, c1, c_list);
}

void TangentCircles(const Circle& c1, const Point3d& p2, const Line& l3, std::list<Circle>& c_list)
{
	TangentCircles(c1, l3, p2, c_list);
}

void TangentCircles(const Circle& c1, const Point3d& p2, const Circle& c3, std::list<Circle>& c_list)
{
	TangentCircles(p2, c1, c3, c_list);
}

void TangentCircles(const Circle& c1, const Line& l2, const Point3d& p3, std::list<Circle>& c_list)
{
	Circle c2, c2b;
	if(!LineToBigCircles(l2, Point3d(0,0,1), c2, c2b))return;
	Circle c3 = PointToCircle(p3);

	TangentCircles(c1, c2, c3, c_list);
	TangentCircles(c1, c2b, c3, c_list);
}

void TangentCircles(const Circle& c1, const Line& l2, const Line& l3, std::list<Circle>& c_list)
{
	TangentCircles(l2, l3, c1, c_list);
}

void TangentCircles(const Circle& c1, const Line& l2, const Circle& c3, std::list<Circle>& c_list)
{
	TangentCircles(l2, c1, c3, c_list);
}

void TangentCircles(const Circle& c1, const Circle& c2, const Point3d& p3, std::list<Circle>& c_list)
{
	TangentCircles(p3, c1, c2, c_list);
}

void TangentCircles(const Circle& c1, const Circle& c2, const Line& l3, std::list<Circle>& c_list)
{
	TangentCircles(l3, c1, c2, c_list);
}

void TangentCircles(const Circle& c1, const Circle& c2, const Circle& c3, std::list<Circle>& c_list)
{
	// from http://mathworld.wolfram.com/ApolloniusProblem.html

	double x1 = c1.pc.x;
	double y1 = c1.pc.y;
	double x2 = c2.pc.x;
	double y2 = c2.pc.y;
	double x3 = c3.pc.x;
	double y3 = c3.pc.y;

	// x = (Bd - bD - Bcr + bCr) / (aB - bA);  ( 11 )
	// y = (-Ad + aD + Acr - aCr) / (aB - Ab); ( 12 )

	for(int p1 = 0; p1<2; p1++){ // use +ve r1 or -ve r1

		for(int p2 = 0; p2<2; p2++){ // use +ve r2 or -ve r2
			for(int p3 = 0; p3<2; p3++){ // use +ve r3 or -ve r3

				double r1 = (p1 == 0) ? c1.radius : (-c1.radius);
				double r2 = (p2 == 0) ? c2.radius : (-c2.radius);
				double r3 = (p3 == 0) ? c3.radius : (-c3.radius);

				// checking the math

				// a*x + b*y + c*r = d
				// x = (d - b*y - c*r)/a
				// y = (d - a*x - c*r)/b

				// A*x + B*y + C*r = D
				// A*(d - b*y - c*r)/a + B*y + C*r = D
				// A/a*(d - c*r) + C*r - A/a*b*y + B*y = D
				// - A/a*b*y + B*y = D - A/a*(d - c*r) - C*r 
				// y = (D - A/a*(d - c*r) - C*r)/(B - A/a*b)
				// y = (D*a - A*(d - c*r) - C*r*a)/(B*a - A*b)
				// y = (D*a - A*d + A*c*r - C*r*a)/(B*a - A*b)

				// A*x + B*y + C*r = D
				// A*x + B*(d - a*x - c*r)/b + C*r = D
				// A*x + B/b*(d - c*r) - B*a*x/b + C*r = D
				// A*x - B*a*x/b = D - B/b*(d - c*r) - C*r
				// (A - B*a/b)*x = D - B/b*(d - c*r) - C*r
				// x = (D - B/b*(d - c*r) - C*r)/(A - B*a/b)
				// x = (D*b - B*(d - c*r) - C*r*b)/(A*b - B*a)
				// x = (D*b - B*d + B*c*r - C*r*b)/(A*b - B*a)
				// x = (-D*b + B*d - B*c*r + C*r*b)/(B*a - A*b)
				// x = (B*d -D*b - B*c*r + C*r*b)/(B*a - A*b)

				double a = 2 * (x1 - x2);
				double b = 2 * (y1 - y2);
				double c = 2 * (r1 - r2);
				double d = (x1 * x1 + y1 * y1 - r1 * r1) - (x2 * x2 + y2 * y2 - r2 * r2);

				double A = 2 * (x1 - x3);
				double B = 2 * (y1 - y3);
				double C = 2 * (r1 - r3);
				double D = (x1 * x1 + y1 * y1 - r1 * r1) - (x3 * x3 + y3 * y3 - r3 * r3);

				double aBmbA = (a*B - b*A); // aB - bA

				if(fabs(aBmbA) < 0.000000000000001 )continue;

				// x = k + Kr where
				double k = (B*d - b*D) / aBmbA;
				double K = (-B*c + b*C) / aBmbA;

				// y = l + Lr where
				double l = (-A*d + a*D)/ aBmbA;
				double L = (A*c - a*C) / aBmbA;

				// which can then be plugged back into the quadratic equation ( 1 )
				// (x - x1)*(x - x1) + (y - y1)*(y - y1) - (r + r1)*(r + r1) = 0;

				// expanded
				// x*x + x1*x1 -2*x*x1 + y*y+ y1*y1 - 2*y*y1 - r*r - r1*r1 - 2*r*r1 = 0;
				// with x and y from ( 11 ) and ( 12 )
				// (k + K*r) * (k + K*r) + x1*x1 -2*(k + K*r)*x1 + (l + Lr)*(l + Lr)+ y1*y1 - 2*(l + Lr)*y1 - r*r - r1*r1 - 2*r*r1 = 0;
				// k*k + K*K*r*r + 2*k*K*r + x1*x1 -2*k*x1 -2*K*x1*r + l*l + L*L*r*r + 2*l*L*r + y1*y1 - 2*l*y1 - 2*L*y1*r - r*r - r1*r1 - 2*r*r1 = 0;
				// collected as components of r*r and r
				//  + K*K*r*r + L*L*r*r - r*r + 2*k*K*r -2*K*x1*r + 2*l*L*r - 2*L*y1*r - 2*r*r1 + k*k + x1*x1 -2*k*x1 + l*l + y1*y1 - 2*l*y1 - r1*r1 = 0;
				// quadratic components ( usually called a, b, c ) qa, qb, qc
				double qa = K*K + L*L - 1;
				double qb = 2*k*K - 2*K*x1 + 2*l*L - 2*L*y1 - 2*r1;
				double qc = k*k + x1*x1 -2*k*x1 + l*l + y1*y1 - 2*l*y1 - r1*r1;

				// solve the quadratic equation, r = (-b +- sqrt(b*b - 4*a*c))/(2 * a)
				if(fabs(qa) < 0.000000000000001 )continue;

				for(int qs = 0; qs<2; qs++){
					double bb = qb*qb;
					double ac4 = 4*qa*qc;
					if(ac4 <= bb){
						double r = (-qb + ((qs == 0) ? 1 : -1) * sqrt(bb - ac4))/(2 * qa);
						double x = k + K * r;
						double y = l + L * r;

						// add the circle
						if(r >= 0.0){
							c_list.push_back(Circle(Point(x, y), fabs(r)));
						}
					}
				}
			}
		}
	}
}

void TangentCircles(const PointLineOrCircle& plc1, const PointLineOrCircle& plc2, const PointLineOrCircle& plc3, std::list<Circle>& c_list)
{
	switch(plc1.type)
	{
	case PLC_Point:
		switch(plc2.type)
		{
		case PLC_Point:
			switch(plc3.type)
			{
			case PLC_Point:
				TangentCircles(plc1.p, plc2.p, plc3.p, c_list);
				break;
			case PLC_Line:
				TangentCircles(plc1.p, plc2.p, plc3.l, c_list);
				break;
			case PLC_Circle:
				TangentCircles(plc1.p, plc2.p, plc3.c, c_list);
				break;
			case PLC_TwoCircle:
				TangentCircles(plc1.p, plc2.p, plc3.c, c_list);
				TangentCircles(plc1.p, plc2.p, plc3.c2, c_list);
				break;
			}
			break;
		case PLC_Line:
			switch(plc3.type)
			{
			case PLC_Point:
				TangentCircles(plc1.p, plc2.l, plc3.p, c_list);
				break;
			case PLC_Line:
				TangentCircles(plc1.p, plc2.l, plc3.l, c_list);
				break;
			case PLC_Circle:
				TangentCircles(plc1.p, plc2.l, plc3.c, c_list);
				break;
			case PLC_TwoCircle:
				TangentCircles(plc1.p, plc2.l, plc3.c, c_list);
				TangentCircles(plc1.p, plc2.l, plc3.c2, c_list);
				break;
			}
			break;
		case PLC_Circle:
			switch(plc3.type)
			{
			case PLC_Point:
				TangentCircles(plc1.p, plc2.c, plc3.p, c_list);
				break;
			case PLC_Line:
				TangentCircles(plc1.p, plc2.c, plc3.l, c_list);
				break;
			case PLC_Circle:
				TangentCircles(plc1.p, plc2.c, plc3.c, c_list);
				break;
			case PLC_TwoCircle:
				TangentCircles(plc1.p, plc2.c, plc3.c, c_list);
				TangentCircles(plc1.p, plc2.c, plc3.c2, c_list);
				break;
			}
			break;
		case PLC_TwoCircle:
			switch(plc3.type)
			{
			case PLC_Point:
				TangentCircles(plc1.p, plc2.c, plc3.p, c_list);
				TangentCircles(plc1.p, plc2.c2, plc3.p, c_list);
				break;
			case PLC_Line:
				TangentCircles(plc1.p, plc2.c, plc3.l, c_list);
				TangentCircles(plc1.p, plc2.c2, plc3.l, c_list);
				break;
			case PLC_Circle:
				TangentCircles(plc1.p, plc2.c, plc3.c, c_list);
				TangentCircles(plc1.p, plc2.c2, plc3.c, c_list);
				break;
			case PLC_TwoCircle:
				TangentCircles(plc1.p, plc2.c, plc3.c, c_list);
				TangentCircles(plc1.p, plc2.c, plc3.c2, c_list);
				TangentCircles(plc1.p, plc2.c2, plc3.c, c_list);
				TangentCircles(plc1.p, plc2.c2, plc3.c2, c_list);
				break;
			}
			break;
		}
		break;
	case PLC_Line:
		switch(plc2.type)
		{
		case PLC_Point:
			switch(plc3.type)
			{
			case PLC_Point:
				TangentCircles(plc1.l, plc2.p, plc3.p, c_list);
				break;
			case PLC_Line:
				TangentCircles(plc1.l, plc2.p, plc3.l, c_list);
				break;
			case PLC_Circle:
				TangentCircles(plc1.l, plc2.p, plc3.c, c_list);
				break;
			case PLC_TwoCircle:
				TangentCircles(plc1.l, plc2.p, plc3.c, c_list);
				TangentCircles(plc1.l, plc2.p, plc3.c2, c_list);
				break;
			}
			break;
		case PLC_Line:
			switch(plc3.type)
			{
			case PLC_Point:
				TangentCircles(plc1.l, plc2.l, plc3.p, c_list);
				break;
			case PLC_Line:
				TangentCircles(plc1.l, plc2.l, plc3.l, c_list);
				break;
			case PLC_Circle:
				TangentCircles(plc1.l, plc2.l, plc3.c, c_list);
				break;
			case PLC_TwoCircle:
				TangentCircles(plc1.l, plc2.l, plc3.c, c_list);
				TangentCircles(plc1.l, plc2.l, plc3.c2, c_list);
				break;
			}
			break;
		case PLC_Circle:
			switch(plc3.type)
			{
			case PLC_Point:
				TangentCircles(plc1.l, plc2.c, plc3.p, c_list);
				break;
			case PLC_Line:
				TangentCircles(plc1.l, plc2.c, plc3.l, c_list);
				break;
			case PLC_Circle:
				TangentCircles(plc1.l, plc2.c, plc3.c, c_list);
				break;
			case PLC_TwoCircle:
				TangentCircles(plc1.l, plc2.c, plc3.c, c_list);
				TangentCircles(plc1.l, plc2.c, plc3.c2, c_list);
				break;
			}
			break;
		case PLC_TwoCircle:
			switch(plc3.type)
			{
			case PLC_Point:
				TangentCircles(plc1.l, plc2.c, plc3.p, c_list);
				TangentCircles(plc1.l, plc2.c2, plc3.p, c_list);
				break;
			case PLC_Line:
				TangentCircles(plc1.l, plc2.c, plc3.l, c_list);
				TangentCircles(plc1.l, plc2.c2, plc3.l, c_list);
				break;
			case PLC_Circle:
				TangentCircles(plc1.l, plc2.c, plc3.c, c_list);
				TangentCircles(plc1.l, plc2.c2, plc3.c, c_list);
				break;
			case PLC_TwoCircle:
				TangentCircles(plc1.l, plc2.c, plc3.c, c_list);
				TangentCircles(plc1.l, plc2.c, plc3.c2, c_list);
				TangentCircles(plc1.l, plc2.c2, plc3.c, c_list);
				TangentCircles(plc1.l, plc2.c2, plc3.c2, c_list);
				break;
			}
			break;
		}
		break;
	case PLC_Circle:
		switch(plc2.type)
		{
		case PLC_Point:
			switch(plc3.type)
			{
			case PLC_Point:
				TangentCircles(plc1.c, plc2.p, plc3.p, c_list);
				break;
			case PLC_Line:
				TangentCircles(plc1.c, plc2.p, plc3.l, c_list);
				break;
			case PLC_Circle:
				TangentCircles(plc1.c, plc2.p, plc3.c, c_list);
				break;
			case PLC_TwoCircle:
				TangentCircles(plc1.c, plc2.p, plc3.c, c_list);
				TangentCircles(plc1.c, plc2.p, plc3.c2, c_list);
				break;
			}
			break;
		case PLC_Line:
			switch(plc3.type)
			{
			case PLC_Point:
				TangentCircles(plc1.c, plc2.l, plc3.p, c_list);
				break;
			case PLC_Line:
				TangentCircles(plc1.c, plc2.l, plc3.l, c_list);
				break;
			case PLC_Circle:
				TangentCircles(plc1.c, plc2.l, plc3.c, c_list);
				break;
			case PLC_TwoCircle:
				TangentCircles(plc1.c, plc2.l, plc3.c, c_list);
				TangentCircles(plc1.c, plc2.l, plc3.c2, c_list);
				break;
			}
			break;
		case PLC_Circle:
			switch(plc3.type)
			{
			case PLC_Point:
				TangentCircles(plc1.c, plc2.c, plc3.p, c_list);
				break;
			case PLC_Line:
				TangentCircles(plc1.c, plc2.c, plc3.l, c_list);
				break;
			case PLC_Circle:
				TangentCircles(plc1.c, plc2.c, plc3.c, c_list);
				break;
			case PLC_TwoCircle:
				TangentCircles(plc1.c, plc2.c, plc3.c, c_list);
				TangentCircles(plc1.c, plc2.c, plc3.c2, c_list);
				break;
			}
			break;
		case PLC_TwoCircle:
			switch(plc3.type)
			{
			case PLC_Point:
				TangentCircles(plc1.c, plc2.c, plc3.p, c_list);
				TangentCircles(plc1.c, plc2.c2, plc3.p, c_list);
				break;
			case PLC_Line:
				TangentCircles(plc1.c, plc2.c, plc3.l, c_list);
				TangentCircles(plc1.c, plc2.c2, plc3.l, c_list);
				break;
			case PLC_Circle:
				TangentCircles(plc1.c, plc2.c, plc3.c, c_list);
				TangentCircles(plc1.c, plc2.c2, plc3.c, c_list);
				break;
			case PLC_TwoCircle:
				TangentCircles(plc1.c, plc2.c, plc3.c, c_list);
				TangentCircles(plc1.c, plc2.c, plc3.c2, c_list);
				TangentCircles(plc1.c, plc2.c2, plc3.c, c_list);
				TangentCircles(plc1.c, plc2.c2, plc3.c2, c_list);
				break;
			}
			break;
		}
		break;
	case PLC_TwoCircle:
		switch(plc2.type)
		{
		case PLC_Point:
			switch(plc3.type)
			{
			case PLC_Point:
				TangentCircles(plc1.c, plc2.p, plc3.p, c_list);
				TangentCircles(plc1.c2, plc2.p, plc3.p, c_list);
				break;
			case PLC_Line:
				TangentCircles(plc1.c, plc2.p, plc3.l, c_list);
				TangentCircles(plc1.c2, plc2.p, plc3.l, c_list);
				break;
			case PLC_Circle:
				TangentCircles(plc1.c, plc2.p, plc3.c, c_list);
				TangentCircles(plc1.c2, plc2.p, plc3.c, c_list);
				break;
			case PLC_TwoCircle:
				TangentCircles(plc1.c, plc2.p, plc3.c, c_list);
				TangentCircles(plc1.c, plc2.p, plc3.c2, c_list);
				TangentCircles(plc1.c2, plc2.p, plc3.c, c_list);
				TangentCircles(plc1.c2, plc2.p, plc3.c2, c_list);
				break;
			}
			break;
		case PLC_Line:
			switch(plc3.type)
			{
			case PLC_Point:
				TangentCircles(plc1.c, plc2.l, plc3.p, c_list);
				TangentCircles(plc1.c2, plc2.l, plc3.p, c_list);
				break;
			case PLC_Line:
				TangentCircles(plc1.c, plc2.l, plc3.l, c_list);
				TangentCircles(plc1.c2, plc2.l, plc3.l, c_list);
				break;
			case PLC_Circle:
				TangentCircles(plc1.c, plc2.l, plc3.c, c_list);
				TangentCircles(plc1.c2, plc2.l, plc3.c, c_list);
				break;
			case PLC_TwoCircle:
				TangentCircles(plc1.c, plc2.l, plc3.c, c_list);
				TangentCircles(plc1.c, plc2.l, plc3.c2, c_list);
				TangentCircles(plc1.c2, plc2.l, plc3.c, c_list);
				TangentCircles(plc1.c2, plc2.l, plc3.c2, c_list);
				break;
			}
			break;
		case PLC_Circle:
			switch(plc3.type)
			{
			case PLC_Point:
				TangentCircles(plc1.c, plc2.c, plc3.p, c_list);
				TangentCircles(plc1.c2, plc2.c, plc3.p, c_list);
				break;
			case PLC_Line:
				TangentCircles(plc1.c, plc2.c, plc3.l, c_list);
				TangentCircles(plc1.c2, plc2.c, plc3.l, c_list);
				break;
			case PLC_Circle:
				TangentCircles(plc1.c, plc2.c, plc3.c, c_list);
				TangentCircles(plc1.c2, plc2.c, plc3.c, c_list);
				break;
			case PLC_TwoCircle:
				TangentCircles(plc1.c, plc2.c, plc3.c, c_list);
				TangentCircles(plc1.c, plc2.c, plc3.c2, c_list);
				TangentCircles(plc1.c2, plc2.c, plc3.c, c_list);
				TangentCircles(plc1.c2, plc2.c, plc3.c2, c_list);
				break;
			}
			break;
		case PLC_TwoCircle:
			switch(plc3.type)
			{
			case PLC_Point:
				TangentCircles(plc1.c, plc2.c, plc3.p, c_list);
				TangentCircles(plc1.c, plc2.c2, plc3.p, c_list);
				TangentCircles(plc1.c2, plc2.c, plc3.p, c_list);
				TangentCircles(plc1.c2, plc2.c2, plc3.p, c_list);
				break;
			case PLC_Line:
				TangentCircles(plc1.c, plc2.c, plc3.l, c_list);
				TangentCircles(plc1.c, plc2.c2, plc3.l, c_list);
				TangentCircles(plc1.c2, plc2.c, plc3.l, c_list);
				TangentCircles(plc1.c2, plc2.c2, plc3.l, c_list);
				break;
			case PLC_Circle:
				TangentCircles(plc1.c, plc2.c, plc3.c, c_list);
				TangentCircles(plc1.c, plc2.c2, plc3.c, c_list);
				TangentCircles(plc1.c2, plc2.c, plc3.c, c_list);
				TangentCircles(plc1.c2, plc2.c2, plc3.c, c_list);
				break;
			case PLC_TwoCircle:
				TangentCircles(plc1.c, plc2.c, plc3.c, c_list);
				TangentCircles(plc1.c, plc2.c, plc3.c2, c_list);
				TangentCircles(plc1.c, plc2.c2, plc3.c, c_list);
				TangentCircles(plc1.c, plc2.c2, plc3.c2, c_list);
				TangentCircles(plc1.c2, plc2.c, plc3.c, c_list);
				TangentCircles(plc1.c2, plc2.c, plc3.c2, c_list);
				TangentCircles(plc1.c2, plc2.c2, plc3.c, c_list);
				TangentCircles(plc1.c2, plc2.c2, plc3.c2, c_list);
				break;
			}
			break;
		}
		break;
	}
}

Circle PointToCircle(const Point3d& p)
{
	return Circle(Point(p.x, p.y), 0.00000000001);
}

bool LineToBigCircles(const Line& lin, const Point3d& z_axis, Circle& c1, Circle& c2)
{
	double dp = z_axis * lin.v.Normalized();
	if(dp > 0.999999999999)return false;
	Point3d left = z_axis ^ lin.v.Normalized();
	double r = 10000000000.0;
	Point3d centre1 = lin.p0 + left * r;
	c1 = Circle(Point(centre1.x, centre1.y), r);
	Point3d centre2 = lin.p0 - left * r;
	c2 = Circle(Point(centre2.x, centre2.y), r);
	return true;
}

void intersect(const Circle& c1, const Circle& c2, std::list<Point> &list)
{
	// from  http://local.wasp.uwa.edu.au/~pbourke/geometry/2circle/
	// a = (r0 * r0 - r1 * r1 + d * d ) / (2 * d)

	Point join(c2.pc - c1.pc);
	double d = join.magnitude();
	if(d < 0.00000000000001)return;
	double r0 = c1.radius;
	double r1 = c2.radius;
	if(r0 + r1 - d < -GEOM_TOL)return;// circles too far from each other
	if(fabs(r0 - r1) > d + GEOM_TOL)return; // one circle is within the other

	Point forward = join;
	forward.normalize();
	Point left = ~forward;

	if(r0 + r1 - d <= GEOM_TOL){
		list.push_back(c1.pc + forward * r0);
	}
	else if(fabs(fabs(r0 - r1) - d) < GEOM_TOL){
		if(r0 > r1)list.push_back(c1.pc + forward * r0);
		else list.push_back(c1.pc - forward * r0);
	}
	else{
		double a = (r0 * r0 - r1 * r1 + d * d ) / (2 * d);
		double h = sqrt(r0 * r0 - a * a);
		list.push_back(c1.pc + forward * a + left * h);
		list.push_back(c1.pc + forward * a - left * h);
	}
}

Point3d ClosestPointOnLine(const Line& line, const Point3d &p){
	// returns closest vertex on line to point pt 
	double t;
	return line.Near(p, t);
}
