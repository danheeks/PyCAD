// svg.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

//**********************************************************************************
// Status and my 2 cents:
//		Most of the SVG spec is completely implemented. A bnf parser would be better,
//		but most of the attribute strings should be parsed correctly.
//		
//		SVG tags and attribute names should not be case sensitive, but they are.
//		they should also be able to be inside of namespaces, not sure if this works
//
//		Matrices are fundamentally broken, there is a note about that below
//
//		blocks and links don't work. not sure how these should be handled
//		autocad style or should they be exploded?
//
//		there was going to be a setting to explode curves to lines upon deserialization
//		but, it seems like a better idea to implement such a thing as a command in heekscad
//
//		Rounded rectangles aren't handled
//
//		Elliptic arcs need there start and end angles calculated, but there wasn't an elliptic
//		arc element at time of writing
//
//		Line widths and colors are not imported
//	
//		there are fundamentally different ways of deserializing the stream, ie.
//			exploding curves
//			using rects instead of lines	
//			grouping all objects in <g> tags with sketches
//			creating lines with linewidths, or faces of the right width
//		maybe the import command should give a dialog that populates a CDeserializationProfile?
//***********************************************************************************

#include "stdafx.h"
#include "svg.h"
#if 0
#include "HLine.h"
#include "HArc.h"
#include "HCircle.h"
#include "HEllipse.h"
#include "HSpline.h"
#include "Sketch.h"
#include "tinyxml.h"
#include "Area.h"
#include "HArea.h" // remove this
#endif
#include "strconv.h"

#include <sstream>

CSvgRead::CSvgRead(const wchar_t* filepath, bool usehspline, bool unite)
{
	m_fail = false;
	m_stroke_width = 0.0;
	m_fill_opacity = 0.0;
	m_current_area = NULL;
	m_usehspline = usehspline;
	m_unite = unite;
#if 0
	to do
	m_sketch = 0;
#endif
	Read(filepath);
}

void CSvgRead::Read(const wchar_t* filepath)
{
	// start the file
	m_fail = false;
	m_transform = geoff_geometry::Matrix();

	TiXmlDocument doc(Ttc(filepath));
	if (!doc.LoadFile())
	{
		if(doc.Error())
		{
			m_fail = true;
		}
		return;
	}

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	// block: name
	{
		pElem=hDoc.FirstChildElement().Element();
		if (!pElem) return;
		std::string name(pElem->Value());

		if(name != "svg")
		{
			m_fail = true;
			return;
		}

		// save this for later
		hRoot=TiXmlHandle(pElem);
	}

	// loop through all the objects
	for(pElem = hRoot.FirstChildElement().Element(); pElem;	pElem = pElem->NextSiblingElement())
	{
		ReadSVGElement(pElem);
	}

	// add the sketches
	if (theApp.m_svg_unite)
	{
#if 0
		to do
		std::list<HeeksObj*> objects_to_delete;
		CArea area;
		ConvertToArea::ObjectsToArea(m_sketches_to_add, objects_to_delete, area);

		area.Reorder(); // unite outsides which overlap and remove insides

		CSketch* new_object = MakeNewSketchFromArea(area);
		theApp.AddUndoably(new_object, NULL, NULL);
		theApp.DeleteUndoably(objects_to_delete);
#endif
	}
	else
	{
		theApp.AddUndoably(m_sketches_to_add, NULL);
	}

}

CSvgRead::~CSvgRead()
{
}

std::string CSvgRead::RemoveCommas(std::string input)
{
	//SVG allows for arbitrary whitespace and commas everywhere
	//sscanf ignores whitespace, so the only problem is commas
	for(unsigned int i=0; i < input.length(); i++)
		if(input[i] == ',')
			input[i] = ' ';
	return input;
}

