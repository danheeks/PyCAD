// Mosaic.cpp

// Copyright 2025, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "Mosaic.h"

#include <set>

MosaicNode::MosaicNode(const Point& p)
{

}

MosaicSpanConnector::MosaicSpanConnector(MosaicSpan* span, bool forward)
{
	m_span = span;
	m_forward = forward;
}

MosaicSpan::MosaicSpan(const Span& span, MosaicNode* start_node, MosaicNode* end_node)
{
	m_span = span;
	m_start_node = start_node;
	m_end_node = end_node;
}

void MosaicNode::Connect(MosaicSpan* span, bool forward)
{
	Point direction;
	if (forward)direction = span->m_span.GetVector(0.0);
	else direction = -span->m_span.GetVector(1.0);
	double this_angle = atan2(direction.y, direction.x);

	for (std::list<MosaicSpanConnector>::const_iterator It = m_span_list.begin(); It != m_span_list.end(); It++)
	{
		const MosaicSpanConnector &connector = *It;
		Point direction;
		if (connector.m_forward)direction = connector.m_span->m_span.GetVector(0.0);
		else direction = -connector.m_span->m_span.GetVector(1.0);

		double angle = atan2(direction.y, direction.x);

		if (this_angle < angle)
		{
			m_span_list.insert(It, MosaicSpanConnector(span, forward));
			return;
		}
	}

	m_span_list.push_back(MosaicSpanConnector(span, forward));
}


Mosaic::Mosaic()
{

}

void Mosaic::Insert(const Span& span, bool reversed)
{
	// find intersections with each existing span
	std::list<Point> pts;
	for (std::list<MosaicSpan>::iterator It = m_spans.begin(); It != m_spans.end(); It++)
	{
		MosaicSpan& mspan = *It;
		mspan.m_span.Intersect(span, pts);
	}

	std::list<Span> spans;
	span.Split(pts, spans);

	// now each of these span's start or end touch a node or not, but don't cross any spans

	for (std::list<Span>::iterator SpIt = spans.begin(); SpIt != spans.end(); SpIt++)
	{
		Span& span = *SpIt;

		MosaicNode* start_node = NULL;
		FindNode(span.m_p, start_node);

		if (start_node == NULL)
		{
			m_nodes.push_back(MosaicNode(span.m_p));
			start_node = &(m_nodes.back());
		}

		MosaicNode* end_node = NULL;
		FindNode(span.m_v.m_p, end_node);

		if (end_node == NULL)
		{
			m_nodes.push_back(MosaicNode(span.m_v.m_p));
			end_node = &(m_nodes.back());
		}

		// add mosaic span
		MosaicSpan mspan(span, start_node, end_node);
		m_spans.push_back(mspan);
		MosaicSpan* pspan = &m_spans.back();
		start_node->Connect(pspan, true);
		end_node->Connect(pspan, false);
	}
}

void Mosaic::Insert(const CArea& area, bool reversed)
{
	for (std::list<CCurve>::const_iterator It = area.m_curves.begin(); It != area.m_curves.end(); It++)
	{
		const CCurve& curve = *It;
		std::list<Span> spans;
		curve.GetSpans(spans);
		for (std::list<Span>::iterator SpIt = spans.begin(); SpIt != spans.end(); SpIt++)
		{
			Span& span = *SpIt;
			Insert(span, reversed);
		}
	}
}

void Mosaic::FindNode(const Point& p, MosaicNode* node)
{
	for (std::list<MosaicNode>::iterator It = m_nodes.begin(); It != m_nodes.end(); It++)
	{
		MosaicNode& mnode = *It;
		if (p == mnode.m_p)
		{
			node = &mnode;
			return;
		}
	}
}

void Mosaic::GetResult(CArea& area, MosaicResultType result_type)
{
	// walk the structure until every span has been walked
	std::list<MosaicSpan*> m_spans_to_do;
	for (std::list<MosaicSpan>::iterator It = m_spans.begin(); It != m_spans.end(); It++)
	{
		m_spans_to_do.push_back(&(*It));
	}

	std::set<MosaicSpan*> spans_done;

	while (m_spans_to_do.size() > 0)
	{
		MosaicSpan* span_to_do = m_spans_to_do.front();

		// ignore if already processed
		if (spans_done.find(span_to_do) != spans_done.end())
		{
			m_spans_to_do.pop_front();
			continue;
		}

		spans_done.insert(span_to_do);

		CCurve curve;
		curve.append(span_to_do->m_span.m_p);
		curve.append(span_to_do->m_span.m_v);  // start curve with this span

		MosaicNode* start_node = span_to_do->m_start_node;
		MosaicNode* working_node = start_node;

		while (1)
		{
			// where to go next
			std::list<MosaicSpanConnector>::reverse_iterator FindIt = working_node->m_span_list.rend();
			for (std::list<MosaicSpanConnector>::reverse_iterator It = working_node->m_span_list.rbegin(); It != working_node->m_span_list.rend(); It++)
			{
				MosaicSpanConnector& connector = *It;
				if (connector.m_span == span_to_do)
				{

				}
			}
		}

	}
}