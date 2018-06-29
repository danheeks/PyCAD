#include "stdafx.h"

#include "App.h"
#include "Grid.h"
#include "InputMode.h"
#include "SelectMode.h"
#include "Viewport.h"

CApp theApp;

CApp::CApp()
{
	for (int i = 0; i<NUM_BACKGROUND_COLORS; i++)background_color[i] = HeeksColor(0, 0, 0);
	m_background_mode = BackgroundModeOneColor;
	current_color = HeeksColor(0, 0, 0);
	cur_mouse_pos = IPoint(0, 0);
	m_view_units = 1.0;
	m_current_viewport = NULL;
	grid_mode = 3;
	input_mode_object = NULL;
	m_select_mode = new CSelectMode();

}

CApp::~CApp()
{
}

void CApp::GetBox(CBox &box)
{
	box.Insert(CBox(-10.0, -10.0, -10.0, 10.0, 10.0, 10.0));
}

void CApp::glCommands(bool select, bool marked, bool no_color)
{

}

bool CApp::OnInit()
{

	// NOTE: A side-effect of calling the SetInputMode() method is
	// that the GetOptions() method is called.  To that end, all
	// configuration settings should be read BEFORE this point.
	SetInputMode(m_select_mode);

	return true;
}


void CApp::EnableBlend()
{
	//if (!m_antialiasing)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	}
}

void CApp::DisableBlend()
{
	//if(!m_antialiasing)
	glDisable(GL_BLEND);
}

void CApp::SetInputMode(CInputMode *new_mode){
	if(!new_mode)return;
	//if(m_frame)m_current_viewport->EndDrawFront();
	if(new_mode->OnModeChange()){
		input_mode_object = new_mode;
	}
	else{
		input_mode_object = m_select_mode;
	}
	//if(m_frame && m_frame->m_input_canvas)m_frame->RefreshInputCanvas();
	//if(m_frame && m_frame->m_options)m_frame->RefreshOptions();
	//if(m_graphics_text_mode != GraphicsTextModeNone)Repaint();
}

void CApp::glCommandsAll(const CViewPoint &view_point)
{
#if 0
	CreateLights();
	glDisable(GL_LIGHTING);
	Material().glMaterial(1.0);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glLineWidth(1);
	glDepthMask(1);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glShadeModel(GL_FLAT);
	view_point.SetPolygonOffset();

	std::list<HeeksObj*> after_others_objects;

	for (std::list<HeeksObj*>::iterator It = m_objects.begin(); It != m_objects.end(); It++)
	{
		HeeksObj* object = *It;
		if (object->OnVisibleLayer() && object->m_visible)
		{
			if (object->DrawAfterOthers())after_others_objects.push_back(object);
			else
			{
				object->glCommands(false, m_marked_list->ObjectMarked(object), false);
			}
		}
	}

	// draw any last_objects
	for (std::list<HeeksObj*>::iterator It = after_others_objects.begin(); It != after_others_objects.end(); It++)
	{
		HeeksObj* object = *It;
		object->glCommands(false, m_marked_list->ObjectMarked(object), false);
	}

	glDisable(GL_POLYGON_OFFSET_FILL);
	for (std::list< void(*)() >::iterator It = m_on_glCommands_list.begin(); It != m_on_glCommands_list.end(); It++)
	{
		void(*callbackfunc)() = *It;
		(*callbackfunc)();
	}
	glEnable(GL_POLYGON_OFFSET_FILL);

	input_mode_object->OnRender();
	if (m_transform_gl_list)
	{
		glPushMatrix();
		double m[16];
		extract_transposed(m_drag_matrix, m);
		glMultMatrixd(m);
		glCallList(m_transform_gl_list);
		glPopMatrix();
	}

	// draw the ruler
	if (m_show_ruler && m_ruler->m_visible)
	{
		m_ruler->glCommands(false, false, false);
	}
#endif
	// draw the grid
	glDepthFunc(GL_LESS);
	RenderGrid(&view_point);
	glDepthFunc(GL_LEQUAL);

#if 0
	// draw the datum
	RenderDatumOrCurrentCoordSys();

	DestroyLights();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (m_hidden_for_drag.size() == 0 || !m_show_grippers_on_drag)m_marked_list->GrippersGLCommands(false, false);

	// draw the input mode text on the top
	if (m_graphics_text_mode != GraphicsTextModeNone)
	{
		wxString screen_text1, screen_text2;

		if (input_mode_object && input_mode_object->GetTitle())
		{
			screen_text1.Append(input_mode_object->GetTitle());
			screen_text1.Append(_T("\n"));
		}
		if (m_graphics_text_mode == GraphicsTextModeWithHelp && input_mode_object)
		{
			const wxChar* help_str = input_mode_object->GetHelpText();
			if (help_str)
			{
				screen_text2.Append(help_str);
			}
		}
		render_screen_text(screen_text1, screen_text2, false);
	}
#endif
}

double CApp::GetPixelScale(void){
	return m_current_viewport->m_view_point.m_pixel_scale;
}

geoff_geometry::Matrix CApp::GetDrawMatrix(bool get_the_appropriate_orthogonal)
{
	return geoff_geometry::Matrix();
}
