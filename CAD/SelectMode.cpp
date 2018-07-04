// SelectMode.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "SelectMode.h"
#include "InputMode.h"
#if 0
#include "Tool.h"
#include "MarkedObject.h"
#endif
#include "ViewPoint.h"
#if 0
#include "MagDragWindow.h"
#include "MarkedList.h"
#include "DigitizeMode.h"
#include "Gripper.h"
#include "GraphicsCanvas.h"
#include "HeeksFrame.h"
#include "GripperSelTransform.h"
#include "InputModeCanvas.h"
#endif
#include "Viewport.h"

CClickPoint::CClickPoint(const IPoint& point, unsigned long depth)
{
	m_point = point;
	m_depth = depth;
	m_valid = false;
}

bool CClickPoint::GetPos(double *pos)
{
	if(!m_valid)
	{
		theApp.m_current_viewport->SetViewport();
		theApp.m_current_viewport->m_view_point.SetProjection(true);
		theApp.m_current_viewport->m_view_point.SetModelview();

		geoff_geometry::Point3d screen_pos(m_point.x, theApp.m_current_viewport->GetViewportSize().GetHeight() - m_point.y, (double)m_depth/4294967295.0);
		geoff_geometry::Point3d world_pos = theApp.m_current_viewport->m_view_point.glUnproject(screen_pos);
		world_pos.get(m_pos);
		m_valid = true;
	}
	if(m_valid)
	{
		memcpy(pos, m_pos, 3*sizeof(double));
	}

	return m_valid;
}

CSelectMode::CSelectMode(){
	control_key_initially_pressed = false;
	window_box_exists = false;
	m_doing_a_main_loop = false;
	m_button_down = false;
	m_middle_button_down = false;
	m_just_one = false;
}

bool CSelectMode::GetLastClickPosition(double *pos)
{
	return m_last_click_point.GetPos(pos);
}

const wchar_t* CSelectMode::GetTitle()
{
//	return m_doing_a_main_loop ? (m_prompt_when_doing_a_main_loop.c_str()):(_("Select Mode").c_str());
	return L"Select Mode";
}

static std::wstring str_for_GetHelpText;

const wchar_t* CSelectMode::GetHelpText()
{
#if 0
	str_for_GetHelpText = std::wstring(_("Left button for selecting objects"))
		+ _T("\n") + _("( with Ctrl key for extra objects)")
		+ _T("\n") + _("( with Shift key for similar objects)")
		+ _T("\n") + _("Drag with left button to window select");

	if(theApp.m_dragging_moves_objects)str_for_GetHelpText.Append(std::wstring(_T("\n")) + _("or to move object if on an object"));
	str_for_GetHelpText.Append(std::wstring(_T("\n")) + _("Mouse wheel to zoom in and out"));

	if(theApp.ctrl_does_rotate){
		str_for_GetHelpText.Append(std::wstring(_T("\n")) + _("Middle button to pan view"));
		str_for_GetHelpText.Append(std::wstring(_T("\n")) + _("( with Ctrl key to rotate view )"));
	}
	else{
		str_for_GetHelpText.Append(std::wstring(_T("\n")) + _("Middle button to rotate view"));
		str_for_GetHelpText.Append(std::wstring(_T("\n")) + _("( with Ctrl key to pan view )"));
	}

	str_for_GetHelpText.Append(std::wstring(_T("\n")) + _("Right button for object menu"));
	str_for_GetHelpText.Append(std::wstring(_T("\n")) + _("See options window to hide this help"));
	str_for_GetHelpText.Append(std::wstring(_T("\n")) + _T("( ") + _("view options") + _T("->") + _("screen text") + _T(" )"));

	if(m_doing_a_main_loop)
	{
		str_for_GetHelpText.Append(std::wstring(_T("\n")) + _("Press Esc key to cancel"));
		if(theApp.m_marked_list->size() > 0)str_for_GetHelpText.Append(std::wstring(_T("\n")) + _("Press Return key to accept selection"));
	}
#endif
	return str_for_GetHelpText.c_str();
}

#if 0
static GripperSelTransform drag_object_gripper(GripData(GripperTypeTranslate, 0, 0, 0), NULL);
#endif

