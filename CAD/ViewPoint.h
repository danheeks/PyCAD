// ViewPoint.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "IPoint.h"

class CViewport;

class CViewPoint{
private:
	double m_initial_pixel_scale;
	bool m_perspective;
	CViewport* m_viewport;

	void SetProjection2(bool use_depth_testing);
	int ChooseBestPlane(int plane)const;
	
public:
	bool m_section;
	Point3d m_lens_point;
	Point3d m_target_point;
	Point3d m_vertical;
	double m_pixel_scale;  // not valid for perspective
	double m_view_angle;  // only valid for perspective
	double m_projm[16], m_modelm[16]; 
	int m_window_rect[4];
	bool m_matrix_valid;
	double m_near_plane;
	double m_far_plane;
	CBox m_extra_depth_box;
	CBox m_extra_view_box;

	CViewPoint(CViewport* viewport);
	~CViewPoint(void);
	CViewPoint(const CViewPoint &c);

	const CViewPoint& operator=(const CViewPoint &c);

	const Point3d rightwards_vector(void)const{return Point3d(m_lens_point, m_target_point) ^ m_vertical;}
	const Point3d forwards_vector(void)const{return Point3d(m_lens_point, m_target_point);}
	void Turn(double ang_x, double ang_y);
	void TurnI(IPoint point_diff);
	void TurnVertical(double ang_x, double ang_y);
	void TurnVertical(IPoint point_diff);
	void Shift(const Point3d &tv);
	void ShiftI(const IPoint &point_diff);
	void Scale(double multiplier, bool use_initial_pixel_scale = false);
	void Scale(const IPoint &diff, bool reversed = false);
	void Twist(double angle);
	void Twist(IPoint start, IPoint point_diff);
	void SetViewport()const;
	void SetProjection(bool use_depth_testing);
	void SetPickProjection(IRect &pick_box);
	void SetModelview(void);
	void SetView(const Point3d &unity, const Point3d &unitz, int margin);
	Point3d glUnproject(const Point3d &v)const;
	Point3d glProject(const Point3d &v)const;
	void SetPolygonOffset(void)const;
	void WindowMag(IRect &window_box);
	void SetViewAroundAllObjects(int margin);
	Line SightLine(const IPoint &point);
	int GetTwoAxes(Point3d& vx, Point3d& vy, bool flattened_onto_screen, int plane)const;
	void Set90PlaneDrawMatrix(Matrix &mat)const;
	void SetPerspective(bool perspective);
	bool GetPerspective(){return m_perspective;}
};
