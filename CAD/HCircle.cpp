// HCircle.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HCircle.h"
#include "HLine.h"
#include "HILine.h"
#include "HArc.h"
#include "Gripper.h"
#include "DigitizeMode.h"
#include "Drawing.h"
#include "Property.h"
#include "Sketch.h"
#include "../Geom/Geom.h"

HCircle::HCircle(const HCircle &c){
	operator=(c);
}

HCircle::HCircle(const Point3d &c, const Point3d& axis, double radius, const HeeksColor* col) :color(*col){
	m_c = c;
	m_axis = axis;
	m_radius = radius;
}

HCircle::~HCircle(){
}

const HCircle& HCircle::operator=(const HCircle &c){
	ExtrudedObj<IdNamedObj>::operator=(c);

	m_c = c.m_c;
	m_axis = c.m_axis;
	m_radius = c.m_radius;
	color = c.color;
	return *this;
}

const wchar_t* HCircle::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/circle.png";
	return iconpath.c_str();
}

//segments - number of segments per full revolution!
void HCircle::GetSegments(void(*callbackfunc)(const double *p, bool start), double pixels_per_mm)const
{
	Point3d x_axis, y_axis;
	m_axis.arbitrary_axes(x_axis, y_axis);

	double radius = m_radius;
	int segments = (int)(fabs(pixels_per_mm * radius + 1));

	double theta = 6.28318530717958 / (double)segments;
	double tangetial_factor = tan(theta);
	double radial_factor = 1 - cos(theta);

	double x = radius;
	double y = 0.0;

	bool start = true;

	for (int i = 0; i < segments + 1; i++)
	{
		Point3d p = m_c + x_axis * x + y_axis * y;
		(*callbackfunc)(p.getBuffer(), start);

		double tx = -y;
		double ty = x;

		x += tx * tangetial_factor;
		y += ty * tangetial_factor;

		double rx = -x;
		double ry = -y;

		x += rx * radial_factor;
		y += ry * radial_factor;

		start = false;
	}
}

static HCircle* circle_for_glVertexFunction = NULL;
static void glVertexFunction(const double *p, bool start)
{
	glVertex3d(p[0], p[1], p[2]);
}

static void glVertexThickened(const double *p, bool start)
{
	glVertex3d(p[0] + circle_for_glVertexFunction->m_extrusion_vector[0] * circle_for_glVertexFunction->m_thickness
		, p[1] + circle_for_glVertexFunction->m_extrusion_vector[1] * circle_for_glVertexFunction->m_thickness
		, p[2] + circle_for_glVertexFunction->m_extrusion_vector[2] * circle_for_glVertexFunction->m_thickness);
}

void HCircle::glCommands(bool select, bool marked, bool no_color){
	if (!no_color){
		theApp->glColorEnsuringContrast(color);
	}
	GLfloat save_depth_range[2];
	if (marked){
		glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
		glDepthRange(0, 0);
		glLineWidth(2);
	}


	glBegin(GL_LINE_STRIP);
	GetSegments(glVertexFunction, theApp->GetPixelScale());
	if (m_thickness != 0.0)
	{
		circle_for_glVertexFunction = this;
		GetSegments(glVertexThickened, theApp->GetPixelScale());
	}
	glEnd();

	if (marked){
		glLineWidth(1);
		glDepthRange(save_depth_range[0], save_depth_range[1]);
	}
}

HeeksObj *HCircle::MakeACopy(void)const{
	HCircle *new_object = new HCircle(*this);
	return new_object;
}

void HCircle::Transform(const Matrix& m){
	double sx = 1.0;
	m.GetScale(sx);
	m_radius *= sx;
	m_axis = m_axis.TransformedOnlyRotation(m);
	m_c = m_c.Transformed(m);
}

void HCircle::GetBox(CBox &box){
	Point3d x_axis, y_axis;
	m_axis.arbitrary_axes(x_axis, y_axis);

	Point3d x = x_axis * m_radius;
	Point3d y = y_axis * m_radius;

	box.Insert((m_c - x - y).getBuffer());
	box.Insert((m_c + x - y).getBuffer());
	box.Insert((m_c + x + y).getBuffer());
	box.Insert((m_c - x + y).getBuffer());
}

