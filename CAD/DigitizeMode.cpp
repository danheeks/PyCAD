// DigitizeMode.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "DigitizeMode.h"
#include "SelectMode.h"
#include "MarkedList.h"
#include "CoordinateSystem.h"
#include "DigitizeMode.h"
#include "Drawing.h"
#include "PointOrWindow.h"
#include "Viewport.h"
#include "GripData.h"
#include "Property.h"

DigitizeMode::DigitizeMode(){
	point_or_window = new PointOrWindow(false);
	wants_to_exit_main_loop = false;
}

DigitizeMode::~DigitizeMode(void){
	delete point_or_window;
}

static std::wstring digitize_title_coords_string;

const wchar_t* DigitizeMode::GetTitle()
{
	{
		digitize_title_coords_string = m_prompt_when_doing_a_main_loop;
		std::wstring xy_str;
		digitize_title_coords_string.append(L"\n");

		if (fabs(digitized_point.m_point.z) < 0.00000001)
		{
			wchar_t buf[256];
			swprintf(buf, 256, L"X%g Y%g", digitized_point.m_point.x, digitized_point.m_point.y);
			digitize_title_coords_string += buf;
		}
		else if (fabs(digitized_point.m_point.y) < 0.00000001)
		{
			wchar_t buf[256];
			swprintf(buf, 256, L"X%g Z%g", digitized_point.m_point.x, digitized_point.m_point.z);
			digitize_title_coords_string += buf;
		}
		else if (fabs(digitized_point.m_point.x) < 0.00000001)
		{
			wchar_t buf[256];
			swprintf(buf, 256, L"Y%g Z%g", digitized_point.m_point.y, digitized_point.m_point.z);
			digitize_title_coords_string += buf;
		}
		else
		{
			wchar_t buf[256];
			swprintf(buf, 256, L"X%g Y%g Z%g", digitized_point.m_point.x, digitized_point.m_point.y, digitized_point.m_point.z);
			digitize_title_coords_string += buf;
		}

		const wchar_t* type_str = NULL;
		switch(digitized_point.m_type)
		{
		case DigitizeEndofType:
			type_str = L"end";
			break;
		case DigitizeIntersType:
			type_str = L"intersection";
			break;
		case DigitizeMidpointType:
			type_str = L"midpoint";
			break;
		case DigitizeCentreType:
			type_str = L"centre";
			break;
		case DigitizeScreenType:
			type_str = L"screen";
			break;
		case DigitizeNearestType:
			type_str = L"nearest";
			break;
		case DigitizeTangentType:
			type_str = L"tangent";
			break;
		default:
			break;
		}

		if(type_str)
		{
			digitize_title_coords_string.append(L" (");
			digitize_title_coords_string.append(type_str);
			digitize_title_coords_string.append(L")");
		}

		return digitize_title_coords_string.c_str();
	}
	return L"Digitize Mode";
}

static std::wstring digitize_help_text;

const wchar_t* DigitizeMode::GetHelpText()
{
	digitize_help_text.assign(L"Press Esc key to cancel");
	digitize_help_text.append(L"\n");
	digitize_help_text.append(L"Left button to accept position");
	return digitize_help_text.c_str();
}

void DigitizeMode::OnMouse( MouseEvent& event ){
	if(event.LeftDown()){
		point_or_window->OnMouse(event);
		lbutton_point = digitize(IPoint(event.GetX(), event.GetY()));
	}
	else if(event.LeftUp()){
		if(lbutton_point.m_type != DigitizeNoItemType){
			digitized_point = lbutton_point;
			wants_to_exit_main_loop = true;
		}
	}
	else if(event.Moving()){
		digitize(IPoint(event.GetX(), event.GetY()));
		point_or_window->OnMouse(event);
		theApp->RefreshInputCanvas();
		theApp->OnInputModeTitleChanged();
	}
}

static Matrix global_matrix_relative_to_screen;

static const Matrix& digitizing_matrix(bool calculate = false){
	if(calculate){
		if(theApp->digitize_screen){
			Matrix mat = theApp->GetDrawMatrix(false);
			Point3d origin = Point3d(0, 0, 0).Transformed(mat);
			Point3d x1 = origin + Point3d(1, 0, 0);
			Point3d y1 = origin + Point3d(0, 1, 0);
			Point3d po = origin;
			po = theApp->m_current_viewport->m_view_point.glUnproject(po);
			x1 = theApp->m_current_viewport->m_view_point.glUnproject(x1);
			y1 = theApp->m_current_viewport->m_view_point.glUnproject(y1);
			
			global_matrix_relative_to_screen = Matrix(origin, Point3d(po, x1).Normalized(), Point3d(po, y1).Normalized());
		}
		else{
			global_matrix_relative_to_screen = theApp->GetDrawMatrix(true);
		}
	}
	return global_matrix_relative_to_screen;
}

