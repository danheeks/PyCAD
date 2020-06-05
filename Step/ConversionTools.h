// ConversionTools.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

extern bool ConvertLineArcsToWire2(const std::list<HeeksObj *> &list, TopoDS_Wire& wire);
extern void SortEdges( std::vector<TopoDS_Edge> & edges );
extern bool ConvertEdgesToFaceOrWire(const std::vector<TopoDS_Edge> &edges, std::list<TopoDS_Shape> &face_or_wire, bool face_not_wire);
extern bool ConvertSketchToFaceOrWire(HeeksObj* object, std::list<TopoDS_Shape> &face_or_wire, bool face_not_wire);
extern bool ConvertFaceToSketch2(const TopoDS_Face& face, HeeksObj* sketch, double deviation);
extern bool ConvertWireToSketch(const TopoDS_Wire& wire, HeeksObj* sketch, double deviation);
extern bool ConvertEdgeToSketch2(const TopoDS_Edge& edge, HeeksObj* sketch, double deviation, bool reverse = false);
extern HeeksObj* SplitArcsIntoLittleLines(HeeksObj* sketch);
extern bool ConvertSketchToEdges(HeeksObj *object, std::list< std::vector<TopoDS_Edge> > &edges);
extern TopoDS_Wire EdgesToWire(const std::vector<TopoDS_Edge> &edges);
extern bool SketchToWires(HeeksObj* sketch, std::list<TopoDS_Wire> &wire_list);
