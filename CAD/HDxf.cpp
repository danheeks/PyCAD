// HDxf.cpp
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "HDxf.h"
#include "HLine.h"
#include "HArc.h"
#include "HCircle.h"
#if 0
#include "HEllipse.h"
#include "HSpline.h"
#endif
#include "HPoint.h"
#include "Sketch.h"
#if 0
#include "HText.h"
#endif
#include "strconv.h"

// static
bool HeeksDxfRead::m_make_as_sketch = false;
bool HeeksDxfRead::m_ignore_errors = false;
bool HeeksDxfRead::m_read_points = false;
std::wstring HeeksDxfRead::m_layer_name_suffixes_to_discard = L"_DOT,_DOTSMALL,_DOTBLANK,_OBLIQUE,_CLOSEDBLANK";
bool HeeksDxfRead::m_add_uninstanced_blocks = false;

HeeksDxfRead::HeeksDxfRead(const wchar_t* filepath, bool undoable) : CDxfRead(Ttc(filepath)), m_undoable(undoable)
{
	m_current_block = NULL;
	m_ucs_matrix = Matrix();
}

HeeksColor *HeeksDxfRead::ActiveColorPtr(Aci_t & aci)
{
	static HeeksColor color;
	color = HeeksColor(aci);
	return(&color);
}

HeeksColor hidden_color(128, 128, 128);

void HeeksDxfRead::OnReadUCS(const double* ucs_point)
{
#if 0
	to do
	Matrix tm;
	tm.SetTranslation(make_point(ucs_point), Point3d(0, 0, 0));
	extract(tm, m_ucs_matrix);
#endif
}

void HeeksDxfRead::OnReadBlock(const char* block_name, const double* base_point)
{
	if (m_blocks.find( std::wstring(Ctt(block_name)) ) == m_blocks.end())
	{
#if 0
		to do
		m_current_block = new CSketch();
		m_current_block->OnEditString(Ctt(block_name));
		m_blocks.insert( std::make_pair( std::wstring(Ctt(block_name)), m_current_block ) );
#endif
	}
	else
	{
		m_current_block = m_blocks[std::wstring(Ctt(block_name))];
	}
}

void HeeksDxfRead::OnReadInsert(const char* block_name, const double* insert_point, double rotation_angle)
{
	if (m_blocks.find( std::wstring(Ctt(block_name)) ) == m_blocks.end())
	{
		return; // block not foundblock_name, const double* insert_point, double rotation_angle
	}
	else
	{
#if 0
		to do
		BlockName_t b_name = std::wstring(Ctt(block_name));
		CSketch* block = m_blocks[b_name];
		CSketch* block_copy = new CSketch(*block);
		Matrix tm;
		tm.SetTranslationPart(make_vector(insert_point));
		Matrix rm;
		rm.SetRotation(gp_Ax1(Point3d(0, 0, 0), Point3d(0, 0, 1)), rotation_angle * 0.01745329251994329);
		double m[16];
		extract(tm * rm, m);
		block_copy->ModifyByMatrix(m);
		AddObject(block_copy);

		inserted_blocks.insert(b_name);
#endif
	}
}

void HeeksDxfRead::OnReadEndBlock()
{
	m_current_block = NULL;
}

void HeeksDxfRead::OnReadLine(const double* s, const double* e, bool hidden)
{
	HLine* new_object = new HLine(Point3d(s), Point3d(e), hidden ? (&hidden_color) : ActiveColorPtr(m_aci));
	if (m_thickness != 0.0)
	{
		new_object->m_thickness = m_thickness;
		for (int i = 0; i < 3; i++)new_object->m_extrusion_vector[i] = m_extrusion_vector[i];
	}
	AddObject(new_object);
}

void HeeksDxfRead::OnReadPoint(const double* s)
{
#if 0
	if(m_read_points)
	{
		HPoint* new_object = new HPoint(make_point(s), ActiveColorPtr(m_aci));
		AddObject(new_object);
	}
#endif
}

void HeeksDxfRead::OnReadArc(const double* s, const double* e, const double* c, bool dir, bool hidden)
{
	Point3d p0(s);
	Point3d p1(e);
	Point3d up(0, 0, 1);
	if(!dir)up = -up;
	Point3d pc(c);
	HArc* new_object = new HArc(p0, p1, up, c, hidden ? (&hidden_color) : ActiveColorPtr(m_aci));
	if (m_thickness != 0.0)
	{
		new_object->m_thickness = m_thickness;
		for (int i = 0; i < 3; i++)new_object->m_extrusion_vector[i] = m_extrusion_vector[i];
	}
	AddObject(new_object);
}

