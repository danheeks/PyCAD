// TransformTools.cpp
/*
 * Copyright (c) 2009, Dan Heeks
 * This program is released under the BSD license. See the file COPYING for
 * details.
 */

#include "stdafx.h"
#include "TransformTools.h"
#include "MarkedList.h"
#if 0
#include "HLine.h"
#include "HILine.h"
#endif
#include "HeeksConfig.h"

//static double from[3];
static double centre[3];

//static
void TransformTools::RemoveUncopyable()
{
	std::list<HeeksObj*> uncopyable_objects;
	for(std::list<HeeksObj*>::const_iterator It = theApp.m_marked_list->list().begin(); It != theApp.m_marked_list->list().end(); It++)
	{
		HeeksObj* object = *It;
		if(!object->CanBeCopied())uncopyable_objects.push_back(object);
	}
	if(uncopyable_objects.size() > 0)theApp.m_marked_list->Remove(uncopyable_objects, true);
}

//static
void TransformTools::Translate(bool copy)
{
	// pick items
	if(theApp.m_marked_list->size() == 0){
		theApp.PickObjects(L"Pick objects to move");
	}
	if(theApp.m_marked_list->size() == 0)return;

	// get number of copies
	HeeksConfig config;
	int ncopies;
	config.Read(L"TranslateNumCopies", &ncopies, 1);
	if(copy)
	{
		// check for uncopyable objects
		RemoveUncopyable();
		if(theApp.m_marked_list->size() == 0)return;
	}

	// clear the selection
	std::list<HeeksObj *> selected_items = theApp.m_marked_list->list();
	theApp.m_marked_list->Clear(true);

	double from[3], to[3];
	config.Read(L"TranslateFromX", &from[0], 0.0);
	config.Read(L"TranslateFromY", &from[1], 0.0);
	config.Read(L"TranslateFromZ", &from[2], 0.0);
	config.Read(L"TranslateToX", &to[0], 0.0);
	config.Read(L"TranslateToY", &to[1], 0.0);
	config.Read(L"TranslateToZ", &to[2], 0.0);

	//if(!theApp.InputFromAndTo(from, to, copy ? &ncopies : NULL))return;

	if(copy)
	{
		if(ncopies < 1)return;
		config.Write(L"TranslateNumCopies", ncopies);
	}
	config.Write(L"TranslateFromX", from[0]);
	config.Write(L"TranslateFromY", from[1]);
	config.Write(L"TranslateFromZ", from[2]);
	config.Write(L"TranslateToX", to[0]);
	config.Write(L"TranslateToY", to[1]);
	config.Write(L"TranslateToZ", to[2]);

	theApp.StartHistory();

#if 0
	to do
	// transform the objects
	if(copy)
	{
		for(int i = 0; i<ncopies; i++)
		{
			geoff_geometry::Matrix mat;
			mat.SetTranslationPart(make_vector(make_point(from), make_point(to)) * (i + 1));
			double m[16];
			extract(mat, m);
			for(std::list<HeeksObj*>::iterator It = selected_items.begin(); It != selected_items.end(); It++)
			{
				HeeksObj* object = *It;
				HeeksObj* new_object = object->MakeACopy();
				object->m_owner->Add(new_object, NULL);
				theApp.TransformUndoably(new_object, m);
			}
		}
		theApp.m_marked_list->Clear(true);
	}
	else
	{
		geoff_geometry::Matrix mat;
		mat.SetTranslationPart(make_vector(make_point(from), make_point(to)));
		double m[16];
		extract(mat, m);
		theApp.TransformUndoably(selected_items, m);
	}
#endif

	theApp.EndHistory();
}