void DigitizeMode::OnModeChange(void){
	point_or_window->reset();
	point_or_window->OnModeChange();
	digitize(theApp->cur_mouse_pos);
}

bool make_point_from_doubles(const std::list<double> &dlist, std::list<double>::const_iterator &It, Point3d& pnt, bool four_doubles = false)
{
	if (It == dlist.end())return false;
	if (four_doubles)It++;
	pnt.x = (*It);
	It++;
	pnt.y = (*It);
	It++;
	pnt.z = (*It);
	It++;
	return true;
}

int convert_doubles_to_pnts(const std::list<double> &dlist, std::list<Point3d> &plist, bool four_doubles = false)
{
	int nump = 0;
	for (std::list<double>::const_iterator It = dlist.begin(); It != dlist.end();)
	{
		Point3d pnt;
		if (!make_point_from_doubles(dlist, It, pnt, four_doubles))break;
		plist.push_back(pnt);
		nump++;
	}
	return nump;
}

DigitizedPoint DigitizeMode::digitize1(const IPoint &input_point){
	Line ray = theApp->m_current_viewport->m_view_point.SightLine(input_point);
	std::list<DigitizedPoint> compare_list;
	std::list<HeeksObj*> objects;
	if(theApp->digitize_end || theApp->digitize_inters || theApp->digitize_centre || theApp->digitize_midpoint || theApp->digitize_nearest || theApp->digitize_tangent){
		point_or_window->SetWithPoint(input_point);
		theApp->m_marked_list->ignore_coords_only = true;
		theApp->ColorPickLowestObjects(point_or_window->box_chosen, false, objects);
		theApp->m_marked_list->ignore_coords_only = false;
	}
	if(theApp->digitize_end){
		for (std::list<HeeksObj*>::iterator It = objects.begin(); It != objects.end(); It++){
			HeeksObj* object = *It;
			std::list<GripData> vl;
			object->GetGripperPositionsTransformed(&vl, true);
			for (std::list<GripData>::iterator It = vl.begin(); It != vl.end(); It++)
			{
				Point3d& pnt = (*It).m_p;
				compare_list.push_back(DigitizedPoint(pnt, DigitizeEndofType));
			}
		}
	}
	if(theApp->digitize_inters){
		if (objects.size() > 1)
		{
			for (std::list<HeeksObj*>::iterator It = objects.begin(); It != objects.end(); It++)
			{
				HeeksObj* object = *It;
				std::list<HeeksObj*>::iterator It2 = It;
				It2++;
				for (; It2 != objects.end(); It2++)
				{
					HeeksObj* object2 = *It2;
					std::list<double> rl;
					if(object->Intersects(object2, &rl))
					{
						std::list<Point3d> plist;
						convert_doubles_to_pnts(rl, plist);
						for(std::list<Point3d>::iterator It = plist.begin(); It != plist.end(); It++)
						{
							Point3d& pnt = *It;
							compare_list.push_back(DigitizedPoint(pnt, DigitizeIntersType));
						}
					}
				}
			}
		}
	}
	if(theApp->digitize_midpoint){
		for (std::list<HeeksObj*>::iterator It = objects.begin(); It != objects.end(); It++)
		{
			HeeksObj* object = *It;
			Point3d p;
			if(object->GetMidPoint(p)){
				compare_list.push_back(DigitizedPoint(p, DigitizeMidpointType));
			}
		}
	}
	if(theApp->digitize_nearest){
		for (std::list<HeeksObj*>::iterator It = objects.begin(); It != objects.end(); It++)
		{
			HeeksObj* object = *It;
			Line ray;
			Point3d p;
			if(object->FindNearPoint(ray, p)){
				compare_list.push_back(DigitizedPoint(p, DigitizeNearestType));
			}
		}
	}
	if(theApp->digitize_tangent){
		for (std::list<HeeksObj*>::iterator It = objects.begin(); It != objects.end(); It++)
		{
			HeeksObj* object = *It;
			Line ray;
			Point3d p;
			if (object->FindPossTangentPoint(ray, p)){
				compare_list.push_back(DigitizedPoint(Point3d(p), DigitizeTangentType, object));
			}
		}
	}
	double min_dist = -1;
	DigitizedPoint *best_digitized_point = NULL;
	if(compare_list.size() >0){
		std::list<DigitizedPoint>::iterator It;
		double dist;
		for(It = compare_list.begin(); It != compare_list.end(); It++){
			DigitizedPoint *this_digitized_point = &(*It);
			double t;
			dist = ray.Near(this_digitized_point->m_point, t).Dist(this_digitized_point->m_point);
			if(dist * theApp->GetPixelScale() < 2)dist = 2/theApp->GetPixelScale();
			if(dist * theApp->GetPixelScale()>10)continue;
			bool use_this = false;
			if(best_digitized_point == NULL)use_this = true;
			else if(this_digitized_point->importance() > best_digitized_point->importance())use_this = true;
			else if(this_digitized_point->importance() == best_digitized_point->importance() && dist<min_dist)use_this = true;
			if(use_this){
				min_dist = dist;
				best_digitized_point = this_digitized_point;
			}
		}
	}
	if(theApp->digitize_centre && (min_dist == -1 || min_dist * theApp->GetPixelScale()>5)){
		Point3d pos;
		for (std::list<HeeksObj*>::iterator It = objects.begin(); It != objects.end(); It++)
		{
			HeeksObj* object = *It;
			Point3d p, p2;
			int num = object->GetCentrePoints(p, p2);
			if(num == 1)
			{
				compare_list.push_back(DigitizedPoint(p, DigitizeCentreType));
			}
			else if(num == 2)
			{
				double t;
				double dist1 = ray.Near(p, t).Dist(p);
				double dist2 = ray.Near(p2, t).Dist(p2);
				compare_list.push_back(DigitizedPoint(Point3d((dist1 < dist2) ? p : p2), DigitizeCentreType));
			}
			else
				continue;

			best_digitized_point = &(compare_list.back());
			break;
		}
	}
	DigitizedPoint point;
	if(best_digitized_point){
		point = *best_digitized_point;
	}
	else if(theApp->digitize_coords){
		point = Digitize(ray);
	}
	
	return point;
}

