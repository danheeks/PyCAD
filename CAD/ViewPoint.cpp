// ViewPoint.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "ViewPoint.h"
#include "Viewport.h"

CViewPoint::CViewPoint(CViewport* viewport){
	m_lens_point = Point3d(0, 0, 200);
	m_target_point = Point3d(0, 0, 0);
	m_vertical = Point3d(0, 1, 0);
	m_perspective = false;
	m_pixel_scale = 10;
	m_view_angle = 30;
	m_section = false;
	m_matrix_valid = false;
	m_viewport = viewport;
}

CViewPoint::~CViewPoint(void){
}

CViewPoint::CViewPoint(const CViewPoint &c){
	m_matrix_valid = false;
	operator=(c);
}

const CViewPoint& CViewPoint::operator=(const CViewPoint &c){
	m_lens_point = c.m_lens_point;
	m_target_point = c.m_target_point;
	m_vertical = c.m_vertical;
	m_perspective = c.m_perspective;
	m_pixel_scale = c.m_pixel_scale;
	m_view_angle = c.m_view_angle;
	m_section = c.m_section;
	memcpy(m_projm, c.m_projm, 16*sizeof(double));
	memcpy(m_modelm, c.m_modelm, 16*sizeof(double));
	memcpy(m_window_rect, c.m_window_rect, 4*sizeof(int));
	m_matrix_valid = c.m_matrix_valid;
	m_viewport = c.m_viewport;
	m_extra_depth_box = c.m_extra_depth_box;
	m_extra_view_box = c.m_extra_view_box;
	return *this;
}

void CViewPoint::TurnI(IPoint point_diff){
	if(point_diff.x > 100)point_diff.x = 100;
	else if(point_diff.x < -100)point_diff.x = -100;
	if(point_diff.y > 100)point_diff.y = 100;
	else if(point_diff.y < -100)point_diff.y = -100;
	IPoint size = m_viewport->GetViewportSize();
	double c=(size.GetWidth()+size.GetHeight())/20;
	Turn(point_diff.x/c, point_diff.y/c);
}

void CViewPoint::Turn(double ang_x, double ang_y){
	Point3d f(m_lens_point, m_target_point);
	double fl = f.magnitude();
	Point3d uu = m_vertical.Normalized();
	Point3d r = (f ^ uu).Normalized();
	m_lens_point = m_lens_point - r * sin(ang_x)*fl;
	m_lens_point = m_lens_point + f*(1-cos(ang_x));
	f = f + r * sin(ang_x) * fl;
	f = f - f * (1-cos(ang_x));
	r = (f ^ uu).Normalized();
	m_lens_point = m_lens_point + uu * sin(ang_y)*fl;
	m_lens_point = m_lens_point + f * (1-cos(ang_y));
	m_vertical = m_vertical + f * sin(ang_y)/fl;
	m_vertical = m_vertical - uu * (1-cos(ang_y));
}

void CViewPoint::TurnVertical(IPoint point_diff){
	if(point_diff.x > 100)point_diff.x = 100;
	else if(point_diff.x < -100)point_diff.x = -100;
	if(point_diff.y > 100)point_diff.y = 100;
	else if(point_diff.y < -100)point_diff.y = -100;
	IPoint size = m_viewport->GetViewportSize();
	double c=(size.GetWidth()+size.GetHeight())/20;
	TurnVertical(point_diff.x/c, point_diff.y/c);
}