void CSvgRead::ProcessArea()
{
#if 0
	// convert the current area into output lines and arcs
	for (std::list<CCurve>::iterator It = m_current_area->m_curves.begin(); It != m_current_area->m_curves.end(); It++)
	{
		CCurve& curve = *It;
		std::list<Span> spans;
		curve.GetSpans(spans);
		for (std::list<Span>::iterator It2 = spans.begin(); It2 != spans.end(); It2++)
		{
			Span& span = *It2;
			if (span.m_v.m_type == 0)
			{
				HLine *line = new HLine(geoff_geometry::Point3d(span.m_p.x, span.m_p.y, 0.0), geoff_geometry::Point3d(span.m_v.m_p.x, span.m_v.m_p.y, 0.0), &theApp.current_color);
				ModifyByMatrix(line);
				AddSketchIfNeeded();
				m_sketch->Add(line, NULL);
			}
			else
			{
				// add an arc
				HArc *arc = new HArc(geoff_geometry::Point3d(span.m_p.x, span.m_p.y, 0.0), geoff_geometry::Point3d(span.m_v.m_p.x, span.m_v.m_p.y, 0.0), gp_Circ(gp_Ax2(geoff_geometry::Point3d(span.m_v.m_c.x, span.m_v.m_c.y, 0.0), geoff_geometry::Point3d(0.0, 0.0, (span.m_v.m_type > 0)?1.0: -1.0)), span.m_p.dist(span.m_v.m_c)), &theApp.current_color);
				ModifyByMatrix(arc);
				AddSketchIfNeeded();
				m_sketch->Add(arc, NULL);
			}
		}
	}
#endif
}

void CSvgRead::ReadStyle(TiXmlAttribute* a)
{
	const char* s = a->Value();
	unsigned int len = strlen(s);
	std::string name, value;
	bool reading_name = true;
	for (unsigned int i = 0; i < len; i++)
	{
		if (s[i] == ':')reading_name = false;
		else if (s[i] == ';')
		{
			if (name == "stroke-width")sscanf(value.c_str(), "%lf", &m_stroke_width);
			else if (name == "fill-opacity")sscanf(value.c_str(), "%lf", &m_fill_opacity);
			reading_name = true;
			name = "";
			value = "";
		}
		else if (s[i] != ' ')
		{
			if (reading_name)name += s[i];
			else value += s[i];
		}
	}
//fill:#840000; fill - opacity:0.0;  stroke:#840000; stroke - width:629.921; stroke - opacity:1;  stroke - linecap:round; stroke - linejoin:round;
}

void CSvgRead::ReadG(TiXmlElement* pElem)
{
#if 0
	m_current_area = new CArea;
	m_sketch = NULL; // start a new sketch
	//CArea::m_fit_arcs = false;

	// get the attributes
	for (TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if (name == "style")ReadStyle(a);
	}

	// loop through all the child elements, looking for path
	for (pElem = TiXmlHandle(pElem).FirstChildElement().Element(); pElem; pElem = pElem->NextSiblingElement())
	{
		ReadSVGElement(pElem);
	}

	//m_current_area->Reorder();
	ProcessArea();
	delete m_current_area;
	m_current_area = NULL;
#endif
}

void CSvgRead::ReadSVGElement(TiXmlElement* pElem)
{
	std::string name(pElem->Value());
	m_transform_stack.push_back(m_transform);

	ReadTransform(pElem);

	if(name == "g")
	{
		ReadG(pElem);
	}

	if(name == "path")
	{
		ReadPath(pElem);
	}

	if(name == "rect")
	{
		ReadRect(pElem);
	}

	if(name == "circle")
	{
		ReadCircle(pElem);
	}

	if(name == "ellipse")
	{
		ReadEllipse(pElem);
	}
	
	if(name == "line")
	{
		ReadLine(pElem);
	}

	if(name == "polyline")
	{
		ReadPolyline(pElem,false);
	}

	if(name == "polygon")
	{
		ReadPolyline(pElem,true);
	}

	m_transform = m_transform_stack.back();
	m_transform_stack.pop_back();
	

}

