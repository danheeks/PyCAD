// LineArcDrawing.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "LineArcDrawing.h"
#include "HeeksObj.h"
#include "HLine.h"
#include "HArc.h"
#include "HILine.h"
#include "HCircle.h"
#if 0
#include "HEllipse.h"
#include "HSpline.h"
#endif
#include "Sketch.h"
#include "SelectMode.h"
#include "DigitizeMode.h"
#include "Property.h"

LineArcDrawing line_strip;

LineArcDrawing::LineArcDrawing(void){
	m_previous_direction_set = false;
	m_previous_direction = Point3d(1, 0, 0);
	drawing_mode = LineDrawingMode;
	m_A_down = false;
	m_container = NULL;
	radius_for_circle = 5.0;
	circle_mode = ThreePointsCircleMode;
	m_add_to_sketch = true;
}

LineArcDrawing::~LineArcDrawing(void){
}

class SetPreviousDirection:public Undoable{
private:
	LineArcDrawing *drawing;
	Point3d old_direction;
	Point3d new_direction;
	bool old_previous_direction_set;

public:
	SetPreviousDirection(LineArcDrawing *d, const Point3d& n)
	{
		drawing = d;
		old_direction = drawing->m_previous_direction;
		new_direction = n;
		old_previous_direction_set = drawing->m_previous_direction_set;
	}

	// Tool's virtual functions
	const wchar_t* GetTitle(){return L"set previous direction";}
	void Run(bool redo)
	{
		drawing->m_previous_direction = new_direction;
		drawing->m_previous_direction_set = true;
	}
	void RollBack()
	{
		if(old_previous_direction_set)drawing->m_previous_direction = old_direction;
		drawing->m_previous_direction_set = old_previous_direction_set;
	}
};

void LineArcDrawing::set_previous_direction(){
	if(PrevObject() == NULL)return;

	if(PrevObject()->GetType() == LineType){
		Point3d s, e;
		if(PrevObject()->GetStartPoint(s) && PrevObject()->GetEndPoint(e))
		{
			theApp->DoUndoable(new SetPreviousDirection(this, Point3d(s, e)));
		}
	}
	else if(PrevObject()->GetType() == ArcType){
		Point3d circlev(((HArc*)PrevObject())->m_axis);
		Point3d endv(((HArc*)PrevObject())->C, ((HArc*)PrevObject())->B);
		theApp->DoUndoable(new SetPreviousDirection(this, (circlev ^ endv).Normalized()));
	}
}

int LineArcDrawing::number_of_steps()
{
	switch(drawing_mode)
	{
	case CircleDrawingMode:
		switch(circle_mode)
		{
		case ThreePointsCircleMode:
			return 3;
		case CentreAndRadiusCircleMode:
			return 1;
		default:
			break;
		}
		break;
	case SplineDrawingMode:
		switch(spline_mode)
		{
		case CubicSplineMode:
			return 4;
		case QuarticSplineMode:
			return 3;
		case RationalSplineMode:
			return 20;
		}
		break;
	case EllipseDrawingMode:
		return 3;
	default:
		break;
	}
	return 2;
}

int LineArcDrawing::step_to_go_to_after_last_step()
{
	switch(drawing_mode)
	{
	case LineDrawingMode:
	case ArcDrawingMode:
		return 1;
	case SplineDrawingMode:
		return 3;
	case ILineDrawingMode:
	case CircleDrawingMode:
	case EllipseDrawingMode:
	default:
		return 0;
	}
}

bool LineArcDrawing::is_a_draw_level(int level)
{
	if(drawing_mode == SplineDrawingMode && spline_mode == RationalSplineMode)
		return level>=3;
	return Drawing::is_a_draw_level(level);
}

bool LineArcDrawing::is_an_add_level(int level)
{
	switch(drawing_mode)
	{
	case CircleDrawingMode:
		switch(circle_mode)
		{
		case ThreePointsCircleMode:
			return level == 2;
		case CentreAndRadiusCircleMode:
			return level == 0;
		default:
			break;
		}
		break;
	case EllipseDrawingMode:
		return level == 2;
	case SplineDrawingMode:
		switch(spline_mode)
		{
		case CubicSplineMode:
			return level == 3;
		case QuarticSplineMode:
			return level == 2;
		case RationalSplineMode:
			return level == 20;
		}
		break;
	default:
		break;
	}
	return level == 1;
}