void HCircle::GetGripperPositions(std::list<GripData> *list, bool just_for_endof){
	if (!just_for_endof)
	{
		Point3d x_axis, y_axis;
		m_axis.arbitrary_axes(x_axis, y_axis);
		Point3d s = (m_c + x_axis * m_radius);
		list->push_back(GripData(GripperTypeTranslate, m_c));
		list->push_back(GripData(GripperTypeStretch, s));
	}
}

void HCircle::GetProperties(std::list<Property *> *list){
	list->push_back(PropertyPnt(this, L"centre", &m_c));
	list->push_back(PropertyPnt(this, L"axis", &m_axis));
	list->push_back(new PropertyLength(this, L"radius", &m_radius));

	ExtrudedObj<IdNamedObj>::GetProperties(list);
}

bool HCircle::FindNearPoint(const Line &ray, double *point){
#if 0 // to do
	std::list< Point3d > rl;
	ClosestPointsLineAndCircle(ray, GetCircle(), rl);
	if (rl.size()>0)
	{
		extract(rl.front(), point);
		return true;
	}
#endif
	return false;
}

bool HCircle::FindPossTangentPoint(const Line &ray, double *point){
	// any point on this circle is a possible tangent point
	return FindNearPoint(ray, point);
}

bool HCircle::Stretch(const Point3d &p, const Point3d &shift, void* data){
	m_radius = m_c.Dist(p + shift);

	return true;
}

bool HCircle::GetCentrePoint(Point3d &pos)
{
	pos = m_c;
	return true;
}

bool HCircle::GetAxis(Point3d &pos)
{
	pos = m_axis;
	return true;
}

bool HCircle::GetRadius(double &r)
{
	r = m_radius;
	return true;
}

void HCircle::WriteToXML(TiXmlElement *element)
{
	element->SetAttribute("col", color.COLORREF_color());
	element->SetDoubleAttribute("r", m_radius);
	element->SetDoubleAttribute("cx", m_c.x);
	element->SetDoubleAttribute("cy", m_c.y);
	element->SetDoubleAttribute("cz", m_c.z);
	element->SetDoubleAttribute("ax", m_axis.x);
	element->SetDoubleAttribute("ay", m_axis.y);
	element->SetDoubleAttribute("az", m_axis.z);
	ExtrudedObj<IdNamedObj>::WriteToXML(element);
}

void HCircle::ReadFromXML(TiXmlElement* element)
{
	int int_value;
	if (element->Attribute("col", &int_value))color = HeeksColor((long)int_value);
	element->Attribute("r", &m_radius);
	element->Attribute("cx", &m_c.x);
	element->Attribute("cy", &m_c.y);
	element->Attribute("cz", &m_c.z);
	element->Attribute("ax", &m_axis.x);
	element->Attribute("ay", &m_axis.y);
	element->Attribute("az", &m_axis.z);

	ExtrudedObj<IdNamedObj>::ReadFromXML(element);
}

int HCircle::Intersects(const HeeksObj *object, std::list< double > *rl)const
{
	int numi = 0;

	switch (object->GetType())
	{
	case SketchType:
		return(((CSketch *)object)->Intersects(this, rl));

	case LineType:
		return(((HLine *)object)->Intersects(this, rl));

	case ILineType:
		return(((HILine *)object)->Intersects(this, rl));

	case ArcType:
		return(((HArc *)object)->Intersects(this, rl));

	case CircleType:
	{
		Point leftInters, rightInters;
		int	n = GetCircle().Intof(((HCircle*)object)->GetCircle(), leftInters, rightInters);
		if (n > 0)
		{
			if (rl)add_pnt_to_doubles(leftInters, *rl);
			numi++;
		}
		if (n > 1)
		{
			if (rl)add_pnt_to_doubles(rightInters, *rl);
			numi++;
		}
	}
	break;
	}

	return numi;
}

#if 0 // to do

