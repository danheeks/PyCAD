#include "stdafx.h"
#include "Viewport.h"
#include "InputMode.h"

CViewport::CViewport() :m_frozen(false), m_refresh_wanted_on_thaw(false), m_w(0), m_h(0), m_view_point(this), m_need_update(false), m_need_refresh(false)
{
	theApp.m_current_viewport = this;
}

CViewport::CViewport(int w, int h) : m_frozen(false), m_refresh_wanted_on_thaw(false), m_w(w), m_h(h), m_view_point(this), m_need_update(false), m_need_refresh(false)
{
	theApp.m_current_viewport = this;
}

void CViewport::SetViewport()
{
	m_view_point.SetViewport();
}

void CViewport::glCommands()
{
	glDrawBuffer(GL_BACK);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	if (0/*theApp.m_antialiasing*/)
	{
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	}
	else
	{
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
	}

	SetViewport();

	switch (theApp.m_background_mode)
	{
	case BackgroundModeTwoColors:
	case BackgroundModeTwoColorsLeftToRight:
	case BackgroundModeFourColors:
	{
		// draw graduated background

		glClear(GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0.0, 1.0, 0.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// set up which colors to use
		HeeksColor c[4];
		for (int i = 0; i<4; i++)c[i] = theApp.background_color[i];
		switch (theApp.m_background_mode)
		{
		case BackgroundModeTwoColors:
			c[2] = c[0];
			c[3] = c[1];
			break;
		case BackgroundModeTwoColorsLeftToRight:
			c[1] = c[0];
			c[3] = c[2];
			break;
		default:
			break;
		}

		glShadeModel(GL_SMOOTH);
		glBegin(GL_QUADS);
		c[0].glColor();
		glVertex2f(0.0, 1.0);
		c[1].glColor();
		glVertex2f(0.0, 0.0);
		c[3].glColor();
		glVertex2f(1.0, 0.0);
		c[2].glColor();
		glVertex2f(1.0, 1.0);


		glEnd();
		glShadeModel(GL_FLAT);
	}
	break;

	default:
		break;
	}

	m_view_point.SetProjection(true);
	m_view_point.SetModelview();

	switch (theApp.m_background_mode)
	{
	case BackgroundModeOneColor:
	{
		// clear the back buffer
		theApp.background_color[0].glClearColor(/*theApp.m_antialiasing ? 0.0f : */1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	break;
	case BackgroundModeTwoColors:
	case BackgroundModeTwoColorsLeftToRight:
	case BackgroundModeFourColors:
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	break;
	case BackgroundModeSkyDome:
	{
#if 0
		// draw sky dome
		glClear(GL_DEPTH_BUFFER_BIT);
		double radius = m_view_point.m_far_plane * 0.5;
		geoff_geometry::Point3d x(1, 0, 0);
		geoff_geometry::Point3d y(0, 1, 0);
		geoff_geometry::Point3d z(0, 0, 1);
		glShadeModel(GL_SMOOTH);
		glBegin(GL_TRIANGLES);
		glEnable(GL_CULL_FACE);
		for (int sector = 0; sector<4; sector++)
		{
			double ang0 = 0.7853981633974 - 1.5707963267948 * sector;
			double ang1 = 0.7853981633974 - 1.5707963267948 * (sector + 1);

			for (int panel = 0; panel<4; panel++)// vertical sections
			{
				double vang0 = -1.5707963267948 + 0.7853981633974 * panel;
				double vang1 = -1.5707963267948 + 0.7853981633974 * (panel + 1);
				geoff_geometry::Point3d p0 = m_view_point.m_lens_point + (x * (radius * cos(ang0) * cos(vang0))) + (radius * y * sin(ang0) * cos(vang0)) + (radius * z * sin(vang0));
				geoff_geometry::Point3d p1 = m_view_point.m_lens_point + (x * (radius * cos(ang1) * cos(vang0))) + (radius * y * sin(ang1) * cos(vang0)) + (radius * z * sin(vang0));
				geoff_geometry::Point3d p2 = m_view_point.m_lens_point + (x * (radius * cos(ang0) * cos(vang1))) + (radius * y * sin(ang0) * cos(vang1)) + (radius * z * sin(vang1));
				geoff_geometry::Point3d p3 = m_view_point.m_lens_point + (x * (radius * cos(ang1) * cos(vang1))) + (radius * y * sin(ang1) * cos(vang1)) + (radius * z * sin(vang1));
				HeeksColor c0, c1;
				switch (panel)
				{
				case 0:
					c0 = theApp.background_color[9];
					c1 = theApp.background_color[8];
					break;
				case 1:
					c0 = theApp.background_color[8];
					c1 = theApp.background_color[7];
					break;
				case 2:
					c0 = theApp.background_color[6];
					c1 = theApp.background_color[5];
					break;
				case 3:
					c0 = theApp.background_color[5];
					c1 = theApp.background_color[4];
					break;
				default:
					break;
				}

				if (panel != 4)
				{
					c0.glColor();
					glVertex3d(p0.x, p0.y, p0.z);
					c1.glColor();
					glVertex3d(p3.x, p3.y, p3.z);
					glVertex3d(p2.x, p2.y, p2.z);
				}
				if (panel != 0)
				{
					c0.glColor();
					glVertex3d(p0.x, p0.y, p0.z);
					glVertex3d(p1.x, p1.y, p1.z);
					c1.glColor();
					glVertex3d(p3.x, p3.y, p3.z);
				}


			}
		}
		glEnd();
		glShadeModel(GL_FLAT);
		glDisable(GL_CULL_FACE);
		glClear(GL_DEPTH_BUFFER_BIT);
#endif
	}
	break;
	}

	// render everything
	theApp.glCommandsAll(m_view_point);

	// mark various XOR drawn items as not drawn
	m_render_on_front_done = false;
}

#if 0
void CViewport::DrawFront(void){
	if (!m_render_on_front_done){
		FrontRender();
		m_render_on_front_done = true;
	}
}

void CViewport::EndDrawFront(void){
	if (m_render_on_front_done){
		FrontRender();
		m_render_on_front_done = false;
	}
}

void CViewport::FrontRender(void){
	SetViewport();
	m_view_point.SetProjection(false);
	m_view_point.SetModelview();

	SetXOR();

	theApp.input_mode_object->OnFrontRender();

	EndXOR();

	m_render_on_front_done = true;
}

void CViewport::SetIdentityProjection(){
	glViewport(0, 0, m_w, m_h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-0.5, m_w - 0.5, -0.5, m_h - 0.5, 0, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void CViewport::SetXOR(void){
	glGetIntegerv(GL_DRAW_BUFFER, &m_save_buffer_for_XOR);
	glDrawBuffer(GL_FRONT);
	glDepthMask(0);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_LOGIC_OP);
	glLogicOp(GL_XOR);
	glColor3ub(255, 255, 255);
}

void CViewport::EndXOR(void){
	glDisable(GL_COLOR_LOGIC_OP);
	glDepthMask(1);
	glEnable(GL_DEPTH_TEST);
	glDrawBuffer(m_save_buffer_for_XOR);
	glFlush();
}
#endif

void CViewport::OnMouseEvent(MouseEvent& event)
{
	theApp.m_current_viewport = this;
	this->m_need_refresh = false;
	this->m_need_update = false;
	if (event.m_leftDown)
	{
		int a = 0;
		a = 3;
	}
	if (theApp.input_mode_object)theApp.input_mode_object->OnMouse(event);

#if 0
	for (std::list< void(*)(wxMouseEvent&) >::iterator It = theApp.m_lbutton_up_callbacks.begin(); It != theApp.m_lbutton_up_callbacks.end(); It++)
	{
		void(*callbackfunc)(wxMouseEvent& event) = *It;
		(*callbackfunc)(event);
	}
#endif
}

#if 0
void CViewport::OnMagExtents(bool rotate, int margin)
{
	m_view_points.clear();
	if (rotate){
		m_orthogonal = true;
		SetViewPoint(margin);
	}
	else{
		m_view_point.SetViewAroundAllObjects(margin);
		StoreViewPoint();
	}
}
#endif

geoff_geometry::Point3d getClosestOrthogonal(const geoff_geometry::Point3d &v)
{
	double best_dp = 0;
	geoff_geometry::Point3d best_v;

	geoff_geometry::Point3d test_v[6] = { geoff_geometry::Point3d(1, 0, 0), geoff_geometry::Point3d(-1, 0, 0), geoff_geometry::Point3d(0, 1, 0), geoff_geometry::Point3d(0, -1, 0), geoff_geometry::Point3d(0, 0, 1), geoff_geometry::Point3d(0, 0, -1) };
	for (int i = 0; i<6; i++)
	{
		double dp = test_v[i] * v;
		if (dp > best_dp)
		{
			best_dp = dp;
			best_v = test_v[i];
		}
	}
	return best_v;
}

void CViewport::SetViewPoint(int margin){
	if (m_orthogonal){
		geoff_geometry::Point3d vz = getClosestOrthogonal(-m_view_point.forwards_vector());
		geoff_geometry::Point3d v2 = m_view_point.m_vertical;
		v2 = v2 + vz * (-(v2 * vz)); // remove any component in new vz direction	    
		geoff_geometry::Point3d vy = getClosestOrthogonal(v2);
		m_view_point.SetView(vy, vz, margin);
		StoreViewPoint();
		return;
	}

	geoff_geometry::Point3d vy(0, 1, 0), vz(0, 0, 1);
	m_view_point.SetView(vy, vz, margin);
	StoreViewPoint();
}

#if 0
void CViewport::InsertViewBox(const CBox& box)
{
	m_view_point.m_extra_view_box.Insert(box);
}
#endif

void CViewport::StoreViewPoint(void){
	m_view_points.push_back(m_view_point);
}

#if 0
void CViewport::RestorePreviousViewPoint(void){
	if (m_view_points.size()>0){
		m_view_point = m_view_points.back();
		m_view_points.pop_back();
	}
}

void CViewport::DrawObjectsOnFront(const std::list<HeeksObj*> &list, bool do_depth_testing){
	SetViewport();
	m_view_point.SetProjection(do_depth_testing);
	m_view_point.SetModelview();

	glDrawBuffer(GL_FRONT);
	glDepthFunc(GL_LEQUAL);

	theApp.CreateLights();
	glDisable(GL_LIGHTING);
	Material().glMaterial(1.0);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glShadeModel(GL_FLAT);

	m_view_point.SetPolygonOffset();

	for (std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++)
	{
		HeeksObj* object = *It;
		object->glCommands(false, false, false);
	}

	theApp.DestroyLights();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glFlush();
}

void CViewport::FindMarkedObject(const IPoint &point, MarkedObject* marked_object){
	theApp.m_marked_list->FindMarkedObject(point, marked_object);
}

void CViewport::DrawWindow(IRect &rect, bool allow_extra_bits){
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-0.5, m_w - 0.5, -0.5, m_h - 0.5, 0, 10);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	int x1 = rect.x;
	int y1 = rect.y;
	int x2 = rect.x + rect.width;
	int y2 = rect.y + rect.height;

	glBegin(GL_LINE_STRIP);
	glVertex2f((GLfloat)x1, (GLfloat)y2);
	glVertex2f((GLfloat)x1, (GLfloat)y1);
	glVertex2f((GLfloat)x2, (GLfloat)y1);
	glVertex2f((GLfloat)x2, (GLfloat)y2);
	glVertex2f((GLfloat)x1, (GLfloat)y2);
	glEnd();

	// draw extra bits
	if (rect.width < 0 && allow_extra_bits){
		int extra_x = -5;
		int extra_y = -5;
		if (rect.height > 0)extra_y = -extra_y;

		glBegin(GL_LINE_STRIP);
		glVertex2f((GLfloat)x1 - extra_x, (GLfloat)y1);
		glVertex2f((GLfloat)x1 - extra_x, (GLfloat)y1 - extra_y);
		glVertex2f((GLfloat)x1, (GLfloat)y1 - extra_y);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex2f((GLfloat)x2 + extra_x, (GLfloat)y1);
		glVertex2f((GLfloat)x2 + extra_x, (GLfloat)y1 - extra_y);
		glVertex2f((GLfloat)x2, (GLfloat)y1 - extra_y);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex2f((GLfloat)x1 - extra_x, (GLfloat)y2);
		glVertex2f((GLfloat)x1 - extra_x, (GLfloat)y2 + extra_y);
		glVertex2f((GLfloat)x1, (GLfloat)y2 + extra_y);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex2f((GLfloat)x2 + extra_x, (GLfloat)y2);
		glVertex2f((GLfloat)x2 + extra_x, (GLfloat)y2 + extra_y);
		glVertex2f((GLfloat)x2, (GLfloat)y2 + extra_y);
		glEnd();
	}

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

#endif