// MagDragWindow.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#if !defined MagDragWindowHEADER
#define MagDragWindowHEADER

#include "InputMode.h"

class MagDragWindow: public CInputMode{
private:
	IRect window_box;
	bool window_box_exists;
	CInputMode *save_input_mode;

public:
	MagDragWindow(){save_input_mode = NULL; window_box_exists = false;}

	// virtual functions for InputMode
	const wchar_t* GetTitle(){return L"Magnifying by dragging a window";}
	void OnMouse( MouseEvent& event );
	void OnModeChange(void);
	void OnFrontRender();
};

#endif
