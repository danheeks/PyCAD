// LeftAndRightCode.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include <stdafx.h>
#include "LeftAndRight.h"
#include "MouseEvent.h"

CLeftAndRight::CLeftAndRight()
{
	m_right_up_with_left_down_done = false;
	m_left_up_with_right_down_done = false;
}

bool CLeftAndRight::LeftAndRightPressed(MouseEvent& event, bool &event_used)
{
	bool left_and_right_pressed = false;

	if(event.LeftUp()){
		if(m_right_up_with_left_down_done){
			m_right_up_with_left_down_done = false;
			left_and_right_pressed = true;
			event_used = true;
		}
		else if(event.m_rightDown){
			m_left_up_with_right_down_done = true;
			event_used = true;
		}
	}
	else if(event.RightUp()){
		if(m_left_up_with_right_down_done){
			m_left_up_with_right_down_done = false;
			left_and_right_pressed = true;
			event_used = true;
		}
		else if(event.m_leftDown){
			m_right_up_with_left_down_done = true;
			event_used = true;
		}
	}

	return left_and_right_pressed;
}


