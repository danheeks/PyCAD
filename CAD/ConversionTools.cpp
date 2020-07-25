#include "stdafx.h"
#include "ConversionTools.h"
#include "../Geom/geometry.h"
#include "../Geom/Area.h"
#include "Sketch.h"
#include "HCircle.h"
#include "HLine.h"
#include "HArc.h"

static CArea* area_to_add_to = NULL;
static CCurve* curve_to_add_to = NULL;

static void MakeNewCurveIfNecessary(HeeksObj* object)
{
	if (curve_to_add_to)
	{
		// does this object join the end of the current curve
		Point3d s;
		if (!object->GetStartPoint(s) || (Point(s.x, s.y) != curve_to_add_to->m_vertices.back().m_p))
		{
			// it doesn't join. we need a new curve
			curve_to_add_to = NULL;
		}
	}

	if (curve_to_add_to == NULL)
	{
		// make a new curve
		area_to_add_to->m_curves.push_back(CCurve());
		curve_to_add_to = &(area_to_add_to->m_curves.back());

		// add the start point of this object
		Point3d s;
		if (object->GetStartPoint(s))
		{
			curve_to_add_to->append(Point(s.x, s.y));
		}
	}
}

static void AddObjectToArea(HeeksObj* object)
{
	switch (object->GetType())
	{
	case SketchType:
	{
		for (HeeksObj* child = ((CSketch*)object)->GetFirstChild(); child; child = ((CSketch*)object)->GetNextChild())
		{
			AddObjectToArea(child);
		}
	}
	break;

	case LineType:
	{
		MakeNewCurveIfNecessary(object);
		Point3d e;
		if (object->GetEndPoint(e))curve_to_add_to->append(Point(e.x, e.y));
	}
	break;
	case ArcType:
	{
		MakeNewCurveIfNecessary(object);
		Point3d e, c;
		if (object->GetEndPoint(e) && object->GetCentrePoint(c))
		{
			int dir = (((HArc*)object)->m_axis.z > 0.0) ? 1 : -1;
			curve_to_add_to->append(CVertex(dir, Point(e.x, e.y), Point(c.x, c.y)));
		}
	}
	break;
	case CircleType:
	{
		// add a couple of arcs
		Point3d c;
		if (object->GetCentrePoint(c))
		{
			MakeNewCurveIfNecessary(object);
			Point pc(c.x, c.y);
			double radius = ((HCircle*)object)->GetDiameter() * 0.5;
			Point p0 = pc + Point(radius, 0.0);
			Point p1 = pc + Point(-radius, 0.0);
			curve_to_add_to->append(p0);
			curve_to_add_to->append(CVertex(1, p1, pc));
			curve_to_add_to->append(CVertex(1, p0, pc));
		}
	}
	break;
	}
}

void ObjectsToArea(const std::list<HeeksObj*> &objects, CArea& area, std::list<HeeksObj*> *objects_to_delete)
{
	area_to_add_to = &area;
	curve_to_add_to = NULL;

	for (std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++){
		HeeksObj* object = *It;
		AddObjectToArea(object);
		if (objects_to_delete)objects_to_delete->push_back(object);
	}
}

CArea area_for_ObjectToArea;

CArea ObjectToArea(HeeksObj* object)
{
	area_for_ObjectToArea.m_curves.clear();
	std::list<HeeksObj*> list;
	list.push_back(object);
	ObjectsToArea(list, area_for_ObjectToArea);
	//area_for_ObjectToArea.Reorder();
	return area_for_ObjectToArea;
}

void CombineSelectedSketches(){

	CSketch* sketch1 = NULL;

	std::list<HeeksObj*> objects;
	theApp->GetSelection(objects);
	std::list<HeeksObj*>::const_iterator It;
	for (std::list<HeeksObj*>::iterator It = objects.begin(); It != objects.end(); It++){
		HeeksObj* object = *It;
		int t = object->GetType();
		if (t == SketchType)
		{
			if (sketch1)
			{
				std::list<HeeksObj*> new_lines_and_arcs;
				for (HeeksObj* o = object->GetFirstChild(); o; o = object->GetNextChild())
				{
					new_lines_and_arcs.push_back(o);
				}
				theApp->DeleteUndoably(object);
				theApp->AddUndoably(new_lines_and_arcs, sketch1);
			}
			else
			{
				sketch1 = (CSketch*)object;
			}
		}
	}

	if (sketch1)sketch1->ReOrderSketch(SketchOrderTypeReOrder);
}