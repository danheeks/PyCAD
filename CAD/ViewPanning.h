// ViewPanning.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#pragma once

#include "InputMode.h"

class ViewPanning: public CInputMode{
	IPoint button_down_point;
	IPoint CurrentPoint;

public:
	// virtual functions for InputMode
	const wchar_t* GetTitle(){return L"View Pan";}
	const wchar_t* GetHelpText();
	void OnMouse( MouseEvent& event );
};
