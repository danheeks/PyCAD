// Drawing.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "Drawing.h"
#include "HeeksObj.h"
#include "HeeksColor.h"
#include "Property.h"
#include "LineArcDrawing.h"
#include "SelectMode.h"
#include "DigitizeMode.h"
#include "Viewport.h"
#include "KeyCode.h"

Drawing::Drawing(void): m_getting_position(false), m_inhibit_coordinate_change(false){
	null_view = new ViewSpecific(0);
	SetView(0);
}

Drawing::~Drawing(void){
	delete null_view;

	std::map<int, ViewSpecific*>::iterator It;
	for(It = view_map.begin(); It != view_map.end(); It++){
		ViewSpecific *view = It->second;
		delete view;
	}

	ClearObjectsMade();
}

HeeksObj* Drawing::TempObject()
{
	if(m_temp_object_in_list.size() == 0)return NULL;
	return m_temp_object_in_list.back();
}

void Drawing::AddToTempObjects(HeeksObj* object)
{
	m_temp_object_in_list.push_back(object);
}

void Drawing::AddObjectsMade()
{
	theApp->AddUndoably(m_temp_object_in_list,((ObjList*)GetOwnerForDrawingObjects()));
	if(DragDoneWithXOR())theApp->m_current_viewport->DrawObjectsOnFront(m_temp_object_in_list, true);
	m_temp_object_in_list.clear();
}

void Drawing::ClearObjectsMade()
{
	for(std::list<HeeksObj*>::iterator It = m_temp_object_in_list.begin(); It != m_temp_object_in_list.end(); It++)
	{
		HeeksObj* object = *It;
		if (!object->NeverDelete())delete object;
	}
	m_temp_object_in_list.clear();
}

void Drawing::ClearPrevObject()
{
	m_prev_object = NULL;
}

void Drawing::RecalculateAndRedraw(const IPoint& point)
{
	set_digitize_plane();

	DigitizedPoint end = theApp->m_digitizing->digitize(point);
	if(end.m_type == DigitizeNoItemType)return;

	if(is_a_draw_level(GetDrawStep()))
	{
		if(DragDoneWithXOR())theApp->m_current_viewport->EndDrawFront();
		calculate_item(end);
		if(DragDoneWithXOR())theApp->m_current_viewport->DrawFront();
		else theApp->Repaint(true);
	}
}

void Drawing::AddPoint()
{
	// kill focus on control being typed into
//	theApp->m_frame->m_input_canvas->DeselectProperties();
//	theApp->ProcessPendingEvents();

	if(theApp->m_digitizing->digitized_point.m_type == DigitizeNoItemType)return;

	bool calculated = false;
	theApp->StartHistory();
	if(is_an_add_level(GetDrawStep())){
		calculated = calculate_item(theApp->m_digitizing->digitized_point);
		if(calculated){
			before_add_item();
			m_prev_object = TempObject();
			AddObjectsMade();
			set_previous_direction();
		}
	}
	
	ClearObjectsMade();
	SetStartPosUndoable(theApp->m_digitizing->digitized_point);
	theApp->m_digitizing->reference_point = theApp->m_digitizing->digitized_point;

	int next_step = GetDrawStep() + 1;
	if(next_step >= number_of_steps()){
		next_step = step_to_go_to_after_last_step();
	}
	SetDrawStepUndoable(next_step);
	theApp->EndHistory();
	m_getting_position = false;
	m_inhibit_coordinate_change = false;
	theApp->OnInputModeTitleChanged();
}

void Drawing::OnMouse( MouseEvent& event )
{
	bool event_used = false;

	if(LeftAndRightPressed(event, event_used))
	{
		if(DragDoneWithXOR())theApp->m_current_viewport->EndDrawFront();
		ClearObjectsMade();
		theApp->RestoreInputMode();
	}

	if(!event_used){
		if(event.m_middleDown || event.GetWheelRotation() != 0)
		{
//			theApp->m_select_mode->OnMouse(event);
		}
		else{
			if(event.LeftDown()){
				if(!m_inhibit_coordinate_change)
				{
					button_down_point = theApp->m_digitizing->digitize(IPoint(event.GetX(), event.GetY()));
				}
			}
			else if(event.LeftUp()){
				if(m_inhibit_coordinate_change){
					m_inhibit_coordinate_change = false;
				}
				else{
					set_digitize_plane();
					theApp->m_digitizing->digitized_point = button_down_point;
					if(m_getting_position){
						m_inhibit_coordinate_change = true;
						m_getting_position = false;
					}
					else{
						AddPoint();
					}
				}
			}
			else if(event.RightUp()){
				// do context menu same as select mode
//				theApp->m_select_mode->OnMouse(event);
			}
			else if(event.Moving()){
				if(!m_inhibit_coordinate_change){
					RecalculateAndRedraw(IPoint(event.GetX(), event.GetY()));
					theApp->RefreshInputCanvas();
				}
			}
		}
	}
}

