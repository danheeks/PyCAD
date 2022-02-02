// EllipseDrawing.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#if 0
this must be ported to python and removed from here

#include "EllipseDrawing.h"
#include "HeeksObj.h"
#include "HEllipse.h"
#include "HSpline.h"
#include "Sketch.h"
#include "DigitizeMode.h"
#include "Property.h"

static bool DigitizedPointGetCubicSpline(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, const DigitizedPoint& d4, Handle_Geom_BSplineCurve &spline)
{
	TColgp_Array1OfPnt poles(1, 4);
	poles.SetValue(1, P2G(d1.m_point)); poles.SetValue(2, P2G(d3.m_point)); poles.SetValue(3, P2G(d4.m_point)); poles.SetValue(4, P2G(d2.m_point));
	Handle(Geom_BezierCurve) curve = new Geom_BezierCurve(poles);
	GeomConvert_CompCurveToBSplineCurve convert(curve);

	spline = convert.BSplineCurve();
	return true;
}

static bool DigitizedPointGetQuarticSpline(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, Handle_Geom_BSplineCurve &spline)
{
	TColgp_Array1OfPnt poles(1, 3);
	poles.SetValue(1, P2G(d1.m_point)); poles.SetValue(2, P2G(d3.m_point)); poles.SetValue(3, P2G(d2.m_point));
	Handle(Geom_BezierCurve) curve = new Geom_BezierCurve(poles);
	GeomConvert_CompCurveToBSplineCurve convert(curve);

	spline = convert.BSplineCurve();

	return true;
}

bool DigitizedPointGetRationalSpline(std::list<DigitizedPoint> &spline_points, const DigitizedPoint& d4, Handle_Geom_BSplineCurve &spline)
{
	TColgp_Array1OfPnt poles(1, spline_points.size() + 1);
	int idx = 1;
	std::list<DigitizedPoint>::iterator it;
	for (it = spline_points.begin(); it != spline_points.end(); ++it)
	{
		poles.SetValue(idx, P2G((*it).m_point));
		idx++;
	}
	poles.SetValue(spline_points.size() + 1, P2G(d4.m_point));
	Handle(Geom_BezierCurve) curve = new Geom_BezierCurve(poles);
	GeomConvert_CompCurveToBSplineCurve convert(curve);

	spline = convert.BSplineCurve();

	return true;

}

bool DigitizedPointGetEllipse(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, gp_Elips& e)
{
	double d = d2.m_point.Dist(d1.m_point);
	e.SetLocation(P2G(d1.m_point));
	e.SetMajorRadius(d);
	e.SetMinorRadius(d / 2);

	Point3d vec = d2.m_point - d1.m_point;
	vec = vec / d;
	double rot = atan2(vec.y, vec.x);

	gp_Dir up(0, 0, 1);
	gp_Pnt zp(0, 0, 0);
	e.Rotate(gp_Ax1(P2G(d1.m_point), up), rot);

	gp_Dir x_axis = e.XAxis().Direction();
	gp_Dir y_axis = e.YAxis().Direction();
	double maj_r = d;

	//We have to rotate the incoming vector to be in our coordinate system
	gp_Pnt cir = P2G(d3.m_point - d1.m_point);
	cir.Rotate(gp_Ax1(zp, up), -rot + M_PI / 2);

	double nradius = 1 / sqrt((1 - (1 / maj_r)*(1 / maj_r)*cir.Y()*cir.Y()) / (cir.X() * cir.X()));
	if (nradius < maj_r)
		e.SetMinorRadius(nradius);
	else
	{
		e.SetMajorRadius(nradius);
		e.SetMinorRadius(maj_r);
		e.Rotate(gp_Ax1(P2G(d1.m_point), up), M_PI / 2);
	}


	return true;
}

EllipseDrawing ellipse_drawing;

EllipseDrawing::EllipseDrawing(void){
	drawing_mode = EllipseDrawingMode;
}

EllipseDrawing::~EllipseDrawing(void){
}

