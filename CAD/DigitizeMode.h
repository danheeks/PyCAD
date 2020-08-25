// DigitizeMode.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "InputMode.h"
#include "DigitizedPoint.h"

class CViewPoint;
class PointOrWindow;

class DigitizeMode:public CInputMode{
private:
	PointOrWindow *point_or_window;
	DigitizedPoint lbutton_point;

	DigitizedPoint digitize1(const IPoint &input_point);

public:
	DigitizedPoint digitized_point;
	DigitizedPoint reference_point;	// the last point the operator explicitly defined (as opposed to mouse movements over the graphics canvas)
	std::wstring prompt;
	bool wants_to_exit_main_loop;

	DigitizeMode();
	virtual ~DigitizeMode(void);

	// InputMode's virtual functions
	int GetType(){ return InputModeTypeDigitize; }
	const wchar_t* GetTitle();
	const wchar_t* GetHelpText();
	void OnMouse( MouseEvent& event );
//	void OnKeyDown(KeyCode key_code);
	void OnModeChange(void);
	void OnFrontRender();
	void GetProperties(std::list<Property *> *list);

	DigitizedPoint Digitize(const Line &ray);
	DigitizedPoint digitize(const IPoint &point);
	void SetOnlyCoords(HeeksObj* object, bool onoff);
	bool OnlyCoords(HeeksObj* object);

	// calculate tangent points
	static bool GetLinePoints(const DigitizedPoint& d1, const DigitizedPoint& d2, Point3d &p1, Point3d &p2);
	static bool GetArcPoints(const DigitizedPoint& d1, const Point3d *initial_direction, const DigitizedPoint& d2, Point3d &p1, Point3d &p2, Point3d &centre, Point3d &axis);
	static bool GetTangentCircle(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, Circle& c);
#if 0
	static bool GetEllipse(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, gp_Elips& e);
#endif
	static bool GetCircleBetween(const DigitizedPoint& d1, const DigitizedPoint& d2, Circle& c);
#if 0
	static bool GetQuarticSpline(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, Handle_Geom_BSplineCurve &spline);
	static bool GetCubicSpline(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, const DigitizedPoint& d4, Handle_Geom_BSplineCurve &spline);
	static bool GetRationalSpline(std::list<DigitizedPoint> &spline_points, const DigitizedPoint& d4, Handle_Geom_BSplineCurve &spline);
#endif

};


