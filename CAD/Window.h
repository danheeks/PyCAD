// Window.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#if 0
to do port this to python

#pragma once

#include "InputMode.h"

class WindowDragging: public CInputMode{
public:
	IRect window_box;
	bool finish_dragging;
	bool box_found;
	bool box_drawn_with_cross;

	WindowDragging();

	// virtual functions from InputMode
	const wchar_t* GetTitle(){return L"Dragging a window";}
	void OnMouse( MouseEvent& event );

	void reset(void);
};
#endif