void CViewPoint::TurnVertical(double ang_x, double ang_y){
	Point3d f = m_target_point - m_lens_point;
	Point3d uu = m_vertical.Normalized();
	Point3d r = (f ^ uu).Normalized();
	Point3d temp_target(m_target_point.x, m_target_point.y, 0);
	double dist = Point3d(m_lens_point.x, m_lens_point.y, 0).Dist(temp_target);
	double a_old = atan2(m_lens_point.y - m_target_point.y, m_lens_point.x - m_target_point.x);
	double a_new = a_old - ang_x;
	m_lens_point.x = m_target_point.x + dist * cos(a_new);
	m_lens_point.y =  m_target_point.y + dist * sin(a_new);
	Point3d temp_vertical(m_vertical.x, m_vertical.y, 0);
	temp_target.x = 0;
	temp_target.y = 0;
	dist = temp_vertical.Dist(temp_target);
	a_old = atan2(temp_vertical.y - temp_target.y, temp_vertical.x - temp_target.x);
	a_new = a_old - ang_x;
	m_vertical.x = temp_target.x + dist * cos(a_new);
	m_vertical.y = temp_target.y + dist * sin(a_new);
	f = Point3d(m_lens_point, m_target_point);
	uu = m_vertical.Normalized();
	r = (f ^ uu).Normalized();
	if(ang_y>PI/2)ang_y = PI/2;
	else if(ang_y<-PI/2)ang_y = -PI/2;
	double fl = f.magnitude();
	double div_magn = 1/fl;
	bool beyond_vertical = false;
	double z_increase = sin(ang_y)*f.z*div_magn - (1-cos(ang_y))*uu.z;
	if((m_vertical.z + z_increase )<0 && z_increase<0)beyond_vertical = true;
	double angle = ang_y/2;
	m_lens_point = m_lens_point + uu * fl * sin(angle) + f * (1-cos(angle));
	m_vertical = m_vertical + f * (sin(angle) * div_magn) + uu * ( cos(angle) - 1);
	if(beyond_vertical){
		m_vertical.z = 0;
		m_vertical.Normalize();
		f = (m_vertical ^ r) * fl;
		m_lens_point = m_target_point - f;
	}
}

void CViewPoint::Shift(const Point3d &tv){
	Point3d r = rightwards_vector().Normalized();
	Point3d f = forwards_vector().Normalized();
	Point3d u = m_vertical.Normalized();
	Point3d new_vector = r * tv.x + f * tv.z + u * tv.y;
	m_lens_point = m_lens_point + new_vector;
	m_target_point = m_target_point + new_vector;
}

void CViewPoint::ShiftI(const IPoint &point_diff){
	double div_x = (double)(point_diff.x)/m_pixel_scale;
	double div_y = (double)(point_diff.y)/m_pixel_scale;
	Point3d f = m_target_point - m_lens_point;
	Point3d uu = m_vertical.Normalized();
	Point3d r = (f ^ uu).Normalized();
	m_target_point = m_target_point - r * div_x;
	m_lens_point = m_lens_point - r * div_x;
	m_target_point = m_target_point + uu * div_y;
	m_lens_point = m_lens_point + uu * div_y;
}

void CViewPoint::WindowMag(IRect &window_box){
	IPoint size = m_viewport->GetViewportSize();
	double width_ratio=fabs((double)(window_box.width)/(double)(size.GetWidth()));
	double height_ratio=fabs((double)(window_box.height)/(double)(size.GetHeight()));
	if(height_ratio>width_ratio)width_ratio=height_ratio;
	double old_xcen = (double)(size.GetWidth())*0.5;
	double old_ycen = (double)(size.GetHeight())*0.5;
	double new_xcen = (double)window_box.x+(double)(window_box.width)*0.5;
	double new_ycen = (double)window_box.y+(double)(window_box.height)*0.5;
	double move_xcen = new_xcen-old_xcen;
	double move_ycen = new_ycen-old_ycen;
	Point3d f = m_target_point - m_lens_point;
	Point3d uu = m_vertical.Normalized();
	Point3d r = (f ^ uu).Normalized();
	m_target_point = m_target_point + r * move_xcen/m_pixel_scale;
	m_lens_point = m_lens_point + r * move_xcen/m_pixel_scale;
	m_target_point = m_target_point + uu * move_ycen/m_pixel_scale;
	m_lens_point = m_lens_point + uu * move_ycen/m_pixel_scale;
	//m_pixel_scale/=width_ratio;

	Scale(1.0/width_ratio);
}

void CViewPoint::Scale(double multiplier, bool use_initial_pixel_scale){
	if(use_initial_pixel_scale)m_pixel_scale = m_initial_pixel_scale;
	m_pixel_scale *= multiplier;
	if(m_pixel_scale > 1000000)m_pixel_scale = 1000000;
	if(m_pixel_scale < 0.000001)m_pixel_scale = 0.000001;

	// for perspective, move forward
	if(m_perspective)
	{
		Point3d f = m_target_point - m_lens_point;
		Point3d v= Point3d(f * (multiplier - 1));
		m_lens_point = Point3d(m_lens_point + v);
		if(m_lens_point.Dist(m_target_point) < 10){
			m_target_point = Point3d(m_lens_point + f.Normalized() * 10);
		}
	}
}