void HeeksDxfRead::OnReadCircle(const double* s, const double* c, bool dir, bool hidden)
{
#if 0
	to do
	Point3d p0 = make_point(s);
	//Point3d p1 = make_point(e);
	Point3d up(0, 0, 1);
	if(!dir)up = -up;
	Point3d pc = make_point(c);
	Circle circle(Point3d(pc, up), p0.Distance(pc));
	HCircle* new_object = new HCircle(circle, hidden ? (&hidden_color) : ActiveColorPtr(m_aci));
	if (m_thickness != 0.0)
	{
		new_object->m_thickness = m_thickness;
		for (int i = 0; i < 3; i++)new_object->m_extrusion_vector[i] = m_extrusion_vector[i];
	}
	AddObject(new_object);
#endif
}

#if 0
void HeeksDxfRead::OnReadSpline(TColgp_Array1OfPnt &control, TColStd_Array1OfReal &weight, TColStd_Array1OfReal &knot,TColStd_Array1OfInteger &mult, int degree, bool periodic, bool rational)
{
	try{
		Geom_BSplineCurve spline(control,weight,knot,mult,degree,periodic,rational);
		HSpline* new_object = new HSpline(spline, ActiveColorPtr(m_aci));
		AddObject(new_object);
	}
	catch(Standard_Failure)
	{
		if (! IgnoreErrors()) throw;	// Re-throw the exception.
	}
}
#endif

void HeeksDxfRead::OnReadSpline(struct SplineData& sd)
{
#if 0
	bool closed = (sd.flag & 1) != 0;
	bool periodic = (sd.flag & 2) != 0;
	bool rational = (sd.flag & 4) != 0;
	// bool planar = (sd.flag & 8) != 0;
	// bool linear = (sd.flag & 16) != 0;

	SplineData sd_copy = sd;

	if(closed)
	{
		// add some more control points
		sd_copy.control_points += 3;

		//for(int i = 0; i<3; i++
		//sd_copy.controlx
	}

	TColgp_Array1OfPnt control (1,/*closed ? sd.controlx.size() + 1:*/sd.controlx.size());
	TColStd_Array1OfReal weight (1,sd.controlx.size());

	std::list<double> knoto;
	std::list<int> multo;

	std::list<double>::iterator ity = sd.controly.begin();
	std::list<double>::iterator itz = sd.controlz.begin();
	std::list<double>::iterator itw = sd.weight.begin();

	unsigned i=1; //int i=1;
	for(std::list<double>::iterator itx = sd.controlx.begin(); itx!=sd.controlx.end(); ++itx)
	{
		Point3d pnt(*itx,*ity,*itz);
		control.SetValue(i,pnt);
		if(sd.weight.empty())
			weight.SetValue(i,1);
		else
		{
			weight.SetValue(i,*itw);
			++itw;
		}
		++i;
		++ity;
		++itz;
	}

	i=1;
	double last_knot = -1;
	for(std::list<double>::iterator it = sd.knot.begin(); it!=sd.knot.end(); ++it)
	{
		if(*it != last_knot)
		{
			knoto.push_back(*it);
			multo.push_back(1);
			i++;
		}
		else
		{
			int temp = multo.back();
			multo.pop_back();
			multo.push_back(temp+1);
		}
		last_knot = *it;
	}

	TColStd_Array1OfReal knot (1, knoto.size());
	TColStd_Array1OfInteger mult (1, knoto.size());

	std::list<int>::iterator itm = multo.begin();
	i = 1;
	for(std::list<double>::iterator it = knoto.begin(); it!=knoto.end(); ++it)
	{
		knot.SetValue(i,*it);
		int m = *itm;
		if(closed && (i == 1 || i == knoto.size()))m = 1;
		mult.SetValue(i, m);
		++itm;
		++i;
	}

	OnReadSpline(control, weight, knot, mult, sd.degree, periodic, rational);
#endif
}

void HeeksDxfRead::OnReadEllipse(const double* c, double major_radius, double minor_radius, double rotation, double start_angle, double end_angle, bool dir)
{
#if 0
	Point3d up(0, 0, 1);
	if(!dir)up = -up;
	Point3d pc = make_point(c);
	gp_Elips ellipse(Point3d(pc, up), major_radius, minor_radius);
	ellipse.Rotate(gp_Ax1(pc,up),rotation);
	HEllipse* new_object = new HEllipse(ellipse, start_angle, end_angle, ActiveColorPtr(m_aci));
	AddObject(new_object);
#endif
}