void CSelectMode::OnLeftDown( MouseEvent& event )
{
	button_down_point = IPoint(event.GetX(), event.GetY());
	CurrentPoint = button_down_point;
	m_button_down = true;
#if 0
	m_highlighted_objects.clear();

	if(theApp.m_dragging_moves_objects)
	{
		MarkedObjectManyOfSame marked_object;
		theApp.FindMarkedObject(button_down_point, &marked_object);
		if(marked_object.m_map.size()>0)
		{
			HeeksObj* object = marked_object.GetFirstOfTopOnly();

			if (event.ShiftDown())
			{
			} // End if - then

			while(object)
			{
				if(object->GetType() == GripperType)
				{
					theApp.m_current_viewport->DrawFront();
					theApp.drag_gripper = (Gripper*)object;
					theApp.m_digitizing->SetOnlyCoords(theApp.drag_gripper, true);
					theApp.m_digitizing->digitize(button_down_point);
					theApp.grip_from = theApp.m_digitizing->digitized_point.m_point;
					theApp.grip_to = theApp.grip_from;
					double from[3];
					from[0] = theApp.grip_from.x;
					from[1] = theApp.grip_from.y;
					from[2] = theApp.grip_from.z;

					std::list<HeeksObj*> selected_objects;
					for(std::list<HeeksObj*>::iterator It = theApp.m_marked_list->list().begin(); It != theApp.m_marked_list->list().end(); It++)
					{
						HeeksObj* object = *It;
						if(object->CanBeDragged())selected_objects.push_back(object);
					}

					theApp.drag_gripper->OnGripperGrabbed(selected_objects, true, from);
					theApp.grip_from = geoff_geometry::Point3d(from[0], from[1], from[2]);
					theApp.m_current_viewport->EndDrawFront();
					return;
				}
				object = marked_object.Increment();
			}
		}
	}
#endif
}

void CSelectMode::OnMiddleDown( MouseEvent& event )
{
	m_middle_button_down = true;
	button_down_point = IPoint(event.GetX(), event.GetY());
	CurrentPoint = button_down_point;
	theApp.m_current_viewport->StoreViewPoint();
	theApp.m_current_viewport->m_view_point.SetStartMousePoint(button_down_point);
}

#if 0
void CSelectMode::GetObjectsInWindow(MouseEvent& event, std::list<HeeksObj*> &objects)
{
	if(window_box.width > 0){
		// only select objects which are completely within the window
		MarkedObjectManyOfSame marked_object;
		theApp.m_marked_list->ObjectsInWindow(window_box, &marked_object, false);
		std::set<HeeksObj*> obj_set;
		for(HeeksObj* object = marked_object.GetFirstOfTopOnly(); object; object = marked_object.Increment())if(object->GetType() != GripperType)
			obj_set.insert(object);

		int bottom = window_box.y;
		int top = window_box.y + window_box.height;
		int height = abs(window_box.height);
		if(top < bottom)
		{
			int temp = bottom;
			bottom = top;
			top = temp;
		}

		IRect strip_boxes[4];
		// top
		strip_boxes[0] = IRect(window_box.x - 1, top, window_box.width + 2, 1);
		// bottom
		strip_boxes[1] = IRect(window_box.x - 1, bottom - 1, window_box.width + 2, 1);
		// left
		strip_boxes[2] = IRect(window_box.x - 1, bottom, 1, height);
		// right
		strip_boxes[3] = IRect(window_box.x + window_box.width, bottom, 1, height);

		for(int i = 0; i<4; i++)
		{
			MarkedObjectManyOfSame marked_object2;
			theApp.m_marked_list->ObjectsInWindow(strip_boxes[i], &marked_object2, false);
			for(HeeksObj* object = marked_object2.GetFirstOfTopOnly(); object; object = marked_object2.Increment())if(object->GetType() != GripperType)
				obj_set.erase(object);
		}

		for(std::set<HeeksObj*>::iterator It = obj_set.begin(); It != obj_set.end(); It++)
		{
			if(!event.ControlDown() || !theApp.m_marked_list->ObjectMarked(*It))objects.push_back(*It);
		}
	}
	else{
		// select all the objects in the window, even if only partly in the window
		MarkedObjectManyOfSame marked_object;
		theApp.m_marked_list->ObjectsInWindow(window_box, &marked_object, false);
		for(HeeksObj* object = marked_object.GetFirstOfTopOnly(); object; object = marked_object.Increment())
		{
			if(object->GetType() != GripperType && (!event.ControlDown() || !theApp.m_marked_list->ObjectMarked(object)))
				objects.push_back(object);
		}
	}
}
#endif