void CViewPoint::Scale(const IPoint &diff, bool reversed){
	double mouse_ydiff = diff.y;
	if(reversed)mouse_ydiff = -mouse_ydiff;
	IPoint size = m_viewport->GetViewportSize();
	double fraction=(mouse_ydiff)/((double)(size.GetHeight()));
	double multiplier = fraction;
	bool increasing=(multiplier>0);
	if(!increasing)multiplier = -multiplier;
	multiplier = 1 - multiplier;
	if(multiplier<0.00001)multiplier=0.00001;
	if(increasing)multiplier = 1/multiplier;
	if(multiplier< 0.1)multiplier = 0.1;
	Scale(multiplier, true);
}	

void CViewPoint::Twist(double angle){
	Point3d f = m_target_point - m_lens_point;
	Point3d uu = m_vertical.Normalized();
	Point3d r = (f ^ uu).Normalized();
	m_vertical = r * -sin(angle) + uu * cos(angle);
}

void CViewPoint::Twist(IPoint start, IPoint point_diff){
	Point3d f = m_target_point - m_lens_point;
	Point3d uu = m_vertical.Normalized();
	Point3d r = (f ^ uu).Normalized();
	Point3d screen_target = glProject(m_target_point);
	Point3d screen_start((double)start.x, (double)start.y, 0);
	Point3d screen_end((double)(start.x + point_diff.x), (double)(start.y + point_diff.y), 0);
	double old_angle = atan2(screen_start.y - screen_target.y, screen_start.x - screen_target.x);
	double angle = atan2(screen_end.y - screen_target.y, screen_end.x - screen_target.x);
	m_vertical = r * -sin(angle - old_angle) + uu * cos(angle - old_angle);
}

void CViewPoint::SetProjection2(bool use_depth_testing){
	double rad;
	CBox box;
	if(use_depth_testing)
	{
		theApp->GetBox(box);
		box.Insert(m_extra_depth_box);
		box.Insert(m_extra_view_box);
	}
	if(!use_depth_testing){
		m_near_plane = 0;
		m_far_plane = 100000000;
		rad = 20000000;
	}
	else if(!box.m_valid){
		m_near_plane = 0.2;
		m_far_plane = 2000;
		rad = 1000;
	}
	else{
		double boxc[3];
		box.Centre(boxc);
		rad = box.Radius();
		Point3d to_centre_of_box = Point3d(m_lens_point, Point3d(boxc));
		Point3d f = forwards_vector();
		f.Normalize();
		double distance =to_centre_of_box*f;
		if(m_section)rad /= 100;
		m_near_plane = distance - rad;
		m_far_plane = distance + rad;
	}

	IPoint size = m_viewport->GetViewportSize();
	double w = size.GetWidth()/m_pixel_scale;
	double h = size.GetHeight()/m_pixel_scale;
	double s = sqrt(w*w + h*h);
	m_near_plane -= s/2;
	m_far_plane += s/2;

	double hw = (double)(size.GetWidth())/2;
	double hh = (double)(size.GetHeight())/2;
	if(m_perspective)
	{
		double fovy = m_view_angle;
		if(h>w && w>0) fovy = 2 * 180/PI * atan( tan(m_view_angle/2 * PI/180) * h/w );
		if(m_near_plane < m_far_plane / 200)m_near_plane = m_far_plane / 200;
		gluPerspective(fovy, w/h, m_near_plane, m_far_plane);
	}
	else
	{
		glOrtho((-0.5 - hw)/m_pixel_scale, (hw+0.5)/m_pixel_scale, (-0.5-hh)/m_pixel_scale, (0.5+hh)/m_pixel_scale, (GLfloat)m_near_plane, (GLfloat)m_far_plane);
	}
}

void CViewPoint::SetProjection(bool use_depth_testing){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	SetProjection2(use_depth_testing);
	IPoint size = m_viewport->GetViewportSize();
	m_window_rect[0] = 0;
	m_window_rect[1] = 0;
	m_window_rect[2]=size.GetWidth();
	m_window_rect[3]=size.GetHeight();
	glGetDoublev (GL_PROJECTION_MATRIX, m_projm);
	m_matrix_valid = true;
}

