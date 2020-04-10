// SolidTools.cpp
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "SolidTools.h"
#include "Curve.h"
#include "Area.h"

#if 0
void GetSolidMenuTools(std::list<Tool*>* t_list){
	// Tools for multiple selected items.
	bool solids_in_marked_list = false;

	// check to see what types have been marked
	std::list<HeeksObj*>::const_iterator It;
	for (It = theApp->m_marked_list->list().begin(); It != theApp->m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		switch (object->GetType()){
		case SolidType:
		case StlSolidType:
			solids_in_marked_list = true;
		}
	}

	if (solids_in_marked_list)
	{
		t_list->push_back(new SaveSolids);
		t_list->push_back(new OutlineSolids);
	}
}

void SaveSolids::Run(){
	std::list<HeeksObj*> objects;

	for (std::list<HeeksObj*>::const_iterator It = theApp->m_marked_list->list().begin(); It != theApp->m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		switch (object->GetType())
		{
		case SolidType:
		case StlSolidType:
		{
			objects.push_back(object);
		}
		break;
		}
	}

	if (objects.size() > 0)
	{
		std::wstring filepath(_T(""));
		{
			// get last used filepath
			HeeksConfig config;
			config.Read(_T("SolidExportFilepath"), &filepath, _T(""));
		}

		wxFileDialog fd(theApp->m_frame, L"Save solid file", wxEmptyString, filepath, std::wstring(L"Solid Files") + _T(" |*.igs;*.iges;*.stp;*.step;*.stl;*.cpp;*.py|") + L"IGES files" + _T(" (*.igs *.iges)|*.igs;*.iges|") + L"STEP files" + _T(" (*.stp *.step)|*.stp;*.step|") + L"STL files" + _T(" (*.stl)|*.stl|") + L"CPP files" + _T(" (*.cpp)|*.cpp|") + L"OpenCAMLib python files" + _T(" (*.py)|*.py"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		fd.SetFilterIndex(0);

		if (fd.ShowModal() == wxID_CANCEL)return;
		filepath = fd.GetPath();

		std::wstring wf(filepath);
		wf.LowerCase();

		if (wf.EndsWith(_T(".stl")))
		{
			theApp->SaveSTLFile(objects, filepath, -1.0, NULL, theApp->m_stl_save_as_binary);
		}
		else if (wf.EndsWith(_T(".cpp")))
		{
			theApp->SaveCPPFile(objects, filepath);
		}
		else if (wf.EndsWith(_T(".py")))
		{
			theApp->SavePyFile(objects, filepath);
		}
		else if (CShape::ExportSolidsFile(objects, filepath))
		{
		}
		else
		{
			theApp->DoMessageBox(L"Invalid solid file type chosen");
			return;
		}

		{
			// save last used filepath
			HeeksConfig config;
			config.Write(_T("SolidExportFilepath"), filepath);
		}
	}
}

static std::list<CCurve> OutlineSolids_curves;

static void OutlineSolids_triangle(const double* x, const double* n)
{
	gp_Vec axis(0, 0, 1);
	gp_Vec norm = gp_Vec(gp_Pnt(x[0], x[1], x[2]), gp_Pnt(x[3], x[4], x[5])) ^ gp_Vec(gp_Pnt(x[0], x[1], x[2]), gp_Pnt(x[6], x[7], x[8]));
	if (norm * axis > 0)
	{
		CArea tri_area;
		CCurve curve;
		Point p0(x[0], x[1]);
		Point p1(x[3], x[4]);
		Point p2(x[6], x[7]);
		curve.m_vertices.push_back(p0);
		curve.m_vertices.push_back(p1);
		curve.m_vertices.push_back(p2);
		curve.m_vertices.push_back(p0);
		if (curve.GetArea() < 0)
		{
			OutlineSolids_curves.push_back(curve);
		}
	}
}

void OutlineSolids::Run(){
	bool save_fit_arcs = CArea::m_fit_arcs;
	CArea::m_fit_arcs = false;
	OutlineSolids_curves.clear();
	for (std::list<HeeksObj*>::const_iterator It = theApp->m_marked_list->list().begin(); It != theApp->m_marked_list->list().end(); It++){
		HeeksObj* object = *It;
		object->GetTriangles(OutlineSolids_triangle, 0.01);
	}

	if (OutlineSolids_curves.size() > 0)
	{
		CArea area = CArea::UniteCurves(OutlineSolids_curves);
		CArea new_area;
		for (std::list<CCurve>::iterator It = area.m_curves.begin(); It != area.m_curves.end(); It++)
		{
			CCurve& curve = *It;
			curve.Reverse();
			double a = curve.GetArea();
			if (fabs(a) > 0.001)
				new_area.append(curve);
		}

		if (theApp->m_fit_arcs_on_solid_outline)
			new_area.FitArcs();
		theApp->AddUndoably(MakeNewSketchFromArea(new_area), NULL, NULL);
	}
	CArea::m_fit_arcs = save_fit_arcs;

}

#endif