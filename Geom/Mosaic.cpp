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

void Mosaic::CopySpanPointers(std::list<MosaicSpan*>& copy_spans)
{
	for (std::list<MosaicSpan>::iterator It = m_spans.begin(); It != m_spans.end(); It++)
	{
		MosaicSpan* mspan = &(*It);
		copy_spans.push_back(mspan);
	}
}

void Mosaic::Insert(const Span& span)
{
	// find intersections with each existing span
	std::list<Point> all_pts;

	std::list<MosaicSpan*> copy_spans;
	CopySpanPointers(copy_spans);

	for (std::list<MosaicSpan*>::iterator It = copy_spans.begin(); It != copy_spans.end(); It++)
	{
		MosaicSpan* mspan = *It;
		std::list<Point> pts;
		mspan->m_span.Intersect(span, pts);

		// split span
		for (std::list<Point>::iterator PIt = pts.begin(); PIt != pts.end(); PIt++)
		{
			Point& p = *PIt;
			if (p == mspan->m_span.m_p || p == mspan->m_span.m_v.m_p)
				continue; // don't split start or end points

			m_nodes.push_back(MosaicNode(p)); // add a new node at the split point
			MosaicNode* new_node = &(m_nodes.back());

			// disconnect end of span
			mspan->m_end_node->Disconnect(mspan);
			MosaicNode* old_end_node = mspan->m_end_node;
			CVertex old_end_vertex = mspan->m_span.m_v;
			mspan->m_end_node = new_node;
			mspan->m_span.m_v.m_p = p;
			new_node->Connect(mspan, false);

			// add mosaic span
			Span span(p, old_end_vertex);
			m_spans.push_back(MosaicSpan(span, new_node, old_end_node));
			mspan = &m_spans.back();
			mspan->m_start_node->Connect(mspan, true);
			mspan->m_end_node->Connect(mspan, false);
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
			if (reversed)span.Reverse();
			Insert(span);
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

static std::list<MosaicSpanConnector>::iterator WorkingNodeSpanIt;
static std::list<MosaicSpanConnector>::reverse_iterator WorkingNodeSpanReverseIt;
static MosaicResultType WorkingNodeResultType;
static MosaicNode* StartNode;
static MosaicNode* WorkingNode;
static MosaicSpan* SpanToDo;

static void IncrementIterator()
{
	if(WorkingNodeResultType == MosaicResultTypeTurnLeft)
	{
		WorkingNodeSpanReverseIt++;
		if (WorkingNodeSpanReverseIt == WorkingNode->m_span_list.rend())
			WorkingNodeSpanReverseIt = WorkingNode->m_span_list.rbegin(); // keep looping
	}
	else
	{
		WorkingNodeSpanIt++;
		if (WorkingNodeSpanIt == WorkingNode->m_span_list.end())
			WorkingNodeSpanIt = WorkingNode->m_span_list.begin(); // keep looping
	}
}

static void SetStartIterator()
{
	if (WorkingNodeResultType == MosaicResultTypeTurnLeft)
	{
		WorkingNodeSpanReverseIt = WorkingNode->m_span_list.rbegin();
		for (; WorkingNodeSpanReverseIt != WorkingNode->m_span_list.rend(); WorkingNodeSpanReverseIt++)
		{
			MosaicSpanConnector& connector = *WorkingNodeSpanReverseIt;
			if (connector.m_span == SpanToDo)
			{
				break;
			}
		}
	}
	else
	{
		WorkingNodeSpanIt = WorkingNode->m_span_list.begin();
		for (; WorkingNodeSpanIt != WorkingNode->m_span_list.end(); WorkingNodeSpanIt++)
		{
			MosaicSpanConnector& connector = *WorkingNodeSpanIt;
			if (connector.m_span == SpanToDo)
			{
				break;
			}
		}
	}

	IncrementIterator(); // move on to the one after the current span
}

static MosaicSpanConnector& CurrentConnector()
{
	if (WorkingNodeResultType == MosaicResultTypeTurnLeft)
	{
		return *WorkingNodeSpanReverseIt;
	}
	else
	{
		return *WorkingNodeSpanIt;
	}
}

void Mosaic::GetResult(CArea& area, MosaicResultType result_type)
{
	// walk the structure until every span has been walked
	std::list<MosaicSpan*> spans_to_do;
	CopySpanPointers(spans_to_do);

	std::set<MosaicSpan*> spans_done;
	WorkingNodeResultType = result_type;

	while (spans_to_do.size() > 0)
	{
		SpanToDo = spans_to_do.front();

		// ignore if already processed
		if (spans_done.find(SpanToDo) != spans_done.end()){ spans_to_do.pop_front(); continue; }

		// mark as done
		spans_done.insert(SpanToDo);

		// start curve with this span
		CCurve curve;
		curve.append(SpanToDo->m_span.m_p);
		curve.append(SpanToDo->m_span.m_v);

		StartNode = SpanToDo->m_start_node;
		WorkingNode = SpanToDo->m_end_node;

		while (WorkingNode != NULL)
		{
			// where to go next

			// find current span on the node
			SetStartIterator();

			for (;;IncrementIterator())
			{
				MosaicSpanConnector& connector = CurrentConnector();

				if (connector.m_forward)
				{
					// add this span
					curve.append(connector.m_span->m_span.m_v);
					spans_done.insert(connector.m_span);

					// move on to the next node
					WorkingNode = connector.m_span->m_end_node;
					SpanToDo = connector.m_span;

					if (WorkingNode == StartNode)
					{
						area.append(curve);
						WorkingNode = NULL;
					}
					break;
				}
				else
				{
					// no entry. We started badly
					WorkingNode = NULL;
					break;
				}
			}
		}

	}
}