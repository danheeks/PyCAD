// RegularShapesDrawing.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Drawing.h"

enum RegularShapeMode
{
	RectanglesRegularShapeMode,
	PolygonsRegularShapeMode,
	ObroundRegularShapeMode
};

enum PolygonMode
{
	ExcribedMode,
	InscribedMode
};

class RegularShapesDrawing: public Drawing{
private:
	// Drawing's virtual functions
	bool calculate_item(DigitizedPoint &end);
	int number_of_steps(){return 2;}
	int step_to_go_to_after_last_step(){return 0;}
	bool is_an_add_level(int level){return level == 1;}

	void CalculateRectangle(double x, double y, const geoff_geometry::Point3d& p0, const geoff_geometry::Point3d& p1, const geoff_geometry::Point3d& p2, const geoff_geometry::Point3d& p3, const geoff_geometry::Point3d& xdir, const geoff_geometry::Point3d& ydir, const geoff_geometry::Point3d& zdir);
	void CalculatePolygon(const geoff_geometry::Point3d& p0, const geoff_geometry::Point3d& p1, const geoff_geometry::Point3d& zdir);
	void CalculateObround(const geoff_geometry::Point3d& p0, const geoff_geometry::Point3d& p1, const geoff_geometry::Point3d& xdir, const geoff_geometry::Point3d& zdir);

public:
	RegularShapeMode m_mode;
	PolygonMode p_mode;
	int m_number_of_side_for_polygon;
	double m_rect_radius;
	double m_obround_radius;

	RegularShapesDrawing(void);
	virtual ~RegularShapesDrawing(void);

	// InputMode's virtual functions
	const wchar_t* GetTitle();
	void GetProperties(std::list<Property *> *list);

	void ClearSketch();
};

extern RegularShapesDrawing regular_shapes_drawing;
