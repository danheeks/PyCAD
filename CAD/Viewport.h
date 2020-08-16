
#pragma once

#include "ViewPoint.h"
#include "MouseEvent.h"

class CViewport
{
protected:
	bool m_LButton;
	IPoint m_CurrentPoint;
	bool m_render_on_front_done;
	int m_save_buffer_for_XOR;
	std::list<CViewPoint> m_view_points;
	bool m_frozen;
	bool m_refresh_wanted_on_thaw;
	int m_w;
	int m_h;

public:
	CViewPoint m_view_point;
	bool m_orthogonal;
	bool m_need_update;
	bool m_need_refresh;

	CViewport();
	CViewport(int w, int h);

	void SetViewport();
	void glCommands();
	void ClearViewpoints();
	void SetViewPoint(int margin);
	void InsertViewBox(const CBox& box);
	void StoreViewPoint(void);
	void RestorePreviousViewPoint(void);
	void DrawObjectsOnFront(const std::list<HeeksObj*> &list, bool do_depth_testing = false);
	void DrawFront(void);
	void EndDrawFront(void);
	void FrontRender(void);
	void SetIdentityProjection();
	void SetXOR(void);
	void EndXOR(void);
	void DrawWindow(IRect &rect, bool allow_extra_bits); // extra bits are added to the corners when dragging from right to left
	void WidthAndHeightChanged(int w, int h){ m_w = w; m_h = h; }
	IPoint GetViewportSize(){ return IPoint(m_w, m_h); }
	void GetViewportSize(int *w, int *h){ *w = m_w; *h = m_h; }
	int GetWidth(){ return m_w; }
	int GetHeight(){ return m_h; }
	void OnMouseEvent(MouseEvent& event);
	void OnMagExtents(bool rotate, int margin);
	void WindowMag(IRect &window_box);
	void OnWheelRotation(int wheelRotation, int x, int y);
};