// RuledSurface.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "RuledSurface.h"
#include "Wire.h"
#include "Face.h"
#include "ConversionTools.h"
#include "strconv.h"

void PickCreateRuledSurface()
{
#if 0
	if(wxGetApp().m_marked_list->size() == 0)
	{
		wxGetApp().PickObjects(_("pick some sketches"));
	}

	if(wxGetApp().m_marked_list->size() > 0)
	{
		std::list<TopoDS_Wire> wire_list;

		std::list<HeeksObj*> sketches_to_delete;

		for(std::list<HeeksObj *>::const_iterator It = wxGetApp().m_marked_list->list().begin(); It != wxGetApp().m_marked_list->list().end(); It++)
		{
			HeeksObj* object = *It;
			if(object->GetType() == SketchType)
			{
				std::list<HeeksObj*> list;
				list.push_back(object);
				TopoDS_Wire wire;
				if(ConvertLineArcsToWire2(list, wire))
				{
					wire_list.push_back(wire);
					if(wxGetApp().m_loft_removes_sketches)sketches_to_delete.push_back(object);
				}
			}
		}

		TopoDS_Shape shape;
		if(CreateRuledSurface(wire_list, shape, true))
		{
			wxGetApp().StartHistory();
			wxGetApp().DeleteUndoably(sketches_to_delete);
			HeeksObj* new_object = CShape::MakeObject(shape, _("Ruled Surface"), SOLID_TYPE_UNKNOWN, HeeksColor(51, 45, 51), 1.0f);
			wxGetApp().AddUndoably(new_object, NULL, NULL);
			wxGetApp().EndHistory();
		}

	}
#endif
}

void ConvertToFaceOrWire(std::list<HeeksObj*> list, std::list<TopoDS_Shape> &faces_or_wires, bool face_not_wire)
{
#if 0
	std::list<HeeksObj*> sketches_or_faces_to_delete;

	for(std::list<HeeksObj *>::const_iterator It = list.begin(); It != list.end(); It++)
	{
		HeeksObj* object = *It;
		switch(object->GetType())
		{
		case SketchType:
		case CircleType:
			{
				if(ConvertSketchToFaceOrWire(object, faces_or_wires, face_not_wire))
				{
					if(wxGetApp().m_extrude_removes_sketches)sketches_or_faces_to_delete.push_back(object);
				}
			}
			break;

		case FaceType:
			faces_or_wires.push_back(((CFace*)object)->Face());
			if(wxGetApp().m_extrude_removes_sketches)sketches_or_faces_to_delete.push_back(object);
			break;

		default:
			break;
		}
	}

	wxGetApp().DeleteUndoably(sketches_or_faces_to_delete);
#endif
}

HeeksObj* CreateExtrusionOrRevolution(std::list<HeeksObj*> list, double height_or_angle, bool solid_if_possible, bool revolution_not_extrusion, double taper_angle_for_extrusion, bool add_new_objects)
{
#if 0
	std::list<TopoDS_Shape> faces_or_wires;

	ConvertToFaceOrWire(list, faces_or_wires, (fabs(taper_angle_for_extrusion) <= 0.0000001) && solid_if_possible);

	std::list<TopoDS_Shape> new_shapes;
	gp_Trsf trsf = wxGetApp().GetDrawMatrix(false);
	if(revolution_not_extrusion)
	{
		CreateRevolutions(faces_or_wires, new_shapes, gp_Ax1(gp_Pnt(0, 0, 0).Transformed(trsf), gp_Vec(1, 0, 0).Transformed(trsf)), height_or_angle);
	}
	else
	{
		CreateExtrusions(faces_or_wires, new_shapes, gp_Vec(0, 0, height_or_angle).Transformed(trsf), taper_angle_for_extrusion, solid_if_possible);
	}
	HeeksObj* new_object = 0;
	if(new_shapes.size() > 0)
	{
		for(std::list<TopoDS_Shape>::iterator It = new_shapes.begin(); It != new_shapes.end(); It++){
			TopoDS_Shape& shape = *It;
			new_object = CShape::MakeObject(shape, revolution_not_extrusion ? _("Revolved Solid") : _("Extruded Solid"), SOLID_TYPE_UNKNOWN, wxGetApp().current_color, 1.0f);
			if(add_new_objects)
				wxGetApp().AddUndoably(new_object, NULL, NULL);
			else
				break;
		}
	}

	for(std::list<TopoDS_Shape>::iterator It = faces_or_wires.begin(); It != faces_or_wires.end(); It++)
	{
		TopoDS_Shape shape = *It;
		shape.Free();
	}
	return new_object;
#else
	return NULL;
#endif
}

