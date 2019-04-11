// MarkedList.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "MarkedList.h"
#include "HeeksObj.h"
#include "MarkedObject.h"
#include "DigitizeMode.h"
#include "SelectMode.h"
#include "PointOrWindow.h"
#include "GripperSelTransform.h"
#include "Picking.h"
#include "App.h"
#include "Viewport.h"
using namespace std;

MarkedList::MarkedList(){
	gripping = false;
	point_or_window = new PointOrWindow(true);
	gripper_marked_list_changed = false;
	ignore_coords_only = false;
	m_filter = -1;
}

MarkedList::~MarkedList(void){
	delete point_or_window;
	std::list<Gripper*>::iterator It;
	for(It = move_grips.begin(); It != move_grips.end(); It++){
		Gripper* gripper = *It;
		gripper->m_index = 0;
	}
	delete_move_grips(false);
}

void MarkedList::delete_move_grips(bool check_app_grippers){
	std::list<Gripper*>::iterator It;
	for(It = move_grips.begin(); It != move_grips.end(); It++){
		Gripper* gripper = *It;
		if(check_app_grippers){
			if(gripper == theApp.cursor_gripper)theApp.cursor_gripper = NULL;
			if(gripper == theApp.drag_gripper)theApp.drag_gripper = NULL;
		}
		delete gripper;
	}
	move_grips.clear();
}

void MarkedList::create_move_grips(){
	delete_move_grips(true);
	int number_of_grips_made = 0;
	std::list<HeeksObj*>::iterator Iter ;
	for(Iter = m_list.begin(); Iter != m_list.end() && number_of_grips_made<100; Iter++){
		HeeksObj* object = *Iter;
		if(object->GetType() == GripperType)continue;
		std::list<GripData> vl;
		std::list<GripData>::iterator It;
		object->GetGripperPositionsTransformed(&vl, false);
		for(It = vl.begin(); It != vl.end() && number_of_grips_made<100; It++)
		{
			move_grips.push_back(new GripperSelTransform(*It, object));
			number_of_grips_made++;
		}
	}
}

void MarkedList::update_move_grips(){
	if(gripping)return;
	std::list<HeeksObj*>::iterator Iter ;
	std::list<Gripper*>::iterator Iter2;
	Iter2 = move_grips.begin();
	for(Iter = m_list.begin(); Iter != m_list.end(); Iter++){
		if(Iter2 == move_grips.end())break;
		HeeksObj* object = *Iter;
		if(object->GetType() == GripperType)continue;
		std::list<GripData> vl;
		std::list<GripData>::iterator It;
		object->GetGripperPositionsTransformed(&vl, false);
		for(It = vl.begin(); It != vl.end(); It++){
			Gripper* gripper = *Iter2;
			gripper->m_data = *It;
			Iter2++;
			if(Iter2 == move_grips.end())break;
		}
	}
}

void MarkedList::render_move_grips(bool select, bool no_color){
	std::list<Gripper*>::iterator It;
	for(It = move_grips.begin(); It != move_grips.end(); It++){
		if (select)SetPickingColor((*It)->GetIndex());
		(*It)->glCommands(select, false, no_color);
	}
}

void MarkedList::create_grippers(){
	if(gripping)return;
	if(gripper_marked_list_changed)create_move_grips();
	else update_move_grips();
	gripper_marked_list_changed = false;
}

void MarkedList::GrippersGLCommands(bool select, bool no_color){
	if(size()>0){
		create_grippers();
		render_move_grips(select, no_color);
	}
}

