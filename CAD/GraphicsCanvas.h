// GraphicsCanvas.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once


class CGraphicsCanvas: public wxGLCanvas{
public:
	CGraphicsCanvas():wxGLCanvas((wxWindow*)0, 0, (int*)0){} // just for boost python
    CGraphicsCanvas(wxWindow* parent);
    virtual ~CGraphicsCanvas(){};

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
    void OnMouse( wxMouseEvent& event );

private:
    DECLARE_EVENT_TABLE()
};