//static
bool HCircle::GetLineTangentPoints(const Circle& c1, const Circle& c2, const Point3d& a, const Point3d& b, Point3d& p1, Point3d& p2)
{
	// find the tangent points for a line between two circles

	// they need to be flat ( compared with each other )
	if (fabs(fabs(c1.Axis().Direction() * c2.Axis().Direction()) - 1.0) > 0.00000001)return false;
	if (fabs(Point3d(c1.Location()) * c1.Axis().Direction() - Point3d(c2.Location()) * c1.Axis().Direction()) > 0.00000001)return false;

	// not concentric
	if (c1.Location().IsEqual(c2.Location(), theApp->m_geom_tol))return false;

	// find left and right
	Point3d join(c1.Location(), c2.Location());
	Point3d forward = join.Normalized();
	Point3d left1 = c1.Axis().Direction() ^ join;
	Point3d left2 = c2.Axis().Direction() ^ join;
	if (left1 * left2 < 0)left2 = -left2;
	Point3d c1a(c1.Location(), a);
	Point3d c2b(c2.Location(), b);

	bool is_left1 = c1a * left1 > 0;
	bool is_left2 = c2b * left2 > 0;

	double r1 = c1.Radius();
	double r2 = c2.Radius();

	double H = join.Magnitude();
	double O = (is_left1 == is_left2) ? fabs(r1 - r2) : (r1 + r2);
	double A = sqrt(H*H - O*O);
	double sinang = O / H;
	double cosang = A / H;

	double f1 = 1.0;
	double l1 = is_left1 ? 1.0 : -1.0;
	double f2 = 1.0;
	double l2 = is_left2 ? 1.0 : -1.0;

	if (is_left1 == is_left2){
		if (r1 < r2)
		{
			f1 = -1.0;
			f2 = -1.0;
		}
	}
	else
	{
		f1 = 1.0;
		f2 = -1.0;
	}

	p1 = c1.Location() + forward * (f1 * r1 * sinang) + left1 * (l1 * r1 * cosang);
	p2 = c2.Location() + forward * (f2 * r2 * sinang) + left2 * (l2 * r2 * cosang);

	return true;
}

// static
bool HCircle::GetLineTangentPoint(const Circle& c, const Point3d& a, const Point3d& b, Point3d& p)
{
	// find the tangent point for a line from near point "a" on the circle to point "b"

	// find left and right
	Point3d join(c.Location(), b);
	Point3d forward = join.Normalized();
	Point3d left = c.Axis().Direction() ^ join;
	Point3d ca(c.Location(), a);

	bool is_left = ca * left > 0;
	double r = c.Radius();

	double H = join.Magnitude();
	double O = r;
	double A = sqrt(H*H - O*O);
	double sinang = O / H;
	double cosang = A / H;

	double l = is_left ? 1.0 : -1.0;
	p = c.Location() + forward * (r * sinang) + left * (l * r * cosang);

	return true;
}

// static
bool HCircle::GetArcTangentPoints(const Circle& c, const gp_Lin &line, const Point3d& p, double radius, Point3d& p1, Point3d& p2, Point3d& centre, Point3d& axis)
{
	// find the arc that fits tangentially from the line to the circle
	// returns p1 - on the line, p2 - on the circle, centre - the centre point of the arc

	// they need to be flat ( compared with each other )
	if (fabs(Point3d(c.Axis().Direction()) * Point3d(line.Direction())) > 0.00000001)return false;
	if (fabs(Point3d(c.Location()) * c.Axis().Direction() - Point3d(line.Location()) * c.Axis().Direction()) > 0.00000001)return false;

	// make a concentric circle
	bool p_infront_of_centre = Point3d(p) * Point3d(line.Direction()) > Point3d(c.Location()) * Point3d(line.Direction());
	double R = p_infront_of_centre ? (c.Radius() - radius) : (c.Radius() + radius);
	if (R < 0.000000000000001)return false;
	Circle concentric_circle(gp_Ax2(c.Axis().Location(), c.Axis().Direction()), R);

	// make an offset line
	Point3d left = c.Axis().Direction() ^ line.Direction();
	bool p_on_left_of_line = Point3d(p) * left > Point3d(line.Location()) * left;
	gp_Lin offset_line(Point3d(line.Location() + left * (p_on_left_of_line ? radius : -radius)), line.Direction());

	// find intersection between concentric circle and offset line
	{
		std::list<Point3d> rl;
		intersect(offset_line, concentric_circle, rl);
		double best_dp = 0.0;
		Point3d* best_pnt = NULL;
		for (std::list<Point3d>::iterator It = rl.begin(); It != rl.end(); It++)
		{
			Point3d& pnt = *It;
			double dp = Point3d(pnt) * Point3d(line.Direction());
			if (best_pnt == NULL || (p_infront_of_centre && dp > best_dp) || (!p_infront_of_centre && dp < best_dp))
			{
				best_pnt = &pnt;
				best_dp = dp;
			}
		}

		if (best_pnt == NULL)return false;

		// use as centre of arc
		centre = *best_pnt;
	}

	// make a circle at this point
	Circle circle_for_arc(gp_Ax2(centre, c.Axis().Direction()), radius);

	// intersect with original line
	{
		std::list<Point3d> rl;
		intersect(line, circle_for_arc, rl);
		if (rl.size() == 0)return false;

		// use this point for p1
		p1 = rl.front();
	}

	// intersect with original circle
	{
		std::list<Point3d> rl;
		intersect(c, circle_for_arc, rl);
		if (rl.size() == 0)return false;

		// use this point for p2
		p2 = rl.front();
	}

	axis = c.Axis().Direction();

	return true;
}

