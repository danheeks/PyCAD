// RegularShapesDrawing.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "RegularShapesDrawing.h"
#if 0
#include "Sketch.h"
#include "HLine.h"
#include "HArc.h"
#endif
#include "Property.h"

RegularShapesDrawing regular_shapes_drawing;

RegularShapesDrawing::RegularShapesDrawing(void)
{
	m_mode = RectanglesRegularShapeMode;
	m_number_of_side_for_polygon = 6;
	m_rect_radius = 0.0;
	m_obround_radius = 2.0;
}

RegularShapesDrawing::~RegularShapesDrawing(void)
{
}

void RegularShapesDrawing::ClearSketch()
{
#if 0
	to do
	if(TempObject())((CSketch*)TempObject())->Clear();
#endif
}

bool RegularShapesDrawing::calculate_item(DigitizedPoint &end)
{
#if 0
	to do
		if (end.m_type == DigitizeNoItemType)return false;

	if(TempObject() && TempObject()->GetType() != SketchType){
		ClearObjectsMade();
	}

	// make sure sketch exists
	if(TempObject()==NULL){
		AddToTempObjects(new CSketch);
	}

	Matrix mat = theApp->GetDrawMatrix(true);
	Point3d xdir = Point3d(1, 0, 0).Transformed(mat);
	Point3d ydir = Point3d(0, 1, 0).Transformed(mat);
	Point3d zdir = Point3d(0, 0, 1).Transformed(mat);

	Point3d p0 = GetStartPos().m_point;
	Point3d p2 = end.m_point;

	double x = Point3d(p2) * Point3d(xdir) - Point3d(p0) * Point3d(xdir);
	double y = Point3d(p2) * Point3d(ydir) - Point3d(p0) * Point3d(ydir);

	Point3d p1 = p0 + xdir * x;
	Point3d p3 = p0 + ydir * y;

	// swap left and right, if user dragged to the left
	if(x < 0){
		Point3d t = p0;
		p0 = p1;
		p1 = t;
		t = p3;
		p3 = p2;
		p2 = t;
	}

	// swap top and bottom, if user dragged upward
	if(y < 0){
		Point3d t = p0;
		p0 = p3;
		p3 = t;
		t = p1;
		p1 = p2;
		p2 = t;
	}

	// add ( or modify ) lines and arcs
	switch(m_mode)
	{
	case RectanglesRegularShapeMode:
		CalculateRectangle(x, y, p0, p1, p2, p3, xdir, ydir, zdir);
		break;
	case PolygonsRegularShapeMode:
		CalculatePolygon(GetStartPos().m_point, end.m_point, zdir);
		break;
	case ObroundRegularShapeMode:
		CalculateObround(GetStartPos().m_point, end.m_point, xdir, zdir);
		break;
	}
#endif
	return true;
}

