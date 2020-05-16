// ViewRotating.cp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "ViewRotating.h"
#include "SelectMode.h"
#include "CadApp.h"
#include "Viewport.h"

void ViewRotating::OnMouse( MouseEvent& event )
{
	if(event.LeftDown() || event.MiddleDown())
	{
		button_down_point = IPoint(event.GetX(), event.GetY());
		CurrentPoint = button_down_point;
		theApp->m_current_viewport->StoreViewPoint();
//		theApp->m_current_viewport->m_view_point.SetStartMousePoint(button_down_point);
	}
	else if(event.Moving())
	{
		IPoint dm;
		dm.x = event.GetX() - CurrentPoint.x;
		dm.y = event.GetY() - CurrentPoint.y;

		if(event.m_leftDown)
		{
			if(theApp->m_rotate_mode)
			{
				theApp->m_current_viewport->m_view_point.TurnI(dm);
			}
			else
			{
				theApp->m_current_viewport->m_view_point.TurnVertical(dm);
			}
		}
		else if(event.m_middleDown)
		{
			theApp->m_current_viewport->m_view_point.ShiftI(dm);
		}

		theApp->Repaint();
		CurrentPoint = IPoint(event.GetX(), event.GetY());
	}
	else if(event.RightUp()){
		// do context menu same as select mode
//		theApp->m_select_mode->OnMouse(event);
	}
//	if(event.GetWheelRotation() != 0)theApp->m_select_mode->OnMouse(event);

}