class two_circles{
public:
	Circle m_c1;
	Circle m_c2;

	two_circles(const Circle& c1, const Circle& c2)
	{
		m_c1 = c1;
		m_c2 = c2;
	}
};

class two_points{
public:
	Point3d m_p1;
	Point3d m_p2;
	Point3d m_c;

	two_points(const Point3d& p1, const Point3d& p2, const Point3d& c)
	{
		m_p1 = p1;
		m_p2 = p2;
		m_c = c;
	}
};

static const two_points* find_best_points_pair(const std::list<two_points>& point_pairs, const Point3d& a, const Point3d& b)
{
	const two_points* best_pair = NULL;
	double best_pair_error = 0.0;
	for (std::list<two_points>::const_iterator It = point_pairs.begin(); It != point_pairs.end(); It++)
	{
		const two_points& point_pair = *It;
		double d1 = a.Distance(point_pair.m_p1);
		double d2 = b.Distance(point_pair.m_p2);
		double pair_error = d1 + d2;
		if (best_pair == NULL || pair_error < best_pair_error)
		{
			best_pair = &point_pair;
			best_pair_error = pair_error;
		}
	}

	return best_pair;
}

// static
bool HCircle::GetArcTangentPoints(const Circle& c1, const Circle &c2, const Point3d& a, const Point3d& b, double radius, Point3d& p1, Point3d& p2, Point3d& centre, Point3d& axis)
{
	// find the arc that fits tangentially from the circle to the circle
	// returns p1 - on c1, p2 - on c2, centre - the centre point of the arc

	// theApp need to be flat ( compared with each other )
	if (fabs(fabs(c1.Axis().Direction() * c2.Axis().Direction()) - 1.0) > 0.00000001)return false;
	if (fabs(Point3d(c1.Location()) * c1.Axis().Direction() - Point3d(c2.Location()) * c1.Axis().Direction()) > 0.00000001)return false;

	// not concentric
	if (c1.Location().IsEqual(c2.Location(), theApp->m_geom_tol))return false;

	// find left and right
	Point3d join(c1.Location(), c2.Location());
	Point3d forward = join.Normalized();
	Point3d left1 = c1.Axis().Direction() ^ join;
	Point3d left2 = c2.Axis().Direction() ^ join;
	if (left1 * left2 < 0)left2 = -left2;
	Point3d c1a(c1.Location(), a);
	Point3d c2b(c2.Location(), b);

	double r1 = c1.Radius();
	double r2 = c2.Radius();

	std::list<Circle> c1_list, c2_list;
	c1_list.push_back(Circle(gp_Ax2(c1.Location(), c1.Axis().Direction()), r1 + radius));
	if (radius > r1)c1_list.push_back(Circle(gp_Ax2(c1.Location(), c1.Axis().Direction()), radius - r1));
	c2_list.push_back(Circle(gp_Ax2(c2.Location(), c2.Axis().Direction()), r2 + radius));
	if (radius > r2)c2_list.push_back(Circle(gp_Ax2(c2.Location(), c2.Axis().Direction()), radius - r2));

	std::list<two_circles> combinations;
	for (std::list<Circle>::iterator It1 = c1_list.begin(); It1 != c1_list.end(); It1++)
	{
		for (std::list<Circle>::iterator It2 = c2_list.begin(); It2 != c2_list.end(); It2++)
		{
			combinations.push_back(two_circles(*It1, *It2));
		}
	}

	std::list<Point3d> intersections;
	for (std::list<two_circles>::iterator It = combinations.begin(); It != combinations.end(); It++)
	{
		two_circles& circles = *It;
		intersect(circles.m_c1, circles.m_c2, intersections);
	}

	std::list<two_points> point_pairs;
	for (std::list<Point3d>::iterator It = intersections.begin(); It != intersections.end(); It++)
	{
		Point3d& pnt = *It;
		Circle arc_circle(gp_Ax2(pnt, c1.Axis().Direction()), radius);
		std::list<Point3d> rl1, rl2;
		intersect(arc_circle, c1, rl1);
		intersect(arc_circle, c2, rl2);
		if (rl1.size() > 0 && rl2.size() > 0)
		{
			point_pairs.push_back(two_points(rl1.front(), rl2.front(), arc_circle.Location()));
		}
	}

	const two_points* best_pair = find_best_points_pair(point_pairs, a, b);

	if (best_pair)
	{
		p1 = best_pair->m_p1;
		p2 = best_pair->m_p2;
		centre = best_pair->m_c;
		axis = c1.Axis().Direction();
		return true;
	}

	return false;
}

