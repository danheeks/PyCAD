// Grid.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Grid.h"
#include "ViewPoint.h"
#include "Viewport.h"

static void RenderGrid(const CViewPoint *view_point, double max_number_across, bool in_between_spaces, bool miss_main_lines, const HeeksColor *bg, const HeeksColor *cc, unsigned char brightness, int plane_mode){
	Point3d sp[4];
	double zval = 0.5;
	IPoint size = theApp->m_current_viewport->GetViewportSize();
	sp[0] = Point3d(0, 0, zval);
	sp[1] = Point3d(size.GetWidth(), 0, zval);
	sp[2] = Point3d(size.GetWidth(), size.GetHeight(), zval);
	sp[3] = Point3d(0, size.GetHeight(), zval);
	Point3d vx, vy;
	int plane_mode2 = view_point->GetTwoAxes(vx, vy, false, plane_mode);
	Point3d datum(0, 0, 0);
	Matrix orimat = *(theApp->GetDrawMatrix(false));
	datum = datum.Transformed(orimat);
	orimat = Matrix(datum, vx, vy);
	Point3d unit_forward = view_point->forwards_vector().Normalized();
	double plane_dp = fabs((Point3d(0, 0, 1).Transformed(orimat) - datum) * unit_forward);
	if(plane_dp < 0.3)return;
	Plane plane(datum, Point3d(0, 0, 1).Transformed(orimat) - datum);
	{
		for(int i =0; i<4; i++){
			Point3d p1 = view_point->glUnproject(sp[i]);
			sp[i].z = 0;
			Point3d p2 = view_point->glUnproject(sp[i]);
			if(p1.Dist(p2) < 0.00000000001)return;
			Line line = Line(p1, p2);
			Point3d pnt;
			double t;
			if(plane.Intof(line, pnt, t))
			{
				sp[i].x = (pnt * vx) - (datum * vx);
				sp[i].y = (pnt * vy) - (datum * vy);
				sp[i].z = 0;
			}
		}
	}
	CBox b;
	{
		for(int i = 0; i<4; i++){
			b.Insert(sp[i].x, sp[i].y, sp[i].z);
		}
	}
	double width = b.Width();
	double height = b.Height();
	double biggest_dimension;
	if(height > width)biggest_dimension = height;
	else biggest_dimension = width;
	double widest_spacing = biggest_dimension/max_number_across;
	bool dimmer = false;
	double dimness_ratio = 1.0;
	double spacing;

	/*
	if(theApp->draw_to_grid){
		spacing = theApp->digitizing_grid;
		if(!miss_main_lines)spacing *= 10;
		if(spacing<0.0000000001)return;
		if(biggest_dimension / spacing > max_number_across * 1.5)return;
		if(biggest_dimension / spacing > max_number_across){
			dimmer = true;
			dimness_ratio = (max_number_across * 1.5 - biggest_dimension / spacing)/ (max_number_across * 0.5);
		}

	}
	else*/{
		double l = log10(widest_spacing / theApp->m_view_units);

		double intl = (int)l;
		if(l>0)intl++;

		spacing = pow(10.0, intl) * theApp->m_view_units;
	}

	if(theApp->grid_mode == 3){
		dimmer = true;
		dimness_ratio *= plane_dp;
		dimness_ratio *= plane_dp;
	}
	double ext2d[4];
	ext2d[0] = b.m_x[0];
	ext2d[1] = b.m_x[1];
	ext2d[2] = b.m_x[3];
	ext2d[3] = b.m_x[4];
	{
		for(int i = 0; i<4; i++){
			double intval = (int)(ext2d[i]/spacing);
			if(i <2 ){
				if(ext2d[i]<0)intval--;
			}
			else{
				if(ext2d[i]>0)intval++;
			}

			ext2d[i] = intval * spacing;
		}
	}
	if(cc){
		HeeksColor col = *cc;
		if(theApp->grid_mode == 3){
			switch(plane_mode2){
			case 0:
				col.green = (unsigned char)(0.6 * (double)(bg->green));
				break;
			case 1:
				col.red = (unsigned char)(0.9 * (double)(bg->red));
				break;
			case 2:
				col.blue = bg->blue;
				break;
			}
		}

		if(dimmer){
			double d_brightness = (double)brightness;
			d_brightness *= dimness_ratio;
			unsigned char uc_brightness = (unsigned char)d_brightness;

			glColor4ub(col.red, col.green, col.blue, uc_brightness);
		}
		else{
			glColor4ub(col.red, col.green, col.blue, brightness);
		}
	}
	glBegin(GL_LINES);
	double extra = 0;
	if(in_between_spaces)extra = spacing * 0.5;
	for(double x = ext2d[0] - extra; x<ext2d[2] + extra; x += spacing){
		if(miss_main_lines){
			double xr = x/spacing/5;
			if( fabs(  xr - (double)(int)(xr+ (xr>0 ? 0.5:-0.5)) ) < 0.1)continue;
		}
		Point3d temp(datum + (vx * x) + (vy * ext2d[1]));
		glVertex3d(temp.x, temp.y, temp.z);
		temp = (datum + (vx * x) + (vy * ext2d[3]));
		glVertex3d(temp.x, temp.y, temp.z);
	}
	for(double y = ext2d[1] - extra; y<ext2d[3] + extra; y += spacing){
		if(miss_main_lines){
			double yr = y/spacing/5;
			if( fabs(  yr - (double)(int)(yr+(yr>0 ? 0.5:-0.5)) ) < 0.1)continue;
		}
		Point3d temp = (datum + (vx * ext2d[0]) + (vy * y));
		glVertex3d(temp.x, temp.y, temp.z);
		temp = (datum + (vx * ext2d[2]) + (vy * y));
		glVertex3d(temp.x, temp.y, temp.z);
	}
	glEnd();
}

