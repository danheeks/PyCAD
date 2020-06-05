// ConversionTools.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "ConversionTools.h"
#include "MarkedList.h"
#include "HEllipse.h"
#include "HSpline.h"
#include "Wire.h"
#include "Face.h"
#include "Edge.h"
#include "Shape.h"
#include "strconv.h"
#include "Curve.h"

#include <sstream>
#include <vector>
#include <algorithm>
#include <functional>


static gp_Pnt GetStart(const TopoDS_Edge &edge)
{
    BRepAdaptor_Curve curve(edge);
    double uStart = curve.FirstParameter();
    gp_Pnt PS;
    gp_Vec VS;
    curve.D1(uStart, PS, VS);

    return(PS);
}

static gp_Pnt GetEnd(const TopoDS_Edge &edge)
{
    BRepAdaptor_Curve curve(edge);
    double uEnd = curve.LastParameter();
    gp_Pnt PE;
    gp_Vec VE;
    curve.D1(uEnd, PE, VE);

    return(PE);
}

struct EdgeComparison : public std::binary_function<const TopoDS_Edge &, const TopoDS_Edge &, bool >
{
    EdgeComparison( const TopoDS_Edge & edge )
    {
        m_reference_edge = edge;
    }

    bool operator()( const TopoDS_Edge & lhs, const TopoDS_Edge & rhs ) const
    {

        std::vector<double> lhs_distances;
        lhs_distances.push_back( GetStart(m_reference_edge).Distance( GetStart(lhs) ) );
        lhs_distances.push_back( GetStart(m_reference_edge).Distance( GetEnd(lhs) ) );
        lhs_distances.push_back( GetEnd(m_reference_edge).Distance( GetStart(lhs) ) );
        lhs_distances.push_back( GetEnd(m_reference_edge).Distance( GetEnd(lhs) ) );
        std::sort(lhs_distances.begin(), lhs_distances.end());

        std::vector<double> rhs_distances;
        rhs_distances.push_back( GetStart(m_reference_edge).Distance( GetStart(rhs) ) );
        rhs_distances.push_back( GetStart(m_reference_edge).Distance( GetEnd(rhs) ) );
        rhs_distances.push_back( GetEnd(m_reference_edge).Distance( GetStart(rhs) ) );
        rhs_distances.push_back( GetEnd(m_reference_edge).Distance( GetEnd(rhs) ) );
        std::sort(rhs_distances.begin(), rhs_distances.end());

        return(*(lhs_distances.begin()) < *(rhs_distances.begin()));
    }

    TopoDS_Edge m_reference_edge;
};

void SortEdges( std::vector<TopoDS_Edge> & edges )
{
	for (std::vector<TopoDS_Edge>::iterator l_itEdge = edges.begin(); l_itEdge != edges.end(); l_itEdge++)
    {
		// We've already begun.  Just sort based on the previous point's location.
		std::vector<TopoDS_Edge>::iterator l_itNextEdge = l_itEdge;
		l_itNextEdge++;

		if (l_itNextEdge != edges.end())
		{
			EdgeComparison compare( *l_itEdge );
			std::sort( l_itNextEdge, edges.end(), compare );
		} // End if - then
    } // End for
} // End SortEdges() method

static gp_Circ HArcGetCircle(HeeksObj* object)
{
	Point3d c, a;
	object->GetCentrePoint(c);
	object->GetAxis(a);
	gp_Ax2 axis(P2G(c), P2D(a));
	double r;
	object->GetRadius(r);
	return gp_Circ(axis, r);
}

bool ConvertLineArcsToWire2(const std::list<HeeksObj *> &list, TopoDS_Wire &wire)
{
	std::vector<TopoDS_Edge> edges;
	std::list<HeeksObj*> list2;
	std::list<HeeksObj*>::const_iterator It;
	for(It = list.begin(); It != list.end(); It++){
		HeeksObj* object = *It;
		if(object->GetType() == SketchType){
			for(HeeksObj* child = object->GetFirstChild(); child; child = object->GetNextChild())
			{
				list2.push_back(child);
			}
		}
		else{
			list2.push_back(object);
		}
	}

	for(std::list<HeeksObj*>::iterator It = list2.begin(); It != list2.end(); It++){
		HeeksObj* object = *It;
		int t = object->GetType();
		if (t == LineType)
		{
			Point3d s, e;
			object->GetStartPoint(s);
			object->GetEndPoint(e);
			edges.push_back(BRepBuilderAPI_MakeEdge(P2G(s), P2G(e)));
		}
		else if (t == ArcType)
		{
			Point3d s, e, c, a;
			object->GetStartPoint(s);
			object->GetEndPoint(e);
			edges.push_back(BRepBuilderAPI_MakeEdge(HArcGetCircle(object), P2G(s), P2G(e)));
		}
		else if (t == HSpline::m_type)
		{
			HSpline* spline = (HSpline*)object;
			edges.push_back(BRepBuilderAPI_MakeEdge(spline->m_spline));
		}
	}

	if(edges.size() > 0){
		wire = EdgesToWire(edges);
		return true;
	}

	return false;
}