void CViewPoint::SetPickProjection(IRect &pick_box){
	IPoint size = m_viewport->GetViewportSize();
	int vp[4] ={0, 0, size.GetWidth(), size.GetHeight()};
	double box_width = pick_box.width;
	double box_height = pick_box.height;
	double x_centre = (double)pick_box.x + box_width/2;
	double y_centre = (double)pick_box.y + box_height/2;
	if(box_width == 0)box_width = 1;
	if(box_height == 0)box_height = 1;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPickMatrix(x_centre, y_centre, abs(box_width), abs(box_height), vp);
	SetProjection2(true);
}

void CViewPoint::SetModelview(void){
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(m_lens_point.x, m_lens_point.y, m_lens_point.z, m_target_point.x, m_target_point.y, m_target_point.z, m_vertical.x, m_vertical.y, m_vertical.z);
	glGetDoublev (GL_MODELVIEW_MATRIX, m_modelm);
}

void CViewPoint::SetViewport(void)const{
	IPoint size = m_viewport->GetViewportSize();
	glViewport(0, 0, size.GetWidth(), size.GetHeight());
}

void CViewPoint::SetView(const Point3d &unity, const Point3d &unitz, int margin){
	m_target_point = Point3d(0, 0, 0);
	m_lens_point = m_target_point + unitz;
	m_vertical = unity;
	m_pixel_scale = 10;
	SetViewAroundAllObjects(margin);
}

Point3d CViewPoint::glUnproject(const Point3d &v)const{
	if(!m_matrix_valid)return Point3d(0, 0, 0);
	double x, y, z;
	gluUnProject(v.x, v.y, v.z, m_modelm, m_projm, m_window_rect, &x, &y, &z);
	Point3d temp(x, y, z);
	return temp;
}

Point3d CViewPoint::glProject(const Point3d &v)const{
	if(!m_matrix_valid)return Point3d(0, 0, 0);
	double x, y, z;
	gluProject(v.x, v.y, v.z, m_modelm, m_projm, m_window_rect, &x, &y, &z);
	Point3d temp(x, y, z);
	return temp;
}

void CViewPoint::SetPolygonOffset(void)const{
	glPolygonOffset(1.0, 1.0);
}

void CViewPoint::SetViewAroundAllObjects(int margin){
	CBox box;

	theApp->GetBox(box);
	box.Insert(m_extra_view_box);

	if(!box.m_valid)return;
	Point3d r = rightwards_vector().Normalized();
	CBox window;
	IPoint size = m_viewport->GetViewportSize();
	double width_ratio = (double)(size.GetWidth())/(double)(size.GetHeight());
	int width = size.GetWidth();
	int height = size.GetHeight();
	for(int i = 0; i<8; i++){
		double p[3];
		box.vert(i, p);
		double x = r * Point3d(p);
		double y = m_vertical * Point3d(p);
		window.Insert(x, y, 0);
	}
	Point3d uf = Point3d(m_lens_point, m_target_point).Normalized();
	double boxc[3];
	box.Centre(boxc);
	double cx = r * Point3d(boxc);
	double cy = m_vertical * Point3d(boxc);
	double m = fabs((window.m_x[3] - cx)/(0.255341921221036 * width_ratio));
	double y_dist =  fabs((window.m_x[4] - cy)/0.255341921221036);
	if(y_dist>m)m = y_dist;
	m += fabs(uf * Point3d(boxc));
	m_target_point = Point3d(boxc);
	m_lens_point = m_target_point - (uf * m);
	double Width = window.Width();
	double Height = window.Height();
	double pw = width - margin;
	double ph = height - margin;
	if(Width<0.00001)Width = 0.00001;
	if(Height<0.00001)Height = 0.00001;
	if(pw<0.00001)pw = 0.00001;
	if(ph<0.00001)ph = 0.00001;
	double px = pw/Width;
	double py = ph/Height;
	if(px<py)m_pixel_scale = px;
	else m_pixel_scale = py;
	m_section = false;
}

Line CViewPoint::SightLine(const IPoint &point){
	Point3d screen_point(point.x, m_viewport->GetViewportSize().GetHeight()-point.y, 0);
	Point3d s = glUnproject(screen_point);
	screen_point.z = 1;
	Point3d e = glUnproject(screen_point);
	return Line(s, e);
}