void GetGridBox(const CViewPoint *view_point, CBox &ext){
	Point3d sp[4];
	double zval = 0.5;
	IPoint size = theApp->m_current_viewport->GetViewportSize();
	sp[0] = Point3d(0, 0, zval);
	sp[1] = Point3d(size.GetWidth(), 0, zval);
	sp[2] = Point3d(size.GetWidth(), size.GetHeight(), zval);
	sp[3] = Point3d(0, size.GetHeight(), zval);
	Point3d vx, vy;
	view_point->GetTwoAxes(vx, vy, false, 0);
	Point3d datum(0, 0, 0);
	Matrix orimat = *(theApp->GetDrawMatrix(false));
	datum = datum.Transformed(orimat);
	orimat = Matrix(datum, vx, vy);
	Plane plane(datum, Point3d(0, 0, 1).Transformed(orimat) - datum);
	{
		for(int i =0; i<4; i++){
			Point3d p1 = view_point->glUnproject(sp[i]);
			sp[i].z = 0;
			Point3d p2 = view_point->glUnproject(sp[i]);
			Line line = Line(p1, p2);
			Point3d pnt;
			double t;
			if (plane.Intof(line, pnt, t))
			{
				ext.Insert(pnt.x, pnt.y, pnt.z);
			}
		}
	}
}

static void RenderGrid(const CViewPoint *view_point, int plane)
{
	switch(theApp->grid_mode){
	case 1:
		{
			const HeeksColor& bg = theApp->background_color[0];
			HeeksColor cc = bg.best_black_or_white();
			Point3d v_bg((double)bg.red, (double)bg.green, (double)bg.blue);
			Point3d v_cc((double)cc.red, (double)cc.green, (double)cc.blue);
			Point3d v_contrast = v_cc - v_bg;
			Point3d unit_contrast = v_contrast.Normalized();
			double l1, l2, l3;
			if(v_cc * Point3d(1,1,1)>0){
				l1 = 200;
				l2 = 130;
				l3 = 80;
			}
			else{
				l1 = 100;
				l2 = 30;
				l3 = 10;
			}
			if(l1>v_contrast.magnitude())l1 = v_contrast.magnitude();
			if(l2>v_contrast.magnitude())l2 = v_contrast.magnitude();
			if(l3>v_contrast.magnitude())l3 = v_contrast.magnitude();
			Point3d uf = (view_point->forwards_vector()).Normalized();
			Point3d vx, vy;
			view_point->GetTwoAxes(vx, vy, false, plane);
			Point3d datum(0, 0, 0);
			Matrix orimat = *(theApp->GetDrawMatrix(false));
			datum = datum.Transformed(orimat);
			orimat = Matrix(datum, vx, vy);
			Point3d v_up = Point3d(0,0, 1).Transformed(orimat) - datum;
			double fufz = fabs(uf * v_up);
			if(fufz<0.7){
				double there = (fufz - 0.3) / 0.4;
				l1 *= there;
				l2 *= there;
			}
			Point3d v_gc1 = v_bg + unit_contrast * l1;
			Point3d v_gc2 = v_bg + unit_contrast * l2;
			Point3d v_gc3 = v_bg + unit_contrast * l3;
			glColor3ub((unsigned char)(v_gc3.x), (unsigned char)(v_gc3.y), (unsigned char)(v_gc3.z));
			RenderGrid(view_point, 200, false, true, NULL, NULL, 0, plane);
			glColor3ub((unsigned char)(v_gc2.x), (unsigned char)(v_gc2.y), (unsigned char)(v_gc2.z));
			RenderGrid(view_point, 20, true, false, NULL, NULL, 0, plane);
			glColor3ub((unsigned char)(v_gc1.x), (unsigned char)(v_gc1.y), (unsigned char)(v_gc1.z));
			RenderGrid(view_point, 20, false, false, NULL, NULL, 0, plane);
		}
		break;

	case 2:
	case 3:
		{
			const HeeksColor& bg = theApp->background_color[0];
			HeeksColor cc = bg.best_black_or_white();
			bool light_color = cc.red + cc.green + cc.blue > 384;
			theApp->EnableBlend();
			RenderGrid(view_point, 200, false, true, &bg, &cc, light_color ? 40:10, plane);
			RenderGrid(view_point, 20, true, false, &bg, &cc, light_color ? 80:20, plane);
			RenderGrid(view_point, 20, false, false, &bg, &cc, light_color ? 120:30, plane);
			theApp->DisableBlend();
		}
		break;
	}
}

void RenderGrid(const CViewPoint *view_point){
	RenderGrid(view_point, 0);
}