bool ConvertSketchToEdges(HeeksObj *object, std::list< std::vector<TopoDS_Edge> > &edges)
{
    std::list<HeeksObj*> line_arc_list;

	if(object->GetType() == SketchType)
	{
		std::list<HeeksObj*> new_separate_sketches;
		theApp->SketchSplit(object, new_separate_sketches);
		if(new_separate_sketches.size() > 1)
		{
			// call again with each separate sketch
			for(std::list<HeeksObj*>::iterator It = new_separate_sketches.begin(); It != new_separate_sketches.end(); It++)
			{
				HeeksObj* sketch = *It;
				if(!ConvertSketchToEdges(sketch, edges))return false;
			}
			return true;
		}
		else
		{
			for(HeeksObj* child = object->GetFirstChild(); child; child = object->GetNextChild())
			{
				line_arc_list.push_back(child);
			}
			std::vector<TopoDS_Edge> empty_list;
			edges.push_back(empty_list);
		}
	}
	else
	{
		line_arc_list.push_back(object);
	}

	if(edges.size() == 0)
	{
		std::vector<TopoDS_Edge> empty_list;
		edges.push_back(empty_list);
	}

	const double max_tolerance = 10.0;
	for(std::list<HeeksObj*>::const_iterator It = line_arc_list.begin(); It != line_arc_list.end(); It++){
	    try {
            HeeksObj* object = *It;
			if (object->GetType() == LineType){
				bool done = false;
				double tolerance = TOLERANCE;
				while ((!done) && (tolerance < max_tolerance))
				{
					Point3d s, e;
					object->GetStartPoint(s);
					object->GetEndPoint(e);
					if (!(s == e))
					{
						BRep_Builder aBuilder;
						TopoDS_Vertex start, end;

						aBuilder.MakeVertex(start, P2G(s), TOLERANCE);
						start.Orientation(TopAbs_REVERSED);

						aBuilder.MakeVertex(end, P2G(e), TOLERANCE);
						end.Orientation(TopAbs_FORWARD);

						BRepBuilderAPI_MakeEdge edge(start, end);
						if (!edge.IsDone())
						{	// return(false);
							tolerance *= 10.0;
						}
						else
						{
							edges.back().push_back(edge.Edge());
							done = true;
						}
					}
					else
					{
						break;
					}

					if (!done)
					{
						return(false);
					}
				} // End while
			}
			else if (object->GetType() == ArcType)
			{
				bool done = false;
				double tolerance = TOLERANCE;
				while ((!done) && (tolerance < max_tolerance))
				{
					Point3d s, e;
					object->GetStartPoint(s);
					object->GetEndPoint(e);

					BRep_Builder aBuilder;
					TopoDS_Vertex start, end;

					aBuilder.MakeVertex(start, P2G(s), TOLERANCE);
					start.Orientation(TopAbs_REVERSED);

					aBuilder.MakeVertex(end, P2G(e), TOLERANCE);
					end.Orientation(TopAbs_FORWARD);

					BRepBuilderAPI_MakeEdge edge(HArcGetCircle(object), start, end);
					if (!edge.IsDone())
					{
						// return(false);
						tolerance *= 10.0;
					}
					else
					{
						edges.back().push_back(edge.Edge());
						done = true;
					}
				} // End while

				if (!done)
				{
					return(false);
				}
			}
                    
			else if (object->GetType() == CircleType)
            {
                edges.back().push_back(BRepBuilderAPI_MakeEdge(HArcGetCircle(object)));
            }
			else if (object->GetType() == HEllipse::m_type)
            {
                HEllipse* ellipse = (HEllipse*)object;
                edges.back().push_back(BRepBuilderAPI_MakeEdge(ellipse->GetEllipse()));
            }
			else if (object->GetType() == HSpline::m_type)
            {
                HSpline* spline = (HSpline*)object;
                edges.back().push_back(BRepBuilderAPI_MakeEdge(spline->m_spline));
            }
			else
			{
				std::wstring message;
				message.append(L"Cannot convert object type ");
				message.append(object->GetTypeString());
				message.append(L" to edge");
				theApp->DoMessageBox(message.c_str());
				return(false);
			}
	    } // End try
	    catch(Standard_Failure)
	    {
	        Handle_Standard_Failure e = Standard_Failure::Caught();
			theApp->DoMessageBox((std::wstring(L"Error converting sketch to face: ") + Ctt(e->GetMessageString())).c_str());
			return false;
	    }
	}

    return(true);
}