int EllipseDrawing::number_of_steps()
{
	switch (drawing_mode)
	{
	case SplineDrawingMode:
		switch (spline_mode)
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

int EllipseDrawing::step_to_go_to_after_last_step()
{
	switch (drawing_mode)
	{
	case SplineDrawingMode:
		return 3;
	case EllipseDrawingMode:
	default:
		return 0;
	}
}

bool EllipseDrawing::is_a_draw_level(int level)
{
	if (drawing_mode == SplineDrawingMode && spline_mode == RationalSplineMode)
		return level >= 3;
	return Drawing::is_a_draw_level(level);
}

bool EllipseDrawing::is_an_add_level(int level)
{
	switch (drawing_mode)
	{
	case EllipseDrawingMode:
		return level == 2;
	case SplineDrawingMode:
		switch (spline_mode)
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

void EllipseDrawing::AddPoint()
{
	switch (drawing_mode)
	{
	case EllipseDrawingMode:
		Drawing::AddPoint();
		break;
	default:
		Drawing::AddPoint();
		break;
	}
}

bool EllipseDrawing::calculate_item(DigitizedPoint &end){
	if (number_of_steps() > 1 && GetStartPos().m_type == DigitizeNoItemType)return false;
	if (end.m_type == DigitizeNoItemType)return false;

	switch (drawing_mode)
	{
	case EllipseDrawingMode:
		if (TempObject() && TempObject()->GetType() != HEllipse::m_type){
			ClearObjectsMade();
		}

		if (TempObject() == NULL)
		{
			gp_Elips elip;
			DigitizedPointGetEllipse(GetBeforeStartPos(), GetStartPos(), end, elip);

			AddToTempObjects(new HEllipse(elip, &theApp->GetCurrentColor()));

		}
		else
		{
			gp_Elips elip;
			DigitizedPointGetEllipse(GetBeforeStartPos(), GetStartPos(), end, elip);
			((HEllipse*)TempObject())->SetEllipse(elip);
		}
		return true;

	case SplineDrawingMode:
	{
		if (TempObject() && TempObject()->GetType() != HSpline::m_type){
			ClearObjectsMade();
		}

		Handle_Geom_BSplineCurve spline;
		switch (spline_mode)
		{
		case CubicSplineMode:
			DigitizedPointGetCubicSpline(GetBeforeBeforeStartPos(), GetBeforeStartPos(), GetStartPos(), end, spline);
			break;
		case QuarticSplineMode:
			DigitizedPointGetQuarticSpline(GetBeforeStartPos(), GetStartPos(), end, spline);
			break;
		case RationalSplineMode:
			DigitizedPointGetRationalSpline(spline_points, end, spline);
			break;
		}

		if (TempObject() == NULL){
			AddToTempObjects(new HSpline(spline, &theApp->GetCurrentColor()));
		}
		else{
			((HSpline*)TempObject())->m_spline = spline;
		}

		return true;
	}
	default:
		break;
	}

	return false;
}

HeeksObj* EllipseDrawing::GetOwnerForDrawingObjects()
{
	return theApp->GetObjPointer(); //Object always needs to be added somewhere
}


static std::wstring str_for_GetTitle;

const wchar_t* EllipseDrawing::GetTitle()
{
	switch (drawing_mode)
	{
	case EllipseDrawingMode:
		str_for_GetTitle = std::wstring(L"Ellipse drawing mode");
		str_for_GetTitle.append(std::wstring(L" : "));

		str_for_GetTitle.append(std::wstring(L"center and 2 points mode"));
		str_for_GetTitle.append(std::wstring(L"\n  "));
		if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(L"click on center point"));
		else if (GetDrawStep() == 1)
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

		switch (spline_mode){
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

	default:
		return L"unknown";
	}
}


void EllipseDrawing::set_cursor(void){
}

void EllipseDrawing::OnModeChange(void){
	// on start of drawing mode
	Drawing::OnModeChange();

	ClearPrevObject();
}

void EllipseDrawing::set_draw_step_not_undoable(int s)
{
	Drawing::set_draw_step_not_undoable(s);
	if (drawing_mode == SplineDrawingMode && spline_mode == RationalSplineMode)
	{
		spline_points.push_back(GetStartPos());
	}
}

#endif