void RegularShapesDrawing::CalculateRectangle(double x, double y, const Point3d& p0, const Point3d& p1, const Point3d& p2, const Point3d& p3, const Point3d& xdir, const Point3d& ydir, const Point3d& zdir)
{
#if 0
	to do
		bool radii_wanted = false;
	bool x_lines_disappear = false;
	bool y_lines_disappear = false;

	if(m_rect_radius > 0.0000000001)
	{
		if(fabs(x) - m_rect_radius*2 > -0.0000000001 && fabs(y) - m_rect_radius*2 > -0.0000000001)
			radii_wanted = true;
		if(fabs(fabs(x) - m_rect_radius*2) < 0.0000000001)
			x_lines_disappear = true;
		if(fabs(fabs(y) - m_rect_radius*2) < 0.0000000001)
			y_lines_disappear = true;
	}

	int good_num = 4;

	if(radii_wanted)
	{
		if(x_lines_disappear && y_lines_disappear)good_num = 2;
		else if(x_lines_disappear || y_lines_disappear)good_num = 4;
		else good_num = 8;
	}

	if(TempObject()->GetNumChildren() != good_num)ClearSketch();
	// check first item
	else if(TempObject()->GetFirstChild()->GetType() != (radii_wanted ? ArcType:LineType))
		ClearSketch();

	if(radii_wanted)
	{
		if(x_lines_disappear && y_lines_disappear)
		{
			// make two arcs, for a circle
			HArc* arcs[2];
			if(TempObject()->GetNumChildren() > 0)
			{
				HeeksObj* object = TempObject()->GetFirstChild();
				for(int i = 0; i<2; i++)
				{
					arcs[i] = (HArc*)object;
					object = TempObject()->GetNextChild();
				}
			}
			else
			{
				for(int i = 0; i<2; i++)
				{
					arcs[i] = new HArc(Point3d(0, 0, 0), Point3d(0, 0, 0), Circle(), &(theApp->current_color));
					TempObject()->Add(arcs[i], NULL);
				}
			}
			arcs[0]->A = p0 + xdir * m_rect_radius;
			arcs[0]->B = p3 + xdir * m_rect_radius;
			arcs[0]->SetCircle(Circle(Point3d(Point3d(p0 + xdir * m_rect_radius + ydir * m_rect_radius), zdir), m_rect_radius));
			arcs[1]->A = arcs[0]->B;
			arcs[1]->B = arcs[0]->A;
			arcs[1]->SetCircle(arcs[0]->GetCircle());
		}
		else if(x_lines_disappear || y_lines_disappear)
		{
			// arc-line-arc-line
			HArc* arcs[2];
			HLine* lines[2];
			if(TempObject()->GetNumChildren() > 0)
			{
				HeeksObj* object = TempObject()->GetFirstChild();
				for(int i = 0; i<2; i++)
				{
					arcs[i] = (HArc*)object;
					object = TempObject()->GetNextChild();
					lines[i] = (HLine*)object;
					object = TempObject()->GetNextChild();
				}
			}
			else
			{
				for(int i = 0; i<2; i++)
				{
					arcs[i] = new HArc(Point3d(0, 0, 0), Point3d(0, 0, 0), Circle(), &(theApp->current_color));
					TempObject()->Add(arcs[i], NULL);
					lines[i] = new HLine(Point3d(0, 0, 0), Point3d(0, 0, 0), &(theApp->current_color));
					TempObject()->Add(lines[i], NULL);
				}
			}

			if(x_lines_disappear){
				arcs[0]->A = p2 - ydir * m_rect_radius;
				arcs[0]->B = p3 - ydir * m_rect_radius;
				arcs[0]->SetCircle(Circle(Point3d(Point3d(p3 + xdir * m_rect_radius - ydir * m_rect_radius), zdir), m_rect_radius));
				lines[0]->A = arcs[0]->B;
				lines[0]->B = p0 + ydir * m_rect_radius;
				arcs[1]->A = lines[0]->B;
				arcs[1]->B = p1 + ydir * m_rect_radius;
				arcs[1]->SetCircle(Circle(Point3d(Point3d(p0 + xdir * m_rect_radius + ydir * m_rect_radius), zdir), m_rect_radius));
				lines[1]->A = arcs[1]->B;
				lines[1]->B = arcs[0]->A;
			}
			else{
				arcs[0]->A = p1 - xdir * m_rect_radius;
				arcs[0]->B = p2 - xdir * m_rect_radius;
				arcs[0]->SetCircle(Circle(Point3d(Point3d(p1 - xdir * m_rect_radius + ydir * m_rect_radius), zdir), m_rect_radius));
				lines[0]->A = arcs[0]->B;
				lines[0]->B = p3 + xdir * m_rect_radius;
				arcs[1]->A = lines[0]->B;
				arcs[1]->B = p0 + xdir * m_rect_radius;
				arcs[1]->SetCircle(Circle(Point3d(Point3d(p0 + xdir * m_rect_radius + ydir * m_rect_radius), zdir), m_rect_radius));
				lines[1]->A = arcs[1]->B;
				lines[1]->B = arcs[0]->A;
			}
		}
		else{
			// arc-line-arc-line-arc-line-arc-line
			HLine* lines[4];
			HArc* arcs[4];
			if(TempObject()->GetNumChildren() > 0)
			{
				HeeksObj* object = TempObject()->GetFirstChild();
				for(int i = 0; i<4; i++)
				{
					arcs[i] = (HArc*)object;
					object = TempObject()->GetNextChild();
					lines[i] = (HLine*)object;
					object = TempObject()->GetNextChild();
				}
			}
			else
			{
				for(int i = 0; i<4; i++)
				{
					arcs[i] = new HArc(Point3d(0, 0, 0), Point3d(0, 0, 0), Circle(), &(theApp->current_color));
					TempObject()->Add(arcs[i], NULL);
					lines[i] = new HLine(Point3d(0, 0, 0), Point3d(0, 0, 0), &(theApp->current_color));
					TempObject()->Add(lines[i], NULL);
				}
			}

			arcs[0]->A = p1 - xdir * m_rect_radius;
			arcs[0]->B = p1 + ydir * m_rect_radius;
			arcs[0]->SetCircle(Circle(Point3d(Point3d(p1 - xdir * m_rect_radius + ydir * m_rect_radius), zdir), m_rect_radius));
			lines[0]->A = arcs[0]->B;
			lines[0]->B = p2 - ydir * m_rect_radius;
			arcs[1]->A = lines[0]->B;
			arcs[1]->B = p2 - xdir * m_rect_radius;
			arcs[1]->SetCircle(Circle(Point3d(Point3d(p2 - xdir * m_rect_radius - ydir * m_rect_radius), zdir), m_rect_radius));
			lines[1]->A = arcs[1]->B;
			lines[1]->B = p3 + xdir * m_rect_radius;
			arcs[2]->A = lines[1]->B;
			arcs[2]->B = p3 - ydir * m_rect_radius;
			arcs[2]->SetCircle(Circle(Point3d(Point3d(p3 + xdir * m_rect_radius - ydir * m_rect_radius), zdir), m_rect_radius));
			lines[2]->A = arcs[2]->B;
			lines[2]->B = p0 + ydir * m_rect_radius;
			arcs[3]->A = lines[2]->B;
			arcs[3]->B = p0 + xdir * m_rect_radius;
			arcs[3]->SetCircle(Circle(Point3d(Point3d(p0 + xdir * m_rect_radius + ydir * m_rect_radius), zdir), m_rect_radius));
			lines[3]->A = arcs[3]->B;
			lines[3]->B = arcs[0]->A;
		}
	}
	else
	{
		// line-line-line-line
		HLine* lines[4];
		if(TempObject()->GetNumChildren() > 0)
		{
			HeeksObj* object = TempObject()->GetFirstChild();
			for(int i = 0; i<4; i++)
			{
				lines[i] = (HLine*)object;
				object = TempObject()->GetNextChild();
			}
		}
		else
		{
			for(int i = 0; i<4; i++)
			{
				lines[i] = new HLine(Point3d(0, 0, 0), Point3d(0, 0, 0), &(theApp->current_color));
				TempObject()->Add(lines[i], NULL);
			}
		}

		lines[0]->A = p0;
		lines[0]->B = p1;
		lines[1]->A = p1;
		lines[1]->B = p2;
		lines[2]->A = p2;
		lines[2]->B = p3;
		lines[3]->A = p3;
		lines[3]->B = p0;
	}
#endif
}