void CSelectMode::OnLeftUp( MouseEvent& event )
{
#if 0
	if(theApp.drag_gripper)
	{
		double to[3], from[3];
		theApp.m_digitizing->digitize(IPoint(event.GetX(), event.GetY()));
		extract(theApp.m_digitizing->digitized_point.m_point, to);
		theApp.grip_to = theApp.m_digitizing->digitized_point.m_point;
		extract(theApp.grip_from, from);
		theApp.drag_gripper->OnGripperReleased(from, to);
		theApp.m_digitizing->SetOnlyCoords(theApp.drag_gripper, false);
		theApp.drag_gripper = NULL;
	}
	else if(window_box_exists)
	{
		if(!event.ControlDown())theApp.m_marked_list->Clear(true);
		std::list<HeeksObj*> obj_list;
		GetObjectsInWindow(event, obj_list);
		theApp.m_marked_list->Add(obj_list, true);
		theApp.m_current_viewport->DrawWindow(window_box, true); // undraw the window
		window_box_exists = false;
	}
	else
	{
		// select one object
		m_last_click_point = CClickPoint();
		MarkedObjectOneOfEach marked_object;
		theApp.FindMarkedObject(IPoint(event.GetX(), event.GetY()), &marked_object);
		if(marked_object.m_map.size()>0){
			HeeksObj* previously_marked = NULL;
			if(theApp.m_marked_list->size() == 1)
			{
				previously_marked = *(theApp.m_marked_list->list().begin());
			}
			HeeksObj* o = marked_object.GetFirstOfTopOnly();
			unsigned long depth = marked_object.GetDepth();
			HeeksObj* object = o;

			while(o)
			{
				if(o == previously_marked)
				{
					object = o;
					break;
				}

				o = marked_object.Increment();

				if(o)
				{
					// prefer highest order objects
					if((o->GetType() < object->GetType()) || (o->GetType() == object->GetType() && marked_object.GetDepth() < depth))
					{
						object = o;
						depth = marked_object.GetDepth();
					}
				}
			}
			if(!event.ShiftDown() && !event.ControlDown())
			{
				theApp.m_marked_list->Clear(true);
			}
			if (theApp.m_marked_list->ObjectMarked(object))
			{
				if (!event.ShiftDown())
				{
					theApp.m_marked_list->Remove(object, true);
				}
			}
			else
			{
				theApp.m_marked_list->Add(object, true);
				m_last_click_point = CClickPoint(IPoint(event.GetX(), event.GetY()), depth);
				geoff_geometry::Line ray = theApp.m_current_viewport->m_view_point.SightLine(IPoint(event.GetX(), event.GetY()));
				double ray_start[3], ray_direction[3];
				extract(ray.Location(), ray_start);
				extract(ray.Direction(), ray_direction);
				marked_object.GetFirstOfTopOnly();
				object->SetClickMarkPoint(marked_object.GetCurrent(), ray_start, ray_direction);
			}
		}
		else
		{
			if(!event.ShiftDown() && !event.ControlDown())
			{
				theApp.m_marked_list->Clear(true);
			}
		}
	}

	if(m_just_one && m_doing_a_main_loop && (theApp.m_marked_list->size() > 0))
	{
		ExitMainLoop();
	}
	else
	{
		theApp.m_current_viewport->m_need_refresh = true;
	}
#endif
}

