// Solid.cpp

#ifdef OPEN_CASCADE_INCLUDED

#include "Solid.h"
#include "Area.h"
#include "Curve.h"
#include "Box.h"

#include <fstream>

CSolid::CSolid(const TopoDS_Shape& shape) :m_shape(shape)
{
}

void CSolid::read_from_file(const std::wstring& step_file_path)
{
	char oldlocale[1000];
	strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

	char fp[1024];
	wcstombs(fp, step_file_path.c_str(), 1024);

	Standard_CString aFileName = (Standard_CString)(fp);
	STEPControl_Reader Reader;
	int status = Reader.ReadFile(aFileName);

	if (status == IFSelect_RetDone)
	{
		int num = Reader.NbRootsForTransfer();
		if (num > 0)
		{
			Handle_Standard_Transient root = Reader.RootForTransfer(1);
			Reader.TransferEntity(root);
			m_shape = Reader.Shape(1);
		}
	}

	setlocale(LC_NUMERIC, oldlocale);
}

CSolid* CSolid::BooleanCut(const CSolid& solid)const
{
	return new CSolid(BRepAlgoAPI_Cut(m_shape, solid.m_shape));
}

CSolid* CSolid::BooleanUnion(const CSolid& solid)const
{
	return new CSolid(BRepAlgoAPI_Fuse(m_shape, solid.m_shape));
}

CSolid* CSolid::BooleanCommon(const CSolid& solid)const
{
	return new CSolid(BRepAlgoAPI_Common(m_shape, solid.m_shape));
}

void CSolid::WriteStep(const std::wstring& step_file_path)
{
	char oldlocale[1000];
	strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

	char fp[1024];
	wcstombs(fp, step_file_path.c_str(), 1024);

	Standard_CString aFileName = (Standard_CString)(fp);
	STEPControl_Writer writer;
	writer.Transfer(m_shape, STEPControl_AsIs);
	writer.Write(aFileName);

	setlocale(LC_NUMERIC, oldlocale);
}

bool ConvertCurveToEdges(const CCurve& curve, std::vector<TopoDS_Edge> &edges)
{
	const Point *prev_p = NULL;
	const double max_tolerance = 10.0;

	for (std::list<CVertex>::const_iterator VIt = curve.m_vertices.begin(); VIt != curve.m_vertices.end(); VIt++)
	{
		const CVertex& vertex = *VIt;

		if (prev_p)
		{
			Span span(*prev_p, vertex);

				bool done = false;

				//if (span.m_p != span.m_v.m_p)
				{
					BRep_Builder aBuilder;
					TopoDS_Vertex start, end;

					aBuilder.MakeVertex(start, gp_Pnt(span.m_p.x, span.m_p.y, 0.0), TOLERANCE);
					start.Orientation(TopAbs_REVERSED);

					aBuilder.MakeVertex(end, gp_Pnt(span.m_v.m_p.x, span.m_v.m_p.y, 0.0), TOLERANCE);
					end.Orientation(TopAbs_FORWARD);

					if (vertex.m_type == 0)
					{
						BRepBuilderAPI_MakeEdge edge(start, end);
						if (edge.IsDone())
						{
							edges.push_back(edge.Edge());
							done = true;
						}
					}
					else
					{
						gp_Circ circ(gp_Ax2(gp_Pnt(span.m_v.m_c.x, span.m_v.m_c.y, 0.0), (span.m_v.m_type > 0) ? gp_Dir(0, 0, 1):gp_Dir(0, 0, -1)), span.m_v.m_c.dist(span.m_p));
						BRepBuilderAPI_MakeEdge edge(circ, start, end);
						if (edge.IsDone())
						{
							edges.push_back(edge.Edge());
							done = true;
						}
					}
				
				}

				if (!done)
				{
					return(false);
				}

		}
		prev_p = &(vertex.m_p);
	}


	return(true);
}

TopoDS_Wire EdgesToWire(const std::vector<TopoDS_Edge> &edges)
{
	BRepBuilderAPI_MakeWire wire_maker;
	for (std::vector<TopoDS_Edge>::const_iterator It = edges.begin(); It != edges.end(); It++)
	{
		const TopoDS_Edge &edge = *It;
		wire_maker.Add(edge);
	}

	return wire_maker.Wire();
}