void RegularShapesDrawing::CalculatePolygon(const Point3d& p0, const Point3d& p1, const Point3d& zdir)
{
#if 0
	to do

	if(p0.IsEqual(p1, theApp->m_geom_tol))return;

	if(TempObject()->GetNumChildren() != m_number_of_side_for_polygon)
		ClearSketch();
	HLine** lines = (HLine**)malloc(m_number_of_side_for_polygon * sizeof(HLine*));

	if(TempObject()->GetNumChildren() > 0)
	{
		HeeksObj* object = TempObject()->GetFirstChild();
		for(int i = 0; i<m_number_of_side_for_polygon; i++)
		{
			lines[i] = (HLine*)object;
			object = TempObject()->GetNextChild();
		}
	}
	else
	{
		for(int i = 0; i<m_number_of_side_for_polygon; i++)
		{
			lines[i] = new HLine(Point3d(0, 0, 0), Point3d(0, 0, 0), &(theApp->current_color));
			TempObject()->Add(lines[i], NULL);
		}
	}

    double radius = p0.Distance(p1);
    double sideAngle=0;
    double angle0;
    double angle1;

    switch (p_mode)
    {
        case InscribedMode:
            //inscribed circle
            sideAngle =2.0 * M_PI / m_number_of_side_for_polygon;
            radius = radius/cos((sideAngle/2));
            for(int i = 0; i<m_number_of_side_for_polygon; i++)
            {
                Point3d xdir(make_vector(p0, p1));
                Point3d ydir = zdir ^ xdir;
                angle0 = (sideAngle * i)+(sideAngle/2);
                angle1 = (sideAngle * (i+1))+(sideAngle/2);
                lines[i]->A = p0 + xdir * ( cos(angle0) * radius ) + ydir * ( sin(angle0) * radius );
                if(i == m_number_of_side_for_polygon - 1)lines[i]->B = lines[0]->A;
                lines[i]->B = p0 + xdir * ( cos(angle1) * radius ) + ydir * ( sin(angle1) * radius );
            }
        break;
        case ExcribedMode:
            //excribed circle
            for(int i = 0; i<m_number_of_side_for_polygon; i++)
            {
                Point3d xdir(make_vector(p0, p1));
                Point3d ydir = zdir ^ xdir;
                angle0 = 2.0 * M_PI / m_number_of_side_for_polygon * i;
                angle1 = 2.0 * M_PI / m_number_of_side_for_polygon * (i+1);
                lines[i]->A = p0 + xdir * ( cos(angle0) * radius ) + ydir * ( sin(angle0) * radius );
                if(i == m_number_of_side_for_polygon - 1)lines[i]->B = lines[0]->A;
                lines[i]->B = p0 + xdir * ( cos(angle1) * radius ) + ydir * ( sin(angle1) * radius );
            }
        break;
    }
	free(lines);
#endif
}