//static
void TransformTools::Rotate(bool copy)
{
#if 0
	//rotation axis - Z axis by default
	geoff_geometry::Point3d axis_Dir = geoff_geometry::Point3d(0,0,1);
	geoff_geometry::Point3d line_Pos = geoff_geometry::Point3d(0,0,0);

	// pick items
	if(theApp.m_marked_list->size() == 0){
		theApp.PickObjects(_("Pick objects to rotate"));
	}
	if(theApp.m_marked_list->size() == 0)return;

	// get number of copies
	HeeksConfig config;
	int ncopies;
	config.Read(_T("RotateNumCopies"), &ncopies, 1);
	if(copy)
	{
		// check for uncopyable objects
		RemoveUncopyable();
		if(theApp.m_marked_list->size() == 0)return;
	}

	// clear the selection
	std::list<HeeksObj *> selected_items = theApp.m_marked_list->list();
	theApp.m_marked_list->Clear(true);

	double angle;
	config.Read(_T("RotateAngle"), &angle, 90.0);

	// enter angle, plane and position
	double axis[3];
	double pos[3];
	config.Read(_T("RotateAxisX"), &axis[0], 0.0);
	config.Read(_T("RotateAxisY"), &axis[1], 0.0);
	config.Read(_T("RotateAxisZ"), &axis[2], 1.0);
	config.Read(_T("RotatePosX"), &pos[0], 0.0);
	config.Read(_T("RotatePosY"), &pos[1], 0.0);
	config.Read(_T("RotatePosZ"), &pos[2], 0.0);

	double axial_shift;
	config.Read(_T("RotateAxialShift"), &axial_shift, 0.0);

	if(!theApp.InputAngleWithPlane(angle, axis, pos, copy ? &ncopies : NULL, &axial_shift))return;
	if(copy)
	{
		if(ncopies < 1)return;
		config.Write(_T("RotateNumCopies"), ncopies);
	}
	config.Write(_T("RotateAngle"), angle);
	config.Write(_T("RotateAxialShift"), axial_shift);
	config.Write(_T("RotateAxisX"), axis[0]);
	config.Write(_T("RotateAxisY"), axis[1]);
	config.Write(_T("RotateAxisZ"), axis[2]);
	config.Write(_T("RotatePosX"), pos[0]);
	config.Write(_T("RotatePosY"), pos[1]);
	config.Write(_T("RotatePosZ"), pos[2]);
	axis_Dir = geoff_geometry::Point3d(axis[0], axis[1], axis[2]);
	line_Pos = geoff_geometry::Point3d(pos[0], pos[1], pos[2]);

	// transform the objects
	theApp.StartHistory();
	if(copy)
	{
		for(int i = 0; i<ncopies; i++)
		{
			geoff_geometry::Matrix mat;
			mat.SetRotation(gp_Ax1(line_Pos, axis_Dir), angle * M_PI/180 * (i+1));
			geoff_geometry::Matrix tmat;
			tmat.SetTranslation(geoff_geometry::Point3d(axis_Dir.XYZ() * (axial_shift * ((double)(i+1)) / ncopies)));
			mat = tmat * mat;
			double m[16];
			extract(mat, m);
			for(std::list<HeeksObj*>::iterator It = selected_items.begin(); It != selected_items.end(); It++)
			{
				HeeksObj* object = *It;
				HeeksObj* new_object = object->MakeACopy();
				theApp.TransformUndoably(new_object, m);             // Rotate the duplicate object.
				theApp.AddUndoably(new_object, object->m_owner, NULL);// And add it to this object's owner
			}
		}
		theApp.m_marked_list->Clear(true);
	}
	else
	{
		geoff_geometry::Matrix mat;
		mat.SetRotation(gp_Ax1(line_Pos, axis_Dir), angle * M_PI/180);
		geoff_geometry::Matrix tmat;
		tmat.SetTranslation(geoff_geometry::Point3d(axis_Dir.XYZ() * axial_shift));
		mat = tmat * mat;
		double m[16];
		extract(mat, m);
		theApp.TransformUndoably(selected_items, m);
	}
	theApp.EndHistory();
#endif
}

