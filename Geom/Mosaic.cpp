// Mosaic.cpp

// Copyright 2025, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "Mosaic.h"

#include <set>

MosaicNode::MosaicNode(const Point& p)
{
	m_p = p;
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

void MosaicNode::Disconnect(MosaicSpan* span)
{
	for (std::list<MosaicSpanConnector>::const_iterator It = m_span_list.begin(); It != m_span_list.end(); It++)
	{
		const MosaicSpanConnector& connector = *It;
		if (connector.m_span == span)
		{
			m_span_list.erase(It);
			return;
		}
	}
}

Mosaic::Mosaic()
{

}

void Mosaic::Insert(const Span& span, bool reversed)
{
	// find intersections with each existing span
	std::list<Point> all_pts;

	std::list<MosaicSpan> copy_spans = m_spans;
	for (std::list<MosaicSpan>::iterator It = m_spans.begin(); It != m_spans.end(); It++)
	{
		MosaicSpan& mspan = *It;
		std::list<Point> pts;
		mspan.m_span.Intersect(span, pts);

		// split span
		for (std::list<Point>::iterator PIt = pts.begin(); PIt != pts.end(); PIt++)
		{
			Point& p = *PIt;
			if (p == mspan.m_span.m_p || p == mspan.m_span.m_v.m_p)
				continue; // don't split start or end points

			m_nodes.push_back(MosaicNode(p)); // add a new node at the split point
			MosaicNode* new_node = &(m_nodes.back());

			// disconnect end of span
			mspan.m_end_node->Disconnect(&mspan);
			MosaicNode* old_end_node = mspan.m_end_node;
			mspan.m_end_node = new_node;
			mspan.m_span.m_v.m_p = p;
			new_node->Connect(&mspan, false);

			// add mosaic span
			Span span(p, mspan.m_span.m_v);
			m_spans.push_back(MosaicSpan(span, new_node, old_end_node));
			MosaicSpan* new_span = &m_spans.back();
			new_span->m_start_node->Connect(new_span, true);
			new_span->m_end_node->Connect(new_span, false);
		}

		for (std::list<Point>::iterator PIt = pts.begin(); PIt != pts.end(); PIt++)
			all_pts.push_back((*PIt));
	}

	std::list<Span> spans;
	span.Split(all_pts, spans);

	// now each of these span's start or end touch a node or not, but don't cross any spans

	for (std::list<Span>::iterator SpIt = spans.begin(); SpIt != spans.end(); SpIt++)
	{
		Span& span = *SpIt;

		MosaicNode* start_node = NULL;
		FindNode(span.m_p, &start_node);

		if (start_node == NULL)
		{
			m_nodes.push_back(MosaicNode(span.m_p));
			start_node = &(m_nodes.back());
		}

		MosaicNode* end_node = NULL;
		FindNode(span.m_v.m_p, &end_node);

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

void Mosaic::FindNode(const Point& p, MosaicNode** node)
{
	for (std::list<MosaicNode>::iterator It = m_nodes.begin(); It != m_nodes.end(); It++)
	{
		MosaicNode& mnode = *It;
		if (p == mnode.m_p)
		{
			*node = &mnode;
			return;
		}
	}
}

void Mosaic::GetResult(CArea& area, MosaicResultType result_type)
{
	// walk the structure until every span has been walked
	std::list<MosaicSpan*> spans_to_do;
	for (std::list<MosaicSpan>::iterator It = m_spans.begin(); It != m_spans.end(); It++)
	{
		spans_to_do.push_back(&(*It));
	}

	std::set<MosaicSpan*> spans_done;

	while (spans_to_do.size() > 0)
	{
		MosaicSpan* span_to_do = spans_to_do.front();

		// ignore if already processed
		if (spans_done.find(span_to_do) != spans_done.end())
		{
			spans_to_do.pop_front();
			continue;
		}

		spans_done.insert(span_to_do);

		CCurve curve;
		curve.append(span_to_do->m_span.m_p);
		curve.append(span_to_do->m_span.m_v);  // start curve with this span

		MosaicNode* start_node = span_to_do->m_start_node;
		MosaicNode* working_node = span_to_do->m_end_node;

		while (working_node != NULL && working_node != start_node)
		{
			// where to go next

			// find current span on the node
			std::list<MosaicSpanConnector>::iterator It = working_node->m_span_list.begin();
			for (; It != working_node->m_span_list.end(); It++)
			{
				MosaicSpanConnector& connector = *It;
				if (connector.m_span == span_to_do)
				{
					break;
				}
			}

			It++; // move on to the one after the current span
			for (;;It++)
			{
				if (It == working_node->m_span_list.end())
					It = working_node->m_span_list.begin(); // keep looping

				MosaicSpanConnector& connector = *It;
				if (connector.m_span == span_to_do)
				{
					// we've run out of viable spans
					area.append(curve);
					working_node = NULL;
					break;
				}

				if (connector.m_forward)
				{
					// add this span
					curve.append(connector.m_span->m_span.m_v);
					spans_done.insert(connector.m_span);

					// move on to the next node
					working_node = connector.m_span->m_end_node;
					break;
				}
			}
		}

	}
}