void CSvgRead::ReadTransform(TiXmlElement *pElem)
{
#if 0
	// TODO: there are lots of default parameters used in transforms
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "transform")
		{
			std::string s = a->Value();
			s = RemoveCommas(s);
			const char* d = s.c_str();
			int pos=0;
			int count=0;
			geoff_geometry::Matrix ntrsf;
			if(d[pos] == 0)
				break;
			if(strncmp(&d[pos],"translate",9)==0)
			{
				double x,y;
				sscanf(&d[pos],"translate(%lf %lf)%n",&x,&y,&count);
				y=-y;
				ntrsf.SetTranslationPart(geoff_geometry::Point3d(x,y,0));
				m_transform.Multiply(ntrsf);
				pos+=count;
			}
			if(strncmp(&d[pos],"matrix",6)==0)
			{
				double m[16];
				sscanf(&d[pos],"matrix(%lf %lf %lf %lf %lf %lf)%n",&m[0],&m[4],&m[1],&m[5],&m[3],&m[7],&count);
				m[2]=0;
				m[6]=0;
				m[8]=0;
				m[9]=0;
				m[10]=1;
				m[11]=0;
				m[12]=0;
				m[13]=0;
				m[14]=0;
				m[15]=1;

				double d = m[0]*m[5]-m[4]*m[1];
				m[10] = sqrt(d); //The Z component must be of the same magnitude as the rest of
				//the matrix. It really makes no difference what it is, since all z's are 0
				
				//TODO: Uncomment the following lines for matrix support. 
				//Opencascade doesn't support assymetric transforms(non uniform matrices)
				//unforunately most matrix transforms are non uniform, so this usually just 
				//throws exceptions

				//In all probability we will have to transform all shapes by the assymetric matrix
				//this is tricky for ellipses and such
				//probably a v3 feature

				//:JonPry
				
				// ntrsf = make_matrix(m);
				// m_transform.Multiply(ntrsf);	
				pos+=count;
			}
			if(strncmp(&d[pos],"skewX",5)==0)
			{
				//TODO: see above, these are assymetric transforms
				double skew=0;
				sscanf(&d[pos],"skewX(%lf)%n",&skew,&count);
				pos+=count;
			}
			if(strncmp(&d[pos],"skewY",5)==0)
			{
				//TODO: see above, these are assymetric transforms
				double skew=0;
				sscanf(&d[pos],"skewY(%lf)%n",&skew,&count);
				pos+=count;
			}
			if(strncmp(&d[pos],"scale",5)==0)
			{
				double x=0;
				double y=0;
				sscanf(&d[pos],"scale(%lf %lf)%n",&x,&y,&count);
				if(y==0)
					y=x;
				//TODO: assymetric scaling
				ntrsf.SetScale(geoff_geometry::Point3d(0,0,0),x);
				m_transform.Multiply(ntrsf);
				pos+=count;
			}
			if(strncmp(&d[pos],"rotate",6)==0)
			{
				double rot;
				sscanf(&d[pos],"rotate(%lf)%n",&rot,&count);
				ntrsf.SetRotation(gp_Ax1(geoff_geometry::Point3d(0,0,0),geoff_geometry::Point3d(0,0,1)),3*M_PI/2-rot);
				m_transform.Multiply(ntrsf);
				pos+=count;

			}
		}
	}
#endif
}

void CSvgRead::ReadRect(TiXmlElement *pElem)
{
	double x = 0,y = 0,width = 0,height = 0;
	double rx=0; 
	double ry=0;
	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "x")x=a->DoubleValue();
		if(name == "y")y=a->DoubleValue();
		if(name == "width")width=a->DoubleValue();
		if(name == "height")height=a->DoubleValue();
		if(name == "rx")rx=a->DoubleValue();
		if(name == "ry")ry=a->DoubleValue();
	}

	y=-y;height=-height;

	geoff_geometry::Point3d p1(x,y,0);
	geoff_geometry::Point3d p2(x+width,y,0);
	geoff_geometry::Point3d p3(x+width,y+height,0);
	geoff_geometry::Point3d p4(x,y+height,0);

	//TODO: add rounded rectangle support

	OnReadLine(p1,p2);
	OnReadLine(p2,p3);
	OnReadLine(p3,p4);
	OnReadLine(p4,p1);
}

void CSvgRead::ReadEllipse(TiXmlElement *pElem)
{
	double x=0;
	double y=0;
	double rx=0; 
	double ry=0;
	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "cx")x=a->DoubleValue();
		if(name == "cy")y=a->DoubleValue();
		if(name == "rx")rx=a->DoubleValue();
		if(name == "ry")ry=a->DoubleValue();
	}

	y=-y;

	double rot = 0;
	if(ry>rx)
	{
		double temp = rx;
		rx = ry;
		ry = temp;
		rot = PI / 2;
	}
	OnReadEllipse(geoff_geometry::Point3d(x,y,0),rx,ry,rot,0,2*PI);
}

