// LineArcDrawing.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "LineArcDrawing.h"
#include "HeeksObj.h"
#if 0
#include "HLine.h"
#include "HArc.h"
#include "HILine.h"
#include "HCircle.h"
#include "HEllipse.h"
#include "HSpline.h"
#include "Sketch.h"
#endif
#include "SelectMode.h"
#include "DigitizeMode.h"
#include "HeeksConfig.h"
#include "Property.h"


LineArcDrawing line_strip;

LineArcDrawing::LineArcDrawing(void){
	m_previous_direction_set = false;
	m_previous_direction = geoff_geometry::Point3d(1, 0, 0);
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
	geoff_geometry::Point3d old_direction;
	geoff_geometry::Point3d new_direction;
	bool old_previous_direction_set;

public:
	SetPreviousDirection(LineArcDrawing *d, const geoff_geometry::Point3d& n)
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
		geoff_geometry::Point3d s, e;
		if(PrevObject()->GetStartPoint(s) && PrevObject()->GetEndPoint(e))
		{
			theApp.DoUndoable(new SetPreviousDirection(this, geoff_geometry::Point3d(s, e)));
		}
	}
	else if(PrevObject()->GetType() == ArcType){
#if 0
		geoff_geometry::Point3d circlev(((HArc*)PrevObject())->m_axis.Direction());
		geoff_geometry::Point3d endv(((HArc*)PrevObject())->C, ((HArc*)PrevObject())->B);
		theApp.DoUndoable(new SetPreviousDirection(this, (circlev ^ endv).Normalized()));
#endif
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
#if 0
					geoff_geometry::Point3d spos;
					geoff_geometry::Point3d epos;
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
								arc->A = geoff_geometry::Point3d(spos);
								arc->B = geoff_geometry::Point3d(epos);
								AddToTempObjects(arc);
							}
							break;
						case CircleType:
							{
								HArc* arc = new HArc(geoff_geometry::Point3d(spos), geoff_geometry::Point3d(epos), ((HCircle*)tanobject)->GetCircle(), &theApp.current_color);
								arc->A = geoff_geometry::Point3d(spos);
								arc->B = geoff_geometry::Point3d(epos);
								AddToTempObjects(arc);
							}
							break;
						}