HeeksObj* CreatePipeFromProfile(const TopoDS_Wire &spine, std::list<TopoDS_Shape> &faces)
{
#if 0
	std::list<HeeksObj*> pipe_shapes;

	for(std::list<TopoDS_Shape>::iterator It2 = faces.begin(); It2 != faces.end(); It2++)
	{
		TopoDS_Shape& face = *It2;

		try
		{
			// pipe profile algong spine
			BRepOffsetAPI_MakePipe makePipe(spine, face);
			makePipe.Build();
			TopoDS_Shape shape = makePipe.Shape();

			HeeksObj* new_object = CShape::MakeObject(shape, _("Pipe"), SOLID_TYPE_UNKNOWN, wxGetApp().current_color, 1.0f);
			if(new_object)pipe_shapes.push_back(new_object);
		}
		catch (Standard_Failure) {
			Handle_Standard_Failure e = Standard_Failure::Caught();
			theApp->DoMessageBox(wxString(_("Error making pipe")) + _T(": ") + Ctt(e->GetMessageString()));
		}
	}
	if(pipe_shapes.size() > 0)
	{
		wxGetApp().StartHistory();
		for(std::list<HeeksObj*>::iterator It = pipe_shapes.begin(); It != pipe_shapes.end(); It++)
		{
			HeeksObj* object = *It;
			wxGetApp().AddUndoably(object, NULL, NULL);
		}
		wxGetApp().EndHistory();
		return pipe_shapes.front();
	}
#endif

	return NULL;
}

HeeksObj* CreatePipeFromProfile(HeeksObj* spine, HeeksObj* profile)
{
#if 0
	const TopoDS_Wire wire = ((CWire*)spine)->Wire();
	std::list<TopoDS_Shape> faces;
	if(!ConvertSketchToFaceOrWire(profile, faces, true))return NULL;
	HeeksObj* pipe = CreatePipeFromProfile(wire, faces);
	if(pipe)
	{
		wxGetApp().DeleteUndoably(profile);
	}
	return pipe;
#else
	return NULL;
#endif
}

HeeksObj* CreateSweep(std::list<HeeksObj*> &sweep_objects, HeeksObj* sweep_profile, bool solid_if_possible)
{
#if 0
	std::list<TopoDS_Shape> faces_or_wires;
	ConvertToFaceOrWire(sweep_objects, faces_or_wires, solid_if_possible);
	std::list<TopoDS_Wire> wires;
	HeeksObj* pipe = NULL;
	if(SketchToWires(sweep_profile, wires))
	{
		TopoDS_Wire wire = wires.front();
		pipe = CreatePipeFromProfile(wire, faces_or_wires);
	}
	return pipe;
#else
	return NULL;
#endif
}

void PickCreateSweep()
{
#if 0
	// undoable
	if(wxGetApp().m_marked_list->size() == 0)
	{
		wxGetApp().PickObjects(_("pick sketches, faces or circles"), MARKING_FILTER_CIRCLE | MARKING_FILTER_SKETCH | MARKING_FILTER_FACE);
	}

	std::list<HeeksObj*> sweep_objects = wxGetApp().m_marked_list->list();
	wxGetApp().m_marked_list->Clear(true);
	if(!wxGetApp().PickObjects(_("Pick a Sketch to sweep along"), MARKING_FILTER_SKETCH, true))return;
	if(wxGetApp().m_marked_list->list().size() == 0)return;

	HeeksObj* sweep_profile = wxGetApp().m_marked_list->list().front();

	wxGetApp().StartHistory();
	CreateSweep(sweep_objects, sweep_profile, true);
	wxGetApp().EndHistory();
#endif
}

