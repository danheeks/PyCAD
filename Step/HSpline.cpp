// HSpline.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HSpline.h"
#include "Property.h"
#include "tinyxml.h"
#include "Gripper.h"

//static
int HSpline::m_type = 0;

HSpline::HSpline(const HSpline &s):EndedObject(){
	operator=(s);
}

HSpline::HSpline() : EndedObject(){
}

HSpline::HSpline(const Geom_BSplineCurve &s, const HeeksColor* col) :EndedObject(){
	m_spline = Handle(Geom_BSplineCurve)::DownCast(s.Copy());
	gp_Pnt p;
	m_spline->D0(m_spline->FirstParameter(), p);
	A = G2P(p);
	m_spline->D0(m_spline->LastParameter(), p);
	B = G2P(p);
	SetColor(*col);
}

HSpline::HSpline(Handle_Geom_BSplineCurve s, const HeeksColor* col):EndedObject(){
	m_spline = s;//Handle(Geom_BSplineCurve)::DownCast(s->Copy());
	gp_Pnt p;
	m_spline->D0(m_spline->FirstParameter(), p);
	A = G2P(p);
	m_spline->D0(m_spline->LastParameter(), p);
	B = G2P(p);
	SetColor(*col);
}

HSpline::HSpline(const std::list<gp_Pnt> &points, const HeeksColor* col, const gp_Vec* startVec, const gp_Vec* endVec):EndedObject()
{
	Standard_Boolean periodicity = points.front().IsEqual(points.back(), TOLERANCE);

	unsigned int size = points.size();
	if(periodicity == Standard_True)size--;

#ifdef _DEBUG
#undef new
#endif
	TColgp_HArray1OfPnt *Array = new TColgp_HArray1OfPnt(1, size);
#ifdef _DEBUG
#define new  WXDEBUG_NEW
#endif

	unsigned int i = 1;
	for(std::list<gp_Pnt>::const_iterator It = points.begin(); i <= size; It++, i++)
	{
		Array->SetValue(i, *It);
	}

	GeomAPI_Interpolate anInterpolation(Array, periodicity, Precision::Approximation());

	// If tangents provided, load them before performing interpolation
	if (startVec && endVec)
	{
		anInterpolation.Load(*startVec, *endVec, Standard_True);
	}
	else if (startVec)
	{
		anInterpolation.Load(*startVec, gp_Vec(0, 0, 0), Standard_False); // only start
	}
	else if (endVec)
	{
		anInterpolation.Load(gp_Vec(0, 0, 0), *endVec, Standard_False); // only end
	}

	anInterpolation.Perform();

	m_spline = anInterpolation.Curve();
	gp_Pnt p;
	m_spline->D0(m_spline->FirstParameter(), p);
	A = G2P(p);
	m_spline->D0(m_spline->LastParameter(), p);
	B = G2P(p);
	SetColor(*col);
}

HSpline::~HSpline(){
}

const HSpline& HSpline::operator=(const HSpline &s){
	EndedObject::operator=(s);
	m_spline = Handle(Geom_BSplineCurve)::DownCast((s.m_spline)->Copy());;
	return *this;
}

const wchar_t* HSpline::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/spline.png";
	return iconpath.c_str();
}

//segments - number of segments per full revolution!
void HSpline::GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point)const
{

	//TODO: calculate length
	double u0 = m_spline->FirstParameter();
	double u1 = m_spline->LastParameter();
	double uw = u1 - u0;

	gp_Pnt p0, phalf, p1;
	m_spline->D0(u0, p0);
	m_spline->D0(u0 + uw*0.5, phalf);
	m_spline->D0(u1, p1);
	double approx_length = p0.Distance(phalf) + phalf.Distance(p1);

	int segments = (int)(fabs(pixels_per_mm * approx_length + 1));
	double pp[3];

	for(int i = 0; i <= segments; i++)
    {
		gp_Pnt p;
		m_spline->D0(u0 + ((double)i / segments) * uw,p);
		extract(p, pp);
		(*callbackfunc)(pp);
    } 
}

static void glVertexFunction(const double *p){glVertex3d(p[0], p[1], p[2]);}

void HSpline::glCommands(bool select, bool marked, bool no_color){
	if(!no_color){
		theApp->glColorEnsuringContrast(*GetColor());
	}
	GLfloat save_depth_range[2];
	if(marked){
		glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
		glDepthRange(0, 0);
		glLineWidth(2);
	}

	glBegin(GL_LINE_STRIP);
	GetSegments(glVertexFunction, theApp->GetPixelScale());
	glEnd();

	if(marked){
		glLineWidth(1);
		glDepthRange(save_depth_range[0], save_depth_range[1]);
	}
}