#endif
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
#if 0
		to do
	case LineDrawingMode:
		{
			if(TempObject() && TempObject()->GetType() != LineType){
				ClearObjectsMade();
			}
			geoff_geometry::Point3d p1, p2;
			DigitizedPoint::GetLinePoints(GetStartPos(), end, p1, p2);
			if (p1 == p2)return false;
			end.m_point = p2;
			if(TempObject() == NULL){
				AddToTempObjects(new HLine(p1, p2, &theApp.current_color));
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

			geoff_geometry::Point3d centre;
			gp_Dir axis;
			geoff_geometry::Point3d p1, p2;
			bool arc_found = DigitizedPoint::GetArcPoints(GetStartPos(), m_previous_direction_set ? (&m_previous_direction) : NULL, end, p1, p2, centre, axis);
			if (p1.IsEqual(p2, theApp.m_geom_tol))return false;

			if(arc_found)
			{
				if(HArc::TangentialArc(p1, m_previous_direction, p2, centre, axis))
				{
					// arc
					gp_Circ circle(gp_Ax2(centre, axis), centre.Distance(p1));

					if(TempObject() == NULL){
						AddToTempObjects(new HArc(p1, p2, circle, &theApp.current_color));
					}
					else{
						((HArc*)TempObject())->SetCircle(circle);
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
						AddToTempObjects(new HLine(p1, p2, &theApp.current_color));
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
			geoff_geometry::Point3d p1, p2;
			DigitizedPoint::GetLinePoints(GetStartPos(), end, p1, p2);
			if (p1.IsEqual(p2, theApp.m_geom_tol))return false;
			if(TempObject() == NULL){
				AddToTempObjects(new HILine(p1, p2, &theApp.current_color));
			}
			else{
				((HILine*)TempObject())->A = p1;
				((HILine*)TempObject())->B = p2;
			}
		}
		return true;

	case EllipseDrawingMode:
		if(TempObject() && TempObject()->GetType() != EllipseType){
			ClearObjectsMade();
		}

		if(TempObject() == NULL)
		{
			gp_Elips elip;
			DigitizedPoint::GetEllipse(GetBeforeStartPos(), GetStartPos(), end,elip);
					
			AddToTempObjects(new HEllipse(elip, &theApp.current_color));

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
				AddToTempObjects(new HSpline(spline, &theApp.current_color));
			}
			else{
				((HSpline*)TempObject())->m_spline = spline;
			}

			return true;
		}
	case CircleDrawingMode:
		{
			if(TempObject() && TempObject()->GetType() != CircleType){
				ClearObjectsMade();
			}
			
			switch(circle_mode)
			{
			case CentreAndPointCircleMode:
				{
					geoff_geometry::Point3d p1, p2, centre;
					gp_Dir axis;
					DigitizedPoint::GetArcPoints(GetStartPos(), NULL, end, p1, p2, centre, axis);
					radius_for_circle = p1.Distance(p2);

					if(TempObject() == NULL){
						AddToTempObjects(new HCircle(gp_Circ(gp_Ax2(p1, gp_Dir(0, 0, 1)), radius_for_circle), &theApp.current_color));
					}
					else{
						((HCircle*)TempObject())->m_axis.SetLocation(p1);
						((HCircle*)TempObject())->m_radius = radius_for_circle;
					}
				}
				return true;

			case ThreePointsCircleMode:
				{
					gp_Circ c;
					if(DigitizedPoint::GetTangentCircle(GetBeforeStartPos(), GetStartPos(), end, c))
					{
						if(TempObject() == NULL){
							AddToTempObjects(new HCircle(c, &theApp.current_color));
						}
						else{
							((HCircle*)TempObject())->SetCircle(c);
						}
					}
				}
				return true;
			case TwoPointsCircleMode:
				{
					gp_Circ c;
					if(DigitizedPoint::GetCircleBetween(GetStartPos(), end, c))
					{
						if(TempObject() == NULL){
							AddToTempObjects(new HCircle(c, &theApp.current_color));
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
						AddToTempObjects(new HCircle(gp_Circ(gp_Ax2(end.m_point, gp_Dir(0, 0, 1)), radius_for_circle), &theApp.current_color));
					}
					else{
						((HCircle*)TempObject())->m_axis.SetLocation(end.m_point);
						((HCircle*)TempObject())->m_radius = radius_for_circle;
					}
				}
				return true;

			}
		}
		break;
#endif
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
#if 0
			if(m_add_to_sketch)
			{
				if(m_container == NULL)
				{
					m_container = new CSketch();
					theApp.AddUndoably(m_container, NULL, NULL);
				}
				return m_container;
			}
#endif
		}
		break;
	default:
		break;
	}

	return &theApp; //Object always needs to be added somewhere
}


static std::wstring str_for_GetTitle;

const wchar_t* LineArcDrawing::GetTitle()
{
#if 0
	to do
	switch(drawing_mode)
	{
	case LineDrawingMode:
		str_for_GetTitle = std::wstring(_("Line drawing mode"));
		str_for_GetTitle.append(std::wstring(_T(" : ")));
		if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(_("click on start point")));
		else str_for_GetTitle.append(std::wstring(_("click on end of line")));
		return str_for_GetTitle.c_str();

	case ArcDrawingMode:
		str_for_GetTitle = std::wstring(_("Arc drawing mode"));
		str_for_GetTitle.append(std::wstring(_T(" : ")));
		if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(_("click on start point")));
		else str_for_GetTitle.append(std::wstring(_("click on end of arc")));
		return str_for_GetTitle.c_str();

	case ILineDrawingMode:
		str_for_GetTitle = std::wstring(_("Infinite line drawing"));
		str_for_GetTitle.append(std::wstring(_T(" : ")));
		if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(_("click on first point")));
		else str_for_GetTitle.append(std::wstring(_("click on second point")));
		return str_for_GetTitle.c_str();

	case EllipseDrawingMode:
		str_for_GetTitle = std::wstring(_("Ellipse drawing mode"));
		str_for_GetTitle.append(std::wstring(_T(" : ")));

		str_for_GetTitle.append(std::wstring(_("center and 2 points mode")));
		str_for_GetTitle.append(std::wstring(_T("\n  ")));
		if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(_("click on center point")));
		else if(GetDrawStep() == 1) 
		{
			str_for_GetTitle.append(std::wstring(_("click on point on ellipse")));
			str_for_GetTitle.append(std::wstring(_T("\n  ")));
			str_for_GetTitle.append(std::wstring(_("(colinear or orthogonal to axis)")));
		}
		else str_for_GetTitle.append(std::wstring(_("click on another point on ellipse")));
		
		return str_for_GetTitle.c_str();

	case SplineDrawingMode:
		
		str_for_GetTitle = std::wstring(_("Spline drawing mode"));
		str_for_GetTitle.append(std::wstring(_T(" : ")));

		switch(spline_mode){
			case CubicSplineMode:
				str_for_GetTitle.append(std::wstring(_("cubic spline mode")));
				str_for_GetTitle.append(std::wstring(_T("\n  ")));
				if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(_("click on start point")));
				else if (GetDrawStep() == 1) str_for_GetTitle.append(std::wstring(_("click on end point")));
				else if (GetDrawStep() == 2) str_for_GetTitle.append(std::wstring(_("click on first control point")));
				else str_for_GetTitle.append(std::wstring(_("click on second control point")));
				break;
			case QuarticSplineMode:
				str_for_GetTitle.append(std::wstring(_("quartic spline mode")));
				str_for_GetTitle.append(std::wstring(_T("\n  ")));
				if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(_("click on start point")));
				else if (GetDrawStep() == 1) str_for_GetTitle.append(std::wstring(_("click on end point")));
				else str_for_GetTitle.append(std::wstring(_("click on control point")));
				break;
			case RationalSplineMode:
				str_for_GetTitle.append(std::wstring(_("rational spline mode")));
				str_for_GetTitle.append(std::wstring(_T("\n  ")));
				if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(_("click on start point")));
				else if (GetDrawStep() == 1) str_for_GetTitle.append(std::wstring(_("click on first control point")));
				else if (GetDrawStep() == 2) str_for_GetTitle.append(std::wstring(_("click on second control point")));
				else str_for_GetTitle.append(std::wstring(_("click on next control point or endpoint")));
	
				break;
		}

		return str_for_GetTitle;

	case CircleDrawingMode:
		str_for_GetTitle = std::wstring(_("Circle drawing mode"));
		str_for_GetTitle.append(std::wstring(_T(" : ")));

		switch(circle_mode){
		case CentreAndPointCircleMode:
			{
				str_for_GetTitle.append(std::wstring(_("center and point mode")));
				str_for_GetTitle.append(std::wstring(_T("\n  ")));
				if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(_("click on center point")));
				else str_for_GetTitle.append(std::wstring(_("click on point on circle")));
			}
			break;
		case ThreePointsCircleMode:
			{
				str_for_GetTitle.append(std::wstring(_("three points mode")));
				str_for_GetTitle.append(std::wstring(_T("\n  ")));
				if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(_("click on first point")));
				else if (GetDrawStep() == 1)str_for_GetTitle.append(std::wstring(_("click on second point")));
				else str_for_GetTitle.append(std::wstring(_("click on third point")));
			}
			break;
		case TwoPointsCircleMode:
			{
				str_for_GetTitle.append(std::wstring(_("two points mode")));
				str_for_GetTitle.append(std::wstring(_T("\n  ")));
				if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(_("click on first point")));
				else str_for_GetTitle.append(std::wstring(_("click on second point")));
			}
			break;
		case CentreAndRadiusCircleMode:
			{
				str_for_GetTitle.append(std::wstring(_("centre with radius mode")));
				str_for_GetTitle.append(std::wstring(_T("\n  ")));
				str_for_GetTitle.append(std::wstring(_("click on centre point")));
			}
			break;
		}
		return str_for_GetTitle;

	default:
		return _("unknown");
	}