void MarkedList::ObjectsInWindow( IRect window, MarkedObject* marked_object, bool single_picking){
	// render everything with unique colors

	//theApp.m_current_viewport->SetCurrent();

	glDrawBuffer(GL_BACK);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);

	theApp.m_current_viewport->SetViewport();
	theApp.m_current_viewport->m_view_point.SetProjection(true);
	theApp.m_current_viewport->m_view_point.SetModelview();

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glLineWidth(1);
	glDepthMask(1);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glShadeModel(GL_FLAT);
	theApp.m_current_viewport->m_view_point.SetPolygonOffset();

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	theApp.glCommands(true, false, true);

	GrippersGLCommands(true, true);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_COLOR_MATERIAL);

	if (window.width < 0)
	{
		window.x += window.width;
		window.width = abs(window.width);
	}
	if (window.height < 0)
	{
		window.y += window.height;
		window.height = abs(window.height);
	}

	unsigned int pixel_size = 4 * window.width * window.height;
	unsigned char* pixels = (unsigned char*)malloc(pixel_size);
	memset((void*)pixels, 0, pixel_size);
	glReadPixels(window.x, window.y, window.width, window.height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	int half_window_width = 0;
	IPoint window_centre;
	if (single_picking){
		half_window_width = (window.width) / 2;
		window_centre.x = window.x + window.width / 2;
		window_centre.y = window.y + window.height / 2;
	}
	int window_mode = 0;
	while (1){
		if (single_picking){
			int window_size = half_window_width-1;
			if (window_mode == 0)window_size = 0;
			if (window_mode == 1)window_size = half_window_width / 2;
			window.x = window_centre.x - window_size;
			window.y = window_centre.y - window_size;
			window.width = 1 + window_size * 2;
			window.height = 1 + window_size * 2;
		}

		std::set<unsigned int> color_list;

		for (unsigned int i = 0; i < pixel_size; i += 4)
		{
			unsigned int color = pixels[i] | (pixels[i + 1] << 8) | (pixels[i + 2] << 16);
			if (color != 0)
				color_list.insert(color);
		}

		bool added = false;
		for (std::set<unsigned int>::iterator It = color_list.begin(); It != color_list.end(); It++)
		{
			unsigned int name = *It;
			MarkedObject* current_found_object = marked_object;
			bool ignore_coords_only_found = false;
				HeeksObj *object = m_name_index.find(name);

				std::list<HeeksObj*> owner_list;
				while (object && (object != &(theApp)))
				{
					owner_list.push_front(object);
					object = object->m_owner;
				}

				for (std::list<HeeksObj*>::iterator It = owner_list.begin(); It != owner_list.end(); It++)
				{
					object = *It;

					//bool custom_names = object->UsesCustomSubNames();
					if (!ignore_coords_only_found && current_found_object != NULL){
						if (ignore_coords_only && theApp.m_digitizing->OnlyCoords(object)){
							ignore_coords_only_found = true;
						}
						else{
							if ((object->GetType() == GripperType) || ((object->GetMarkingMask() & m_filter) && (object->GetMarkingMask() != 0))){
								int window_size = window.width;
								current_found_object = current_found_object->Add(object, 0, window_size, 0, NULL);
								added = true;
							}
						}
					}
				}
		//	}
		}
		window_mode++;
		if (!single_picking)break;
		if (window_mode > 2)break;
	}

	free(pixels);
}

void MarkedList::Add(std::list<HeeksObj *> &list, bool call_OnChanged){
	std::list<HeeksObj *>::iterator It;
	for(It = list.begin(); It != list.end(); It++){
		HeeksObj *object = *It;
		m_list.push_back(object);
		m_set.insert(object);
	}
	if(call_OnChanged)OnChanged(false, &list, NULL);
}

void MarkedList::Remove(HeeksObj *object, bool call_OnChanged){
	if (!object) return;
	std::list<HeeksObj *> list;
	list.push_back(object);
	Remove(list, call_OnChanged);
}

void MarkedList::Add(HeeksObj *object, bool call_OnChanged){
	if (!object) return;
	if (m_set.find(object) == m_set.end())
	{
		std::list<HeeksObj *> list;
		list.push_back(object);
		Add(list, call_OnChanged);
	}
}