void RegularShapesDrawing::CalculateObround(const Point3d& p0, const Point3d& p1, const Point3d& xdir, const Point3d& zdir)
{
#if 0
	to do
		bool lines_disappear = false;

	if(m_obround_radius > 0.0000000001)
	{
		if(p0.IsEqual(p1, theApp->m_geom_tol))lines_disappear = true;
	}
	else return;

	int good_num = 4;
	if(lines_disappear)good_num = 2;

	if(TempObject()->GetNumChildren() != good_num)ClearSketch();

	if(lines_disappear)
	{
		// make two arcs, for a circle
		HArc* arcs[2];
		if(TempObject()->GetNumChildren() > 0)
		{
			HeeksObj* object = TempObject()->GetFirstChild();
			for(int i = 0; i<2; i++)
			{
				arcs[i] = (HArc*)object;
				object = TempObject()->GetNextChild();
			}
		}
		else
		{
			for(int i = 0; i<2; i++)
			{
				arcs[i] = new HArc(Point3d(0, 0, 0), Point3d(0, 0, 0), Circle(), &(theApp->current_color));
				TempObject()->Add(arcs[i], NULL);
			}
		}
		arcs[0]->A = p0 + xdir * m_obround_radius;
		arcs[0]->B = p0 - xdir * m_obround_radius;
		arcs[0]->SetCircle(Circle(Point3d(p0, zdir), m_obround_radius));
		arcs[1]->A = arcs[0]->B;
		arcs[1]->B = arcs[0]->A;
		arcs[1]->C = arcs[0]->C;
		arcs[1]->m_axis = arcs[0]->m_axis;
	}
	else
	{
		// arc-line-arc-line
		HArc* arcs[2];
		HLine* lines[2];
		if(TempObject()->GetNumChildren() > 0)
		{
			HeeksObj* object = TempObject()->GetFirstChild();
			for(int i = 0; i<2; i++)
			{
				arcs[i] = (HArc*)object;
				object = TempObject()->GetNextChild();
				lines[i] = (HLine*)object;
				object = TempObject()->GetNextChild();
			}
		}
		else
		{
			for(int i = 0; i<2; i++)
			{
				arcs[i] = new HArc(Point3d(0, 0, 0), Point3d(0, 0, 0), Circle(), &(theApp->current_color));
				TempObject()->Add(arcs[i], NULL);
				lines[i] = new HLine(Point3d(0, 0, 0), Point3d(0, 0, 0), &(theApp->current_color));
				TempObject()->Add(lines[i], NULL);
			}
		}

		Point3d along_dir(make_vector(p0, p1));
		Point3d right_dir = along_dir ^ zdir;

		arcs[0]->A = p1 + right_dir * m_obround_radius;
		arcs[0]->B = p1 - right_dir * m_obround_radius;
		arcs[0]->SetCircle(Circle(Point3d(p1, zdir), m_obround_radius));
		lines[0]->A = arcs[0]->B;
		lines[0]->B = p0 - right_dir * m_obround_radius;
		arcs[1]->A = lines[0]->B;
		arcs[1]->B = p0 + right_dir * m_obround_radius;
		arcs[1]->SetCircle(Circle(Point3d(p0, zdir), m_obround_radius));
		lines[1]->A = arcs[1]->B;
		lines[1]->B = arcs[0]->A;
	}
#endif
}