#else
return L"";
#endif
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
	int GetInt()
	{
		return (int)line_drawing_for_GetProperties->drawing_mode;
	}
	void GetChoices(std::list< std::wstring > &choices)
	{
		choices.push_back ( std::wstring ( L"draw tangential arcs" ) );
		choices.push_back ( std::wstring ( L"infinite line" ) );
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
	int GetInt()
	{
		return (int)line_drawing_for_GetProperties->circle_mode;
	}
	void GetChoices(std::list< std::wstring > &choices)
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
			list->push_back(new PropertyLengthWithConfig(NULL, L"radius", &radius_for_circle, L"RadiusForCircle"));
		}
		break;

	default:
		break;
	}

	Drawing::GetProperties(list);
}

bool LineArcDrawing::OnModeChange(void){
	// on start of drawing mode
	if(!Drawing::OnModeChange())return false;
	if(m_container)m_container = NULL;

	HeeksConfig config;
	config.Read(L"RadiusForCircle", &radius_for_circle, 5.0);

	ClearPrevObject();
	m_previous_direction_set = false;

	return true;
}

void LineArcDrawing::set_draw_step_not_undoable(int s)
{
	Drawing::set_draw_step_not_undoable(s);
	if(drawing_mode == SplineDrawingMode && spline_mode == RationalSplineMode)
	{
		spline_points.push_back(GetStartPos());
	}
}