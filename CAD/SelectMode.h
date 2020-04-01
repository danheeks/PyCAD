// SelectMode.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#if 0
#include "InputMode.h"
#include "MouseEvent.h"
//#include "LeftAndRight.h"

class CClickPoint
{
	bool m_valid;
	double m_pos[3];
	IPoint m_point;
	unsigned long m_depth;

public:
	CClickPoint(): m_valid(false){}
	CClickPoint(const IPoint& point, unsigned long depth);

	bool GetPos(double *pos);
};

class CSelectMode: public CInputMode/*, CLeftAndRight*/{
public:
	IPoint CurrentPoint;
	IPoint button_down_point;
	bool control_key_initially_pressed;
	bool m_button_down; // don't trust dragging, if button wasn't received first
	bool m_middle_button_down;
	IRect window_box;
	bool window_box_exists;
	bool m_doing_a_main_loop;
	bool m_just_one;
//	std::wstring m_prompt_when_doing_a_main_loop;
	CClickPoint m_last_click_point;
	std::list<HeeksObj*> m_highlighted_objects;

	CSelectMode();
	virtual ~CSelectMode(void){}

	bool GetLastClickPosition(double *pos);

	void OnLeftDown( MouseEvent& event );
	void OnMiddleDown( MouseEvent& event );
	void GetObjectsInWindow(MouseEvent& event, std::list<HeeksObj*> &objects);
	void OnLeftUp( MouseEvent& event );
	void OnDragging( MouseEvent& event );
	void OnMoving( MouseEvent& event );
	void OnWheelRotation( MouseEvent& event );

	// virtual functions for InputMode
	const wchar_t* GetTitle();
	bool TitleHighlighted(){return m_doing_a_main_loop;}
	const wchar_t* GetHelpText();
	void OnMouse( MouseEvent& event );
	//void OnKeyDown(KeyCode key_code);
	//void OnKeyUp(KeyCode key_code);
	bool OnStart();
	void OnFrontRender();
	void GetProperties(std::list<Property *> *list);
	void OnRender();
};

#endif