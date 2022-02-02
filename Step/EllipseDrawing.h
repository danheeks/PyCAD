// EllipseDrawing.h
// Copyright (c) 2020, Dan Heeks
// This program is released under the GPL3 license. See the file COPYING for details.

#pragma once

# if 0
this must be ported to python

#include "Drawing.h"
#include <list>
#include <geometry.h>

enum EnumDrawingMode{
	EllipseDrawingMode,
	SplineDrawingMode
};

enum EnumSplineDrawingMode{
	CubicSplineMode,
	QuarticSplineMode,
	RationalSplineMode
};


class EllipseDrawing : public Drawing{
private:
	std::list<DigitizedPoint> spline_points;
	HeeksObj* m_container;
	bool m_add_to_sketch;

	// Drawing's virtual functions
	bool calculate_item(DigitizedPoint &end);
	int number_of_steps();
	int step_to_go_to_after_last_step();
	bool is_an_add_level(int level);
	bool is_a_draw_level(int level);
	HeeksObj* GetOwnerForDrawingObjects();
	void AddPoint();

public:
	//	static wxCursor m_cursor_start;
	//	static wxCursor m_cursor_end;
	EnumDrawingMode drawing_mode;
	std::list<EnumDrawingMode> m_save_drawing_mode;
	double radius_for_circle;
	EnumSplineDrawingMode spline_mode;

	EllipseDrawing(void);
	virtual ~EllipseDrawing(void);

	// InputMode's virtual functions
	const wchar_t* GetTitle();
	void set_cursor(void);
	void OnModeChange(void);

	// Drawing's virtual functions
	void set_draw_step_not_undoable(int s);

	void EndDrawing(); // won't stay here;  just a test
};

extern EllipseDrawing ellipse_drawing;

#endif