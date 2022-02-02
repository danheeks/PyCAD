// PointOrWindow.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once
#if 0
to do port this to python

#include "InputMode.h"

class WindowDragging;
class CViewPoint;

class PointOrWindow: public CInputMode{
private:
	IPoint mouse_down_point;
	bool visible;

public:
	bool use_window;
	IRect box_chosen;
	WindowDragging *window;

	PointOrWindow(bool wd);
	~PointOrWindow(void);

	// virtual functions for InputMode
	const wchar_t* GetTitle(){return L"Dragging a window or picking a point";}
	void OnMouse( MouseEvent& event );
	void OnModeChange(void);
	void OnFrontRender();

	void reset(void);
	void SetWithPoint(const IPoint &point);
};
#endif