HeeksObj *HSpline::MakeACopy(void)const{
		HSpline *new_object = new HSpline(*this);
		return new_object;
}

void HSpline::Transform(const Matrix& m){
	gp_Trsf mat = make_matrix(m.e);
	m_spline->Transform(mat);
}

void HSpline::GetBox(CBox &box){
	//TODO: get rid of magic number
	double pp[3];
	double u0 = m_spline->FirstParameter();
	double u1 = m_spline->LastParameter();
	double uw = u1 - u0;

	for(int i = 0; i <= 100; i++)
    {
		gp_Pnt p;
		m_spline->D0(u0 + uw * .01,p);
		extract(p, pp);
		box.Insert(pp);
    } 
}

void HSpline::GetGripperPositions(std::list<GripData> *list, bool just_for_endof){
	if(!just_for_endof)
	{
		for(int i=1; i <= m_spline->NbPoles(); i++)
		{
			gp_Pnt pole = m_spline->Pole(i);
			list->push_back(GripData(GripperTypeStretch,Point3d(pole.X(),pole.Y(),pole.Z()),NULL, true));
		}
	} 
}

void OnSetKnot(double v, HeeksObj* obj, int i)
{
	((HSpline*)obj)->m_spline->SetKnot(i,v);
}

void OnSetWeight(double v, HeeksObj* obj, int i)
{
	((HSpline*)obj)->m_spline->SetWeight(i,v);
}

void OnSetPole(const double *v, HeeksObj* obj, int i)
{
	gp_Pnt p = make_point(v);
	((HSpline*)obj)->m_spline->SetPole(i,p);
}


void HSpline::GetProperties(std::list<Property *> *list){
#if 0
	// to do
	wxChar str[512];
	for(int i=1; i <= m_spline->NbKnots(); i++)
	{
		wxSprintf(str,_T("%s %d"), _("Knot"), i);
		list->push_back(new PropertyDouble(str,m_spline->Knot(i),this,OnSetKnot,i));
		//TODO: Should add the multiplicity property. Complicated to change though.
		//Will blow up bspline unless a bunch of other parameters change as well
		//sprintf(str,"%s %d", _("Multiplicity"), i);
		//list->push_back(new PropertyInt(str,m_spline->Mult(i),this,OnSetMult,i));
	}
	for(int i=1; i <= m_spline->NbPoles(); i++)
	{
		double p[3];
		extract(m_spline->Pole(i),p);
		wxSprintf(str,_T("%s %d"), _("Pole"), i);
		list->push_back(new PropertyVertex(str,p,this,OnSetPole,i));
		wxSprintf(str,_T("%s %d"), _("Weight"), i);
		list->push_back(new PropertyDouble(str,m_spline->Weight(i),this,OnSetWeight,i));
	}
#endif

	EndedObject::GetProperties(list); 
}

bool HSpline::FindNearPoint(const double* ray_start, const double* ray_direction, double *point){
	gp_Lin ray(make_point(ray_start), make_vector(ray_direction));
	std::list< gp_Pnt > rl;

	return false; 
}

bool HSpline::FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point){
	// any point on this ellipse is a possible tangent point
	return FindNearPoint(ray_start, ray_direction, point);
}

bool HSpline::Stretch(const double *p, const double* shift, void* data){

	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	//There may be multiple control points at the same location, so we must move them all
	// This could break if user drags one set of points over another
	// The correct mapping of handles to control points can be determined
	// from the multiplicity set
	for(int i = 1; i <= m_spline->NbPoles(); i++)
	{
		if(m_spline->Pole(i).IsEqual(vp, TOLERANCE))
		{
			m_spline->SetPole(i,m_spline->Pole(i).XYZ() + vshift.XYZ());
		}
  	}
	return false; 
}

void HSpline::WriteToXML(TiXmlElement *element)
{
	element->SetAttribute("rational", m_spline->IsRational()?1:0);
	element->SetAttribute("periodic", m_spline->IsPeriodic()?1:0);
	element->SetAttribute("knots", m_spline->NbKnots());
	element->SetAttribute("poles", m_spline->NbPoles());
	element->SetAttribute("degree", m_spline->Degree());
	for(int i=1; i <= m_spline->NbPoles(); i++)
	{
		gp_Pnt polepnt = m_spline->Pole(i);
		double weight = m_spline->Weight(i);
		TiXmlElement *pole;
		pole = new TiXmlElement("Pole");
		element->LinkEndChild(pole);
		pole->SetAttribute("index",i);
		pole->SetDoubleAttribute("weight",weight);
		pole->SetDoubleAttribute("x", polepnt.X());
		pole->SetDoubleAttribute("y", polepnt.Y());
		pole->SetDoubleAttribute("z", polepnt.Z());
	}

	for(int i=1; i <= m_spline->NbKnots(); i++)
	{
		double knotdoub = m_spline->Knot(i);
		int mult = m_spline->Multiplicity(i);
		TiXmlElement *knot;
		knot = new TiXmlElement("Knot");
		element->LinkEndChild(knot);
		knot->SetAttribute("index",i);
		knot->SetDoubleAttribute("knot",knotdoub);
		knot->SetAttribute("mult", mult);
	}
	EndedObject::WriteToXML(element); 
}