void CSelectMode::OnDragging( MouseEvent& event )
{
	if(event.m_middleDown)
	{
		IPoint dm;
		dm.x = event.GetX() - CurrentPoint.x;
		dm.y = event.GetY() - CurrentPoint.y;
#if 0
		if(theApp.ctrl_does_rotate == event.ControlDown())
		{
			if(theApp.m_rotate_mode)
			{
#endif
				theApp.m_current_viewport->m_view_point.Turn(dm);
#if 0
			}
			else
			{
				theApp.m_current_viewport->m_view_point.TurnVertical(dm);
			}
		}
		else
		{
			theApp.m_current_viewport->m_view_point.Shift(dm, IPoint(event.GetX(), event.GetY()));
		}
#endif
		theApp.m_current_viewport->m_need_update = true;
		theApp.m_current_viewport->m_need_refresh = true;
	}
	else if(event.m_leftDown)
	{
#if 0
		if(theApp.drag_gripper)
		{
			double to[3], from[3];
			theApp.m_digitizing->digitize(IPoint(event.GetX(), event.GetY()));
			extract(theApp.m_digitizing->digitized_point.m_point, to);
			theApp.grip_to = theApp.m_digitizing->digitized_point.m_point;
			extract(theApp.grip_from, from);
			theApp.drag_gripper->OnGripperMoved(from, to);
			theApp.grip_from = geoff_geometry::Point3d(from[0], from[1], from[2]);
			theApp.grip_from = make_point(from);
		}
		else if(abs(button_down_point.x - event.GetX())>2 || abs(button_down_point.y - event.GetY())>2)
		{
			if(theApp.m_dragging_moves_objects && !window_box_exists)
			{
				std::list<HeeksObj*> selected_objects_dragged;
				theApp.m_show_grippers_on_drag = true;

				if(	theApp.m_marked_list->list().size() > 0)
				{
					for(std::list<HeeksObj*>::iterator It = theApp.m_marked_list->list().begin(); It != theApp.m_marked_list->list().end(); It++)
					{
						HeeksObj* object = *It;
						if(object->CanBeDragged())selected_objects_dragged.push_back(object);
					}
				}
				else
				{
					MarkedObjectManyOfSame marked_object;
					theApp.FindMarkedObject(button_down_point, &marked_object);
					if(marked_object.m_map.size()>0){
						HeeksObj* object = marked_object.GetFirstOfTopOnly();
						double min_depth = 0.0;
						HeeksObj* closest_object = NULL;
						while(object)
						{
							if(object->CanBeDragged())
							{
								double depth = marked_object.GetDepth();
								if(closest_object == NULL || depth<min_depth)
								{
									min_depth = depth;
									closest_object = object;
								}
							}
							object = marked_object.Increment();
						}
						if(selected_objects_dragged.size() == 0 && closest_object){
							selected_objects_dragged.push_back(closest_object);
							theApp.m_show_grippers_on_drag = false;
						}
					}
				}

				if(selected_objects_dragged.size() > 0)
				{
					theApp.drag_gripper = &drag_object_gripper;
					theApp.m_digitizing->SetOnlyCoords(theApp.drag_gripper, true);
					theApp.m_digitizing->digitize(button_down_point);
					theApp.grip_from = theApp.m_digitizing->digitized_point.m_point;
					theApp.grip_to = theApp.grip_from;
					double from[3];
					from[0] = theApp.grip_from.x;
					from[1] = theApp.grip_from.y;
					from[2] = theApp.grip_from.z;
					theApp.drag_gripper->OnGripperGrabbed(selected_objects_dragged, theApp.m_show_grippers_on_drag, from);
					theApp.grip_from = geoff_geometry::Point3d(from[0], from[1], from[2]);
					double to[3];
					theApp.m_digitizing->digitize(IPoint(event.GetX(), event.GetY()));
					extract(theApp.m_digitizing->digitized_point.m_point, to);
					theApp.grip_to = theApp.m_digitizing->digitized_point.m_point;
					extract(theApp.grip_from, from);
					theApp.drag_gripper->OnGripperMoved(from, to);
					theApp.grip_from = geoff_geometry::Point3d(from[0], from[1], from[2]);
					return;
				}
			}

			if(window_box_exists && theApp.m_mouse_move_highlighting)
			{
				m_highlighted_objects.clear();
				GetObjectsInWindow(event, m_highlighted_objects);
				theApp.Repaint();
			}

			// do window selection
			if(!m_just_one)
			{
				theApp.m_current_viewport->SetXOR();
				if(window_box_exists)theApp.m_current_viewport->DrawWindow(window_box, true); // undraw the window
				window_box.x = button_down_point.x;
				window_box.width = event.GetX() - button_down_point.x;
				window_box.y = theApp.m_current_viewport->GetViewportSize().GetHeight() - button_down_point.y;
				window_box.height = button_down_point.y - event.GetY();
				theApp.m_current_viewport->DrawWindow(window_box, true);// draw the window
				theApp.m_current_viewport->EndXOR();
				window_box_exists = true;
			}
		}
#endif
	}
	CurrentPoint = IPoint(event.GetX(), event.GetY());
}

