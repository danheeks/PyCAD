// RuledSurface.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

extern void PickCreateRuledSurface(bool delete_sketches);
extern void PickCreateRevolution();
extern bool ConvertLineArcsToWire2(const std::list<HeeksObj*> &list, TopoDS_Wire &wire);
extern HeeksObj* CreatePipeFromProfile(HeeksObj* spine, HeeksObj* profile);
extern HeeksObj* CreateRuledFromSketches(std::list<HeeksObj*> list, bool make_solid);
extern bool CreateRuledSurface(const std::list<TopoDS_Wire> &wire_list, TopoDS_Shape& shape, bool make_solid);
extern void CreateExtrusions(const std::list<TopoDS_Shape> &faces_or_wires, std::list<TopoDS_Shape>& new_shapes, const gp_Vec& extrude_vector, double taper_angle, bool solid_if_possible);
extern void CreateRevolutions(const std::list<TopoDS_Shape> &faces_or_wires, std::list<TopoDS_Shape>& new_shapes, const gp_Ax1& axis, double angle);
extern void CreateSweep(std::list<HeeksObj*> &sweep_objects, HeeksObj* sweep_profile, bool solid_if_possible, const HeeksColor& color);
extern void ConvertToFaceOrWire(std::list<HeeksObj*> list, std::list<TopoDS_Shape> &faces_or_wires, bool face_not_wire);