void LineArcDrawing::AddPoint()
{
	switch(drawing_mode)
	{
	case CircleDrawingMode:
		{
		Drawing::AddPoint();
		}
		break;

	case EllipseDrawingMode:
		Drawing::AddPoint();
		break;

	case LineDrawingMode:
	case ArcDrawingMode:
		{
			// edit the end of the previous item to be the start of the arc
			// this only happens if we are drawing tangents to other objects
			// really need to fill the gap with whatever we are tangent around
			// ellipse,arc,spline or whatever
			if(TempObject() && PrevObject())
			{
				if(PrevObject())
				{
					Point3d spos;
					Point3d epos;
					TempObject()->GetStartPoint(spos);
					PrevObject()->GetEndPoint(epos);
					HeeksObj* tanobject = GetStartPos().m_object1;
					if(current_view_stuff->start_pos.m_type == DigitizeTangentType && tanobject)
					switch(tanobject->GetType())
					{
						case LineType:
							//((HLine*)prev_object)->B = p;
							break;
						case ArcType:
							{
								HArc* arc = new HArc(*(HArc*)tanobject);
								arc->A = Point3d(spos);
								arc->B = Point3d(epos);
								AddToTempObjects(arc);
							}
							break;
						case CircleType:
							{
								HArc* arc = new HArc(Point3d(spos), Point3d(epos), ((HCircle*)tanobject)->m_axis, ((HCircle*)tanobject)->m_c, &theApp->current_color);
								arc->A = Point3d(spos);
								arc->B = Point3d(epos);
								AddToTempObjects(arc);
							}
							break;
					}
				}
			} 

			Drawing::AddPoint();

		}
		break;

	default:
		Drawing::AddPoint();
		break;
	}
}

