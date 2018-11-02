// DigitizeMode.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "DigitizeMode.h"
#include "MarkedObject.h"
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
	m_doing_a_main_loop = false;
	m_callback = NULL;
}

DigitizeMode::~DigitizeMode(void){
	delete point_or_window;
}

static std::wstring digitize_title_coords_string;

const wchar_t* DigitizeMode::GetTitle()
{
	if(m_doing_a_main_loop)
	{
		digitize_title_coords_string = m_prompt_when_doing_a_main_loop;
		std::wstring xy_str;
		digitize_title_coords_string.append(L"\n");
#if 0
		// to do
		if(fabs(digitized_point.m_point.z) < 0.00000001)digitize_title_coords_string += std::wstring::Format(_T("X%g Y%g"), digitized_point.m_point.x, digitized_point.m_point.y);
		else if(fabs(digitized_point.m_point.y) < 0.00000001)digitize_title_coords_string += std::wstring::Format(_T("X%g Z%g"), digitized_point.m_point.x, digitized_point.m_point.z);
		else if(fabs(digitized_point.m_point.x) < 0.00000001)digitize_title_coords_string += std::wstring::Format(_T("Y%g Z%g"), digitized_point.m_point.y, digitized_point.m_point.z);
		else digitize_title_coords_string += std::wstring::Format(_T("X%g Y%g Z%g"), digitized_point.m_point.x, digitized_point.m_point.y, digitized_point.m_point.z);
#endif

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
	if(!m_doing_a_main_loop)return NULL;
	digitize_help_text.assign(L"Press Esc key to cancel");
	digitize_help_text.append(L"\n");
	digitize_help_text.append(L"Left button to accept position");
	return digitize_help_text.c_str();
}

void DigitizeMode::OnMouse( MouseEvent& event ){
	if(event.m_middleDown || event.GetWheelRotation() != 0)
	{
		theApp.m_select_mode->OnMouse(event);
		return;
	}

	if(event.LeftDown()){
		point_or_window->OnMouse(event);
		lbutton_point = digitize(IPoint(event.GetX(), event.GetY()));
	}
	else if(event.LeftUp()){
		if(lbutton_point.m_type != DigitizeNoItemType){
			digitized_point = lbutton_point;
			if(m_doing_a_main_loop){
				//ExitMainLoop();
			}
		}
	}
	else if(event.Moving()){
		digitize(IPoint(event.GetX(), event.GetY()));
		point_or_window->OnMouse(event);
		if(m_doing_a_main_loop)
		{
//			theApp.m_frame->RefreshInputCanvas();
			theApp.OnInputModeTitleChanged();
		}
		if(m_callback)
		{
			double pos[3];
			digitized_point.m_point.get(pos);
			(*m_callback)(pos);
		}
	}
}

#if 0
void DigitizeMode::OnKeyDown(wxKeyEvent& event)
{
	switch(event.GetKeyCode())
	{
	case WXK_ESCAPE:
		digitized_point.m_type = DigitizeNoItemType;
		if(m_doing_a_main_loop)ExitMainLoop();
		break;
	}
}
#endif

static geoff_geometry::Matrix global_matrix_relative_to_screen;

static const geoff_geometry::Matrix& digitizing_matrix(bool calculate = false){
	if(calculate){
		if(theApp.digitize_screen){
			geoff_geometry::Matrix mat = theApp.GetDrawMatrix(false);
			geoff_geometry::Point3d origin = geoff_geometry::Point3d(0, 0, 0).Transformed(mat);
			geoff_geometry::Point3d x1 = origin + geoff_geometry::Point3d(1, 0, 0);
			geoff_geometry::Point3d y1 = origin + geoff_geometry::Point3d(0, 1, 0);
			geoff_geometry::Point3d po = origin;
			po = theApp.m_current_viewport->m_view_point.glUnproject(po);
			x1 = theApp.m_current_viewport->m_view_point.glUnproject(x1);
			y1 = theApp.m_current_viewport->m_view_point.glUnproject(y1);
			
			global_matrix_relative_to_screen = geoff_geometry::Matrix(origin, geoff_geometry::Point3d(po, x1).Normalized(), geoff_geometry::Point3d(po, y1).Normalized());
		}
		else{
			global_matrix_relative_to_screen = theApp.GetDrawMatrix(true);
		}
	}
	return global_matrix_relative_to_screen;
}

bool DigitizeMode::OnModeChange(void){
	point_or_window->reset();
	if(!point_or_window->OnModeChange())return false;
	digitize(theApp.cur_mouse_pos);
	return true;
}

bool make_point_from_doubles(const std::list<double> &dlist, std::list<double>::const_iterator &It, geoff_geometry::Point3d& pnt, bool four_doubles = false)
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

int convert_doubles_to_pnts(const std::list<double> &dlist, std::list<geoff_geometry::Point3d> &plist, bool four_doubles = false)
{
	int nump = 0;
	for (std::list<double>::const_iterator It = dlist.begin(); It != dlist.end();)
	{
		geoff_geometry::Point3d pnt;
		if (!make_point_from_doubles(dlist, It, pnt, four_doubles))break;
		plist.push_back(pnt);
		nump++;
	}
	return nump;
}

int convert_gripdata_to_pnts(const std::list<GripData> &dlist, std::list<geoff_geometry::Point3d> &plist)
{
	int nump = 0;
	for (std::list<GripData>::const_iterator It = dlist.begin(); It != dlist.end(); ++It)
	{
		geoff_geometry::Point3d pnt;
		pnt.x = ((*It).m_x);
		pnt.y = ((*It).m_y);
		pnt.z = ((*It).m_z);
		plist.push_back(pnt);
		nump++;
	}
	return nump;
}


DigitizedPoint DigitizeMode::digitize1(const IPoint &input_point){
	geoff_geometry::Line ray = theApp.m_current_viewport->m_view_point.SightLine(input_point);
	std::list<DigitizedPoint> compare_list;
	MarkedObjectManyOfSame marked_object;
	if(theApp.digitize_end || theApp.digitize_inters || theApp.digitize_centre || theApp.digitize_midpoint || theApp.digitize_nearest || theApp.digitize_tangent){
		point_or_window->SetWithPoint(input_point);
		theApp.m_marked_list->ignore_coords_only = true;
		theApp.m_marked_list->ObjectsInWindow(point_or_window->box_chosen, &marked_object);
		theApp.m_marked_list->ignore_coords_only = false;
	}
	if(theApp.digitize_end){
		if(marked_object.m_map.size()>0){
			HeeksObj* object = marked_object.GetFirstOfBottomOnly();
			while(object){
				std::list<GripData> vl;
				object->GetGripperPositionsTransformed(&vl, true);
				std::list<geoff_geometry::Point3d> plist;
				convert_gripdata_to_pnts(vl, plist);
				for(std::list<geoff_geometry::Point3d>::iterator It = plist.begin(); It != plist.end(); It++)
				{
					geoff_geometry::Point3d& pnt = *It;
					compare_list.push_back(DigitizedPoint(pnt, DigitizeEndofType));
				}
				object = marked_object.Increment();
			}
		}
	}
	if(theApp.digitize_inters){
		if(marked_object.m_map.size()>0){
			std::list<HeeksObj*> object_list;
			HeeksObj* object = marked_object.GetFirstOfBottomOnly();
			while(object){
				object_list.push_back(object);
				object = marked_object.Increment();
			}

			if(object_list.size() > 1)
			{
				for(std::list<HeeksObj*>::iterator It = object_list.begin(); It != object_list.end(); It++)
				{
					HeeksObj* object = *It;
					std::list<HeeksObj*>::iterator It2 = It;
					It2++;
					for(; It2 != object_list.end(); It2++)
					{
						HeeksObj* object2 = *It2;
						std::list<double> rl;
						if(object->Intersects(object2, &rl))
						{
							std::list<geoff_geometry::Point3d> plist;
							convert_doubles_to_pnts(rl, plist);
							for(std::list<geoff_geometry::Point3d>::iterator It = plist.begin(); It != plist.end(); It++)
							{
								geoff_geometry::Point3d& pnt = *It;
								compare_list.push_back(DigitizedPoint(pnt, DigitizeIntersType));
							}
						}
					}
				}
			}
		}
	}
	if(theApp.digitize_midpoint){
		if(marked_object.m_map.size()>0){
			HeeksObj* object = marked_object.GetFirstOfBottomOnly();
			while(object){
				geoff_geometry::Point3d p;
				if(object->GetMidPoint(p)){
					compare_list.push_back(DigitizedPoint(p, DigitizeMidpointType));
				}
				object = marked_object.Increment();
			}
		}
	}
	if(theApp.digitize_nearest){
		if(marked_object.m_map.size()>0){
			HeeksObj* object = marked_object.GetFirstOfEverything();
			while(object){
				geoff_geometry::Line ray;
				geoff_geometry::Point3d p;
				if(object->FindNearPoint(ray, p)){
					compare_list.push_back(DigitizedPoint(p, DigitizeNearestType));
				}
				object = marked_object.Increment();
			}
		}
	}
	if(theApp.digitize_tangent){
		if(marked_object.m_map.size()>0){
			HeeksObj* object = marked_object.GetFirstOfEverything();
			while(object){
				geoff_geometry::Line ray;
				geoff_geometry::Point3d p;
				if (object->FindPossTangentPoint(ray, p)){
					compare_list.push_back(DigitizedPoint(geoff_geometry::Point3d(p), DigitizeTangentType, object));
				}
				object = marked_object.Increment();
			}
		}
	}
	double min_dist = -1;
	double best_dp = 0;
	DigitizedPoint *best_digitized_point = NULL;
	if(compare_list.size() >0){
		std::list<DigitizedPoint>::iterator It;
		double dist;
		double dp;
		for(It = compare_list.begin(); It != compare_list.end(); It++){
			DigitizedPoint *this_digitized_point = &(*It);
			double t;
			dist = ray.Near(this_digitized_point->m_point, t).Dist(this_digitized_point->m_point);
			dp = ray.v * this_digitized_point->m_point - ray.v * ray.p0;
			if(dist * theApp.GetPixelScale() < 2)dist = 2/theApp.GetPixelScale();
			if(dist * theApp.GetPixelScale()>10)continue;
			bool use_this = false;
			if(best_digitized_point == NULL)use_this = true;
			else if(this_digitized_point->importance() > best_digitized_point->importance())use_this = true;
			else if(this_digitized_point->importance() == best_digitized_point->importance() && dist<min_dist)use_this = true;
			if(use_this){
				min_dist = dist;
				best_digitized_point = this_digitized_point;
				best_dp = dp;
			}
		}
	}
	if(theApp.digitize_centre && (min_dist == -1 || min_dist * theApp.GetPixelScale()>5)){
		geoff_geometry::Point3d pos;
		for(HeeksObj* object = marked_object.GetFirstOfEverything(); object != NULL; object = marked_object.Increment()){
			geoff_geometry::Point3d p, p2;
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
				compare_list.push_back(DigitizedPoint(geoff_geometry::Point3d((dist1 < dist2) ? p : p2), DigitizeCentreType));
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
	else if(theApp.digitize_coords){
		point = Digitize(ray);
	}
	
	return point;
}

DigitizedPoint DigitizeMode::Digitize(const geoff_geometry::Line &ray){
	geoff_geometry::Plane pl(geoff_geometry::Point3d(0, 0, 0), geoff_geometry::Point3d(0, 0, 1));
	pl.Transform(digitizing_matrix(true));
	geoff_geometry::Point3d pnt;
	double t;
	if(!pl.Intof(ray, pnt, t)){
		pl = geoff_geometry::Plane(geoff_geometry::Point3d(0, 0, 0), geoff_geometry::Point3d(0, -1, 0));
		if(!pl.Intof(ray, pnt, t))DigitizedPoint();

		pl = geoff_geometry::Plane(geoff_geometry::Point3d(0, 0, 0), geoff_geometry::Point3d(1, 0, 0));
		if(!pl.Intof(ray, pnt, t))DigitizedPoint();
	}

	DigitizedPoint point(pnt, DigitizeCoordsType);

	if(theApp.draw_to_grid){
		geoff_geometry::Point3d plane_vx = geoff_geometry::Point3d(1, 0, 0).Transformed(digitizing_matrix());
		geoff_geometry::Point3d plane_vy = geoff_geometry::Point3d(0, 1, 0).Transformed(digitizing_matrix());
		geoff_geometry::Point3d datum = geoff_geometry::Point3d(0, 0, 0).Transformed(digitizing_matrix());

		double a = geoff_geometry::Point3d(datum) * plane_vx;
		double b = geoff_geometry::Point3d(point.m_point) * plane_vx;
		double c = b - a;
		double extra1 = c > -0.00000001 ? 0.5:-0.5;
		c = (int)(c / theApp.digitizing_grid + extra1) * theApp.digitizing_grid;

		double datum_dotp_y = geoff_geometry::Point3d(datum) * plane_vy;
		double rp_dotp_y = geoff_geometry::Point3d(point.m_point) * plane_vy;
		double d = rp_dotp_y - datum_dotp_y;
		double extra2 = d > -0.00000001 ? 0.5:-0.5;
		d = (int)(d / theApp.digitizing_grid + extra2) * theApp.digitizing_grid;

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

void DigitizeMode::SetOnlyCoords(HeeksObj* object, bool onoff){
	if(onoff)m_only_coords_set.insert(object);
	else m_only_coords_set.erase(object);
}

bool DigitizeMode::OnlyCoords(HeeksObj* object){
	if(m_only_coords_set.find(object) != m_only_coords_set.end())return true;
	return false;
}