// static
bool HCircle::GetArcTangentPoints(const gp_Lin& l1, const gp_Lin &l2, const Point3d& a, const Point3d& b, double radius, Point3d& p1, Point3d& p2, Point3d& centre, Point3d& axis)
{
	// cross product to find axis of arc
	Point3d xp = Point3d(l1.Direction()) ^ Point3d(l2.Direction());
	if (xp.Magnitude() < 0.00000000000001)return false;
	axis = Point3d(xp);

	Point3d left1 = axis ^ l1.Direction();
	Point3d left2 = axis ^ l2.Direction();

	gp_Lin offset_l1_left(l1.Location() + left1 * radius, l1.Direction());
	gp_Lin offset_l1_right(l1.Location() - left1 * radius, l1.Direction());
	gp_Lin offset_l2_left(l2.Location() + left2 * radius, l2.Direction());
	gp_Lin offset_l2_right(l2.Location() - left2 * radius, l2.Direction());

	std::list<Circle> circles;

	Point3d pnt;
	if (intersect(offset_l1_left, offset_l2_left, pnt))
		circles.push_back(Circle(gp_Ax2(pnt, axis), radius));
	if (intersect(offset_l1_left, offset_l2_right, pnt))
		circles.push_back(Circle(gp_Ax2(pnt, axis), radius));
	if (intersect(offset_l1_right, offset_l2_left, pnt))
		circles.push_back(Circle(gp_Ax2(pnt, axis), radius));
	if (intersect(offset_l1_right, offset_l2_right, pnt))
		circles.push_back(Circle(gp_Ax2(pnt, axis), radius));

	std::list<two_points> point_pairs;

	for (std::list<Circle>::iterator It = circles.begin(); It != circles.end(); It++)
	{
		Circle& c = *It;
		std::list<Point3d> rl1, rl2;
		intersect(l1, c, rl1);
		intersect(l2, c, rl2);
		if (rl1.size() > 0 && rl2.size() > 0)
		{
			point_pairs.push_back(two_points(rl1.front(), rl2.front(), c.Location()));
		}
	}

	const two_points* best_pair = find_best_points_pair(point_pairs, a, b);

	if (best_pair)
	{
		p1 = best_pair->m_p1;
		p2 = best_pair->m_p2;
		centre = best_pair->m_c;
		return true;
	}

	return false;
}