void CSvgRead::ReadLine(TiXmlElement *pElem)
{
	double x1=0;
	double y1=0;
	double x2=0; 
	double y2=0;
	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "x1")x1=a->DoubleValue();
		if(name == "y1")y1=a->DoubleValue();
		if(name == "x2")x2=a->DoubleValue();
		if(name == "y2")y2=a->DoubleValue();
	}

	y1=-y1; y2=-y2;

	OnReadLine(geoff_geometry::Point3d(x1,y1,0),geoff_geometry::Point3d(x2,y2,0));
}

void CSvgRead::ReadPolyline(TiXmlElement *pElem, bool close)
{
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "points")
		{
			const char* d = a->Value();
			geoff_geometry::Point3d ppnt(0,0,0);
			geoff_geometry::Point3d spnt(0,0,0);
			bool has_point = false;

			int pos = 0;
			while(1){
				if(d[pos] == 0){
					if(has_point && close)
						OnReadLine(ppnt,spnt);
					break;
				}
				if(isdigit(d[pos])){
					double x,y;
					int count=0;
					sscanf(&d[pos],"%lf,%lf%n",&x,&y,&count);
					y=-y;
					geoff_geometry::Point3d cpnt(x,y,0);
					if(has_point)
						OnReadLine(ppnt,cpnt);
					else
					{
						has_point = true;
						spnt = cpnt;
					}
					ppnt = cpnt;
					pos+=count;
				}
				else{
					pos++;
				}
			}
		}
	}
}


void CSvgRead::ReadCircle(TiXmlElement *pElem)
{
	double x = 0,y = 0,r = 0;
	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "cx")x=a->DoubleValue();
		if(name == "cy")y=a->DoubleValue();
		if(name == "r")r=a->DoubleValue();
	}
	y=-y;

	geoff_geometry::Point3d cp(x,y,0);
	OnReadCircle(cp,r);
}

geoff_geometry::Point3d CSvgRead::ReadStart(const char *text,geoff_geometry::Point3d ppnt,bool isupper)
{
#if 0
	double x, y;
cout << '@' << text <<'@';
	sscanf(text, "%lf%lf", &x, &y);
	y = -y;
	geoff_geometry::Point3d npt(x,y,0);
	if(!isupper)
	{
		npt.SetX(x+ppnt.x);
		npt.SetY(y+ppnt.y);
	}
	OnReadStart();
	return npt;
#else
	return geoff_geometry::Point3d(0, 0, 0);
#endif
}

geoff_geometry::Point3d CSvgRead::ReadLine(const char *text,geoff_geometry::Point3d ppnt,bool isupper)
{
#if 0
	double x, y;
	sscanf(text, "%lf%lf", &x, &y);
	y = -y;
	geoff_geometry::Point3d npt(x,y,0);
	if(!isupper)
	{
		npt.SetX(x+ppnt.x);
		npt.SetY(y+ppnt.y);
	}
	OnReadLine(ppnt,npt);
	return npt;
#else
	return geoff_geometry::Point3d(0, 0, 0);
#endif
}

void CSvgRead::ReadClose(geoff_geometry::Point3d ppnt,geoff_geometry::Point3d spnt)
{
	OnReadLine(ppnt,spnt);
}

geoff_geometry::Point3d CSvgRead::ReadHorizontal(const char *text,geoff_geometry::Point3d ppnt,bool isupper)
{
#if 0
	double x;
	sscanf(text, "%lf", &x);
	geoff_geometry::Point3d npt(x,ppnt.y,0);
	if(!isupper)
		npt.SetX(x+ppnt.x);
	OnReadLine(ppnt,npt);
	return npt;
#else
	return geoff_geometry::Point3d(0, 0, 0);
#endif
}

