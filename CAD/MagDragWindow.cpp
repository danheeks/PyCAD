// MagDragWindow.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#if 0
to do port this to python
#include "MagDragWindow.h"
#include "Viewport.h"

void MagDragWindow::OnMouse( MouseEvent& event )
{
	if(event.LeftDown())
	{
		window_box.x = event.GetX();
		window_box.y = theApp->m_current_viewport->GetViewportSize().GetHeight() - event.GetY();
		window_box_exists = false;
	}
	else if(event.LeftUp())
	{
		window_box.width = event.GetX() - window_box.x;
		window_box.height = (theApp->m_current_viewport->GetViewportSize().GetHeight() - event.GetY()) - window_box.y;
		if(abs(window_box.width)<4){theApp->SetInputMode(save_input_mode); return;}
		if(abs(window_box.height)<4){theApp->SetInputMode(save_input_mode); return;}

		theApp->m_current_viewport->WindowMag(window_box);
		window_box_exists = false;
		theApp->SetInputMode(save_input_mode);
	}
	else if(event.Moving() && event.m_leftDown)
	{
//		theApp->m_frame->m_graphics->SetCurrent();
		theApp->m_current_viewport->SetXOR();
		if(window_box_exists)theApp->m_current_viewport->DrawWindow(window_box, false);// undraw the window
		window_box.width = event.GetX() - window_box.x;
		window_box.height = (theApp->m_current_viewport->GetViewportSize().GetHeight() - event.GetY()) - window_box.y;
		theApp->m_current_viewport->DrawWindow(window_box, false);// draw the window
		theApp->m_current_viewport->EndXOR();
		window_box_exists = true;
	}
}

void MagDragWindow::OnModeChange(void){
	save_input_mode = theApp->input_mode_object;
}

void MagDragWindow::OnFrontRender(){
	if(window_box_exists){
		theApp->m_current_viewport->DrawWindow(window_box, false);
	}
}
#endif