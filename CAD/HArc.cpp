// HArc.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HArc.h"
#include "HLine.h"
#include "HPoint.h"
#include "HILine.h"
//#include "HCircle.h"
#include "tinyxml.h"
#include "Gripper.h"
#include "Sketch.h"
#include "Drawing.h"
#include "DigitizeMode.h"
#include "Property.h"

HArc::HArc(const HArc &line):EndedObject(){
	operator=(line);
}

HArc::HArc(const geoff_geometry::Point3d &a, const geoff_geometry::Point3d &b, const geoff_geometry::Point3d &axis, const geoff_geometry::Point3d &c, const HeeksColor* col) : EndedObject(){
	A = a;
	B = b;
	C = c;
	m_axis = axis;
	m_radius = a.Dist(c);
	SetColor(*col);
}

HArc::~HArc(){
}

const wchar_t* HArc::GetIconFilePath()
{
	static std::wstring iconpath = theApp.GetResFolder() + L"/icons/arc.png";
	return iconpath.c_str();
}

bool HArc::IsDifferent(HeeksObj* other)
{
	HArc* arc = (HArc*)other;

	if(arc->C.Dist(C) > geoff_geometry::TOLERANCE || arc->m_radius != m_radius)
		return true;

	return EndedObject::IsDifferent(other);
}

const HArc& HArc::operator=(const HArc &b){
	EndedObject::operator=(b);
	m_radius = b.m_radius;
	m_axis = b.m_axis;
	C = b.C;
	return *this;
}

HeeksObj* HArc::MakeACopyWithID()
{
	HArc* pnew = (HArc*)EndedObject::MakeACopyWithID();
	return pnew;
}

void HArc::ReloadPointers()
{
	EndedObject::ReloadPointers();
}

#if 0
HArc* arc_for_tool = NULL;

