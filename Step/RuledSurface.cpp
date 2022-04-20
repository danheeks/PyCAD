// RuledSurface.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "RuledSurface.h"
#include "Wire.h"
#include "Face.h"
#include "ConversionTools.h"
#include "strconv.h"

void PickCreateRuledSurface(bool delete_sketches)
{
	std::list<HeeksObj*> objects;
	theApp->GetSelection(objects);
	if (objects.size() == 0)
		return;

	std::list<TopoDS_Wire> wire_list;

	std::list<HeeksObj*> sketches_to_delete;

	for (std::list<HeeksObj *>::iterator It = objects.begin(); It != objects.end(); It++)
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
				if (delete_sketches)sketches_to_delete.push_back(object);
			}
		}
	}

	TopoDS_Shape shape;
	if(CreateRuledSurface(wire_list, shape, true))
	{
		theApp->StartHistory(L"Make Ruled Surface");
		theApp->DeleteUndoably(sketches_to_delete);
		HeeksObj* new_object = CShape::MakeObject(shape, L"Ruled Surface", SOLID_TYPE_UNKNOWN, HeeksColor(51, 45, 51), 1.0f);
		theApp->AddUndoably(new_object, NULL, NULL);
		theApp->EndHistory();
	}
}

void ConvertToFaceOrWire(std::list<HeeksObj*> list, std::list<TopoDS_Shape> &faces_or_wires, bool face_not_wire)
{
	std::list<HeeksObj*> sketches_or_faces_to_delete;

	for(std::list<HeeksObj *>::const_iterator It = list.begin(); It != list.end(); It++)
	{
		HeeksObj* object = *It;
		int t = object->GetType();
		if (t == SketchType || t == CircleType)
		{
			if(ConvertSketchToFaceOrWire(object, faces_or_wires, face_not_wire))
			{
				if(CSettings::extrudeRemovesSketches)sketches_or_faces_to_delete.push_back(object);
			}
		}
		else if (t == CFace::m_type)
		{
			faces_or_wires.push_back(((CFace*)object)->Face());
			if(CSettings::extrudeRemovesSketches)sketches_or_faces_to_delete.push_back(object);
		}
	}

	theApp->DeleteUndoably(sketches_or_faces_to_delete);
}

void CreateExtrusionOrRevolution(double height_or_angle, bool solid_if_possible, bool revolution_not_extrusion, double taper_angle_for_extrusion, const HeeksColor &color)
{
	std::list<TopoDS_Shape> faces_or_wires;

	std::list<HeeksObj*> objects;
	theApp->GetSelection(objects);

	ConvertToFaceOrWire(objects, faces_or_wires, (fabs(taper_angle_for_extrusion) <= 0.0000001) && solid_if_possible);

	std::list<TopoDS_Shape> new_shapes;
	Matrix m = *(theApp->GetDrawMatrix(false));
	gp_Trsf trsf = make_matrix(m.e);
	if(revolution_not_extrusion)
	{
		CreateRevolutions(faces_or_wires, new_shapes, gp_Ax1(gp_Pnt(0, 0, 0).Transformed(trsf), gp_Vec(1, 0, 0).Transformed(trsf)), height_or_angle);
	}
	else
	{
		CreateExtrusions(faces_or_wires, new_shapes, gp_Vec(0, 0, height_or_angle).Transformed(trsf), taper_angle_for_extrusion, solid_if_possible);
	}

	if(new_shapes.size() > 0)
	{
		theApp->StartHistory(L"Make Extrusion");
		for(std::list<TopoDS_Shape>::iterator It = new_shapes.begin(); It != new_shapes.end(); It++){
			TopoDS_Shape& shape = *It;
			HeeksObj* new_object = CShape::MakeObject(shape, revolution_not_extrusion ? L"Revolved Solid" : L"Extruded Solid", SOLID_TYPE_UNKNOWN, color, 1.0f);
			theApp->AddUndoably(new_object, NULL, NULL);
		}
		theApp->EndHistory();
	}

	for(std::list<TopoDS_Shape>::iterator It = faces_or_wires.begin(); It != faces_or_wires.end(); It++)
	{
		TopoDS_Shape shape = *It;
		shape.Free();
	}
}

HeeksObj* CreatePipeFromProfile(const TopoDS_Wire &spine, std::list<TopoDS_Shape> &faces, const HeeksColor& color)
{
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

			HeeksObj* new_object = CShape::MakeObject(shape, L"Pipe", SOLID_TYPE_UNKNOWN, color, 1.0f);
			if(new_object)pipe_shapes.push_back(new_object);
		}
		catch (Standard_Failure) {
			Handle_Standard_Failure e = Standard_Failure::Caught();
			theApp->DoMessageBox((std::wstring(L"Error making pipe: ") + Ctt(e->GetMessageString())).c_str());
		}
	}
	if(pipe_shapes.size() > 0)
	{
		theApp->StartHistory(L"Create Pipe");
		for(std::list<HeeksObj*>::iterator It = pipe_shapes.begin(); It != pipe_shapes.end(); It++)
		{
			HeeksObj* object = *It;
			theApp->AddUndoably(object, NULL, NULL);
		}
		theApp->EndHistory();
		return pipe_shapes.front();
	}

	return NULL;
}

HeeksObj* CreatePipeFromProfile(HeeksObj* spine, HeeksObj* profile, const HeeksColor& color)
{
	const TopoDS_Wire wire = ((CWire*)spine)->Wire();
	std::list<TopoDS_Shape> faces;
	if(!ConvertSketchToFaceOrWire(profile, faces, true))return NULL;
	HeeksObj* pipe = CreatePipeFromProfile(wire, faces, color);
	if(pipe)
	{
		theApp->DeleteUndoably(profile);
	}
	return pipe;
}

void CreateSweep(std::list<HeeksObj*> &sweep_objects, HeeksObj* sweep_profile, bool solid_if_possible, const HeeksColor& color)
{
	std::list<TopoDS_Shape> faces_or_wires;
	ConvertToFaceOrWire(sweep_objects, faces_or_wires, solid_if_possible);
	std::list<TopoDS_Wire> wires;
	HeeksObj* pipe = NULL;
	if(SketchToWires(sweep_profile, wires))
	{
		TopoDS_Wire wire = wires.front();
		pipe = CreatePipeFromProfile(wire, faces_or_wires, color);
	}
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

void PickCreateRevolution()
{
#if 0
	if(theApp->m_marked_list->size() == 0)
	{
		theApp->PickObjects(_("pick sketches, faces or circles"), MARKING_FILTER_CIRCLE | MARKING_FILTER_SKETCH | MARKING_FILTER_FACE);
	}

	double angle;
	{
		HeeksConfig config;
		config.Read(_T("RevolutionAngle"), &angle, 360.0);
	}

	if(InputRevolutionAngle(angle, &theApp->m_extrude_to_solid))
	{
		{
			HeeksConfig config;
			config.Write(_T("RevolutionAngle"), angle);
		}
		if(theApp->m_marked_list->size() > 0)
		{
			CreateExtrusionOrRevolution(theApp->m_marked_list->list(), angle, theApp->m_extrude_to_solid, true, 0.0, true);
		}
	}
#endif
}

bool CreateRuledSurface(const std::list<TopoDS_Wire> &wire_list, TopoDS_Shape& shape, bool make_solid)
{
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
			theApp->DoMessageBox((std::wstring(L"Error making ruled solid: ") + Ctt(e->GetMessageString())).c_str());
			return false;
		}
		catch(...)
		{
			theApp->DoMessageBox(L"Fatal error making ruled solid");
			return false;
		}

		return true;
	}
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

