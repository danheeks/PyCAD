// written by g.j.hawkesford 2006 for Camtek Gmbh
//
// This program is released under the BSD license. See the file COPYING for details.
//

#include "geometry.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//            finite intersections
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef WIN32
#define __min(a,b) ((a<b)?a:b)
#define __max(a,b) ((a>b)?a:b)
#endif


	bool Plane::On(const Point3d &p)const
	{
		Point3d near = Near(p);
		return near.Dist(p) < TOLERANCE;
	}

	Line::Line(const Point3d& p, const Point3d& p1){
		// constructor from 2 points
		p0 = p;
		v = Point3d(p, p1);
		length = v.magnitude();
		minmax();
		ok = (length > TOLERANCE);
	}

	void Line::minmax() {
		MinMax(this->p0, box.min, box.max);
		MinMax(this->v + this->p0, box.min, box.max);
	}

	bool Line::atZ(double z, Point3d& p)const {
		// returns p at z on line
		if(FEQZ(this->v.z)) return false;
		double t = (z - this->p0.z) / this->v.z;
		p = Point3d(this->p0.x + t * this->v.x, this->p0.y + t * this->v.y, z);
		return true;
	}


	bool Line::Shortest(const Line& l2, Line& lshort, double& t1, double& t2)const {
	/*
	Calculate the line segment PaPb that is the shortest route between
	two lines P1P2 and P3P4. Calculate also the values of mua and mub where
	Pa = P1 + t1 (P2 - P1)
	Pb = P3 + t2 (P4 - P3)
	Return FALSE if no solution exists.       P Bourke method.
		Input this 1st line
		Input l2   2nd line
		Output lshort shortest line between lines (if lshort.ok == false, the line intersect at a point lshort.p0)
		Output t1 parameter at intersection on 1st Line
		Output t2 parameter at intersection on 2nd Line

	*/
		Point3d v13(l2.p0, this->p0);
		if(this->ok == false || l2.ok == false) return false;

		double d1343 = v13 * l2.v;		// dot products
		double d4321 = l2.v * this->v;
		double d1321 = v13 * this->v;
		double d4343 = l2.v * l2.v;
		double d2121 = this->v * this->v;

		double denom = d2121 * d4343 - d4321 * d4321;
		if(fabs(denom) < 1.0e-09) return false;
		double numer = d1343 * d4321 - d1321 * d4343;

		t1 = numer / denom;
		t2 = (d1343 + d4321 * t1) / d4343;

		lshort = Line(this->v*t1 + this->p0, l2.v* t2 + l2.p0);
		t1 *= this->length;
		t2 *= l2.length;		// parameter in line length for tolerance checking
		return true;
	}

	int Intof(const Line& l0, const Line& l1, Point3d& intof)
	{
		/* intersection of 2 vectors
		returns 0 for  intercept but not within either vector
		returns 1 for intercept on both vectors

		note that this routine always returns 0 for parallel vectors
		method:
		x = x0 + dx0 * t0	for l0
		...
		...
		x = x1 + dx1 * t1	for l1
		...
		...

		x0 + dx0 * t0 = x1 + dx1 * t1
		dx0 * t0 - dx1 * t1 + x0 - x1 = 0

		setup 3 x 3 determinent for 
		a0 t0 + b0 t1 + c0 = 0
		a1 t0 + b1 t1 + c1 = 0
		a2 t0 + b2 t1 + c2 = 0

		from above a = l0.v
		b = -l1.v
		c = Point3d(l1, l0)
		*/
		//	Point3d a = l0.v;
		if(l0.box.outside(l1.box) == true) return 0;
		Point3d b = -l1.v;
		Point3d c = Point3d(l1.p0, l0.p0);
		Point3d det = l0.v ^ b;
		Point3d t = b ^ c;

		// choose largest determinant & corresponding parameter for accuracy
		double t0 = t.x;
		double d  = det.x;

		if(fabs(det.z) > fabs(det.y)) {
			if(fabs(det.z) > fabs(det.x)) {
				t0 = t.z;
				d = det.z;
			}
		}
		else {
			if(fabs(det.y) > fabs(det.x)) {
				t0 = t.y;
				d = det.y;
			}
		}

		if(fabs(d) < 1.0e-06) return 0;

		t0 /= d;
		intof = l0.v * t0 + l0.p0;

		Point3d other;
		double t1;
		if(Dist(l1, intof, other, t1) > TOLERANCE) return 0;

		t0 *= l0.length;
		if( t0 < -TOLERANCE || t0 > l0.length + TOLERANCE || t1 < -TOLERANCE || t1 > l1.length + TOLERANCE ) return 0;
		return 1;
	}


	double Dist(const Line& l, const Point3d& p, Point3d& pnear, double& t){
		// returns the distance of a point from a line and the near point on the extended line and the parameter of the near point (0-length) in range
		pnear = Near(l, p, t );
		return p.Dist(pnear);
	}

	Point3d Near(const Line& l, const Point3d& p, double& t){
		// returns the near point from a line on the extended line and the parameter of the near point (0-length) in range
		t = (Point3d(l.p0, p) * l.v) / l.length;		// t parametised 0 - line length
		return l.v * (t / l.length) + l.p0;
	}

	Point3d Line::Near(const Point3d& p, double& t)const{
		// returns the near point from a line on the extended line and the parameter of the near point (0-length) in range
		t = (Point3d(this->p0, p) * this->v) / this->length;		// t parametised 0 - line length
		return this->v * (t / this->length) + this->p0;
	}

	bool Line::Intof(const Point3d& p)const
	{
		double d;
		Point3d n = Near(p, d);
		if (n == p)return true;
		return false;
	}

	bool Line::IntofFinite(const Point3d& p)const
	{
		double d;
		Point3d n = Near(p, d);
		if (n.Dist(p) < TOLERANCE)
			return d > -TOLERANCE && d < this->length + TOLERANCE;
		return false;
	}

	int Line::IntofFinite(const Line& l, Point3d& intof, Point3d& other_intof)const
	{
		// check for parallel lines
		if (Intof(l.p0) && Intof(l.p0 + l.v))
		{
			// parallel
			Point3d dir = v;
			dir.normalise();
			double d0 = Point3d(p0) * dir;
			double d1 = Point3d(p0 + v) * dir;
			double d2 = Point3d(l.p0) * dir;
			double d3 = Point3d(l.p0 + l.v) * dir;

			double dorig = d0;
			if (d2 > d3)
			{
				double temp = d2;
				d2 = d3;
				d3 = temp;
			}

			if (d2 > d1 + TOLERANCE)
				return 0;
			if (d3 < d0 - TOLERANCE)
				return 0;
			if (d2 > d0)
				d0 = d2;
			if (d3 < d1)
				d1 = d3;

			Point3d ps = p0 + (dir * (d0 - dorig));
			Point3d pe = p0 + (dir * (d1 - dorig));
			if (ps == pe)
			{
				intof = ps;
				return 1;
			}

			intof = ps;
			other_intof = pe;

			return 2;
		}
		else if (Intof(l, intof))
		{
			if (IntofFinite(intof))
			{
				if (l.IntofFinite(intof))
				{
					return 1;
				}
			}
		}
		return 0;
	}

	double DistSq(const Point3d *p, const Point3d *vl, const Point3d *pf) {
		/// returns the distance squared of pf from the line given by p,vl
		/// vl must be normalised
		Point3d v(*p, *pf);
		Point3d vcp = *vl ^ v;
		double d = vcp.magnitudeSq(); // l * sina
		return d;
	}

	double Dist(const Point3d *p, const Point3d *vl, const Point3d *pf) {
		/// returns the distance of pf from the line given by p,vl
		/// vl must be normalised
		Point3d v(*p, *pf);
		Point3d vcp = *vl ^ v;
		double d = vcp.magnitude(); // l * sina
		return d;
#if 0
		// slower method requires 2 sqrts
		Point3d v(*p, *pf);
		double magv = v.normalise();
		Point3d cp = *vl ^ v;
		double d = magv * cp.magnitude();
		return d;  // l * sina
#endif
	}


	// Triangle3d Constructors
	Triangle3d::Triangle3d(const Point3d& p1, const Point3d& p2, const Point3d& p3) {
		vert1 = p1;
		vert2 = p2;
		vert3 = p3;
		v0 = Point3d(vert1, vert2);
		v1 = Point3d(vert1, vert3);

		// set box
		box.min.x = __min(__min(vert1.x, vert2.x), vert3.x);
		box.min.y = __min(__min(vert1.y, vert2.y), vert3.y);
		box.min.z = __min(__min(vert1.z, vert2.z), vert3.z);

		box.max.x = __max(__max(vert1.x, vert2.x), vert3.x);
		box.max.y = __max(__max(vert1.y, vert2.y), vert3.y);
		box.max.z = __max(__max(vert1.z, vert2.z), vert3.z);
	}

	// Triangle3d methods
	bool    Triangle3d::Intof(const Line& l, Point3d& intof)const {
		// returns intersection triangle to line in intof
		// funtion returns true for intersection, false for no intersection
		// method based on Möller & Trumbore(1997) (Barycentric coordinates)
		// based on incorrect Pseudo code from "Geometric Tools for Computer Graphics" p.487
		if (box.outside(l.box) == true) return false;

		Point3d line(l.v);
		line.normalise();

		Point3d p = line ^ v1;				// cross product
		double tmp = p * v0;				// dot product

		if (FEQZ(tmp)) return false;

		tmp = 1 / tmp;
		Point3d s(vert1, l.p0);

		double u = tmp * (s * p);			// barycentric coordinate
		if (u < 0 || u > 1) return false;	// not inside triangle

		Point3d q = s ^ v0;
		double v = tmp * (line * q);		// barycentric coordinate
		if (v < 0 || v > 1) return false;	// not inside triangle

		if (u + v > 1) return false;		// not inside triangle

		double t = tmp * (v1 * q);
		intof = line * t + l.p0;
		return true;
	}

	bool Triangle3d::Intof(const Plane& pl, Point3d& ps, Point3d& pe)const {
		// intersection plane to triangle
		// pl		is the plane
		// ps		start of intersection
		// pe		end of intersection
		// returns true for valid intersection (false for a corner contact)

		// intersect triangle edges to plane
		double t1, t2, t3;
		Point3d p1, p2, p3;

		//bool b1 = pl.Intof(vert1, v0, p1, t1);
		//bool b2 = pl.Intof(vert2, Point3d(vert2, vert3), p2, t2);
		//bool b3 = pl.Intof(vert3, -v1, p3, t3);
		Line l1(vert1, vert2);
		Line l2(vert2, vert3);
		Line l3(vert3, vert1);
		bool b1 = pl.Intof(l1, p1, t1);
		bool b2 = pl.Intof(l2, p2, t2);
		bool b3 = pl.Intof(l3, p3, t3);

		if (b1 == true && t1 > -TOLERANCE && t1 < 1.0 + TOLERANCE) {
			ps = p1;
			if (b2 == true && t2 > -TOLERANCE && t2 < 1.0 + TOLERANCE) {
				if (b3 == true && t3 > -TOLERANCE && t3 < 1.0 + TOLERANCE) {
					double min2 = fabs(t2);
					if (fabs(1.0 - t2) < min2)min2 = fabs(1.0 - t2);
					double min3 = fabs(t3);
					if (fabs(1.0 - t3) < min3)min3 = fabs(1.0 - t3);
					if (min3 > min2)
					{
						pe = p3;
						return true;
					}
				}
				pe = p2;
				return true;
			}
			if (b3 == true && t3 > -TOLERANCE && t3 < 1.0 + TOLERANCE) {
				pe = p3;
				return true;
			}
			else
				return false;
		}

		if (b2 == true && t2 > -TOLERANCE && t2 < 1.0 + TOLERANCE) {
			ps = p2;

			if (b3 == true && t3 > -TOLERANCE && t3 < 1.0 + TOLERANCE) {
				pe = p3;
				return true;
			}
			else
				return false;
		}
		return false;
	}

	bool Triangle3d::Intof(const Triangle3d& t, std::list<Point3d>& intof_list)const
	{
		Plane plane(this->vert1, this->vert2, this->vert3);
		Point3d ps, pe;
		if (t.Intof(plane, ps, pe))
		{
			if (ps == pe)
			{
				if (this->Inside(ps))
				{
					intof_list.push_back(ps);
					return true;
				}
				return false;
			}

			Line line1(ps, pe);

			Plane plane2(t.vert1, t.vert2, t.vert3);
			Point3d ps2, pe2;
			if (this->Intof(plane2, ps2, pe2))
			{
				if (ps2 == pe2)
				{
					if (t.Inside(ps2))
					{
						intof_list.push_back(ps2);
						return true;
					}
					return false;
				}

				Line line2(ps2, pe2);

				Point3d v(ps, pe);
				v.normalise();
				double d0 = Point3d(ps) * v;
				double d1 = Point3d(pe) * v;
				double d2 = Point3d(ps2) * v;
				double d3 = Point3d(pe2) * v;

				double dorig = d0;
				if (d2 > d3)
				{
					double temp = d2;
					d2 = d3;
					d3 = temp;
				}

				if (d2 > d1)
					return false;
				if (d3 < d0)
					return false;
				if (d2 > d0)
					d0 = d2;
				if (d3 < d1)
					d1 = d3;

				Point3d p0 = ps + (v * (d0 - dorig));
				Point3d p1 = ps + (v * (d1 - dorig));
				if (p1 == p0)
					return false;

				intof_list.push_back(p0);
				intof_list.push_back(p1);

				return true;
			}
		}

		return false;
	}

	bool    Triangle3d::inside(const Point3d& intof)const {
		// returns true if intof is inside triangle by >= TOLERANCE
		// previous call to Intof should be true!!  Not ideal!!

		Line line1(vert1, vert2);
		double t;
		Point3d pn = line1.Near(intof, t);
		if (pn.Dist(intof) < TOLERANCE) return false;

		Line line2(vert2, vert3);
		pn = line2.Near(intof, t);
		if (pn.Dist(intof) < TOLERANCE) return false;

		Line line3(vert1, vert3);
		pn = line3.Near(intof, t);
		if (pn.Dist(intof) < TOLERANCE) return false;

		return true;
	}

	bool Triangle3d::Inside(const Point3d& intof)const
	{
		// returns true if the point is inside the triangle's infinite toblerone ( prism ).
		// no need for a previous call to Intof ( like above )
		Point3d normal;
		getNormal(&normal);

		Point3d N0 = Point3d(vert1, vert2) ^ normal;
		N0.normalise();
		Point3d N1 = Point3d(vert2, vert3) ^ normal;
		N1.normalise();
		Point3d N2 = Point3d(vert3, vert1) ^ normal;
		N2.normalise();

		double d0 = Point3d(intof) * N0 - Point3d(vert1) * N0;
		double d1 = Point3d(intof) * N1 - Point3d(vert2) * N1;
		double d2 = Point3d(intof) * N2 - Point3d(vert3) * N2;

		if ((d0 > -TOLERANCE) && (d1 > -TOLERANCE) && (d2 > -TOLERANCE))
			return true;
		if ((d0 < TOLERANCE) && (d1 < TOLERANCE) && (d2 < TOLERANCE))
			return true;

		return false;
	}

	void Triangle3d::getNormal(Point3d* normal)const {
		// returns the normal to this triangle
		*normal = v0 ^ v1;
		normal->normalise();
	}

	double Triangle3d::getArea()const {
		// returns the area of triangle
		// needs looking at for speed
		Line base(this->vert1, this->vert2);
		double t;
		Point3d pn = base.Near(vert3, t);
		double height = pn.Dist(vert3);
		return 0.5 * base.length * height;
	}

	// box class
	bool Box::outside(const Box& b)const {
		// returns true if this box is outside b
		if(b.ok == false || this->ok == false) return false;	// no box set
		if(this->max.x < b.min.x) return true;
		if(this->max.y < b.min.y) return true;
		if(this->min.x > b.max.x) return true;
		if(this->min.y > b.max.y) return true;
		return false;
	}

	void Box::combine(const Box& b) {
		if(b.max.x > this->max.x) this->max.x = b.max.x;
		if(b.max.y > this->max.y) this->max.y = b.max.y;
		if(b.min.x < this->min.x) this->min.x = b.min.x;
		if(b.min.y < this->min.y) this->min.y = b.min.y;
	}

	void Box3d::combine(const Box3d& b) {
		if(b.max.x > this->max.x) this->max.x = b.max.x;
		if(b.max.y > this->max.y) this->max.y = b.max.y;
		if(b.max.z > this->max.z) this->max.z = b.max.z;
		if(b.min.x < this->min.x) this->min.x = b.min.x;
		if(b.min.y < this->min.y) this->min.y = b.min.y;
		if(b.min.z < this->min.z) this->min.z = b.min.z;
	}

	bool Box3d::outside(const Box3d& b) const{
		// returns true if this box is outside b
		if(b.ok == false || this->ok == false) return false;	// no box set
		if(this->max.x < b.min.x) return true;
		if(this->max.y < b.min.y) return true;
		if(this->max.z < b.min.z) return true;
		if(this->min.x > b.max.x) return true;
		if(this->min.y > b.max.y) return true;
		if(this->min.z > b.max.z) return true;
		return false;
	}
