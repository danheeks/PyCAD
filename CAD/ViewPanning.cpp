// ViewPanning.cpp
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "ViewPanning.h"
#include "SelectMode.h"
#include "App.h"
#include "Viewport.h"

void ViewPanning::OnMouse( MouseEvent& event )
{
	if(event.LeftDown() || event.MiddleDown())
	{
		button_down_point = IPoint(event.GetX(), event.GetY());
		CurrentPoint = button_down_point;
		theApp.m_current_viewport->StoreViewPoint();
		theApp.m_current_viewport->m_view_point.SetStartMousePoint(button_down_point);
	}
	else if(event.Moving())
	{
		IPoint dm;
		dm.x = event.GetX() - CurrentPoint.x;
		dm.y = event.GetY() - CurrentPoint.y;

		if(event.m_leftDown)
		{
			theApp.m_current_viewport->m_view_point.Shift(dm, IPoint(event.GetX(), event.GetY()));
		}

		CurrentPoint = IPoint(event.GetX(), event.GetY());
	}
	else if(event.RightUp()){
		// do context menu same as select mode
		theApp.m_select_mode->OnMouse(event);
	}
	if(event.GetWheelRotation() != 0)theApp.m_select_mode->OnMouse(event);
}

static std::wstring str_for_GetHelpText;

const wchar_t* ViewPanning::GetHelpText(){
	str_for_GetHelpText = std::wstring(L"Drag with the left mouse button");
	return str_for_GetHelpText.c_str();
}

bool ViewPanning::OnModeChange(void)
{
//	end_panning.m_saved_mode = theApp.input_mode_object;
	return true;
}
