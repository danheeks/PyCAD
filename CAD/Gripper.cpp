// Gripper.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Gripper.h"
#include "Material.h"
#include "HeeksColor.h"
#include "CoordinateSystem.h"

static unsigned char circle[18] = {0x1c, 0x00, 0x63, 0x00, 0x41, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x41, 0x00, 0x63, 0x00, 0x1c, 0x00};
static unsigned char translation_circle[30] = {0x00, 0x80, 0x01, 0xc0, 0x00, 0x80, 0x01, 0xc0, 0x06, 0x30, 0x04, 0x10, 0x28, 0x0a, 0x78, 0x0f, 0x28, 0x0a, 0x04, 0x10, 0x06, 0x30, 0x01, 0xc0, 0x00, 0x80, 0x01, 0xc0, 0x00, 0x80};
static unsigned char rotation_object_circle[26] = {0x01, 0xc0, 0x06, 0xb0, 0x04, 0x90, 0x08, 0x88, 0x0f, 0xf8, 0x08, 0x88, 0x04, 0x90, 0x06, 0xb1, 0x61, 0xc2, 0x60, 0x02, 0x10, 0x04, 0x0c, 0x18, 0x03, 0xe0};
static unsigned char angle_circle[32] = {0x10, 0x04, 0x10, 0x04, 0x08, 0x08, 0x08, 0x08, 0x04, 0x10, 0x04, 0x10, 0x02, 0x40, 0x03, 0xe0, 0x07, 0x70, 0x05, 0x50, 0x08, 0x88, 0x08, 0x88, 0x08, 0x08, 0x04, 0x10, 0x06, 0x30, 0x01, 0xc0};
static unsigned char square[18] = {0xff, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xff, 0x80};
static unsigned char cross[18] = {0x80, 0x80, 0x41, 0x00, 0x22, 0x00, 0x14, 0x00, 0x08, 0x00, 0x14, 0x00, 0x22, 0x00, 0x41, 0x00, 0x80, 0x80};
static unsigned char flower[18] = {0x1c, 0x00, 0x22, 0x00, 0x63, 0x00, 0x94, 0x80, 0x88, 0x80, 0x94, 0x80, 0x63, 0x00, 0x22, 0x00, 0x1c, 0x00};

Gripper::Gripper() :m_data(GripperTypeTranslate, Point3d(0.0, 0.0, 0.0))
{

}

Gripper::Gripper(const GripData& data, HeeksObj* parent) : m_data(data), m_gripper_parent(parent){
}

void Gripper::glCommands(bool select, bool marked, bool no_color){
	if(!no_color){
		theApp->glColorEnsuringContrast(HeeksColor(0, 0, 0));
	}

	if (theApp->m_dragging_moves_objects)
	{
			glRasterPos3d(m_data.m_p.x, m_data.m_p.y, m_data.m_p.z);
			switch(m_data.m_type){
		case GripperTypeTranslate:
			glBitmap(16, 15, 8, 7, 10.0, 0.0, translation_circle);
			break;
		case GripperTypeRotateObjectXY:
			glBitmap(16, 13, 8, 4, 10.0, 0.0, rotation_object_circle);
			break;
		case GripperTypeRotateObjectXZ:
			glBitmap(16, 13, 8, 4, 10.0, 0.0, rotation_object_circle);
			break;
		case GripperTypeRotateObjectYZ:
			glBitmap(16, 13, 8, 4, 10.0, 0.0, rotation_object_circle);
			break;
		case GripperTypeAngle:
			glBitmap(14, 16, 9, 11, 10.0, 0.0, angle_circle);
			break;
		case GripperTypeStretch:
			switch(m_data.m_alternative_icon)
			{
			case 1:
				glBitmap(9, 9, 4, 4, 10.0, 0.0, square);
				break;
			case 2:
				glBitmap(9, 9, 4, 4, 10.0, 0.0, cross);
				break;
			case 3:
				glBitmap(9, 9, 4, 4, 10.0, 0.0, flower);
				break;
			default:
				glBitmap(9, 9, 4, 4, 10.0, 0.0, circle);
				break;
			}
			break;
		case GripperTypeObjectScaleX:
		case GripperTypeObjectScaleY:
		case GripperTypeObjectScaleZ:
		case GripperTypeRotateObject:
		case GripperTypeRotate:
		case GripperTypeScale:
		{
			double s = 30.0 / theApp->GetPixelScale();
			if (m_data.m_type == GripperTypeScale)
				s *= 0.5;

			if (!no_color)
			{
				glEnable(GL_LIGHTING);
				glShadeModel(GL_SMOOTH);
			}
			glPushMatrix();

			glTranslated(m_data.m_p.x, m_data.m_p.y, m_data.m_p.z);

			if (m_gripper_parent)
			{
				Matrix object_m;
				m_gripper_parent->GetScaleAboutMatrix(object_m);
				object_m.Translate(-(Point3d(0, 0, 0).Transformed(object_m))); // remove matrix translation
				double m[16];
				object_m.GetTransposed(m);
				glMultMatrixd(m);
			}

			glScaled(s, s, s);
			
			switch (m_data.m_type){
			case GripperTypeObjectScaleX:
				glRotated(90, 0, 1, 0);
				if (!no_color)Material(HeeksColor(255, 0, 0)).glMaterial(1.0);
				CoordinateSystem::RenderArrow();
				break;
			case GripperTypeObjectScaleY:
				glRotated(90, -1, 0, 0);
				if (!no_color)Material(HeeksColor(0, 255, 0)).glMaterial(1.0);
				CoordinateSystem::RenderArrow();
				break;
			case GripperTypeObjectScaleZ:
				if (!no_color)Material(HeeksColor(0, 0, 255)).glMaterial(1.0);
				CoordinateSystem::RenderArrow();
				break;
			case GripperTypeRotateObject:
				if (!no_color)Material(HeeksColor(255, 255, 0)).glMaterial(1.0);
				CoordinateSystem::RenderRotateArrow();
				break;
			case GripperTypeScale:
				glScaled(0.6, 0.6, 0.6);
				if (!no_color)Material(HeeksColor(255, 165, 0)).glMaterial(1.0);
				CoordinateSystem::RenderFootballHexagons();
				//if (!no_color)Material(HeeksColor(255, 180, 180)).glMaterial(1.0);
				CoordinateSystem::RenderFootballPentagons();
			case GripperTypeRotate:
			default:
				if (!no_color)Material(HeeksColor(0, 255, 255)).glMaterial(1.0);
				CoordinateSystem::RenderRotateArrow();
				break;
			}

			glPopMatrix();
			if (!no_color)
			{
				glShadeModel(GL_FLAT);
				glDisable(GL_LIGHTING);
			}
		}
			break;
		default:
			glBitmap(9, 9, 4, 4, 10.0, 0.0, circle);
			break;
			}
	}
	else
	{
		glRasterPos3d(m_data.m_p.x, m_data.m_p.y, m_data.m_p.z);
		glBitmap(9, 9, 4, 4, 10.0, 0.0, circle);
	}
}

void Gripper::Transform(const Matrix &m){
	m_data.m_p = m_data.m_p.Transformed(m);
}