int CViewPoint::ChooseBestPlane(int plane)const{
	Point3d f = forwards_vector();
	double dp[3];
	Matrix orimat = theApp->GetDrawMatrix(false);
	dp[0] = Point3d(0, 0, 1).Transformed(orimat) * f;
	dp[1] = Point3d(0, 1, 0).Transformed(orimat) * f;
	dp[2] = Point3d(1, 0, 0).Transformed(orimat) * f;
	double best_dp = 0.0;
	int best_mode = -1;
	double second_best_dp = 0.0;
	int second_best_mode = -1;
	double third_best_dp;
	int third_best_mode = -1;
	for(int i = 0; i<3; i++){
		if(best_mode == -1){
			best_mode = i;
			best_dp = fabs(dp[i]);
		}
		else{
			if(fabs(dp[i])>best_dp){
				third_best_dp = second_best_dp;
				third_best_mode = second_best_mode;
				second_best_dp = best_dp;
				second_best_mode = best_mode;
				best_mode = i;
				best_dp = fabs(dp[i]);
			}
			else{
				if(second_best_mode == -1){
					second_best_mode = i;
					second_best_dp = fabs(dp[i]);
				}
				else{
					if(fabs(dp[i])>second_best_dp){
						third_best_dp = second_best_dp;
						third_best_mode = second_best_mode;
						second_best_dp = fabs(dp[i]);
						second_best_mode = i;
					}
					else{
						third_best_dp = fabs(dp[i]);
						third_best_mode = i;
					}
				}
			}
		}
	}
	switch(plane){
		case 0:
			return best_mode;
		case 1:
			return second_best_mode;
		default:
			return third_best_mode;
	}
}

int CViewPoint::GetTwoAxes(Point3d& vx, Point3d& vy, bool flattened_onto_screen, int plane)const{
	int plane_mode = ChooseBestPlane(plane);
	Matrix orimat = theApp->GetDrawMatrix(false);

	switch(plane_mode){
	case 0:
		vx = Point3d(1, 0, 0).Transformed(orimat);
		vy = Point3d(0, 1, 0).Transformed(orimat);
		break;

	case 1:
		vx = Point3d(1, 0, 0).Transformed(orimat);
		vy = Point3d(0, 0, 1).Transformed(orimat);
		break;

	case 2:
		vx = Point3d(0, 1, 0).Transformed(orimat);
		vy = Point3d(0, 0, 1).Transformed(orimat);
		break;
	}

	// find closest between vx and vy to screen y
	double dpx = vx * m_vertical;
	double dpy = vy * m_vertical;
	if(fabs(dpx) > fabs(dpy)){
		Point3d vtemp = vx;
		vx = vy;
		vy = vtemp;
	}

	// make sure vz is towards us
	if((vx ^ vy) * forwards_vector() > 0)
	{
		vx = -vx;
	}
	
	if(flattened_onto_screen){
		Point3d f = forwards_vector().Normalized();
		vx = Point3d(vx - (f * (f * vx))).Normalized();
		vy = Point3d(vy - (f * (f * vy))).Normalized();
		Point3d r = rightwards_vector();
		if(fabs(vy * r) > fabs(vx * r)){
			Point3d temp = vx;
			vx = vy;
			vy = temp;
		}
	}
	return plane_mode;
}

void CViewPoint::Set90PlaneDrawMatrix(Matrix &mat)const{
	int plane = ChooseBestPlane(0);
	mat = theApp->GetDrawMatrix(false);
	switch(plane){
	case 1:
		mat = Matrix(Point3d(0, 0, 0).Transformed(mat), Point3d(1, 0, 0).Transformed(mat), Point3d(0, 0, 1).Transformed(mat));
		break;
		
	case 2:
		mat = Matrix(Point3d(0, 0, 0).Transformed(mat), Point3d(0, 1, 0).Transformed(mat), Point3d(0, 0, 1).Transformed(mat));
		break;
	}
}

void CViewPoint::SetPerspective(bool perspective){
	if(m_perspective == perspective)return;

	m_perspective = perspective;
	if(perspective)
	{
		// switch from orthographic to perpective
	}
	else
	{
		// switch from perspective to orthographic
	}
}