HeeksObj* CreateRuledFromSketches(std::list<HeeksObj*> list, bool make_solid)
{
#if 0
	std::list<TopoDS_Wire> wire_list;
	for(std::list<HeeksObj *>::iterator It = list.begin(); It != list.end(); It++)
	{
		HeeksObj* object = *It;
		if(object->GetType() == SketchType)
		{
			std::list<HeeksObj*> s;
			s.push_back(object);
			TopoDS_Wire wire;
			if(ConvertLineArcsToWire2(s, wire))
			{
				wire_list.push_back(wire);
			}
		}
	}

	TopoDS_Shape shape;
	if(CreateRuledSurface(wire_list, shape, make_solid))
	{
		return CShape::MakeObject(shape, _("Ruled Surface"), SOLID_TYPE_UNKNOWN, HeeksColor(51, 45, 51), 1.0f);
	}
#endif
	return NULL;
}

void PickCreateExtrusion()
{
#if 0
	if(wxGetApp().m_marked_list->size() == 0)
	{
		wxGetApp().PickObjects(_("pick sketches, faces or circles"), MARKING_FILTER_CIRCLE | MARKING_FILTER_SKETCH | MARKING_FILTER_FACE);
	}

	double height;
	double taper_angle;
	{
		HeeksConfig config;
		config.Read(_T("ExtrusionHeight"), &height, 10.0);
		config.Read(_T("ExtrusionTaperAngle"), &taper_angle, 0.0);
	}

	if(InputExtrusionHeight(height, &(wxGetApp().m_extrude_to_solid), &taper_angle))
	{
		{
			HeeksConfig config;
			config.Write(_T("ExtrusionHeight"), height);
			config.Write(_T("ExtrusionTaperAngle"), taper_angle);
			config.Write(_T("ExtrudeToSolid"), wxGetApp().m_extrude_to_solid);
		}

		if(wxGetApp().m_marked_list->size() > 0)
		{
			wxGetApp().StartHistory();
			CreateExtrusionOrRevolution(wxGetApp().m_marked_list->list(),height, wxGetApp().m_extrude_to_solid, false, taper_angle);
			wxGetApp().EndHistory();
		}
	}
#endif
}
void PickCreateRevolution()
{
#if 0
	if(wxGetApp().m_marked_list->size() == 0)
	{
		wxGetApp().PickObjects(_("pick sketches, faces or circles"), MARKING_FILTER_CIRCLE | MARKING_FILTER_SKETCH | MARKING_FILTER_FACE);
	}

	double angle;
	{
		HeeksConfig config;
		config.Read(_T("RevolutionAngle"), &angle, 360.0);
	}

	if(InputRevolutionAngle(angle, &wxGetApp().m_extrude_to_solid))
	{
		{
			HeeksConfig config;
			config.Write(_T("RevolutionAngle"), angle);
		}
		if(wxGetApp().m_marked_list->size() > 0)
		{
			CreateExtrusionOrRevolution(wxGetApp().m_marked_list->list(), angle, wxGetApp().m_extrude_to_solid, true, 0.0, true);
		}
	}
#endif
}