//static
void TransformTools::Mirror(bool copy)
{
#if 0
	to do
	// pick items
	if(theApp.m_marked_list->size() == 0){
		theApp.PickObjects(_("Pick objects to mirror"));
	}
	if(theApp.m_marked_list->size() == 0)return;

	if(copy)
	{
		// check for uncopyable objects
		RemoveUncopyable();
		if(theApp.m_marked_list->size() == 0)return;
	}

	// clear the selection
	std::list<HeeksObj *> selected_items = theApp.m_marked_list->list();
	theApp.m_marked_list->Clear(true);

	// pick a line to mirror about
	bool line_found = false;
	geoff_geometry::Line line;
	int save_filter = theApp.m_marked_list->m_filter;
	theApp.PickObjects(_("Pick line to mirror about"), MARKING_FILTER_LINE | MARKING_FILTER_ILINE, true);
	theApp.m_marked_list->m_filter = save_filter;
	for(std::list<HeeksObj *>::const_iterator It = theApp.m_marked_list->list().begin(); It != theApp.m_marked_list->list().end(); It++)
	{
		HeeksObj* object = *It;
		if(object->GetType() == LineType)
		{
			line = ((HLine*)object)->GetLine();
			line_found = true;
		}
		else if(object->GetType() == ILineType)
		{
			line = ((HILine*)object)->GetLine();
			line_found = true;
		}
	}
	if(!line_found)return;

	// transform the objects
	theApp.StartHistory();
	geoff_geometry::Matrix mat;
	mat.SetMirror(gp_Ax1(line.Location(), line.Direction()));
	double m[16];
	extract(mat, m);

	if(copy)
	{
		for(std::list<HeeksObj*>::iterator It = selected_items.begin(); It != selected_items.end(); It++)
		{
			HeeksObj* object = *It;
			HeeksObj* new_object = object->MakeACopy();
			theApp.AddUndoably(new_object, object->m_owner, NULL);
			theApp.TransformUndoably(new_object, m);
		}
		theApp.m_marked_list->Clear(true);
	}
	else
	{
		theApp.TransformUndoably(selected_items, m);
	}
	theApp.EndHistory();
#endif
}

void TransformTools::Scale(bool copy)
{
#if 0
	to do
	// pick items
	if(theApp.m_marked_list->size() == 0){
		theApp.PickObjects(_("Pick objects to scale"));
	}
	if(theApp.m_marked_list->size() == 0)return;

	// get number of copies
	int ncopies;
	HeeksConfig config;
	config.Read(_T("ScaleNumCopies"), &ncopies, 1);
	if(copy)
	{
		// check for uncopyable objects
		RemoveUncopyable();
		if(theApp.m_marked_list->size() == 0)return;

		// input "number of copies"
		if(!theApp.InputInt(_("Enter number of copies"), _("number of copies"), ncopies))return;
		if(ncopies < 1)return;
		config.Write(_T("ScaleNumCopies"), ncopies);
	}

	// clear the selection
	std::list<HeeksObj *> selected_items = theApp.m_marked_list->list();
	theApp.m_marked_list->Clear(true);

	// pick "centre" position
	if(!theApp.PickPosition(_("Click centre position to scale about"), centre))return;

	// enter scale factor
	double scale;
	config.Read(_T("ScaleFactor"), &scale, 2.0);
	if(!theApp.InputDouble(_("Enter scale factor"), _("scale factor"), scale))return;
	config.Write(_T("ScaleFactor"), scale);

	// transform the objects
	theApp.StartHistory();
	if(copy)
	{
		for(int i = 0; i<ncopies; i++)
		{
			geoff_geometry::Matrix mat;
			mat.SetScale(make_point(centre), scale * (i+1));
			double m[16];
			extract(mat, m);
			for(std::list<HeeksObj*>::iterator It = selected_items.begin(); It != selected_items.end(); It++)
			{
				HeeksObj* object = *It;
				HeeksObj* new_object = object->MakeACopy();
				theApp.AddUndoably(new_object, object->m_owner, NULL);
				theApp.TransformUndoably(new_object, m);
			}
		}
		theApp.m_marked_list->Clear(true);
	}
	else
	{
		geoff_geometry::Matrix mat;
		mat.SetScale(make_point(centre), scale);
		double m[16];
		extract(mat, m);
		theApp.TransformUndoably(selected_items, m);
	}
	theApp.EndHistory();
#endif
}
