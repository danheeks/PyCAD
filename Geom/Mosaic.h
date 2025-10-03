#pragma once

// Mosaic is a network of Spans ( lines or arcs ) used for doing Area boolean operations

#include "Area.h"
#include "Curve.h"

class MosaicSpan;

class MosaicSpanConnector
{
public:
	MosaicSpan* m_span;
	bool m_forward; // true if span connects node at start

	MosaicSpanConnector(MosaicSpan* span, bool forward);
};

class MosaicNode
{
public:
	std::list<MosaicSpanConnector> m_span_list; // ordered spans which meet at this node, ordered anti-clockwise
	Point m_p;

	MosaicNode(const Point& p);
	void Connect(MosaicSpan *span, bool forward);
	void Disconnect(MosaicSpan* span);
};

class MosaicSpan
{
public:
	Span m_span;
	MosaicNode* m_start_node;
	MosaicNode* m_end_node;

	MosaicSpan(const Span& span, MosaicNode* start_node, MosaicNode* end_node);
};

enum MosaicResultType
{
	MosaicResultTypeTurnRight,
	MosaicResultTypeTurnLeft,
};

class Mosaic
{
	std::list<MosaicNode> m_nodes;
	std::list<MosaicSpan> m_spans;
public:
	Mosaic();

	void Insert(const CArea& area, bool reversed = false);
	void Insert(const Span& span, bool reversed);
	void GetResult(CArea& area, MosaicResultType result_type = MosaicResultTypeTurnRight);
	void FindNode(const Point& p, MosaicNode** node);
};