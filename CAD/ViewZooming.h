// ViewZooming.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#pragma once

#include "InputMode.h"

class ViewZooming: public CInputMode{
	IPoint button_down_point;
	IPoint CurrentPoint;

public:
	static bool m_reversed;

	// virtual functions for InputMode
	const wchar_t* GetTitle(){return L"View Zoom";}
	const wchar_t* GetHelpText();
	void OnMouse( MouseEvent& event );
	bool OnModeChange(void);
};
