// MarkedList.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "MarkedList.h"
#include "HeeksObj.h"
#include "DigitizeMode.h"
#include "SelectMode.h"
#include "PointOrWindow.h"
#include "GripperSelTransform.h"
#include "Picking.h"
#include "CadApp.h"
#include "Viewport.h"
using namespace std;

MarkedList::MarkedList(){
	gripping = false;
	point_or_window = new PointOrWindow(true);
	gripper_marked_list_changed = false;
	ignore_coords_only = false;
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
			if(gripper == theApp->cursor_gripper)theApp->cursor_gripper = NULL;
			if(gripper == theApp->drag_gripper)theApp->drag_gripper = NULL;
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

bool MarkedList::ObjectMarked(HeeksObj *object){
	return m_set.find(object) != m_set.end();
}

void MarkedList::OnChanged(bool selection_cleared, const std::list<HeeksObj *>* added, const std::list<HeeksObj *>* removed){
	gripper_marked_list_changed = true;
	theApp->ObserversMarkedListChanged(selection_cleared, added, removed);
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
	theApp->Remove(m_list);
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

	theApp->SaveXMLFile(m_list, temp_file.GetFullPath().c_str(), true);

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