void Drawing::OnKeyDown(KeyCode key_code)
{
	switch (key_code){
	case K_F1:
	case K_RETURN:
	case K_ESCAPE:
		// end drawing mode
		ClearObjectsMade();
		theApp->RestoreInputMode();
		break;
	default:
		break;
	}
}

bool Drawing::IsDrawing(CInputMode* i){
	if(i == &line_strip)return true;
//	if(i == &arc_strip)return true;

	return false;
}

void Drawing::OnModeChange(void){
	view_map.clear();
	*null_view = ViewSpecific(0);
	current_view_stuff = null_view;

	if(!IsDrawing(theApp->input_mode_object))SetDrawStepUndoable(0);
}

HeeksObj* Drawing::GetOwnerForDrawingObjects()
{
	return theApp; //Object always needs to be added somewhere
}

void Drawing::SetView(int v){
	if(v == 0){
		current_view_stuff = null_view;
		return;
	}
	if(v == GetView())return;

	std::map<int, ViewSpecific*>::iterator FindIt;
	FindIt = view_map.find(v);
	if(FindIt == view_map.end()){
		current_view_stuff = new ViewSpecific(v);
		view_map.insert(std::pair<int, ViewSpecific*>(v, current_view_stuff));
	}
	else{
		current_view_stuff = FindIt->second;
	}
}

int Drawing::GetView(){
	return current_view_stuff->view;
}

class SetDrawingDrawStep:public Undoable{
private:
	Drawing *drawing;
	int old_step;
	int step;

public:
	SetDrawingDrawStep(Drawing *d, int s){drawing = d; old_step = drawing->GetDrawStep(); step = s;}

	// Tool's virtual functions
	const wchar_t* GetTitle(){return L"set_draw_step";}
	void Run(bool redo){drawing->set_draw_step_not_undoable(step);}
	void RollBack(){drawing->set_draw_step_not_undoable(old_step);}
};

class SetDrawingPosition:public Undoable{
private:
	Drawing *drawing;
	DigitizedPoint prev_pos;
	DigitizedPoint next_pos;

public:
	SetDrawingPosition(Drawing *d, const DigitizedPoint &pos){
		drawing = d;
		prev_pos = d->GetStartPos();
		next_pos = pos;
	}

	// Tool's virtual functions
	const wchar_t* GetTitle(){return L"set_position";}
	void Run(bool redo){drawing->set_start_pos_not_undoable(next_pos);}
	void RollBack(){drawing->set_start_pos_not_undoable(prev_pos);}
};

void Drawing::SetDrawStepUndoable(int s){
	theApp->DoUndoable(new SetDrawingDrawStep(this, s));
}

void Drawing::SetStartPosUndoable(const DigitizedPoint& pos){
	theApp->DoUndoable(new SetDrawingPosition(this, pos));
}

void Drawing::OnFrontRender(){
	if(DragDoneWithXOR() && GetDrawStep()){
		for(std::list<HeeksObj*>::iterator It = m_temp_object_in_list.begin(); It != m_temp_object_in_list.end(); It++){
			HeeksObj *object = *It;
			object->glCommands(false, false, true);
		}
	}

	theApp->m_digitizing->OnFrontRender();
}

void Drawing::OnRender(){
	if(!DragDoneWithXOR() && GetDrawStep()){
		for(std::list<HeeksObj*>::iterator It = m_temp_object_in_list.begin(); It != m_temp_object_in_list.end(); It++){
			HeeksObj *object = *It;
			object->glCommands(false, false, false);
		}
	}
}

void Drawing::GetProperties(std::list<Property *> *list){
	theApp->m_digitizing->GetProperties(list); // x, y, z
}
