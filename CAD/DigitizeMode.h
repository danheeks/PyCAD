// DigitizeMode.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "InputMode.h"
#include "DigitizedPoint.h"

class CViewPoint;
class PointOrWindow;

class DigitizeMode:public CInputMode{
private:
	PointOrWindow *point_or_window;
	DigitizedPoint lbutton_point;

	DigitizedPoint digitize1(const IPoint &input_point);

public:
	DigitizedPoint digitized_point;
	DigitizedPoint reference_point;	// the last point the operator explicitly defined (as opposed to mouse movements over the graphics canvas)
	std::wstring m_prompt_when_doing_a_main_loop;
	bool wants_to_exit_main_loop;

	DigitizeMode();
	virtual ~DigitizeMode(void);

	// InputMode's virtual functions
	const wchar_t* GetTitle();
	const wchar_t* GetHelpText();
	void OnMouse( MouseEvent& event );
//	void OnKeyDown(KeyCode key_code);
	void OnModeChange(void);
	void OnFrontRender();
	void GetProperties(std::list<Property *> *list);

	DigitizedPoint Digitize(const Line &ray);
	DigitizedPoint digitize(const IPoint &point);
	void SetOnlyCoords(HeeksObj* object, bool onoff);
	bool OnlyCoords(HeeksObj* object);
};