static RegularShapesDrawing* RegularShapesDrawing_for_GetProperties = NULL;

const wchar_t* RegularShapesDrawing::GetTitle()
{
	switch(m_mode)
	{
	case RectanglesRegularShapeMode:
		return L"Rectangle drawing";

	case PolygonsRegularShapeMode:
		return L"Polygon drawing";

	case ObroundRegularShapeMode:
		return L"Obround drawing";

	default:
		return L"Regular shapes drawing";
	}
}


static RegularShapesDrawing* line_drawing_for_GetProperties = NULL;

class PropertyDrawingMode:public Property
{
public:
	PropertyDrawingMode() :Property(NULL, L"drawing mode"){}
	Property* MakeACopy(){ return new PropertyDrawingMode(*this); }
	int get_property_type(){ return ChoicePropertyType; }
	void Set(int value)
	{
		RegularShapesDrawing_for_GetProperties->m_mode = (RegularShapeMode)value;
		RegularShapesDrawing_for_GetProperties->ClearSketch();
	}
	int GetInt()const
	{
		return (int)RegularShapesDrawing_for_GetProperties->m_mode;
	}
	void GetChoices(std::list< std::wstring > &choices)const
	{
		choices.push_back(std::wstring(L"draw rectangles"));
		choices.push_back(std::wstring(L"draw polygons"));
		choices.push_back(std::wstring(L"draw slots"));

	}
};

void RegularShapesDrawing::GetProperties(std::list<Property *> *list){
	RegularShapesDrawing_for_GetProperties = this;
	list->push_back(new PropertyDrawingMode());


	if(m_mode == RectanglesRegularShapeMode)list->push_back( new PropertyLength( NULL, L"radius",& m_rect_radius ));
	if(m_mode == ObroundRegularShapeMode)list->push_back( new PropertyLength( NULL, L"radius", &m_obround_radius ));
	if(m_mode == PolygonsRegularShapeMode)
	{
        list->push_back( new PropertyInt(NULL, L"number of sides for polygon", &m_number_of_side_for_polygon ));

        std::list< std::wstring > polygonChoices;
            polygonChoices.push_back ( std::wstring ( L"excribed circle" ) );
            polygonChoices.push_back ( std::wstring ( L"inscribed circle" ) );
        list->push_back ( new PropertyChoice ( NULL, L"polygon mode",  polygonChoices, (int*)&p_mode ) );
	}

	Drawing::GetProperties(list);
}