geoff_geometry::Point3d CSvgRead::ReadVertical(const char *text,geoff_geometry::Point3d ppnt,bool isupper)
{
#if 0
	double y;
	sscanf(text, "%lf", &y);
	y = -y;
	geoff_geometry::Point3d npt(ppnt.x,y,0);
	if(!isupper)
		npt.SetY(y+ppnt.y);
	OnReadLine(ppnt,npt);
	return npt;
#else
	return geoff_geometry::Point3d(0, 0, 0);
#endif
}


struct TwoPoints CSvgRead::ReadCubic(const char *text,geoff_geometry::Point3d ppnt,bool isupper)
{
	struct TwoPoints retpts;
	double x1, y1, x2, y2, x3, y3;
	sscanf(text, "%lf%lf%lf%lf%lf%lf", &x1, &y1, &x2, &y2, &x3, &y3);
	y1 = -y1; y2 = -y2; y3 = -y3;	
cout << "x1=" << x1 << "y1=" << y1 << "x2=" << x2 << "y2=" << y2 << "x3=" << x3 << "y3=" << y3 << "\n";
// All points appear to be relative to the first
	if(!isupper)
	{
		x1+=ppnt.x; y1+=ppnt.y;
//		x2+=x1;	y2+=y1;
//		x3+=x2;	y3+=y2;
		x2+=ppnt.x; y2+=ppnt.y;
		x3+=ppnt.x; y3+=ppnt.y;
cout << "x1=" << x1 << "y1=" << y1 << "x2=" << x2 << "y2=" << y2 << "x3=" << x3 << "y3=" << y3 << "\n";
	}

	geoff_geometry::Point3d pnt1(x1,y1,0);
	retpts.pcpnt = geoff_geometry::Point3d(x2,y2,0);
	retpts.ppnt = geoff_geometry::Point3d(x3,y3,0);

	OnReadCubic(ppnt,pnt1,retpts.pcpnt,retpts.ppnt);
	return retpts;
}

struct TwoPoints CSvgRead::ReadCubic(const char *text,geoff_geometry::Point3d ppnt, geoff_geometry::Point3d pcpnt, bool isupper)
{
	struct TwoPoints retpts;
	double x2, y2, x3, y3;
	sscanf(text, "%lf%lf%lf%lf", &x2, &y2, &x3, &y3);
	y2 = -y2; y3 = -y3;	

	if(!isupper)
	{
		x2+=ppnt.x; y2+=ppnt.y;
		x3+=ppnt.x; y3+=ppnt.y;
	//	x3+=x2;	y3+=y2;
	}

#if 0
	geoff_geometry::Point3d dir=ppnt.XYZ()-pcpnt.XYZ();
	double d = ppnt.Distance(pcpnt);
	geoff_geometry::Point3d pnt1(ppnt.XYZ() + dir.XYZ() * d);
	retpts.pcpnt = geoff_geometry::Point3d(x2,y2,0);
	retpts.ppnt = geoff_geometry::Point3d(x3,y3,0);

	OnReadCubic(ppnt,pnt1,retpts.pcpnt,retpts.ppnt);
#endif

	return retpts;
}

struct TwoPoints CSvgRead::ReadQuadratic(const char *text,geoff_geometry::Point3d ppnt,bool isupper)
{
	struct TwoPoints retpts;
	double x1, y1, x2, y2;
	sscanf(text, "%lf%lf%lf%lf", &x1, &y1, &x2, &y2);
	y1 = -y1; y2 = -y2; 

	if(!isupper)
	{
		x1+=ppnt.x; y1+=ppnt.y;
		x2+=ppnt.x; y2+=ppnt.y;
	}

	retpts.pcpnt = geoff_geometry::Point3d(x1,y1,0);
	retpts.ppnt = geoff_geometry::Point3d(x2,y2,0);

	OnReadQuadratic(ppnt,retpts.pcpnt,retpts.ppnt);
	return retpts;
}

struct TwoPoints CSvgRead::ReadQuadratic(const char *text,geoff_geometry::Point3d ppnt, geoff_geometry::Point3d pcpnt, bool isupper)
{
	struct TwoPoints retpts;
	double x2, y2;
	sscanf(text, "%lf%lf", &x2, &y2);
	y2 = -y2;