void HSpline::ReadFromXML(TiXmlElement *element)
{
	HeeksColor c;
	bool rational = false;
	bool periodic = false;
	int nknots = 0;
	int npoles = 0;
	int degree = 0;

	// get the attributes
	for (TiXmlAttribute* a = element->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "rational"){rational = a->IntValue() != 0;}
		else if(name == "periodic"){periodic = a->IntValue() != 0;}
		else if(name == "knots"){nknots = a->IntValue();}
		else if(name == "poles"){npoles = a->IntValue();}
		else if(name == "degree"){degree = a->IntValue();}
	}

	TColgp_Array1OfPnt tkcontrol (1,npoles);
	TColStd_Array1OfReal tkweight (1,npoles);
	TColStd_Array1OfReal tkknot (1,nknots);
	TColStd_Array1OfInteger tkmult (1,nknots);



	// get the children

	TiXmlElement *pPole = element->FirstChildElement("Pole");
	for(int i=1; i <= npoles; i++)
	{
		double c[3];
		double weight;
		int index = 0;
		// get the attributes
		for(TiXmlAttribute* a = pPole->FirstAttribute(); a; a = a->Next())
		{
			std::string name(a->Name());
			if(name == "x"){c[0] = a->DoubleValue();}
			else if(name == "y"){c[1] = a->DoubleValue();}
			else if(name == "z"){c[2] = a->DoubleValue();}
			else if(name == "weight"){weight = a->DoubleValue();}
			else if(name == "index"){index = a->IntValue();}
		}

		tkcontrol.SetValue(index,make_point(c));
		tkweight.SetValue(index,weight);
		pPole = pPole->NextSiblingElement("Pole");
	}
	
	TiXmlElement *pKnot = element->FirstChildElement("Knot");
	for(int i=1; i <= nknots; i++)
	{
		double knot;
		int mult;
		int index = 0;
		// get the attributes
		for(TiXmlAttribute* a = pKnot->FirstAttribute(); a; a = a->Next())
		{
			std::string name(a->Name());
			if(name == "knot"){knot = a->DoubleValue();}
			else if(name == "mult"){mult = a->IntValue();}
			else if(name == "index"){index = a->IntValue();}
		}
		tkknot.SetValue(index,knot);
		tkmult.SetValue(index,mult);
		pKnot = pKnot->NextSiblingElement("Knot");
	}

	m_spline = new Geom_BSplineCurve(tkcontrol,tkweight,tkknot,tkmult,degree,periodic,rational);
        gp_Pnt p;
        m_spline->D0(m_spline->FirstParameter(), p);
        A = G2P(p);
        m_spline->D0(m_spline->LastParameter(), p);
        B = G2P(p);

	EndedObject::ReadFromXML(element);
}

int HSpline::Intersects(const HeeksObj *object, std::list< double > *rl)const
{
/*	int numi = 0;

	switch(object->GetType())
	{
	case LineType:
		{
			std::list<gp_Pnt> plist;
			intersect(((HLine*)object)->GetLine(), m_spline, plist);
			for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				gp_Pnt& pnt = *It;
				if(((HLine*)object)->Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case ILineType:
		{
			std::list<gp_Pnt> plist;
			intersect(((HILine*)object)->GetLine(), m_spline, plist);
			if(rl)convert_pnts_to_doubles(plist, *rl);
			numi += plist.size();
		}
		break;

	case ArcType:
		{
			std::list<gp_Pnt> plist;
			intersect(m_spline, ((HArc*)object)->m_circle, plist);
			for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				gp_Pnt& pnt = *It;
				if(((HArc*)object)->Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case CircleType:
		{
			std::list<gp_Pnt> plist;
			intersect(m_spline, ((HSpline*)object)->m_circle, plist);
			if(rl)convert_pnts_to_doubles(plist, *rl);
			numi += plist.size();
		}
		break; 
	}
*/
	return 0; //return numi; 
}