#if 0
	#define Slice(str, start, end) (str.Mid(start, end))
//Split (Tokenize) string at specified intervals
	//s == string to split
	//retArray == split up string (out)
	//cpszExp == expression to split at
	//crnStart == start postion to split
	//crnCount == max number of split of strings
	//crbCIComp == true if case insensitive
	void Split( const std::wstring& s, wxArrayString& retArray,  const wchar_t* cpszExp, 
				const size_t& crnStart = 0, const size_t& crnCount = (size_t)-1,
				const bool& crbCIComp = false)
	{
		//sanity checks
		wxASSERT_MSG(cpszExp != NULL, wxT("Invalid value for First Param of std::wstring::Split (cpszExp)"));
		//wxASSERT_MSG(crnCount >= (size_t)-1, wxT("Invalid value for Third Param of std::wstring::Split (crnCount)"));

		retArray.Clear();

		size_t  nOldPos = crnStart,	  //Current start position in this string
				nPos = crnStart;	  //Current end position in this string

		std::wstring szComp,			//this string as-is (if bCIComp is false) or converted to lowercase
				 szExp = cpszExp;   //Expression string, normal or lowercase

		if (crbCIComp)
		{
			szComp = s.Lower();
			szExp.MakeLower();
		}
		else
			szComp = s;

		if(crnCount == (size_t)-1)
		{
		for (; (nPos = szComp.find(szExp, nPos)) != std::wstring::npos;)//Is there another token in the string
			{
			retArray.Add(Slice(s, nOldPos, nPos)); //Insert the token in the array
			nOldPos = nPos += szExp.Length();//Move up the start slice position
			}
	   
		}
		else
		{
		for (int i = crnCount;
				(nPos = szComp.find(szExp, nPos)) != std::wstring::npos &&
				i != 0;
					--i)//Is there another token in the string && have we met nCount?
		{
			retArray.Add(Slice(s, nOldPos, nPos)); //Insert the token in the array
			nOldPos = nPos += szExp.Length();//Move up the start slice position
		}
		}
		if (nOldPos != s.Length())
			retArray.Add( Slice(s, nOldPos, s.Length()) ); //Add remaining characters in string
	}
#endif

void HeeksDxfRead::OnReadText(const double *point, const double height,  const char* text, int hj, int vj)
{
#if 0
	to do
	Matrix trsf;
	trsf.SetTranslation( Point3d( Point3d(0,0,0), Point3d(point[0], point[1], point[2]) ) );
	trsf.SetScaleFactor( height * 1.7 );

	std::wstring txt(Ctt(text));
	txt.Replace(_T("\\P"),_T("\n"),true);
	txt.Replace(_T("%%010"),_T("\n"),true);

	int offset = 0;
	while ((txt.Length() > 0) && (txt[0] == _T('\\')) && ((offset = txt.find(_T(';'))) != -1))
	{
		txt.Remove(0, offset+1);
	}

	wxArrayString retArray;
	Split(txt, retArray, _T("\n"));

	Matrix line_feed_shift;
	line_feed_shift.SetTranslationPart(Point3d(0, -height, 0));

	for(unsigned int i = 0; i<retArray.GetCount(); i++)
	{
		HText *new_object = new HText(trsf, retArray[i], ActiveColorPtr(m_aci),
#ifndef WIN32
			NULL, 
#endif
			hj, vj);
		AddObject(new_object);
		trsf = trsf * line_feed_shift;
	}
#endif
}

