// Drawing.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "InputMode.h"
#include "LeftAndRight.h"
#include "DigitizedPoint.h"

#include <map>

class HeeksObj;

class ViewSpecific{
public:
	int view;
	int draw_step;
	DigitizedPoint before_before_start_pos; // for four items, like cubic spline
	DigitizedPoint before_start_pos; // for three items, like circle from three points
	DigitizedPoint start_pos;

	ViewSpecific(int v){
		view = v;
		draw_step = 0;
	}
};

class Drawing: public CInputMode, CLeftAndRight{
private:
	std::list<HeeksObj*> m_temp_object_in_list;
	HeeksObj* m_prev_object;

protected:
	std::map<int, ViewSpecific*> view_map;
	ViewSpecific *current_view_stuff;
	ViewSpecific *null_view;
	DigitizedPoint button_down_point;

	// Drawing's virtual functions
	virtual void set_digitize_plane(){}
	virtual bool calculate_item(DigitizedPoint &end){return false;}
	virtual void before_add_item(){}
	virtual void set_previous_direction(){}
	virtual int number_of_steps(){return 2;}
	virtual int step_to_go_to_after_last_step(){return 0;}
	virtual bool is_an_add_level(int level){return false;}
	virtual bool is_a_draw_level(int level){return is_an_add_level(level);}
	virtual HeeksObj* GetOwnerForDrawingObjects();

	void SetView(int);
	int GetView();
	void RecalculateAndRedraw(const IPoint& point);
	HeeksObj* TempObject();
	HeeksObj* PrevObject(){return m_prev_object;}
	void AddToTempObjects(HeeksObj* object);
	void AddObjectsMade();
	void ClearPrevObject();

public:
	bool m_getting_position;
	bool m_inhibit_coordinate_change; // so that user can type into properties

	Drawing(void);
	virtual ~Drawing(void);

	// InputMode's virtual functions
	void OnMouse( MouseEvent& event );
	void OnKeyDown(KeyCode key_code);
	void OnModeChange(void);
	void GetProperties(std::list<Property *> *list);
	void OnFrontRender();
	void OnRender();
	const wchar_t* GetTitle(){return L"Drawing";}
	bool IsDrawing(){ return true; }

	// Drawing's virtual functions
	virtual void AddPoint();
	//virtual void clear_drawing_objects(int mode = 0){} // 0 - set temporary objects to NULL,  1 - store the temporary objects as previous_list, 2 - delete and set to NULL
	virtual bool DragDoneWithXOR(){return true;}
	virtual void set_draw_step_not_undoable(int s){current_view_stuff->draw_step = s;}

	void ClearObjectsMade();
	int GetDrawStep()const{return current_view_stuff->draw_step;}
	void SetDrawStepUndoable(int s);
	const DigitizedPoint& GetStartPos()const{return current_view_stuff->start_pos;}
	const DigitizedPoint& GetBeforeStartPos()const{return current_view_stuff->before_start_pos;}
	const DigitizedPoint& GetBeforeBeforeStartPos()const{return current_view_stuff->before_before_start_pos;}
	void SetStartPosUndoable(const DigitizedPoint& pos);

	void set_start_pos_not_undoable(const DigitizedPoint& pos){current_view_stuff->before_before_start_pos = current_view_stuff->before_start_pos; current_view_stuff->before_start_pos = current_view_stuff->start_pos; current_view_stuff->start_pos = pos;}

};

