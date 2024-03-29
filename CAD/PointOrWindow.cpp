// PointOrWindow.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#if 0
to do port this to python
#include "stdafx.h"

#include "PointOrWindow.h"
#include "Window.h"
#include "ViewPoint.h"
#include "App.h"
#include "Viewport.h"

PointOrWindow::PointOrWindow(bool wd){
	if(wd)window = new WindowDragging();
	else window = NULL;
	reset();
	visible = false;
}

PointOrWindow::~PointOrWindow(void){
	if(window)delete window;
}

void PointOrWindow::reset(void){
	if(window)window->reset();
	use_window = false;
}

void PointOrWindow::OnMouse( MouseEvent& event ){
	if(event.LeftDown()){
		mouse_down_point = IPoint(event.GetX(), event.GetY());
	}
	else if ( event.Moving() ){
		if(event.m_leftDown && !use_window && window && abs(event.GetX() - mouse_down_point.x)>2 && abs(event.GetY() - mouse_down_point.y)>2){
			use_window = true;
			box_chosen.x = mouse_down_point.x;
			box_chosen.y = theApp->m_current_viewport->GetViewportSize().GetHeight() - mouse_down_point.y;
			window->window_box = box_chosen;
		}

		if(use_window){
			box_chosen.width = event.GetX() - box_chosen.x;
			box_chosen.height =theApp->m_current_viewport->GetViewportSize().GetHeight() - event.GetY() - box_chosen.y;
		}
		else{
			SetWithPoint(IPoint(event.GetX(), event.GetY()));
		}
	}
}

void PointOrWindow::OnModeChange(void){
	SetWithPoint(theApp->cur_mouse_pos);
	visible = true;
}

void PointOrWindow::SetWithPoint(const IPoint &point){
	box_chosen.x = point.x - 5;
	box_chosen.y = point.y - 5;
	box_chosen.width = 10;
	box_chosen.height = 10;

	use_window = false;
}

void PointOrWindow::OnFrontRender(){
	if(!visible)return;
	glColor3ub(255, 255, 255);
	if(!use_window)
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		theApp->m_current_viewport->SetIdentityProjection();
		theApp->m_current_viewport->DrawWindow(box_chosen, false);
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}
}
#endif