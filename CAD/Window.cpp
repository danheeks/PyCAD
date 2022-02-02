// Window.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#if 0
to do port this to python
#include "stdafx.h"
#include "Window.h"
#include "HeeksObj.h"
#include "Viewport.h"

WindowDragging::WindowDragging(){
	reset();
}

void WindowDragging::reset(void){
	box_found = false;
	finish_dragging = false;
}

void WindowDragging::OnMouse( MouseEvent& event ){
	if(event.LeftDown()){
		window_box.x = event.GetX();
		window_box.y = theApp->m_current_viewport->GetViewportSize().GetHeight() - event.GetY();
	}
	else if(event.LeftUp()){
		window_box.width = event.GetX() - window_box.x;
		window_box.height = (theApp->m_current_viewport->GetViewportSize().GetHeight() - window_box.y) - event.GetY();
		if(abs(window_box.width)<4)box_found = false;
		else if(abs(window_box.height)<4)box_found = false;
		else box_found = true;
		finish_dragging = true;
	}
	else if(event.Moving()){
		window_box.width = event.GetX() - window_box.x;
		window_box.height = (theApp->m_current_viewport->GetViewportSize().GetHeight() - window_box.y) - event.GetY();
	}
}
#endif