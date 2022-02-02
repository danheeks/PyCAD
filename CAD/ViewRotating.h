// ViewRotating.h

// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#if 0
to do port this to python

#pragma once

#include "InputMode.h"

class ViewRotating: public CInputMode{
	IPoint button_down_point;
	IPoint CurrentPoint;
public:
	// virtual functions for InputMode
	const wchar_t* GetTitle(){return L"View Rotate";}
	void OnMouse( MouseEvent& event );
};
#endif