static bool calculate_biarc_points(const gp_Pnt &p0, gp_Vec v_start, const gp_Pnt &p4, gp_Vec v_end, gp_Pnt &p1, gp_Pnt &p2, gp_Pnt &p3)
{
	if(v_start.Magnitude() < 0.0000000001)v_start = gp_Vec(p0, p1);
    if(v_end.Magnitude() < 0.0000000001)v_end = gp_Vec(p3, p4);

	v_start.Normalize();
	v_end.Normalize();

    gp_Vec v = p0.XYZ() - p4.XYZ();

    double a = 2*(v_start*v_end-1);
    double c = v*v;
    double b = (v*2)*(v_start+v_end);

	if(fabs(a) < 0.000000000000001)return false;

    double d = b*b-4*a*c;

    if(d < 0.0)return false;

	double sd = sqrt(d);

    double e1 = (-b - sd) / (2.0 * a);
    double e2 = (-b + sd) / (2.0 * a);

    if(e1 > 0 && e2 > 0)return false;

	double e = e1;
	if(e2 > e)e = e2;

    if(e < 0)return false;

    p1 = p0.XYZ() + v_start.XYZ() * e;
    p3 = p4.XYZ() - v_end.XYZ() * e;
    p2 = p1.XYZ() * 0.5 + p3.XYZ() * 0.5;

	return true;
}

#if 0
static std::list<HeeksObj*>* new_spans_for_CreateArcs = NULL;
static double tolerance_for_CreateArcs = 1.0;
static Handle(Geom_BSplineCurve) spline_for_CreateArcs = NULL;

void CreateArcs(const gp_Pnt &p_start, const gp_Vec &v_start, double t_start, double t_end, gp_Pnt &p_end, gp_Vec &v_end)
{
	spline_for_CreateArcs->D1(t_end, p_end, v_end);

	gp_Pnt p1, p2, p3;

	bool can_do_spline_whole = calculate_biarc_points(p_start, v_start, p_end, v_end, p1, p2, p3);

	HeeksObj* arc_object1 = NULL;
	HeeksObj* arc_object2 = NULL;

	if(can_do_spline_whole)
	{
		CTangentialArc arc1(p_start, v_start, p2);
		CTangentialArc arc2(p2, gp_Vec(p3.XYZ() - p2.XYZ()), p_end);

		gp_Pnt p_middle1, p_middle2;
		spline_for_CreateArcs->D0(t_start + ((t_end - t_start) * 0.25), p_middle1);
		spline_for_CreateArcs->D0(t_start + ((t_end - t_start) * 0.75), p_middle2);

		if(!arc1.radius_equal(p_middle1, tolerance_for_CreateArcs) || !arc2.radius_equal(p_middle2, tolerance_for_CreateArcs))
			can_do_spline_whole = false;
		else
		{
			arc_object1 = arc1.MakeHArc();
			arc_object2 = arc2.MakeHArc();
		}
	}
	else
	{
		// calculate_biarc_points failed, just add a line
		new_spans_for_CreateArcs->push_back(new HLine(p_start, p_end, &(theApp->current_color)));
		return;
	}
	
	if(can_do_spline_whole)
	{
		new_spans_for_CreateArcs->push_back(arc_object1);
		new_spans_for_CreateArcs->push_back(arc_object2);
	}
	else
	{
		double t_middle = t_start + ((t_end - t_start) * 0.5);
		gp_Pnt p_middle;
		gp_Vec v_middle;
		CreateArcs(p_start, v_start, t_start, t_middle, p_middle, v_middle);// recursive
		gp_Pnt new_p_end;
		gp_Vec new_v_end;
		CreateArcs(p_middle, v_middle, t_middle, t_end, new_p_end, new_v_end);
	}
}
#endif

bool HSpline::GetStartPoint(double* pos)
{
	gp_Pnt p;
	m_spline->D0(m_spline->FirstParameter(), p);
	extract(p, pos);
	return true;
}

bool HSpline::GetEndPoint(double* pos)
{
	gp_Pnt p;
	m_spline->D0(m_spline->LastParameter(), p);
	extract(p, pos);
	return true;
}

#if 0
void HSpline::ToBiarcs(const Handle_Geom_BSplineCurve s, std::list<HeeksObj*> &new_spans, double tolerance, double first_parameter, double last_parameter)
{
	new_spans_for_CreateArcs = &new_spans;
	if(tolerance < 0.000000000000001)tolerance = 0.000000000000001;
	tolerance_for_CreateArcs = tolerance;
	gp_Pnt p_start;
	gp_Vec v_start;
	gp_Pnt p_end;
	gp_Vec v_end;
	s->D1(first_parameter, p_start, v_start);
	spline_for_CreateArcs = s;
	CreateArcs(p_start, v_start, first_parameter, last_parameter, p_end, v_end);
}

void HSpline::ToBiarcs(std::list<HeeksObj*> &new_spans, double tolerance)const
{
	ToBiarcs(m_spline, new_spans, tolerance, m_spline->FirstParameter(), m_spline->LastParameter());
}
#endif

void HSpline::Reverse()
{
	m_spline->Reverse();
}