TopoDS_Wire EdgesToWire(const std::vector<TopoDS_Edge> &edges)
{
	BRepBuilderAPI_MakeWire wire_maker;
	for(std::vector<TopoDS_Edge>::const_iterator It = edges.begin(); It != edges.end(); It++)
	{
		const TopoDS_Edge &edge = *It;
		wire_maker.Add(edge);
	}

	return wire_maker.Wire();
}

bool SketchToWires(HeeksObj* sketch, std::list<TopoDS_Wire> &wire_list)
{
    std::list< std::vector<TopoDS_Edge> > edges_list;

    if (!ConvertSketchToEdges(sketch, edges_list))
        return false;

	for(std::list< std::vector<TopoDS_Edge> >::iterator It = edges_list.begin(); It != edges_list.end(); It++)
	{
		std::vector<TopoDS_Edge> &edges = *It;
		wire_list.push_back(EdgesToWire(edges));
	}
	return true;
}

bool ConvertEdgesToFaceOrWire(const std::vector<TopoDS_Edge> &edges, std::list<TopoDS_Shape> &face_or_wire, bool face_not_wire)
{
	// It's not enough to add the edges to the wire in an arbitrary order.  If the adjacent edges
	// don't connect then the wire ends up losing one of the edges.  We must sort the edge objects
	// so that they're connected (or best we can) before constructing the TopoDS_Wire object from
	// them.
	// So, please call SortEdges before getting to here.
	try
	{
		TopoDS_Wire wire = EdgesToWire(edges);

		if(face_not_wire)
		{
			BRepBuilderAPI_MakeFace make_face(wire);
			if(make_face.IsDone() == Standard_False)face_or_wire.push_back(wire);
			else face_or_wire.push_back(make_face.Face());
		}
		else
		{
			face_or_wire.push_back(wire);
		}
	}
	catch (Standard_Failure) {
		Handle_Standard_Failure e = Standard_Failure::Caught();
		std::wstring message(L"Error converting sketch to face: ");
		message.append(Ctt(e->GetMessageString()));
		theApp->DoMessageBox(message.c_str());
		return false;
	}
	catch(...)
	{
		theApp->DoMessageBox(L"Fatal Error converting sketch to face");
		return false;
	}

	return true;
}

bool ConvertSketchToFaceOrWire(HeeksObj* object, std::list<TopoDS_Shape> &face_or_wire, bool face_not_wire)
{
    std::list< std::vector<TopoDS_Edge> > edges;

    if (! ConvertSketchToEdges(object, edges))
    {
        return(false);
    }

	for(std::list< std::vector<TopoDS_Edge> >::iterator It = edges.begin(); It != edges.end(); It++)
	{
		std::vector<TopoDS_Edge> &list = *It;
		if(list.size() > 0)
		{
			SortEdges(list);
			if(!ConvertEdgesToFaceOrWire(list, face_or_wire, face_not_wire))return false;
		}
	}

	return true;
}

bool ConvertFaceToSketch2(const TopoDS_Face& face, HeeksObj* sketch, double deviation)
{
	// given a face, this adds lines and arcs to the given sketch
	// loop through all the loops
	TopoDS_Wire outerWire=BRepTools::OuterWire(face);

	for (TopExp_Explorer expWire(face, TopAbs_WIRE); expWire.More(); expWire.Next())
	{
		const TopoDS_Shape &W = expWire.Current();
		for(BRepTools_WireExplorer expEdge(TopoDS::Wire(W)); expEdge.More(); expEdge.Next())
		{
			const TopoDS_Shape &E = expEdge.Current();
			if(!ConvertEdgeToSketch2(TopoDS::Edge(E), sketch, deviation))return false;
		}
	}

	return true; // success
}


bool ConvertWireToSketch(const TopoDS_Wire& wire, HeeksObj* sketch, double deviation)
{
    const TopoDS_Shape &W = wire;
    for(BRepTools_WireExplorer expEdge(TopoDS::Wire(W)); expEdge.More(); expEdge.Next())
    {
        const TopoDS_Shape &E = expEdge.Current();
        if(!ConvertEdgeToSketch2(TopoDS::Edge(E), sketch, deviation))return false;
    }

	return true; // success
}