bool LineArcDrawing::calculate_item(DigitizedPoint &end){
	if(number_of_steps() > 1 && GetStartPos().m_type == DigitizeNoItemType)return false;
	if(end.m_type == DigitizeNoItemType)return false;

	switch(drawing_mode)
	{
	case LineDrawingMode:
		{
			if(TempObject() && TempObject()->GetType() != LineType){
				ClearObjectsMade();
			}
			Point3d p1, p2;
			DigitizedPoint::GetLinePoints(GetStartPos(), end, p1, p2);
			if (p1 == p2)return false;
			end.m_point = p2;
			if(TempObject() == NULL){
				AddToTempObjects(new HLine(p1, p2, &theApp->current_color));
			}
			else{
				((HLine*)TempObject())->A = p1;
				((HLine*)TempObject())->B = p2;
			}
		}
		return true;

	case ArcDrawingMode:
		{
			// tangential arcs
			if(TempObject() && TempObject()->GetType() != ArcType){
				ClearObjectsMade();
			}

			Point3d centre;
			Point3d axis;
			Point3d p1, p2;
			bool arc_found = DigitizedPoint::GetArcPoints(GetStartPos(), m_previous_direction_set ? (&m_previous_direction) : NULL, end, p1, p2, centre, axis);
			if (p1 == p2)return false;

			if(arc_found)
			{
				if(HArc::TangentialArc(p1, m_previous_direction, p2, centre, axis))
				{
					// arc
					if(TempObject() == NULL){
						AddToTempObjects(new HArc(p1, p2, axis, centre, &theApp->current_color));
					}
					else{
						((HArc*)TempObject())->m_axis = axis;
						((HArc*)TempObject())->C = centre;
						((HArc*)TempObject())->A = p1;
						((HArc*)TempObject())->B = p2;
					}
				}
				else
				{
					// line
					if(TempObject() && TempObject()->GetType() != LineType){
						ClearObjectsMade();
					}
					if(TempObject()==NULL){
						AddToTempObjects(new HLine(p1, p2, &theApp->current_color));
					}
					else{
						((HLine*)TempObject())->A = p1;
						((HLine*)TempObject())->B = p2;
					}
				}
			}
		}
		return true;

	case ILineDrawingMode:
		{
			if(TempObject() && TempObject()->GetType() != ILineType){
				ClearObjectsMade();
			}
			Point3d p1, p2;
			DigitizedPoint::GetLinePoints(GetStartPos(), end, p1, p2);
			if (p1 == p2)return false;
			if(TempObject() == NULL){
				AddToTempObjects(new HILine(p1, p2, &theApp->current_color));
			}
			else{
				((HILine*)TempObject())->A = p1;
				((HILine*)TempObject())->B = p2;
			}
		}
		return true;
#if 0
	case EllipseDrawingMode:
		if(TempObject() && TempObject()->GetType() != EllipseType){
			ClearObjectsMade();
		}

		if(TempObject() == NULL)
		{
			gp_Elips elip;
			DigitizedPoint::GetEllipse(GetBeforeStartPos(), GetStartPos(), end,elip);
					
			AddToTempObjects(new HEllipse(elip, &theApp->current_color));

		}
		else
		{
			gp_Elips elip;
			DigitizedPoint::GetEllipse(GetBeforeStartPos(), GetStartPos(), end,elip);
			((HEllipse*)TempObject())->SetEllipse(elip);
		}
		return true;	

	case SplineDrawingMode:
		{
			if(TempObject() && TempObject()->GetType() != SplineType){
				ClearObjectsMade();
			}

			Handle_Geom_BSplineCurve spline;
			switch(spline_mode)
			{
				case CubicSplineMode:
					DigitizedPoint::GetCubicSpline(GetBeforeBeforeStartPos(), GetBeforeStartPos(), GetStartPos(), end, spline);
					break;
				case QuarticSplineMode:
					DigitizedPoint::GetQuarticSpline(GetBeforeStartPos(), GetStartPos(), end, spline);
					break;
				case RationalSplineMode:
					DigitizedPoint::GetRationalSpline(spline_points, end, spline);
					break;
			}

			if(TempObject() == NULL){
				AddToTempObjects(new HSpline(spline, &theApp->current_color));
			}
			else{
				((HSpline*)TempObject())->m_spline = spline;
			}

			return true;
		}
#endif

	case CircleDrawingMode:
		{
			if(TempObject() && TempObject()->GetType() != CircleType){
				ClearObjectsMade();
			}
			
			switch(circle_mode)
			{
			case CentreAndPointCircleMode:
				{
					Point3d p1, p2, centre;
					Point3d axis;
					DigitizedPoint::GetArcPoints(GetStartPos(), NULL, end, p1, p2, centre, axis);
					radius_for_circle = p1.Dist(p2);

					if(TempObject() == NULL){
						AddToTempObjects(new HCircle(p1, Point3d(0, 0, 1), radius_for_circle, &theApp->current_color));
					}
					else{
						((HCircle*)TempObject())->m_c = p1;
						((HCircle*)TempObject())->m_radius = radius_for_circle;
					}
				}
				return true;

			case ThreePointsCircleMode:
				{
					Circle c;
					if(DigitizedPoint::GetTangentCircle(GetBeforeStartPos(), GetStartPos(), end, c))
					{
						if(TempObject() == NULL){
							AddToTempObjects(new HCircle(c.pc, Point3d(0,0,1), c.radius, &theApp->current_color));
						}
						else{
							((HCircle*)TempObject())->SetCircle(c);
						}
					}
				}
				return true;
			case TwoPointsCircleMode:
				{
					Circle c;
					if(DigitizedPoint::GetCircleBetween(GetStartPos(), end, c))
					{
						if(TempObject() == NULL){
							AddToTempObjects(new HCircle(c.pc, Point3d(0, 0, 1), c.radius, &theApp->current_color));
						}
						else{
							((HCircle*)TempObject())->SetCircle(c);
						}
					}
				}
				return true;
			case CentreAndRadiusCircleMode:
				{
					if(TempObject()==NULL){
						AddToTempObjects(new HCircle(end.m_point, Point3d(0, 0, 1), radius_for_circle, &theApp->current_color));
					}
					else{
						((HCircle*)TempObject())->m_c = end.m_point;
						((HCircle*)TempObject())->m_radius = radius_for_circle;
					}
				}
				return true;

			}
		}
		break;

	default:
		break;
	}

	return false;
}

HeeksObj* LineArcDrawing::GetOwnerForDrawingObjects()
{
	switch(drawing_mode)
	{
	case LineDrawingMode:
	case ArcDrawingMode:
		{
			if(m_add_to_sketch)
			{
				if(m_container == NULL)
				{
					m_container = new CSketch();
					theApp->AddUndoably(m_container, NULL, NULL);
				}
				return m_container;
			}
		}
		break;
	default:
		break;
	}

	return theApp; //Object always needs to be added somewhere
}


static std::wstring str_for_GetTitle;

