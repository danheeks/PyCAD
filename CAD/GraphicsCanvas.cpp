// GraphicsCanvas.cpp
// Copyright (c) 2018, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "GraphicsCanvas.h"

BEGIN_EVENT_TABLE(CGraphicsCanvas, wxGLCanvas)
    EVT_SIZE(CGraphicsCanvas::OnSize)
	EVT_ERASE_BACKGROUND(CGraphicsCanvas::OnEraseBackground)
    EVT_PAINT(CGraphicsCanvas::OnPaint)
    EVT_MOUSE_EVENTS(CGraphicsCanvas::OnMouse)
END_EVENT_TABLE()

static int graphics_attrib_list[] = {
	WX_GL_RGBA,
	1,
	WX_GL_DOUBLEBUFFER,
	1,
	WX_GL_DEPTH_SIZE,
	1,
	WX_GL_MIN_RED,
	1,
	WX_GL_MIN_GREEN,
	1,
	WX_GL_MIN_BLUE,
	1,
	WX_GL_MIN_ALPHA,
	0,
	0
};

CGraphicsCanvas::CGraphicsCanvas(wxWindow* parent)
	: wxGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, _T("some text"), graphics_attrib_list)
{
}

void CGraphicsCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    /* must always be here */
    wxPaintDC dc(this);

#ifndef __WXMOTIF__
//    if (!GetContext()) return;
#endif

 //   SetCurrent();

//	glCommands();

    SwapBuffers();
}

void CGraphicsCanvas::OnSize(wxSizeEvent& event)
{
    // this is also necessary to update the context on some platforms
//    wxGLCanvas::OnSize(event);
    int w, h;
    GetClientSize(&w, &h);
//	WidthAndHeightChanged(w, h);
	Refresh();
}

void CGraphicsCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
	// Do nothing, to avoid flashing on MSW
}
void CGraphicsCanvas::OnMouse( wxMouseEvent& event )
{
	event.Skip();
}
