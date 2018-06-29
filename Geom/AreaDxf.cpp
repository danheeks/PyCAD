// AreaDxf.cpp
// Copyright (c) 2011, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "AreaDxf.h"
#include "Area.h"

AreaDxfRead::AreaDxfRead(CArea* area, const char* filepath) :CDxfRead(filepath), m_area(area), m_add_reversed(false), m_add_at_front(false){}

bool AreaDxfRead::StartCurveIfNecessary(const double* s, const double* e)
{
	bool start_a_new_curve = false;
	m_add_reversed = false;
	m_add_at_front = false;
	Point ps(s);

	if ((m_area->m_curves.size() == 0) || (m_area->m_curves.back().m_vertices.size() == 0))
	{
		start_a_new_curve = true;
	}
	else
	{
		if (m_area->m_curves.back().m_vertices.back().m_p != ps)
		{
			Point pe(e);
			if (m_area->m_curves.back().m_vertices.back().m_p == pe)
			{
				m_add_reversed = true;
			}
			else
			{
				if (m_area->m_curves.back().m_vertices.front().m_p == pe)
				{
					m_add_at_front = true;
				}
				else if (m_area->m_curves.back().m_vertices.front().m_p == ps)
				{
					m_add_at_front = true;
					m_add_reversed = true;
				}
				else
					start_a_new_curve = true;
			}
		}
	}

	if (start_a_new_curve)
	{
		// start a new curve
		m_area->m_curves.push_back(CCurve());
		m_area->m_curves.back().m_vertices.push_back(ps);
	}

	return start_a_new_curve;
}

void AreaDxfRead::OnReadLine(const double* s, const double* e)
{
	StartCurveIfNecessary(s, e);
	if (m_add_at_front)
	{
		if (m_add_reversed)
			m_area->m_curves.back().m_vertices.push_front(Point(e));
		else
			m_area->m_curves.back().m_vertices.push_front(Point(s));
	}
	else
	{
		if (m_add_reversed)
			m_area->m_curves.back().m_vertices.push_back(Point(s));
		else
			m_area->m_curves.back().m_vertices.push_back(Point(e));
	}
}

void AreaDxfRead::OnReadArc(const double* s, const double* e, const double* c, bool dir)
{
	StartCurveIfNecessary(s, e);
	if (m_add_at_front)
	{
		m_area->m_curves.back().m_vertices.front().m_c = Point(c);
		m_area->m_curves.back().m_vertices.front().m_type = (dir == m_add_reversed) ? -1 : 1;
		if (m_add_reversed)
			m_area->m_curves.back().m_vertices.push_front(Point(e));
		else
			m_area->m_curves.back().m_vertices.push_front(Point(s));
	}
	else
	{
		if (m_add_reversed)
			m_area->m_curves.back().m_vertices.push_back(CVertex(dir ? -1 : 1, Point(s), Point(c)));
		else
			m_area->m_curves.back().m_vertices.push_back(CVertex(dir ? 1 : -1, Point(e), Point(c)));
	}
}