const wchar_t* LineArcDrawing::GetTitle()
{
	switch(drawing_mode)
	{
	case LineDrawingMode:
		str_for_GetTitle = std::wstring(L"Line drawing mode");
		str_for_GetTitle.append(std::wstring(L" : "));
		if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(L"click on start point"));
		else str_for_GetTitle.append(std::wstring(L"click on end of line"));
		return str_for_GetTitle.c_str();

	case ArcDrawingMode:
		str_for_GetTitle = std::wstring(L"Arc drawing mode");
		str_for_GetTitle.append(std::wstring(L" : "));
		if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(L"click on start point"));
		else str_for_GetTitle.append(std::wstring(L"click on end of arc"));
		return str_for_GetTitle.c_str();

	case ILineDrawingMode:
		str_for_GetTitle = std::wstring(L"Infinite line drawing");
		str_for_GetTitle.append(std::wstring(L" : "));
		if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(L"click on first point"));
		else str_for_GetTitle.append(std::wstring(L"click on second point"));
		return str_for_GetTitle.c_str();

	case EllipseDrawingMode:
		str_for_GetTitle = std::wstring(L"Ellipse drawing mode");
		str_for_GetTitle.append(std::wstring(L" : "));

		str_for_GetTitle.append(std::wstring(L"center and 2 points mode"));
		str_for_GetTitle.append(std::wstring(L"\n  "));
		if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(L"click on center point"));
		else if(GetDrawStep() == 1) 
		{
			str_for_GetTitle.append(std::wstring(L"click on point on ellipse"));
			str_for_GetTitle.append(std::wstring(L"\n  "));
			str_for_GetTitle.append(std::wstring(L"(colinear or orthogonal to axis)"));
		}
		else str_for_GetTitle.append(std::wstring(L"click on another point on ellipse"));
		
		return str_for_GetTitle.c_str();

	case SplineDrawingMode:
		
		str_for_GetTitle = std::wstring(L"Spline drawing mode");
		str_for_GetTitle.append(std::wstring(L" : "));

		switch(spline_mode){
			case CubicSplineMode:
				str_for_GetTitle.append(std::wstring(L"cubic spline mode"));
				str_for_GetTitle.append(std::wstring(L"\n  "));
				if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(L"click on start point"));
				else if (GetDrawStep() == 1) str_for_GetTitle.append(std::wstring(L"click on end point"));
				else if (GetDrawStep() == 2) str_for_GetTitle.append(std::wstring(L"click on first control point"));
				else str_for_GetTitle.append(std::wstring(L"click on second control point"));
				break;
			case QuarticSplineMode:
				str_for_GetTitle.append(std::wstring(L"quartic spline mode"));
				str_for_GetTitle.append(std::wstring(L"\n  "));
				if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(L"click on start point"));
				else if (GetDrawStep() == 1) str_for_GetTitle.append(std::wstring(L"click on end point"));
				else str_for_GetTitle.append(std::wstring(L"click on control point"));
				break;
			case RationalSplineMode:
				str_for_GetTitle.append(std::wstring(L"rational spline mode"));
				str_for_GetTitle.append(std::wstring(L"\n  "));
				if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(L"click on start point"));
				else if (GetDrawStep() == 1) str_for_GetTitle.append(std::wstring(L"click on first control point"));
				else if (GetDrawStep() == 2) str_for_GetTitle.append(std::wstring(L"click on second control point"));
				else str_for_GetTitle.append(std::wstring(L"click on next control point or endpoint"));
	
				break;
		}

		return str_for_GetTitle.c_str();

	case CircleDrawingMode:
		str_for_GetTitle = std::wstring(L"Circle drawing mode");
		str_for_GetTitle.append(std::wstring(L" : "));

		switch(circle_mode){
		case CentreAndPointCircleMode:
			{
				str_for_GetTitle.append(std::wstring(L"center and point mode"));
				str_for_GetTitle.append(std::wstring(L"\n  "));
				if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(L"click on center point"));
				else str_for_GetTitle.append(std::wstring(L"click on point on circle"));
			}
			break;
		case ThreePointsCircleMode:
			{
				str_for_GetTitle.append(std::wstring(L"three points mode"));
				str_for_GetTitle.append(std::wstring(L"\n  "));
				if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(L"click on first point"));
				else if (GetDrawStep() == 1)str_for_GetTitle.append(std::wstring(L"click on second point"));
				else str_for_GetTitle.append(std::wstring(L"click on third point"));
			}
			break;
		case TwoPointsCircleMode:
			{
				str_for_GetTitle.append(std::wstring(L"two points mode"));
				str_for_GetTitle.append(std::wstring(L"\n  "));
				if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(L"click on first point"));
				else str_for_GetTitle.append(std::wstring(L"click on second point"));
			}
			break;
		case CentreAndRadiusCircleMode:
			{
				str_for_GetTitle.append(std::wstring(L"centre with radius mode"));
				str_for_GetTitle.append(std::wstring(L"\n  "));
				str_for_GetTitle.append(std::wstring(L"click on centre point"));
			}
			break;
		default:
			break;
		}
		return str_for_GetTitle.c_str();

	default:
		return L"unknown";
	}
}