void CSelectMode::OnMoving( MouseEvent& event )
{
	CurrentPoint = IPoint(event.GetX(), event.GetY());

#if 0
	if(theApp.m_mouse_move_highlighting)
	{
		m_highlighted_objects.clear();

		// highlight one object
		MarkedObjectOneOfEach marked_object;
		theApp.FindMarkedObject(IPoint(event.GetX(), event.GetY()), &marked_object);
		if(marked_object.m_map.size()>0){
			HeeksObj* object = marked_object.GetFirstOfTopOnly();
			m_highlighted_objects.push_back(object);
		}
		theApp.Repaint();
	}
#endif
}

void CSelectMode::OnRender()
{
#if 0
	if (m_highlighted_objects.size() > 0)
	{
		theApp.m_highlight_color.glColor();
		Material(theApp.m_highlight_color).glMaterial(1.0);
	}

	for (std::list<HeeksObj*>::iterator It = m_highlighted_objects.begin(); It != m_highlighted_objects.end(); It++)
	{
		HeeksObj* object = *It;
		object->glCommands(false, true, true);
	}
#endif
}

void CSelectMode::OnWheelRotation( MouseEvent& event )
{
	double wheel_value = (double)(event.GetWheelRotation());
	double multiplier = wheel_value /1000.0, multiplier2;
#if 0
	// to do
	if(theApp.mouse_wheel_forward_away)multiplier = -multiplier;
#endif

	// make sure these are actually inverses, so if you
	// zoom in and out the same number of steps, you'll be
	// at the same zoom level again

	if(multiplier > 0) {
		multiplier2 = 1 + multiplier;
	} else {
		multiplier2 = 1/(1 - multiplier);
	}

	IPoint client_size = theApp.m_current_viewport->GetViewportSize();

	double pixelscale_before = theApp.GetPixelScale();
	theApp.m_current_viewport->m_view_point.Scale(multiplier2);
	double pixelscale_after = theApp.GetPixelScale();

	double event_x = event.GetX();
	double event_y = event.GetY();
	double center_x = client_size.GetWidth() / 2.;
	double center_y = client_size.GetHeight() / 2.;

	// how many pixels are we from the center (the center
	// is the point that doesn't move when you zoom)?
	double px = event_x - center_x;
	double py = event_y - center_y;

	// that number of pixels represented how many mm
	// before and after the zoom ...
	double xbefore = px / pixelscale_before;
	double ybefore = py / pixelscale_before;
	double xafter = px / pixelscale_after;
	double yafter = py / pixelscale_after;

	// which caused a change in how many mm at that point
	// on the screen?
	double xchange = xafter - xbefore;
	double ychange = yafter - ybefore;

	// and how many pixels worth of motion is that?
	double x_moved_by = xchange * pixelscale_after;
	double y_moved_by = ychange * pixelscale_after;

	// so move that many pixels to keep the coordinate
	// under the cursor approximately the same
	theApp.m_current_viewport->m_view_point.Shift(IPoint((int)x_moved_by, (int)y_moved_by), IPoint(0, 0));
	theApp.m_current_viewport->m_need_refresh = true;
}

void CSelectMode::OnMouse( MouseEvent& event )
{
	bool event_used = false;
#if 0
	if(LeftAndRightPressed(event, event_used))
	{
		if(m_doing_a_main_loop){
			ExitMainLoop();
		}
	}
#endif

	if(event_used)return;

	if(event.LeftDown())
		OnLeftDown(event);

	if(event.MiddleDown())
		OnMiddleDown(event);

	bool dragging = event.Moving() && (m_button_down || m_middle_button_down);
	bool moving = event.Moving() && (!(m_button_down || m_middle_button_down));
	bool left_up = false;

	if(event.LeftUp())
	{
		if(m_button_down)left_up = true;
		m_button_down = false;
	}

	if(event.MiddleUp())m_middle_button_down = false;

	if(left_up)
	{
		OnLeftUp(event);
	}
	else if(event.RightUp())
	{
#if 0
		MarkedObjectOneOfEach marked_object;
		theApp.FindMarkedObject(IPoint(event.GetX(), event.GetY()), &marked_object);
		theApp.DoDropDownMenu(theApp.m_frame->m_graphics, IPoint(event.GetX(), event.GetY()), &marked_object, false, event.ControlDown());
#endif
	}
	else if(dragging)
	{
		OnDragging(event);
	}
	else if(moving)
	{
		OnMoving(event);
	}

	if(event.GetWheelRotation() != 0)
	{
		OnWheelRotation(event);
	}
}