bool ConvertEdgesToFaceOrWire(const std::vector<TopoDS_Edge> &edges, TopoDS_Shape &face_or_wire, bool face_not_wire)
{
	// It's not enough to add the edges to the wire in an arbitrary order.  If the adjacent edges
	// don't connect then the wire ends up losing one of the edges.  We must sort the edge objects
	// so that they're connected (or best we can) before constructing the TopoDS_Wire object from
	// them.
	// So, please call SortEdges before getting to here.
	try
	{
		TopoDS_Wire wire = EdgesToWire(edges);

		if (face_not_wire)
		{
			BRepBuilderAPI_MakeFace make_face(wire);
			if (make_face.IsDone() == Standard_False)face_or_wire = wire;
			else face_or_wire = make_face.Face();
		}
		else
		{
			face_or_wire = wire;
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

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
	EdgeComparison(const TopoDS_Edge & edge)
	{
		m_reference_edge = edge;
	}

	bool operator()(const TopoDS_Edge & lhs, const TopoDS_Edge & rhs) const
	{

		std::vector<double> lhs_distances;
		lhs_distances.push_back(GetStart(m_reference_edge).Distance(GetStart(lhs)));
		lhs_distances.push_back(GetStart(m_reference_edge).Distance(GetEnd(lhs)));
		lhs_distances.push_back(GetEnd(m_reference_edge).Distance(GetStart(lhs)));
		lhs_distances.push_back(GetEnd(m_reference_edge).Distance(GetEnd(lhs)));
		std::sort(lhs_distances.begin(), lhs_distances.end());

		std::vector<double> rhs_distances;
		rhs_distances.push_back(GetStart(m_reference_edge).Distance(GetStart(rhs)));
		rhs_distances.push_back(GetStart(m_reference_edge).Distance(GetEnd(rhs)));
		rhs_distances.push_back(GetEnd(m_reference_edge).Distance(GetStart(rhs)));
		rhs_distances.push_back(GetEnd(m_reference_edge).Distance(GetEnd(rhs)));
		std::sort(rhs_distances.begin(), rhs_distances.end());

		return(*(lhs_distances.begin()) < *(rhs_distances.begin()));
	}

	TopoDS_Edge m_reference_edge;
};

void SortEdges(std::vector<TopoDS_Edge> & edges)
{
	for (std::vector<TopoDS_Edge>::iterator l_itEdge = edges.begin(); l_itEdge != edges.end(); l_itEdge++)
	{
		// We've already begun.  Just sort based on the previous point's location.
		std::vector<TopoDS_Edge>::iterator l_itNextEdge = l_itEdge;
		l_itNextEdge++;

		if (l_itNextEdge != edges.end())
		{
			EdgeComparison compare(*l_itEdge);
			std::sort(l_itNextEdge, edges.end(), compare);
		} // End if - then
	} // End for
} // End SortEdges() method

bool ConvertCurveToFaceOrWire(const CCurve& curve, TopoDS_Shape &face_or_wire, bool face_not_wire)
{
	std::vector<TopoDS_Edge> edges;

	if (!ConvertCurveToEdges(curve, edges))
	{
		return(false);
	}

	if (edges.size() > 0)
	{
		//SortEdges(edges);
		if (!ConvertEdgesToFaceOrWire(edges, face_or_wire, face_not_wire))return false;
	}

	return true;
}

class ShapeAndSign
{
public:
	TopoDS_Shape m_shape;
	bool m_add; // anti-clockwise shapes are to be added, clockwise shapes are to be removed

	ShapeAndSign(const TopoDS_Shape& shape, bool add) :m_shape(shape), m_add(add){}
};

void ConvertToFaceOrWire(const CArea& area, std::list<ShapeAndSign> &faces_or_wires, bool face_not_wire)
{
	for (std::list<CCurve>::const_iterator It = area.m_curves.begin(); It != area.m_curves.end(); It++)
	{
		const CCurve& curve = *It;
		TopoDS_Shape face_or_wire;
		if (ConvertCurveToFaceOrWire(curve, face_or_wire, face_not_wire))
		{
			faces_or_wires.push_back(ShapeAndSign(face_or_wire, !curve.IsClockwise()));
		}
	}
}

bool CreateRuledSurface(const std::list<TopoDS_Wire> &wire_list, TopoDS_Shape& shape, bool make_solid)
{
	if (wire_list.size() > 0)
	{
		BRepOffsetAPI_ThruSections generator(make_solid ? Standard_True : Standard_True, Standard_False);
		for (std::list<TopoDS_Wire>::const_iterator It = wire_list.begin(); It != wire_list.end(); It++)
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
			//wxMessageBox(wxString(_("Error making ruled solid")) + _T(": ") + Ctt(e->GetMessageString()));
			return false;
		}
		catch (...)
		{
			//wxMessageBox(_("Fatal error making ruled solid"));
			return false;
		}

		return true;
	}
	return false;
}

void CreateExtrusions(const std::list<ShapeAndSign> &faces_or_wires, std::list<ShapeAndSign>& new_shapes, const gp_Vec& extrude_vector, double taper_angle, bool solid_if_possible)
{
	try{
		for (std::list<ShapeAndSign>::const_iterator It = faces_or_wires.begin(); It != faces_or_wires.end(); It++)
		{
			const TopoDS_Shape& face_or_wire = (*It).m_shape;
			if (fabs(taper_angle) > 0.0000001)
			{
				// make an offset face
				double distance = tan(taper_angle * M_PI / 180) * extrude_vector.Magnitude();
				bool wire = (face_or_wire.ShapeType() == TopAbs_WIRE);
				BRepOffsetAPI_MakeOffset offset;
				if (wire)
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
				BRepBuilderAPI_Transform myBRepTransformation(wire_list.back(), mat);
				wire_list.back() = TopoDS::Wire(myBRepTransformation.Shape());

				TopoDS_Shape new_shape;
				if (CreateRuledSurface(wire_list, new_shape, solid_if_possible))
				{
					new_shapes.push_back(ShapeAndSign(new_shape, (*It).m_add));
				}
			}
			else
			{
				BRepPrimAPI_MakePrism generator(face_or_wire, extrude_vector);
				generator.Build();
				new_shapes.push_back(ShapeAndSign(generator.Shape(), (*It).m_add));
			}
		}
	}
	catch (Standard_Failure) {
		Handle_Standard_Failure e = Standard_Failure::Caught();
		cout << "Error making extruded solid: " << e->GetMessageString();
		return;
	}
	catch (...)
	{
		//wxMessageBox(_("Fatal error making extruded solid"));
		return;
	}

}

CSolid::CSolid(const CArea& area, double thickness)
{
	std::list<ShapeAndSign> faces_or_wires;

	ConvertToFaceOrWire(area, faces_or_wires, true);

	std::list<ShapeAndSign> new_shapes;
	CreateExtrusions(faces_or_wires, new_shapes, gp_Vec(0, 0, thickness)/*.Transformed(trsf)*/, 0.0, true);

	bool made = false;

	for (std::list<ShapeAndSign>::iterator It = new_shapes.begin(); It != new_shapes.end(); It++)
	{
		ShapeAndSign shape_and_sign = *It;
		if (made)
		{
			if (shape_and_sign.m_add || true)
			{
				m_shape = BRepAlgoAPI_Fuse(m_shape, shape_and_sign.m_shape);
			}
			else
			{
				m_shape = BRepAlgoAPI_Cut(m_shape, shape_and_sign.m_shape);
			}
		}
		else
		{
			//if (shape_and_sign.m_add)
			//{
				m_shape = shape_and_sign.m_shape;
				made = true;
			//}
		}
	}

	for (std::list<ShapeAndSign>::iterator It = faces_or_wires.begin(); It != faces_or_wires.end(); It++)
	{
		TopoDS_Shape shape = (*It).m_shape;
		shape.Free();
	}
}

CSolid::CSolid(const CCurve& curve, double thickness)
{
	TopoDS_Shape face_or_wire;
	if (!ConvertCurveToFaceOrWire(curve, face_or_wire, true))
		return;

	std::list<ShapeAndSign> faces_or_wires;
	faces_or_wires.push_back(ShapeAndSign(face_or_wire, true));

	std::list<ShapeAndSign> new_shapes;
	CreateExtrusions(faces_or_wires, new_shapes, gp_Vec(0, 0, thickness)/*.Transformed(trsf)*/, 0.0, true);

	if (new_shapes.size() > 0)
	{
		m_shape = new_shapes.front().m_shape;
	}

	for (std::list<ShapeAndSign>::iterator It = faces_or_wires.begin(); It != faces_or_wires.end(); It++)
	{
		TopoDS_Shape shape = (*It).m_shape;
		shape.Free();
	}
}

void CSolid::Translate(const Point3d& v)
{
	gp_Trsf mat;
	mat.SetTranslationPart(gp_Vec(v.x, v.y, v.z));
	BRepBuilderAPI_Transform myBRepTransformation(m_shape, mat);
	m_shape = myBRepTransformation.Shape();
}

void CSolid::Transform(const Matrix& tm)
{
	gp_Trsf mat;
	const double* m = tm.e;
#if OCC_VERSION_HEX >= 0x060600
	mat.SetValues(m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11]);
#else
	mat.SetValues(m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], 0.0001, 0.00000001);