void LineArcDrawing::OnKeyDown(KeyCode key_code)
{
	switch (key_code){
	case K_A:
		// switch to arc drawing mode until a is released
		if (!m_A_down){
			m_A_down = true;
			m_save_drawing_mode.push_back(drawing_mode);
			drawing_mode = ArcDrawingMode;
			theApp->RefreshInputCanvas();
		}
		return;
	default:
		break;
	}

	Drawing::OnKeyDown(key_code);
}

void LineArcDrawing::OnKeyUp(KeyCode key_code)
{
	switch (key_code){
	case K_A:
		// switch back to previous drawing mode
		if (m_save_drawing_mode.size()>0){
			drawing_mode = m_save_drawing_mode.back();
			m_save_drawing_mode.pop_back();
		}
		theApp->RefreshInputCanvas();
		m_A_down = false;
		return;
	default:
		break;
	}

	Drawing::OnKeyUp(key_code);
}

void LineArcDrawing::set_cursor(void){
}

static LineArcDrawing* line_drawing_for_GetProperties = NULL;

class PropertyDrawingMode:public Property
{
public:
	PropertyDrawingMode() :Property(NULL, L"drawing mode"){}
	Property* MakeACopy(){ return new PropertyDrawingMode(*this); }
	int get_property_type(){ return ChoicePropertyType; }
	void Set(int value)
	{
		line_drawing_for_GetProperties->drawing_mode = (EnumDrawingMode)value;
		line_drawing_for_GetProperties->m_save_drawing_mode.clear();
	}
	int GetInt()const
	{
		return (int)line_drawing_for_GetProperties->drawing_mode;
	}
	void GetChoices(std::list< std::wstring > &choices)const
	{
		choices.push_back(std::wstring(L"draw lines"));
		choices.push_back(std::wstring(L"draw tangential arcs"));
		choices.push_back(std::wstring(L"infinite line"));
		choices.push_back ( std::wstring ( L"draw circles" ) );

	}
};

class PropertyCircleMode :public Property
{
public:
	PropertyCircleMode() :Property(NULL, L"circle mode"){}
	Property* MakeACopy(){ return new PropertyCircleMode(*this); }
	int get_property_type(){ return ChoicePropertyType; }
	void Set(int value)
	{
		line_drawing_for_GetProperties->circle_mode = (EnumCircleDrawingMode)value;
	}
	int GetInt()const
	{
		return (int)line_drawing_for_GetProperties->circle_mode;
	}
	void GetChoices(std::list< std::wstring > &choices)const
	{
		choices.push_back(std::wstring(L"centre and point"));
		choices.push_back(std::wstring(L"three points"));
		choices.push_back(std::wstring(L"two points"));
		choices.push_back(std::wstring(L"centre and radius"));

	}
};

void LineArcDrawing::GetProperties(std::list<Property *> *list){
	line_drawing_for_GetProperties = this;

	// add drawing mode
	list->push_back(new PropertyDrawingMode());

	switch(drawing_mode)
	{
	case LineDrawingMode:
		{
			list->push_back(new PropertyStringReadOnly(L"(press 'a' for arcs)", L""));
		}
		break;

	case CircleDrawingMode:
		{
			list->push_back(new PropertyCircleMode());
			list->push_back(new PropertyLength(NULL, L"radius", &radius_for_circle));
		}
		break;

	default:
		break;
	}

	Drawing::GetProperties(list);
}

void LineArcDrawing::OnModeChange(void){
	// on start of drawing mode
	Drawing::OnModeChange();
	if(m_container)m_container = NULL;

	ClearPrevObject();
	m_previous_direction_set = false;
}

void LineArcDrawing::set_draw_step_not_undoable(int s)
{
	Drawing::set_draw_step_not_undoable(s);
	if(drawing_mode == SplineDrawingMode && spline_mode == RationalSplineMode)
	{
		spline_points.push_back(GetStartPos());
	}
}