void HeeksDxfRead::OnReadDimension(int dimension_type, double angle, double angle2, double angle3, double radius_leader_length, const double *def_point, const double *mid, const double *p1, const double *p2, const double *p3, const double *p4, const double *p5)
{
	int type = (dimension_type & 0x07);

	Point3d d(def_point);
	Point3d m(mid);

	Point3d d_to_m(d, m);

	Point3d e = (m + d);

	Point3d forward(1,0,0);

	HeeksColor c(0, 0, 0);

	if(type == 0)
	{
#if 0
		to do
		forward.Rotate(gp_Ax1(Point3d(0, 0, 0), Point3d(0, 0, 1)), (angle/* + angle2 + angle3*/) * -0.01745329251);
#endif
	}
	else
	{
		forward = Point3d(-d_to_m);
		c = HeeksColor(255, 0, 0);
	}

	Point3d left(-forward.y, forward.x, 0.0);

	double arrow_size = 5.0;
	Point3d da1 = d + forward * (-arrow_size) + left * (arrow_size * 0.25);
	Point3d da2 = d + forward * (-arrow_size) + left * (arrow_size * -0.25);

	Point3d ea1 = e + forward * (arrow_size) + left * (arrow_size * 0.25);
	Point3d ea2 = e + forward * (arrow_size) + left * (arrow_size * -0.25);

#if 0
	to do
	AddObject(new HLine(d, da1, &c));
	AddObject(new HLine(da1, da2, &c));
	AddObject(new HLine(da2, d, &c));

	AddObject(new HLine(e, ea1, &c));
	AddObject(new HLine(ea1, ea2, &c));
	AddObject(new HLine(ea2, e, &c));
#endif
}

/**
	Don't add graphics for layer names included in discard-list:
	There are some graphics packages that add setup graphics that we don't want
	to be seen.
	This function discard names presents in m_layer_name_suffixes_to_discard
	(default: _DOT,_DOTSMALL,_DOTBLANK,_OBLIQUE,_CLOSEDBLANK) 
 */
bool HeeksDxfRead::IsValidLayerName( const std::wstring layer_name ) const
{
#if 0
	to do
	std::wstringTokenizer tokens(m_layer_name_suffixes_to_discard,_T(" :;,"));
	while (tokens.HasMoreTokens())
	{
		std::wstring token = tokens.GetNextToken();
		if (layer_name.Find(token) != wxNOT_FOUND)
		{
			return(false);  // We do NOT want this one added.
		}
	}
#endif

	return(true);   // This layername seems fine.
}

void HeeksDxfRead::AddObject(HeeksObj *object)
{
	if (! IsValidLayerName(Ctt(LayerName().c_str())))
	{
		// This is one of the forbidden layer names.  Discard the
		// graphics object and move on.

		delete object;
		return;
	}

	if(theApp->m_in_OpenFile && theApp->m_file_open_matrix)
	{
		object->Transform(*theApp->m_file_open_matrix);
	}

	if(m_make_as_sketch)
	{
		// Check to see if we've already added a sketch for the current layer name.  If not
		// then add one now.

		if (m_sketches.find( std::wstring(Ctt(LayerName().c_str())) ) == m_sketches.end())
		{
			m_sketches.insert( std::make_pair( std::wstring(Ctt(LayerName().c_str())), new CSketch() ) );
		}

		if(m_current_block)m_current_block->Add(object, NULL);
		else
		{
			object->Transform(m_ucs_matrix);
			m_sketches[std::wstring(Ctt(LayerName().c_str()))]->Add( object, NULL );
		}
	}
	else
	{
		if(m_current_block)m_current_block->Add(object, NULL);
		else
		{
			object->Transform(m_ucs_matrix);
			if(m_undoable)theApp->AddUndoably(object, NULL, NULL);
			else theApp->Add( object, NULL );
		}
	}
}

void HeeksDxfRead::AddGraphics()
{
#if 0
	to do
	// add one insert of any blocks which haven't been added at all
	m_current_block = NULL;
	if (HeeksDxfRead::m_add_uninstanced_blocks)
	{
		for (Blocks_t::const_iterator It = m_blocks.begin(); It != m_blocks.end(); It++)
		{
			if (inserted_blocks.find(It->first) == inserted_blocks.end())
			{
				CSketch* block = It->second;
				if (block->GetNumChildren() > 0)
				{
					CSketch* block_copy = new CSketch(*(It->second));
					AddObject(block_copy);
				}
			}
		}
	}

	if (m_make_as_sketch)
	{
		for (Sketches_t::const_iterator l_itSketch = m_sketches.begin(); l_itSketch != m_sketches.end(); l_itSketch++)
		{
			CSketch *pSketch = (CSketch *)(l_itSketch->second);
			if (pSketch->GetNumChildren() > 0)
			{
				((CSketch *)l_itSketch->second)->OnEditString( l_itSketch->first.c_str() );
				l_itSketch->second->ModifyByMatrix(m_ucs_matrix);
				if(m_undoable)theApp->AddUndoably(l_itSketch->second, NULL, NULL );
				else theApp->Add( l_itSketch->second, NULL );
			} // End if - then
		}
	}
#endif
}