#endif
	BRepBuilderAPI_Transform myBRepTransformation(m_shape, mat);
	m_shape = myBRepTransformation.Shape();
}



bool CSolid::GetExtents(double* extents, const double* orig, const double* xdir, const double* ydir, const double* zdir)const
{
	gp_Pnt p_orig(0, 0, 0);
	if (orig)p_orig = gp_Pnt(orig[0], orig[1], orig[2]);
	gp_Vec v_x(1, 0, 0);
	if (xdir)v_x = gp_Vec(xdir[0], xdir[1], xdir[2]);
	gp_Vec v_y(0, 1, 0);
	if (ydir)v_y = gp_Vec(ydir[0], ydir[1], ydir[2]);
	gp_Vec v_z(0, 0, 1);
	if (zdir)v_z = gp_Vec(zdir[0], zdir[1], zdir[2]);

	BRepPrimAPI_MakeBox cuboid_plus_x(gp_Ax2(gp_Pnt(p_orig.XYZ() + 2000000 * v_x.XYZ() + (-1000000) * v_z.XYZ() + (-1000000) * v_y.XYZ()), v_x, v_y), 1000000, 1000000, 1000000);
	BRepPrimAPI_MakeBox cuboid_minus_x(gp_Ax2(gp_Pnt(p_orig.XYZ() + (-2000000) * v_x.XYZ() + (-1000000) * v_z.XYZ() + (-1000000) * v_y.XYZ()), -v_x, v_z), 1000000, 1000000, 1000000);
	BRepPrimAPI_MakeBox cuboid_plus_y(gp_Ax2(gp_Pnt(p_orig.XYZ() + 2000000 * v_y.XYZ() + (-1000000) * v_z.XYZ() + (-1000000) * v_x.XYZ()), v_y, v_z), 1000000, 1000000, 1000000);
	BRepPrimAPI_MakeBox cuboid_minus_y(gp_Ax2(gp_Pnt(p_orig.XYZ() + (-2000000) * v_y.XYZ() + (-1000000) * v_z.XYZ() + (-1000000) * v_x.XYZ()), -v_y, v_x), 1000000, 1000000, 1000000);
	BRepPrimAPI_MakeBox cuboid_plus_z(gp_Ax2(gp_Pnt(p_orig.XYZ() + 2000000 * v_z.XYZ() + (-1000000) * v_x.XYZ() + (-1000000) * v_y.XYZ()), v_z, v_x), 1000000, 1000000, 1000000);
	BRepPrimAPI_MakeBox cuboid_minus_z(gp_Ax2(gp_Pnt(p_orig.XYZ() + (-2000000) * v_z.XYZ() + (-1000000) * v_x.XYZ() + (-1000000) * v_y.XYZ()), -v_z, v_y), 1000000, 1000000, 1000000);

	gp_Vec v_orig(p_orig.XYZ());

	TopoDS_Solid shape[6] =
	{
		cuboid_minus_x,
		cuboid_minus_y,
		cuboid_minus_z,
		cuboid_plus_x,
		cuboid_plus_y,
		cuboid_plus_z
	};

	gp_Vec vector[6] =
	{
		v_x,
		v_y,
		v_z,
		v_x,
		v_y,
		v_z
	};

	for (int i = 0; i<6; i++){
		BRepExtrema_DistShapeShape extrema(m_shape, shape[i]);
		extrema.Perform();
		gp_Pnt p = extrema.PointOnShape1(1);
		gp_Vec v(p.XYZ());
		double dp = v * vector[i];
		double dp_o = v_orig * vector[i];
		extents[i] = dp - dp_o;
	}

	return true;
}

void CSolid::GetBox(CBox& box)const
{
	double orig[3] = { 0, 0, 0 };
	double xdir[3] = { 1, 0, 0 };
	double ydir[3] = { 0, 1, 0 };
	double zdir[3] = { 0, 0, 1 };
	box.m_valid = GetExtents(box.m_x, orig, xdir, ydir, zdir);
}

CSolid* CSolid::Extrusion(const Point3d& v)const
{
#if 0
	std::list<TopoDS_Shape> faces_or_wires;
	faces_or_wires.push_back(m_shape);

	std::list<TopoDS_Shape> new_shapes;
	CreateExtrusions(faces_or_wires, new_shapes, gp_Vec(v.x, v.y, v.z), 0.0, true);

	if (new_shapes.size() > 0)
	{
		return new CSolid(new_shapes.front());
	}
#endif

	return NULL;
}

#endif