class ClickArcCentre: public Tool
{
public:
	HArc *pArc;

public:
	void Run()
	{
		theApp.m_digitizing->digitized_point = DigitizedPoint(pArc->C, DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(theApp.input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}

	const wchar_t* GetTitle(){return _("Click centre point");}
	std::wstring BitmapPath(){return _T("click_arc_midpoint");}
};

ClickArcCentre click_arc_centre;

class ClickArcEndOne: public Tool
{
public:
	HArc *pArc;

public:
	void Run()
	{
		theApp.m_digitizing->digitized_point = DigitizedPoint(pArc->A, DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(theApp.input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}

	const wchar_t* GetTitle(){return _("Click first end point");}
	std::wstring BitmapPath(){return _T("click_arc_end_one");}
};

ClickArcEndOne click_arc_first_one;

class ClickArcEndTwo: public Tool
{
public:
	HArc *pArc;

public:
	void Run()
	{
		theApp.m_digitizing->digitized_point = DigitizedPoint(pArc->B, DigitizeInputType);
		Drawing *pDrawingMode = dynamic_cast<Drawing *>(theApp.input_mode_object);
		if (pDrawingMode != NULL)
		{
			pDrawingMode->AddPoint();
		}
	}

	const wchar_t* GetTitle(){return _("Click second end point");}
	std::wstring BitmapPath(){return _T("click_arc_end_two");}
};

ClickArcEndTwo click_arc_first_two;

void HArc::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	arc_for_tool = this;

	Drawing *pDrawingMode = dynamic_cast<Drawing *>(theApp.input_mode_object);
	if (pDrawingMode != NULL)
	{
		click_arc_centre.pArc = this;
		t_list->push_back(&click_arc_centre);

		click_arc_first_one.pArc = this;
		t_list->push_back(&click_arc_first_one);

		click_arc_first_two.pArc = this;
		t_list->push_back(&click_arc_first_two);
	}
}
#endif


//segments - number of segments per full revolution!
//d_angle - determines the direction and the ammount of the arc to draw
void HArc::GetSegments(void(*callbackfunc)(const double *p, bool start), double pixels_per_mm)const
{
	if(A.Dist(B) < geoff_geometry::TOLERANCE){
		return;
	}


	geoff_geometry::Point3d axis = m_axis;
	geoff_geometry::Point3d x_axis, y_axis;
	axis.arbitrary_axes(x_axis, y_axis);
	geoff_geometry::Point3d centre = C;

	double ax = geoff_geometry::Point3d(A - centre) * x_axis;
	double ay = geoff_geometry::Point3d(A - centre) * y_axis;
	double bx = geoff_geometry::Point3d(B - centre) * x_axis;
	double by = geoff_geometry::Point3d(B - centre) * y_axis;

	double start_angle = atan2(ay, ax);
	double end_angle = atan2(by, bx);

	if(start_angle > end_angle)end_angle += 6.28318530717958;

	double radius = m_radius;
	double d_angle = end_angle - start_angle;
	int segments = (int)(fabs(pixels_per_mm * radius * d_angle / 6.28318530717958 + 1));
	if(segments<3)segments = 3;

    double theta = d_angle / (double)segments;
	while(theta>1.0){segments*=2;theta = d_angle / (double)segments;}
    double tangetial_factor = tan(theta);
    double radial_factor = 1 - cos(theta);

    double x = radius * cos(start_angle);
    double y = radius * sin(start_angle);

	double pp[3];
	bool start = true;

   for(int i = 0; i < segments + 1; i++)
    {
		geoff_geometry::Point3d p = centre + x * x_axis + y * y_axis;
		p.get(pp);
		(*callbackfunc)(pp, start);

        double tx = -y;
        double ty = x;

        x += tx * tangetial_factor;
        y += ty * tangetial_factor;

        double rx = - x;
        double ry = - y;

        x += rx * radial_factor;
        y += ry * radial_factor;

		start = false;
    }
}

static void glVertexFunction(const double *p, bool start){ glVertex3d(p[0], p[1], p[2]); }

void HArc::glCommands(bool select, bool marked, bool no_color){
	if(!no_color){
		theApp.glColorEnsuringContrast(*GetColor());
	}
	GLfloat save_depth_range[2];
	if(marked){
		glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
		glDepthRange(0, 0);
		glLineWidth(2);
	}

	glBegin(GL_LINE_STRIP);
	GetSegments(glVertexFunction, theApp.GetPixelScale());
	glEnd();

	if(marked){
		glLineWidth(1);
		glDepthRange(save_depth_range[0], save_depth_range[1]);
	}
	EndedObject::glCommands(select,marked,no_color);
}

HeeksObj *HArc::MakeACopy(void)const{
		HArc *new_object = new HArc(*this);
		return new_object;
}

void HArc::Transform(const geoff_geometry::Matrix& m){
	EndedObject::Transform(m);
	m_axis = m_axis.Transformed(m);
	C = C.Transformed(m);
	m_radius = C.Dist(A);
}

void HArc::GetBox(CBox &box){
	box.Insert(A.x, A.y, A.z);
	box.Insert(B.x, B.y, B.z);

	if(IsIncluded(geoff_geometry::Point3d(0,m_radius,0)))
		box.Insert(C.x,C.y+m_radius,C.z);
	if(IsIncluded(geoff_geometry::Point3d(0,-m_radius,0)))
		box.Insert(C.x,C.y-m_radius,C.z);
	if(IsIncluded(geoff_geometry::Point3d(m_radius,0,0)))
		box.Insert(C.x+m_radius,C.y,C.z);
	if(IsIncluded(geoff_geometry::Point3d(-m_radius,0,0)))
		box.Insert(C.x-m_radius,C.y,C.z);
}

bool HArc::IsIncluded(geoff_geometry::Point3d pnt)
{
	geoff_geometry::Point3d axis(m_axis);
	geoff_geometry::Point3d x_axis, y_axis;
	axis.arbitrary_axes(x_axis, y_axis);
	geoff_geometry::Point3d centre = C;

	double ax = geoff_geometry::Point3d(A - centre) * x_axis;
	double ay = geoff_geometry::Point3d(A - centre) * y_axis;
	double bx = geoff_geometry::Point3d(B - centre) * x_axis;
	double by = geoff_geometry::Point3d(B - centre) * y_axis;

	double start_angle = atan2(ay, ax);
	double end_angle = atan2(by, bx);

	if(start_angle > end_angle)end_angle += 6.28318530717958;

	double pnt_angle = atan2(geoff_geometry::Point3d(pnt) * y_axis, geoff_geometry::Point3d(pnt) * x_axis);
	if(pnt_angle >= start_angle && pnt_angle <= end_angle)
		return true;
	return false;
}

void HArc::GetGripperPositions(std::list<GripData> *list, bool just_for_endof){
	EndedObject::GetGripperPositions(list,just_for_endof);
	list->push_back(GripData(GripperTypeStretch,C.x,C.y,C.z,&C));
}

static double length_for_properties = 0.0;

void HArc::GetProperties(std::list<Property *> *list){
	list->push_back(PropertyPnt(this, L"start", &A));
	list->push_back(PropertyPnt(this, L"end", &B));
	list->push_back(PropertyPnt(this, L"centre", &C));
	list->push_back(PropertyPnt(this, L"axis", &m_axis));
	length_for_properties = A.Dist(B);
	list->push_back(new PropertyLength(this, L"length", (const double*)&length_for_properties));
	list->push_back(new PropertyLength(this, L"radius", &m_radius));

	HeeksObj::GetProperties(list);
}

int HArc::Intersects(const HeeksObj *object, std::list< double > *rl)const
{
	int numi = 0;

#if 0  // to do
	switch(object->GetType())
	{
    case SketchType:
        return( ((CSketch *)object)->Intersects( this, rl ));

	case LineType:
		{
			std::list<geoff_geometry::Point3d> plist;
			intersect(((HLine*)object)->GetLine(), GetCircle(), plist);
			for(std::list<geoff_geometry::Point3d>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				geoff_geometry::Point3d& pnt = *It;
				if(Intersects(pnt) && ((HLine*)object)->Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case ILineType:
		{
			std::list<geoff_geometry::Point3d> plist;
			intersect(((HILine*)object)->GetLine(), GetCircle(), plist);
			for(std::list<geoff_geometry::Point3d>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				geoff_geometry::Point3d& pnt = *It;
				if(Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case ArcType:
		{
			std::list<geoff_geometry::Point3d> plist;
			intersect(GetCircle(), ((HArc*)object)->GetCircle(), plist);
			for(std::list<geoff_geometry::Point3d>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				geoff_geometry::Point3d& pnt = *It;
				if(Intersects(pnt) && ((HArc*)object)->Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case CircleType:
		{
			std::list<geoff_geometry::Point3d> plist;
			intersect(GetCircle(), ((HCircle*)object)->GetCircle(), plist);
			for(std::list<geoff_geometry::Point3d>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				geoff_geometry::Point3d& pnt = *It;
				if(Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;
	}
#endif

	return numi;
}

bool HArc::Intersects(const geoff_geometry::Point3d &pnt)const
{
#if 0 // to do
	if(!intersect(pnt, GetCircle()))return false;

	if(pnt.IsEqual(A, theApp.m_geom_tol)){
		return true;
	}

	if(pnt.IsEqual(B, theApp.m_geom_tol)){
		return true;
	}

	if(A.IsEqual(B, theApp.m_geom_tol)){
		return false; // no size arc!
	}

	geoff_geometry::Point3d axis(C,m_axis.Direction());
	geoff_geometry::Point3d x_axis = axis.XDirection();
	geoff_geometry::Point3d y_axis = axis.YDirection();
	geoff_geometry::Point3d centre = C;

	double ax = geoff_geometry::Point3d(A - centre) * x_axis;
	double ay = geoff_geometry::Point3d(A - centre) * y_axis;
	double bx = geoff_geometry::Point3d(B - centre) * x_axis;
	double by = geoff_geometry::Point3d(B - centre) * y_axis;
	double px = geoff_geometry::Point3d(pnt - centre) * x_axis;
	double py = geoff_geometry::Point3d(pnt - centre) * y_axis;

	double start_angle = atan2(ay, ax);
	double end_angle = atan2(by, bx);
	double pnt_angle = atan2(py, px);

	// force the angle to be greater than start angle
	if(start_angle > end_angle)end_angle += 6.28318530717958;
	while(pnt_angle < start_angle)pnt_angle += 6.28318530717958;

	// point lies on the arc, if the angle is less than the end angle
	return pnt_angle < end_angle;
#else
	return false;
#endif
}

bool HArc::FindNearPoint(const geoff_geometry::Line &ray, double *point){
#if 0 // to do
	gp_Lin ray(make_point(ray_start), make_vector(ray_direction));
	std::list< geoff_geometry::Point3d > rl;
	ClosestPointsLineAndCircle(ray, GetCircle(), rl);
	if(rl.size()>0)
	{
		geoff_geometry::Point3d p = rl.front();
		if(Intersects(p))
		{
			extract(p, point);
			return true;
		}
	}
#endif
	return false;
}

bool HArc::FindPossTangentPoint(const geoff_geometry::Line &ray, double *point){
	// any point on this arc is a possible tangent point
	return FindNearPoint(ray, point);
}

bool HArc::Stretch(const double *p, const double* shift, void* data){
#if 0 // to do
	geoff_geometry::Point3d vp = make_point(p);
	geoff_geometry::Point3d vshift = make_vector(shift);

	if(A.IsEqual(vp, theApp.m_geom_tol)){
		geoff_geometry::Point3d direction = -(GetSegmentVector(1.0));
		geoff_geometry::Point3d centre;
		geoff_geometry::Point3d axis;
		geoff_geometry::Point3d new_A = geoff_geometry::Point3d(A + vshift);
		if(HArc::TangentialArc(B, direction, new_A, centre, axis))
		{
			m_axis = gp_Ax1(centre, -axis);
			m_radius = new_A.Distance(centre);
			A = new_A;
		}
	}
	else if(B.IsEqual(vp, theApp.m_geom_tol)){
		geoff_geometry::Point3d direction = GetSegmentVector(0.0);
		geoff_geometry::Point3d centre;
		geoff_geometry::Point3d axis;
		geoff_geometry::Point3d new_B = geoff_geometry::Point3d(B + vshift);
		if(HArc::TangentialArc(A, direction, new_B, centre, axis))
		{
			m_axis = gp_Ax1(centre, axis);
			m_radius = A.Distance(centre);
			B = new_B;
		}
	}
#endif
	return false;
}

bool HArc::GetCentrePoint(geoff_geometry::Point3d &pos)
{
	pos = C;
	return true;
}

geoff_geometry::Point3d HArc::GetSegmentVector(double fraction)const
{
	geoff_geometry::Point3d centre = C;
	geoff_geometry::Point3d p = GetPointAtFraction(fraction);
	geoff_geometry::Point3d vp(centre, p);
	geoff_geometry::Point3d vd = geoff_geometry::Point3d(m_axis) ^ vp;
	vd.Normalize();
	return vd;
}

geoff_geometry::Point3d HArc::GetPointAtFraction(double fraction)const
{
#if 0 // to do
	if(A.IsEqual(B, theApp.m_geom_tol)){
		return A;
	}

	geoff_geometry::Point3d axis(C,m_axis.Direction());
	geoff_geometry::Point3d x_axis = axis.XDirection();
	geoff_geometry::Point3d y_axis = axis.YDirection();
	geoff_geometry::Point3d centre = C;

	double ax = geoff_geometry::Point3d(A - centre) * x_axis;
	double ay = geoff_geometry::Point3d(A - centre) * y_axis;
	double bx = geoff_geometry::Point3d(B - centre) * x_axis;
	double by = geoff_geometry::Point3d(B - centre) * y_axis;

	double start_angle = atan2(ay, ax);
	double end_angle = atan2(by, bx);

	if(start_angle > end_angle)end_angle += 6.28318530717958;

	double radius = m_radius;
	double d_angle = end_angle - start_angle;
	double angle = start_angle + d_angle * fraction;
    double x = radius * cos(angle);
    double y = radius * sin(angle);

	return centre + x * x_axis + y * y_axis;
#else
	return geoff_geometry::Point3d(0, 0, 0);
#endif
}

//static
bool HArc::TangentialArc(const geoff_geometry::Point3d &p0, const geoff_geometry::Point3d &v0, const geoff_geometry::Point3d &p1, geoff_geometry::Point3d &centre, geoff_geometry::Point3d &axis)
{
	// returns false if a straight line is needed
	// else returns true and sets centre and axis
	if(p0.Dist(p1) > 0.0000000001 && v0.magnitude() > 0.0000000001){
		geoff_geometry::Point3d v1(p0, p1);
		geoff_geometry::Point3d halfway(p0 + v1 * 0.5);
		geoff_geometry::Plane pl1(halfway, v1);
		geoff_geometry::Plane pl2(p0, v0);
		geoff_geometry::Line plane_line;
		if(pl1.Intof(pl2, plane_line))
		{
			geoff_geometry::Line l1(halfway, v1);
			geoff_geometry::Point3d unused_p2;
			geoff_geometry::Line lshort;
			double t1, t2;
			plane_line.Shortest(l1, lshort, t1, t2);
			centre = lshort.p0;
			axis = -(plane_line.v);
			return true;
		}
	}

	return false; // you'll have to do a line instead
}

void HArc::WriteXML(TiXmlNode *root)
{
	TiXmlElement *element = new TiXmlElement( "Arc" );
	root->LinkEndChild( element );
	element->SetDoubleAttribute("cx", C.x);
	element->SetDoubleAttribute("cy", C.y);
	element->SetDoubleAttribute("cz", C.z);
	element->SetDoubleAttribute("ax", m_axis.x);
	element->SetDoubleAttribute("ay", m_axis.y);
	element->SetDoubleAttribute("az", m_axis.z);
	WriteBaseXML(element);
}

// static member function
HeeksObj* HArc::ReadFromXMLElement(TiXmlElement* pElem)
{
	double axis[3];
	geoff_geometry::Point3d centre(0,0,0);
	HeeksColor c;

	// get the attributes
	double x;
	if(pElem->Attribute("ax", &x))axis[0] = x;
	if(pElem->Attribute("ay", &x))axis[1] = x;
	if(pElem->Attribute("az", &x))axis[2] = x;
	if(pElem->Attribute("cx", &x))centre.x = x;
	if(pElem->Attribute("cy", &x))centre.y = x;
	if(pElem->Attribute("cz", &x))centre.z = x;

	else
	{
		// try the version where the points were children
		int num_points = 0;
		for(TiXmlElement* pElem2 = TiXmlHandle(pElem).FirstChildElement().Element(); pElem2;	pElem2 = pElem2->NextSiblingElement())
		{
			HeeksObj* object = theApp.ReadXMLElement(pElem2);
			if(object->GetType() == PointType)
			{
				num_points++;
				if(num_points == 3)
				{
					centre = ((HPoint*)object)->m_p;
					delete object;
					break;
				}
			}
			delete object;
		}
	}

	HArc* new_object = new HArc(geoff_geometry::Point3d(), geoff_geometry::Point3d(), geoff_geometry::Point3d(), geoff_geometry::Point3d(), &c);
	new_object->ReadBaseXML(pElem);

	new_object->C = centre;
	new_object->m_axis = geoff_geometry::Point3d(axis);
	new_object->m_radius = centre.Dist(new_object->A);

	return new_object;
}

void HArc::Reverse()
{
	geoff_geometry::Point3d temp = A;
	A = B;
	B = temp;
	m_axis = -m_axis;
}

double HArc::IncludedAngle()const
{
	geoff_geometry::Point3d vs = GetSegmentVector(0.0);
	geoff_geometry::Point3d ve = GetSegmentVector(1.0);

	double inc_ang = vs * ve;
	int dir = (this->m_axis.z > 0) ? 1:-1;
	if(inc_ang > 1. - 1.0e-10) return 0;
	if(inc_ang < -1. + 1.0e-10)
		inc_ang = PI;  
	else {									// dot product,   v1 . v2  =  cos ang
		if(inc_ang > 1.0) inc_ang = 1.0;
		inc_ang = acos(inc_ang);									// 0 to M_PI radians

		if(dir * (vs ^ ve).z < 0) inc_ang = 2 * PI - inc_ang ;		// cp
	}
	return dir * inc_ang;
}