void MarkedList::Remove(const std::list<HeeksObj *> &obj_list, bool call_OnChanged){
	std::list<HeeksObj *>::const_iterator It;
	for(It = obj_list.begin(); It != obj_list.end(); It++){
		HeeksObj *object = *It;
		if(m_set.find(object) != m_set.end()){
			m_list.remove(object);
		}
		m_set.erase(object);
	}
	if(call_OnChanged)OnChanged(false, NULL, &obj_list);
}

void MarkedList::Clear(bool call_OnChanged){
	m_list.clear();
	m_set.clear();
	if(call_OnChanged)OnChanged(true, NULL, NULL);
}

void MarkedList::FindMarkedObject(const IPoint &point, MarkedObject* marked_object){
	if(marked_object == NULL)return;
	point_or_window->SetWithPoint(point);
	ObjectsInWindow(point_or_window->box_chosen, marked_object);
}

bool MarkedList::ObjectMarked(HeeksObj *object){
	return m_set.find(object) != m_set.end();
}

void MarkedList::OnChanged(bool selection_cleared, const std::list<HeeksObj *>* added, const std::list<HeeksObj *>* removed){
	gripper_marked_list_changed = true;
	theApp.ObserversMarkedListChanged(selection_cleared, added, removed);
}

void MarkedList::OnChangedAdded(HeeksObj* object)
{
	std::list<HeeksObj *> added;
	added.push_back(object);
	OnChanged(false, &added, NULL);
}

void MarkedList::OnChangedRemoved(HeeksObj* object)
{
	std::list<HeeksObj *> removed;
	removed.push_back(object);
	OnChanged(false, NULL, &removed);
}

void MarkedList::set_ignore_onoff(HeeksObj* object, bool b){
	if(b)m_ignore_set.insert(object);
	else m_ignore_set.erase(object);
}

bool MarkedList::get_ignore(HeeksObj* object){
	if(m_ignore_set.find(object) != m_ignore_set.end())return true;
	return false;
}

void MarkedList::GetProperties(std::list<Property *> *list){
	if(m_list.size() == 1)
	{
		m_list.front()->GetProperties(list);
	}
	else
	{
		// multiple selection
		//list->push_back(new PropertyInt(_("Number of items selected"), m_list.size(), NULL));
		for(std::list<HeeksObj*>::iterator It = m_list.begin(); It != m_list.end(); It++)
		{
			HeeksObj* obj = *It;
			obj->GetProperties(list);
		}
	}
}

void MarkedList::CutSelectedItems()
{
	CopySelectedItems();
	theApp.Remove(m_list);
}

void MarkedList::CopySelectedItems()
{
#if 0
	to do
#if wxCHECK_VERSION(3, 0, 0)
	wxStandardPaths& sp = wxStandardPaths::Get();
#else
	wxStandardPaths sp;
#endif
	sp.GetTempDir();
	wxFileName temp_file(sp.GetTempDir().c_str(), _T("temp_Heeks_clipboard_file.xml"));

	theApp.SaveXMLFile(m_list, temp_file.GetFullPath().c_str(), true);

#if wxUSE_UNICODE
	wifstream ifs(Ttc(temp_file.GetFullPath().c_str()));
#else
	ifstream ifs(temp_file.GetFullPath());
#endif
	if(!ifs)return;

	std::wstring fstr;
	wchar_t str[1024];
	while(!(ifs.eof())){
		ifs.getline(str, 1022);
		fstr.append(str);
		fstr.append(_T("\r\n"));
		if(!ifs)break;
	}

	if (wxTheClipboard->Open())
	{
		// This data object is held by the clipboard,
		// so do not delete them in the app.
		wxTheClipboard->SetData( new wxTextDataObject(fstr));
		wxTheClipboard->Close();
	}
#endif
}

void MarkedList::Reset()
{
	delete_move_grips(true);
}

unsigned int MarkedList::GetIndex(HeeksObj *object) {
	return m_name_index.insert(object);
}

void MarkedList::ReleaseIndex(unsigned int index) {
	return m_name_index.erase(index);
}
