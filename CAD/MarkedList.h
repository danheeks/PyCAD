// MarkedList.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "ViewPoint.h"

class Gripper;
#include <set>

class MarkedList{
private:
	std::list<HeeksObj*> m_list;
	std::set<HeeksObj*> m_set;
	std::set<HeeksObj*> m_ignore_set;

	void delete_move_grips(bool check_app_grippers = false);
	void create_move_grips();
	void update_move_grips();
	void render_move_grips(bool select, bool no_color);
	void OnChangedAdded(HeeksObj* object);
	void OnChangedRemoved(HeeksObj* object);

public:
	bool gripping;
	std::list<Gripper*> move_grips;
	bool gripper_marked_list_changed;
	bool ignore_coords_only;

	MarkedList();
	virtual ~MarkedList(void);

	void create_grippers();
	void Add(std::list<HeeksObj *> &obj_list, bool call_OnChanged);
	void Add(HeeksObj *object, bool call_OnChanged);
	void Remove(const std::list<HeeksObj *> &obj_list, bool call_OnChanged);
	void Remove(HeeksObj *object, bool call_OnChanged);
	bool ObjectMarked(HeeksObj *object);
	void Clear(bool call_OnChanged);
	unsigned int size(void){return m_list.size();}
	std::list<HeeksObj *> &list(void){return m_list;}
	void GrippersGLCommands(bool select, bool no_color);
	void OnChanged(bool selection_cleared, const std::list<HeeksObj *>* added, const std::list<HeeksObj *>* removed);
	void set_ignore_onoff(HeeksObj* object, bool b);
	bool get_ignore(HeeksObj* object);
	void GetProperties(std::list<Property *> *list);
	void CutSelectedItems();
	void CopySelectedItems();
	void Reset();
};
