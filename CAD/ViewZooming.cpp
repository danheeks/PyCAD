// ViewZooming.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "ViewZooming.h"
#include "SelectMode.h"
#include "Viewport.h"

bool ViewZooming::m_reversed = false;

void ViewZooming::OnMouse( MouseEvent& event )
{
	if (event.LeftDown() || event.MiddleDown())
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

		if (event.m_leftDown)
		{
			theApp.m_current_viewport->m_view_point.Scale(IPoint(event.GetX(), event.GetY()), m_reversed);
		}
		else if (event.m_middleDown)
		{
			theApp.m_current_viewport->m_view_point.Shift(dm, IPoint(event.GetX(), event.GetY()));
		}

		theApp.Repaint();
		CurrentPoint = IPoint(event.GetX(), event.GetY());
	}
	else if(event.RightUp()){
		// do context menu same as select mode
		theApp.m_select_mode->OnMouse(event);
	}
	if(event.GetWheelRotation() != 0)theApp.m_select_mode->OnMouse(event);
}

static std::wstring str_for_GetHelpText;

const wchar_t* ViewZooming::GetHelpText(){
	str_for_GetHelpText = std::wstring(L"Drag with the left mouse button") + L"\n" + (m_reversed ? L"Forward to zoom in, Back to zoom out":L"Back to zoom in, Forward to zoom out") + L"\n" +L"Hold middle mouse button down to pan";
	return str_for_GetHelpText.c_str();
}


bool ViewZooming::OnModeChange(void)
{
	return true;
}