#if 0
	Span3d IsPtsSpan3d(const double* a, int n, double tolerance, double* deviation) {
		// returns a span3d if all points are within tolerance
		int np = n / 3;					// number of points
		if(np < 2) return Span3d();		// Invalid span3d
		Point3d sp = Point3d(&a[0]);
		Point3d ep = Point3d(&a[n-3]);
		Line line = IsPtsLine(a, n, tolerance, deviation);
		if(line.ok) return Span3d(sp, ep);	// it's a line

		*deviation = 0;					// cumulative deviation
		Point3d mp = Point3d(&a[np / 2 * 3]);	// mid point
		Plane plane(sp, mp, ep);
		if(plane.ok) {
			// plane of the arc is ok
			// calculate centre point
			Point3d vs(mp, sp);
			vs.normalise();
			Point3d ve(mp, ep);
			ve.normalise();
			Point3d rs = vs ^ plane.normal;
			Point3d re = ve ^ plane.normal;

			Line rsl(sp.Mid(mp), rs, false);
			Line rel(ep.Mid(mp), re, false);

			Point3d pc;
			Intof(rsl, rel, pc);
			double radius = pc.Dist(sp);

			// check other points on circle
			for(int i = 2; i < np - 1; i++) {
				Point3d p(&a[i*3]);
				double dp = fabs(plane.Dist(p));
				double dr = fabs(p.Dist(pc) - radius);
double tolerance = 10.0 * 1.0e-6;
				if(dp > tolerance || dr > tolerance) {
					return Span3d();
				}

			}
			return Span3d(CW, plane.normal, sp, ep, pc);

		}
		return Span3d();
	}
#endif

	Line IsPtsLine(const double* a, int n, double tolerance, double* deviation) {
		// returns a Line if all points are within tolerance
		// deviation is returned as the sum of all deviations of interior points to line(sp,ep)
		int np = n / 3;					// number of points
		*deviation = 0;					// cumulative deviation
		if(np < 2) return Line();		// Invalid line

		Point3d sp(&a[0]);
		Point3d ep(&a[n-3]);
		Line line(sp, ep);				// line start - end

		if(line.ok) {
			for(int j = 1; j < np - 1; j++) {
				Point3d mp(&a[j * 3]);
				double t, d=0;
				if((d = mp.Dist(line.Near(mp, t))) > tolerance) {
					line.ok = false;
					return line;
				}
				*deviation = *deviation + d;
			}
		}
		return line;
	}