bool CreateRuledSurface(const std::list<TopoDS_Wire> &wire_list, TopoDS_Shape& shape, bool make_solid)
{
#if 0
	if(wire_list.size() > 0)
	{
			BRepOffsetAPI_ThruSections generator( make_solid ? Standard_True : Standard_True, Standard_False );
			for(std::list<TopoDS_Wire>::const_iterator It = wire_list.begin(); It != wire_list.end(); It++)
			{
				const TopoDS_Wire &wire = *It;
				generator.AddWire(wire);
			}

		try{
			generator.Build();
			shape = generator.Shape();
		}
		catch (Standard_Failure) {
			Handle_Standard_Failure e = Standard_Failure::Caught();
			theApp->DoMessageBox(wxString(_("Error making ruled solid")) + _T(": ") + Ctt(e->GetMessageString()));
			return false;
		}
		catch(...)
		{
			theApp->DoMessageBox(_("Fatal error making ruled solid"));
			return false;
		}

		return true;
	}
#endif
	return false;
}

void CreateExtrusions(const std::list<TopoDS_Shape> &faces_or_wires, std::list<TopoDS_Shape>& new_shapes, const gp_Vec& extrude_vector, double taper_angle, bool solid_if_possible)
{
	try{
		for(std::list<TopoDS_Shape>::const_iterator It = faces_or_wires.begin(); It != faces_or_wires.end(); It++)
		{
			const TopoDS_Shape& face_or_wire = *It;
			if(fabs(taper_angle) > 0.0000001)
			{
				// make an offset face
				double distance = tan(taper_angle * M_PI/180) * extrude_vector.Magnitude();
				bool wire = (face_or_wire.ShapeType() == TopAbs_WIRE);
				BRepOffsetAPI_MakeOffset offset;
				if(wire)
					offset = BRepOffsetAPI_MakeOffset(TopoDS::Wire(face_or_wire));
				else
					continue; // can't do CreateRuledSurface on faces yet
                offset.Perform(distance);

				// parallel
				std::list<TopoDS_Wire> wire_list;
				wire_list.push_back(TopoDS::Wire(face_or_wire));
				wire_list.push_back(TopoDS::Wire(offset.Shape()));

				gp_Trsf mat;
				mat.SetTranslation(extrude_vector);
				BRepBuilderAPI_Transform myBRepTransformation(wire_list.back(),mat);
				wire_list.back() = TopoDS::Wire(myBRepTransformation.Shape());

				TopoDS_Shape new_shape;
				if(CreateRuledSurface(wire_list, new_shape, solid_if_possible))
				{
					new_shapes.push_back(new_shape);
				}
            }
			else
			{
				BRepPrimAPI_MakePrism generator( face_or_wire, extrude_vector );
				generator.Build();
				new_shapes.push_back(generator.Shape());
			}
		}
	}
	catch (Standard_Failure) {
		Handle_Standard_Failure e = Standard_Failure::Caught();
		theApp->DoMessageBox((std::wstring(L"Error making extruded solid") + L": " + Ctt(e->GetMessageString())).c_str());
	}
	catch(...)
	{
		theApp->DoMessageBox(L"Fatal error making extruded solid");
	}

}

void CreateRevolutions(const std::list<TopoDS_Shape> &faces_or_wires, std::list<TopoDS_Shape>& new_shapes, const gp_Ax1& axis, double angle)
{
	try{
		for(std::list<TopoDS_Shape>::const_iterator It = faces_or_wires.begin(); It != faces_or_wires.end(); It++)
		{
			const TopoDS_Shape& face_or_wire = *It;
			if(fabs(angle - 360.0) < 0.00001)
			{
				BRepPrimAPI_MakeRevol generator( face_or_wire, axis );
				generator.Build();
				new_shapes.push_back(generator.Shape());
			}
			else
			{
				BRepPrimAPI_MakeRevol generator( face_or_wire, axis, angle * M_PI/180 );
				generator.Build();
				new_shapes.push_back(generator.Shape());
			}
		}
	}
	catch (Standard_Failure) {
		Handle_Standard_Failure e = Standard_Failure::Caught();
		theApp->DoMessageBox((std::wstring(L"Error making revolved solid") + L": " + Ctt(e->GetMessageString())).c_str());
	}
	catch(...)
	{
		theApp->DoMessageBox(L"Fatal error making revolved solid");
	}

}