	if(!isupper)
	{
		x2+=ppnt.x; y2+=ppnt.y;
	}
#if 0
	geoff_geometry::Point3d dir=ppnt.XYZ()-pcpnt.XYZ();
	double d = ppnt.Distance(pcpnt);
	geoff_geometry::Point3d pnt1(ppnt.XYZ() + dir.XYZ() * d);
	retpts.pcpnt = pnt1;
	retpts.ppnt = geoff_geometry::Point3d(x2,y2,0);

	OnReadQuadratic(ppnt,retpts.pcpnt,retpts.ppnt);
#endif
	return retpts;
}

geoff_geometry::Point3d CSvgRead::ReadEllipse(const char *text,geoff_geometry::Point3d ppnt,bool isupper)
{
#if 0
	int large_arc_flag, sweep_flag;
	double rx, ry, xrot, x, y;
	sscanf(text, "%lf%lf%lf%d%d%lf%lf", &rx, &ry, &xrot, &large_arc_flag, &sweep_flag, &x, &y);
	y=-y;
	if(!isupper)
	{
		x+=ppnt.x; y+=ppnt.y;
	}

	xrot = -M_PI*xrot/180.0;

	geoff_geometry::Point3d ept(x,y,0);
	geoff_geometry::Point3d up(0,0,1);
	geoff_geometry::Point3d zp(0,0,0);

	//from http://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes

	geoff_geometry::Point3d mid((ppnt.XYZ()-ept.XYZ())/2);
	mid.Rotate(gp_Ax1(zp,up),-xrot);
	
	double error = mid.x * mid.x/(rx*rx) + mid.y*mid.y/(ry*ry);
		if(error > 1-theApp.m_geom_tol)
	{
		rx *= sqrt(error) + theApp.m_geom_tol;
		ry *= sqrt(error) + theApp.m_geom_tol;
	}

	double root = sqrt((rx*rx*ry*ry-rx*rx*mid.y*mid.y-ry*ry*mid.x*mid.x)/(rx*rx*mid.y*mid.y+ry*ry*mid.x*mid.x));
	double cx = root * rx * mid.y / ry;
	double cy = root * -ry * mid.x / rx;
	if(large_arc_flag != sweep_flag)
	{
		cx = -cx; cy = -cy;
	}
	geoff_geometry::Point3d cvec(cx,cy,0);
	cvec.Rotate(gp_Ax1(zp,up),xrot);
	geoff_geometry::Point3d cpnt(cvec.XYZ() + (ept.XYZ()+ppnt.XYZ())/2);

	if(rx<ry)
	{
		double temp = ry;
		ry = rx;
		rx = temp;
		xrot += M_PI/2;
	}

	geoff_geometry::Point3d start(ppnt.XYZ() - cpnt.XYZ());
	geoff_geometry::Point3d end(ept.XYZ()-cpnt.XYZ());
	start.Rotate(gp_Ax1(zp,up),-xrot);
	end.Rotate(gp_Ax1(zp,up),-xrot);

	double start_angle = atan2(start.y/ry,start.x/rx);
	double end_angle = atan2(end.y/ry,end.x/rx);

	if(start_angle<0)
		start_angle+=2*M_PI;
	if(end_angle<0)
		end_angle+=2*M_PI;

	double d_angle = end_angle - start_angle;
	
	if(d_angle < 0)
		d_angle += 2*M_PI;

	if((large_arc_flag && (d_angle < M_PI)) || (!large_arc_flag && (d_angle > M_PI)))
	{
		double temp = start_angle;
		start_angle = end_angle;
		end_angle = temp;
	}

	OnReadEllipse(cpnt,rx,ry,xrot,start_angle,end_angle);
#else
	geoff_geometry::Point3d ept;
#endif
	return ept;
}

// because we can no longer take the first numbers after a command, we have to jump values we have already read in
int CSvgRead::JumpValues(const char *text, int number){
	int pos=0;
	if(text[pos]==32)
		pos++;
	while(number >0 && text[pos]!=0){
		if(text[pos]==32){
			number--;
		}else if(text[pos]==0){
			return pos;
		}
		pos++;
	}
	return pos;
}