DigitizedPoint DigitizeMode::Digitize(const Line &ray){
	Plane pl(Point3d(0, 0, 0), Point3d(0, 0, 1));
	pl.Transform(digitizing_matrix(true));
	Point3d pnt;
	double t;
	if(!pl.Intof(ray, pnt, t)){
		pl = Plane(Point3d(0, 0, 0), Point3d(0, -1, 0));
		if(!pl.Intof(ray, pnt, t))DigitizedPoint();

		pl = Plane(Point3d(0, 0, 0), Point3d(1, 0, 0));
		if(!pl.Intof(ray, pnt, t))DigitizedPoint();
	}

	DigitizedPoint point(pnt, DigitizeCoordsType);

	if(theApp->draw_to_grid){
		Point3d plane_vx = Point3d(1, 0, 0).Transformed(digitizing_matrix());
		Point3d plane_vy = Point3d(0, 1, 0).Transformed(digitizing_matrix());
		Point3d datum = Point3d(0, 0, 0).Transformed(digitizing_matrix());

		double a = Point3d(datum) * plane_vx;
		double b = Point3d(point.m_point) * plane_vx;
		double c = b - a;
		double extra1 = c > -0.00000001 ? 0.5:-0.5;
		c = (int)(c / theApp->digitizing_grid + extra1) * theApp->digitizing_grid;

		double datum_dotp_y = Point3d(datum) * plane_vy;
		double rp_dotp_y = Point3d(point.m_point) * plane_vy;
		double d = rp_dotp_y - datum_dotp_y;
		double extra2 = d > -0.00000001 ? 0.5:-0.5;
		d = (int)(d / theApp->digitizing_grid + extra2) * theApp->digitizing_grid;

		point.m_point = datum + plane_vx * c + plane_vy * d;
	}

	return point;
}

DigitizedPoint DigitizeMode::digitize(const IPoint &point){
	digitized_point = digitize1(point);
	return digitized_point;
}

void DigitizeMode::OnFrontRender(){
	point_or_window->OnFrontRender();
}

void DigitizeMode::GetProperties(std::list<Property *> *list){
	PropertyPnt(list, NULL, &digitized_point.m_point);
}