#if 0
void CSelectMode::OnKeyDown(wxKeyEvent& event)
{
	switch(event.GetKeyCode()){
	case WXK_RETURN:
		if(m_doing_a_main_loop && theApp.m_marked_list->size() > 0)ExitMainLoop();
		break;

	case WXK_ESCAPE:
		if(m_doing_a_main_loop)ExitMainLoop();
		break;
	}

	CInputMode::OnKeyDown(event);
}

void CSelectMode::OnKeyUp(wxKeyEvent& event)
{
	CInputMode::OnKeyUp(event);
}

void CSelectMode::OnFrontRender(){
	if(theApp.drag_gripper){
		theApp.drag_gripper->OnFrontRender();
	}
	if(window_box_exists)theApp.m_current_viewport->DrawWindow(window_box, true);
}

bool CSelectMode::OnStart(){
	return true;
}

void CSelectMode::GetProperties(std::list<Property *> *list){
}

class EndPicking:public Tool{
public:
	void Run(){
		if(theApp.m_select_mode->m_doing_a_main_loop)
		{
			ExitMainLoop();
			theApp.m_frame->RefreshInputCanvas();
		}
		else{
			wxMessageBox(_T("Error! The \"Stop Picking\" button shouldn't have been available!"));
		}
	}
	const wchar_t* GetTitle(){return _("Accept selection");}
	std::wstring BitmapPath(){return _T("endpick");}
};

static EndPicking end_picking;

class CancelPicking:public Tool{
public:
	void Run(){
		if(theApp.m_select_mode->m_doing_a_main_loop)
		{
			theApp.m_marked_list->Clear(false);
			ExitMainLoop();
			theApp.m_frame->RefreshInputCanvas();
		}
		else{
			wxMessageBox(_T("Error! The \"Cancel Picking\" button shouldn't have been available!"));
		}
	}
	const wchar_t* GetTitle(){return _("Cancel selection");}
	std::wstring BitmapPath(){return _T("escpick");}
};

static CancelPicking cancel_picking;

class PickAnything:public Tool{
public:
	void Run(){
		theApp.m_marked_list->m_filter = -1;
		theApp.m_frame->RefreshInputCanvas();
	}
	const wchar_t* GetTitle(){return _("Pick Anything");}
	std::wstring BitmapPath(){return _T("pickany");}
	const wchar_t* GetToolTip(){return _("Set the selection filter to all items");}
};

static PickAnything pick_anything;

class PickEdges:public Tool{
public:
	void Run(){
		theApp.m_marked_list->m_filter = MARKING_FILTER_EDGE;
		theApp.m_frame->RefreshInputCanvas();
	}
	const wchar_t* GetTitle(){return _("Pick Edges");}
	std::wstring BitmapPath(){return _T("pickedges");}
	const wchar_t* GetToolTip(){return _("Set the selection filter to only edges");}
};

static PickEdges pick_edges;

class PickFaces:public Tool{
public:
	void Run(){
		theApp.m_marked_list->m_filter = MARKING_FILTER_FACE;
		theApp.m_frame->RefreshInputCanvas();
	}
	const wchar_t* GetTitle(){return _("Pick Faces");}
	std::wstring BitmapPath(){return _T("pickfaces");}
	const wchar_t* GetToolTip(){return _("Set the selection filter to only faces");}
};

static PickFaces pick_faces;

void CSelectMode::GetTools(std::list<Tool*>* t_list, const IPoint* p)
{
	if(m_doing_a_main_loop)
	{
		t_list->push_back(&end_picking);
		t_list->push_back(&cancel_picking);
	}
	if(theApp.m_marked_list->m_filter != -1)t_list->push_back(&pick_anything);
	if(theApp.m_marked_list->m_filter != MARKING_FILTER_EDGE)t_list->push_back(&pick_edges);
	if(theApp.m_marked_list->m_filter != MARKING_FILTER_FACE)t_list->push_back(&pick_faces);
}
#endif