void CSvgRead::ReadPath(TiXmlElement* pElem)
{
	char cmd;
	// get the attributes
	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "d")
		{
			// add lines and arcs and bezier curves
			std::string in(a->Value());
			in = RemoveCommas(in);
			const char* d = in.c_str();
			int length = strlen(d);
			geoff_geometry::Point3d spnt(0,0,0);
			geoff_geometry::Point3d ppnt(0,0,0);
			geoff_geometry::Point3d pcpnt(0,0,0);
			int pos = 0;
			cmd = 'l';
			while(1){
// if we are looking at a command letter reset the cmd variable
				if(toupper(d[pos])=='M' || toupper(d[pos])=='L' || toupper(d[pos])=='H' || toupper(d[pos])=='V' || toupper(d[pos])=='C' || toupper(d[pos])=='S' || toupper(d[pos])=='Q' || toupper(d[pos])=='T' || toupper(d[pos])=='A' || toupper(d[pos])=='Z' || toupper(d[pos])==' ' ){
					cmd = d[pos];
					pos++;
				}
				if(toupper(cmd) == 'M'){
					// make a sketch
					spnt = ReadStart(&d[pos],ppnt,isupper(cmd)!=0);
					ppnt = spnt;
					pos+=JumpValues(&d[pos],2);
// If there is no command after a move it seems to assume the next one is an l
					cmd = 'l';
				}
				else if(toupper(cmd) == 'L'){
					// add a line
					ppnt = ReadLine(&d[pos],ppnt,isupper(cmd)!=0);
					pos+=JumpValues(&d[pos],2);
				}
				else if(toupper(cmd) == 'H'){
					//horizontal line
					ppnt = ReadHorizontal(&d[pos],ppnt,isupper(cmd)!=0);
					pos+=JumpValues(&d[pos],1);
				}
				else if(toupper(cmd) == 'V'){
					//vertical line
					ppnt = ReadVertical(&d[pos],ppnt,isupper(cmd)!=0);
					pos+=JumpValues(&d[pos],1);
				}
				else if(toupper(cmd) == 'C'){
					// add a cubic bezier curve ( just split into lines for now )
					struct TwoPoints ret = ReadCubic(&d[pos],ppnt,isupper(cmd)!=0);
					ppnt = ret.ppnt;
					pcpnt = ret.pcpnt;
					pos+=JumpValues(&d[pos],6);
				}
				else if(toupper(cmd) == 'S'){
                    // add a cubic bezier curve ( short hand)
					struct TwoPoints ret = ReadCubic(&d[pos],ppnt,pcpnt,isupper(cmd)!=0);
					ppnt = ret.ppnt;
					pcpnt = ret.pcpnt;
					pos+=JumpValues(&d[pos],4);
				}
				else if(toupper(cmd) == 'Q'){
					// add a quadratic bezier curve 
					struct TwoPoints ret = ReadQuadratic(&d[pos+1],ppnt,isupper(cmd)!=0);
					ppnt = ret.ppnt;
					pcpnt = ret.pcpnt;
					pos+=JumpValues(&d[pos],4);
				}
				else if(toupper(cmd) == 'T'){
               		// add a quadratic bezier curve 
					struct TwoPoints ret = ReadQuadratic(&d[pos],ppnt,pcpnt,isupper(cmd)!=0);
					ppnt = ret.ppnt;
					pcpnt = ret.pcpnt;
					pos+=JumpValues(&d[pos],2);
				}
				else if(toupper(cmd) == 'A'){
					// add an elliptic arc
					ppnt = ReadEllipse(&d[pos+1],ppnt,isupper(cmd)!=0);
					pos+=JumpValues(&d[pos],7);
				}
				else if(toupper(cmd) == 'Z'){
					// join to end
					ReadClose(ppnt,spnt);
					ppnt = spnt;
				}
				else if(toupper(cmd) == ' '){
				}
				else if(d[pos] == 0 ){
					break;
				}
				else{
					pos++;
				}
// if we have got to the end of the string, stop
				if(pos>=length){
					break;
				}
				if(pos<0) {
					break;
				}
			}
		}
	}
}

void CSvgRead::ModifyByMatrix(HeeksObj* object)
{
	object->Transform(m_transform);
}

void CSvgRead::OnReadStart()
{
}