bool ConvertEdgeToSketch2(const TopoDS_Edge& edge, HeeksObj* sketch, double deviation, bool reverse)
{
	// enum GeomAbs_CurveType
	// 0 - GeomAbs_Line
	// 1 - GeomAbs_Circle
	// 2 - GeomAbs_Ellipse
	// 3 - GeomAbs_Hyperbola
	// 4 - GeomAbs_Parabola
	// 5 - GeomAbs_BezierCurve
	// 6 - GeomAbs_BSplineCurve
	// 7 - GeomAbs_OtherCurve

	BRepAdaptor_Curve curve(edge);
	GeomAbs_CurveType curve_type = curve.GetType();
	bool sense = (edge.Orientation() == TopAbs_FORWARD);

	if(reverse)sense = !sense;

	switch(curve_type)
	{
		case GeomAbs_Line:
			// make a line
		{
			double uStart = curve.FirstParameter();
			double uEnd = curve.LastParameter();
			gp_Pnt PS;
			gp_Vec VS;
			curve.D1(uStart, PS, VS);
			gp_Pnt PE;
			gp_Vec VE;
			curve.D1(uEnd, PE, VE);
			HeeksObj* new_object = theApp->CreateNewLine(G2P(sense ? PS : PE), G2P(sense ? PE : PS));
			sketch->Add(new_object, NULL);
		}
		break;

		case GeomAbs_Circle:
			// make an arc
		{
			double uStart = curve.FirstParameter();
			double uEnd = curve.LastParameter();
			gp_Pnt PS;
			gp_Vec VS;
			curve.D1(uStart, PS, VS);
			gp_Pnt PE;
			gp_Vec VE;
			curve.D1(uEnd, PE, VE);
			gp_Circ circle = curve.Circle();
			gp_Ax1 axis = circle.Axis();
			if(!sense)
			{
				axis.SetDirection(-axis.Direction());
				circle.SetAxis(axis);
			}

			if(curve.IsPeriodic())
			{
				double period = curve.Period();
				double uHalf = uStart + period/2;
				gp_Pnt PH;
				gp_Vec VH;
				curve.D1(uHalf, PH, VH);
				{
					HeeksObj* new_object = theApp->CreateNewArc(G2P(PS), G2P(PH), D2P(circle.Axis().Direction()), G2P(circle.Location()));
					sketch->Add(new_object, NULL);
				}
				{
					HeeksObj* new_object = theApp->CreateNewArc(G2P(PH), G2P(PE), D2P(circle.Axis().Direction()), G2P(circle.Location()));
					sketch->Add(new_object, NULL);
				}
			}
			else
			{
				HeeksObj* new_object = theApp->CreateNewArc(G2P(sense ? PS : PE), G2P(sense ? PE : PS), D2P(circle.Axis().Direction()), G2P(circle.Location()));
				sketch->Add(new_object, NULL);
			}
		}
		break;

		case GeomAbs_BSplineCurve:
			{
				std::list<HeeksObj*> new_spans;
#if 0
				// to do
				HSpline::ToBiarcs(curve.BSpline(), new_spans, deviation, curve.FirstParameter(), curve.LastParameter());
#endif
				if(sense)
				{
					for(std::list<HeeksObj*>::iterator It = new_spans.begin(); It != new_spans.end(); It++)
						sketch->Add(*It, NULL);
				}
				else
				{
					for(std::list<HeeksObj*>::reverse_iterator It = new_spans.rbegin(); It != new_spans.rend(); It++)
					{
						HeeksObj* object = *It;
						object->Reverse();
						sketch->Add(object, NULL);
					}
				}
			}
			break;


		default:
		{
			// make lots of small lines
			BRepTools::Clean(edge);
			BRepMesh_IncrementalMesh(edge, deviation);

			TopLoc_Location L;
			Handle(Poly_Polygon3D) Polyg = BRep_Tool::Polygon3D(edge, L);
			if (!Polyg.IsNull()) {
				const TColgp_Array1OfPnt& Points = Polyg->Nodes();
				Standard_Integer po;
				gp_Pnt prev_p;
				int i = 0;
				for (po = sense ? Points.Lower():Points.Upper(); (sense && (po <= Points.Upper())) || (!sense && (po >= Points.Lower())); i++) {
					gp_Pnt p = (Points.Value(po)).Transformed(L);
					if(i != 0)
					{
						HeeksObj* new_object = theApp->CreateNewLine(G2P(prev_p), G2P(p));
						sketch->Add(new_object, NULL);
					}
					prev_p = p;
					if(sense)po++;
					else po--;
				}
			}
		}
		break;
	}

	return true;
}

static void AddLineOrArc(HeeksObj* sketch, Span &span)
{
	if(span.m_v.m_type == 0)
	{
		HeeksObj* new_object = theApp->CreateNewLine(Point3d(span.m_p.x, span.m_p.y, 0), Point3d(span.m_v.m_p.x, span.m_v.m_p.y, 0));
		sketch->Add(new_object, NULL);
	}
	else
	{
		Point3d axis = (span.m_v.m_type > 0) ? Point3d(0, 0, 1) : Point3d(0, 0, -1);
		Point3d c(span.m_v.m_c);
		HeeksObj* new_object = theApp->CreateNewArc(Point3d(span.m_p.x, span.m_p.y, 0), Point3d(span.m_v.m_p.x, span.m_v.m_p.y, 0), axis, c);
		sketch->Add(new_object, NULL);
	}
}