// static
bool HCircle::GetArcTangentPoint(const gp_Lin& l, const Point3d& a, const Point3d& b, const Point3d *final_direction, double* radius, Point3d& p, Point3d& centre, Point3d& axis)
{
	// find the tangent point on the line l, for an arc from near point "a" on the line to exact given point "b"

	Point3d c = ClosestPointOnLine(l, b);
	if (c.Distance(b) < 0.000000000001)return false;
	Point3d sideways(Point3d(c, b));

	if (final_direction)
	{
		if ((*final_direction) * Point3d(sideways) >= 0)// b on correct side compared with final direction
		{
			gp_Lin final_dir_line(b, Point3d(final_direction->XYZ()));
			if (!l.Direction().IsEqual(Point3d(final_direction->XYZ()), 0.00000001) && !l.Direction().IsEqual(-Point3d(final_direction->XYZ()), 0.00000001))
			{
				axis = l.Direction() ^ final_dir_line.Direction();
				Point3d perp = axis ^ (*final_direction);
				gp_Lin perp_line(gp_Ax1(b, perp));
				Point3d half_angle_dir[2];
				half_angle_dir[0] = Point3d(perp + sideways);
				half_angle_dir[1] = Point3d(sideways - perp);


				std::list<Point3d> maybe_p;
				for (int i = 0; i<2; i++)
				{
					Point3d pnt;
					if (intersect(final_dir_line, l, pnt))
					{
						gp_Lin half_angle_line(pnt, half_angle_dir[i]);
						if (intersect(half_angle_line, perp_line, centre))
						{
							double R = b.Distance(centre);
							std::list<Point3d> plist;
							intersect(l, Circle(gp_Ax2(centre, axis), R), plist);
							if (plist.size() == 1)
							{
								maybe_p.push_back(plist.front());
							}
						}
					}
				}

				Point3d* best_pnt = NULL;
				double best_dist = 0.0;

				for (std::list<Point3d>::iterator It = maybe_p.begin(); It != maybe_p.end(); It++)
				{
					Point3d& pnt = *It;
					double dist = a.Distance(pnt);
					if (best_pnt == NULL || dist < best_dist)
					{
						best_pnt = &pnt;
						best_dist = dist;
					}
				}

				if (best_pnt)
				{
					p = *best_pnt;
					return true;
				}
			}
		}
		Point3d v0 = -(*final_direction);
		return HArc::TangentialArc(b, v0, a, centre, axis);
	}
	else
	{
		double radius_to_use;
		if (radius)radius_to_use = *radius;
		else
		{
			// fit the minimum radius arc
			radius_to_use = c.Distance(b);
		}
		axis = l.Direction() ^ sideways;
		Circle offset_circle(gp_Ax2(b, axis), radius_to_use);
		gp_Lin offset_line(l.Location() + sideways * radius_to_use, l.Direction());
		std::list<Point3d> plist;
		intersect(offset_line, offset_circle, plist);
		Point3d* best_pnt = NULL;
		double best_distance = 0.0;
		for (std::list<Point3d>::iterator It = plist.begin(); It != plist.end(); It++)
		{
			Point3d& pnt = *It;
			double distance = a.Distance(pnt);
			if (best_pnt == NULL || distance < best_distance)
			{
				best_pnt = &pnt;
				best_distance = distance;
			}
		}
		if (best_pnt)
		{
			centre = *best_pnt;
			std::list<Point3d> rl;
			intersect(l, Circle(gp_Ax2(centre, axis), radius_to_use), rl);
			if (rl.size() == 1)
			{
				p = rl.front();
				return true;
			}
		}
	}

	return false;
}

// static
bool HCircle::GetArcTangentPoint(const Circle& c, const Point3d& a, const Point3d& b, const Point3d *final_direction, double* radius, Point3d& p, Point3d& centre, Point3d& axis)
{
	// find the tangent point on the circle c, for an arc from near point "a" on the circle to exact given point "b"
	if (final_direction)
	{
		// get tangent circles
		gp_Lin final_dir_line(b, Point3d(final_direction->XYZ()));
		std::list<Circle> c_list;
		TangentCircles(c, final_dir_line, b, c_list);

		Circle* best_pnt_circle = NULL;
		Point3d best_pnt;
		double best_dist = 0.0;

		for (std::list<Circle>::iterator It = c_list.begin(); It != c_list.end(); It++)
		{
			Circle& circle = *It;
			std::list<Point3d> p_list;
			intersect(circle, c, p_list);
			if (p_list.size() == 1)
			{
				Point3d& p = p_list.front();
				double dist = p.Distance(a);
				if (best_pnt_circle == NULL || dist < best_dist)
				{
					best_pnt = p;
					best_dist = dist;
					best_pnt_circle = &circle;
				}
			}
		}

		if (best_pnt_circle)
		{
			p = best_pnt;
			centre = best_pnt_circle->Location();
			axis = best_pnt_circle->Axis().Direction();
			return true;
		}
	}

	return false;
}

#endif

void HCircle::SetCircle(Circle c)
{
	m_c = c.pc;
	m_radius = c.radius;
}

Circle HCircle::GetCircle()const
{
	Circle c;
	c.radius = m_radius;
	c.pc = Point(m_c.x, m_c.y);
	return c;
}

double HCircle::GetDiameter()const
{
	return 2 * m_radius;
}