void CSvgRead::OnReadCubic(geoff_geometry::Point3d s, geoff_geometry::Point3d c1, geoff_geometry::Point3d c2, geoff_geometry::Point3d e)
{
#if 0
	TColgp_Array1OfPnt poles(1,4);
	poles.SetValue(1,s); poles.SetValue(2,c1); poles.SetValue(3,c2); poles.SetValue(4,e);
#ifdef _DEBUG
#undef new
#endif
	Handle(Geom_BezierCurve) curve = new Geom_BezierCurve(poles);
#ifdef _DEBUG
#define new  WXDEBUG_NEW
#endif
	GeomConvert_CompCurveToBSplineCurve convert(curve);

	Handle_Geom_BSplineCurve spline = convert.BSplineCurve();
//	Geom_BSplineCurve pspline = *((Geom_BSplineCurve*)spline.Access());
	HSpline* new_object = new HSpline(spline, &theApp.current_color);
	ModifyByMatrix(new_object);
	AddSketchIfNeeded();
	m_sketch->Add(new_object, NULL);
#endif
}

void CSvgRead::OnReadQuadratic(geoff_geometry::Point3d s, geoff_geometry::Point3d c, geoff_geometry::Point3d e)
{
#if 0
	TColgp_Array1OfPnt poles(1, 3);
	poles.SetValue(1, s); poles.SetValue(2, c); poles.SetValue(3, e);
#ifdef _DEBUG
#undef new
#endif
	Handle(Geom_BezierCurve) curve = new Geom_BezierCurve(poles);
#ifdef _DEBUG
#define new  WXDEBUG_NEW
#endif
	GeomConvert_CompCurveToBSplineCurve convert(curve);

	Handle_Geom_BSplineCurve spline = convert.BSplineCurve();
	//	Geom_BSplineCurve pspline = *((Geom_BSplineCurve*)spline.Access());
	HSpline* new_object = new HSpline(spline, &theApp.current_color);
	ModifyByMatrix(new_object);
	AddSketchIfNeeded();
	m_sketch->Add(new_object, NULL);
#endif
}

void CSvgRead::OnReadLine(geoff_geometry::Point3d p1, geoff_geometry::Point3d p2)
{
#if 0
	if (m_current_area != NULL && m_stroke_width > 0.00001)
	{
		// add an obround to the current area
		CCurve curve;
		curve.append(CVertex(Point(p1.x, p1.y)));
		curve.append(CVertex(Point(p2.x, p2.y)));
		CArea area;
		area.append(curve);
		area.Thicken(m_stroke_width * 0.5);
		//m_current_area->Union(area);
		for (std::list<CCurve>::iterator It = area.m_curves.begin(); It != area.m_curves.end(); It++)
		{
			CCurve& c = *It;
			m_current_area->append(c);
		}
	}
	else
	{
		HLine *line = new HLine(p1, p2, &theApp.current_color);
		ModifyByMatrix(line);
		AddSketchIfNeeded();
		m_sketch->Add(line, NULL);
	}
#endif
}

void CSvgRead::OnReadEllipse(geoff_geometry::Point3d c, double maj_r, double min_r, double rot, double start, double end)
{
#if 0
	geoff_geometry::Point3d up(0,0,1);
	gp_Elips elip(gp_Ax2(c,geoff_geometry::Point3d(0,0,1)),maj_r,min_r);
	elip.Rotate(gp_Ax1(c,up),rot);
	HEllipse *new_object = new HEllipse(elip,start,end,&theApp.current_color);
	ModifyByMatrix(new_object);
	AddSketchIfNeeded();
	m_sketch->Add(new_object, NULL);
#endif
}

void CSvgRead::OnReadCircle(geoff_geometry::Point3d c, double r)
{
#if 0
	geoff_geometry::Point3d up(0,0,1);
	gp_Circ cir(gp_Ax2(c,up),r);
	HCircle *new_object = new HCircle(cir,&theApp.current_color);
	ModifyByMatrix(new_object);
	AddSketchIfNeeded();
	m_sketch->Add(new_object, NULL);
#endif
}

void CSvgRead::AddSketchIfNeeded()
{
#if 0
	if(m_sketch == NULL)
	{
		m_sketch = new CSketch();
		m_sketches_to_add.push_back(m_sketch);
	}
#endif
}
