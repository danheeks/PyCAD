// GripperSelTransform.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "GripperSelTransform.h"
#include "MarkedList.h"
#include "DigitizeMode.h"
#include "Viewport.h"
#include "StretchTool.h"
#include "CadApp.h"
//#include "Sketch.h"
//#include "EndedObject.h"

GripperSelTransform::GripperSelTransform(const GripData& data, HeeksObj* parent):Gripper(data, parent){
}

bool GripperSelTransform::OnGripperGrabbed(bool show_grippers_on_drag, Point3d &from){
	m_initial_grip_pos = m_data.m_p;

	m_from = from;
	m_last_from = from;
	theApp->m_marked_list->gripping = true;
	if ( m_data.m_type <= GripperTypeObjectScaleXY )
	{
		theApp->CreateTransformGLList(theApp->m_marked_list->list(), show_grippers_on_drag);
		theApp->m_drag_matrix = Matrix();
		for (std::list<HeeksObj*>::iterator It = theApp->m_marked_list->list().begin(); It != theApp->m_marked_list->list().end(); It++){
			HeeksObj* object = *It;
			if(object->m_visible)theApp->m_hidden_for_drag.push_back(object);
			object->m_visible = false;
		}
	}

	return true;
}


void GripperSelTransform::OnGripperMoved( Point3d & from, const Point3d & to){
	if ( m_data.m_type == GripperTypeStretch)
	{
		bool stretch_done = false;

		Point3d shift;
		if(m_data.m_move_relative){
			shift = to - from;
		}
		else{
			shift = to - m_initial_grip_pos;
		}

		{
			std::list<HeeksObj *>::iterator It;
			for (It = theApp->m_marked_list->list().begin(); It != theApp->m_marked_list->list().end(); It++)
			{
				HeeksObj* object = *It;
				if(object)
				{
					Point3d p = m_data.m_p;
					stretch_done = object->StretchTemporary(p, shift,m_data.m_data);
				}
			}
		}
		
		if(stretch_done)
		{
			m_data.m_p += shift;
			from += shift;
			m_initial_grip_pos += shift;
		}

		theApp->Repaint(true);
		return;
	}

	Matrix object_m;

	if (theApp->m_marked_list->list().size() > 0)theApp->m_marked_list->list().front()->GetScaleAboutMatrix(object_m);

	MakeMatrix ( from, to, object_m, theApp->m_drag_matrix );

	theApp->Repaint();
}

void GripperSelTransform::OnGripperReleased(const Point3d & from, const Point3d & to)
{
	theApp->DestroyTransformGLList();

	theApp->StartHistory();

	std::list<HeeksObj *> copy_marked_list = theApp->m_marked_list->list();

	for (std::list<HeeksObj *>::iterator It = copy_marked_list.begin(); It != copy_marked_list.end(); It++)
	{
		HeeksObj* object = *It;
		if ( object == m_gripper_parent && m_data.m_type > GripperTypeScale )
		{
			Point3d shift;
			if(m_data.m_move_relative){
				shift = to - from;
			}
			else{
				shift = to - m_initial_grip_pos;
			}

			{
				if(object)theApp->DoUndoable(new StretchTool(object, m_initial_grip_pos, shift, m_data.m_data));
			}
			m_data.m_p += shift;
		}
		else
		{
			Matrix mat;
			Matrix object_m;
			if (theApp->m_marked_list->list().size() > 0)theApp->m_marked_list->list().front()->GetScaleAboutMatrix(object_m);
			MakeMatrix ( from, to, object_m, mat );
			theApp->TransformUndoably(object, mat);
		}
	}

	if ( m_data.m_type <= GripperTypeObjectScaleXY )
	{
		for(std::list<HeeksObj*>::iterator It = theApp->m_hidden_for_drag.begin(); It != theApp->m_hidden_for_drag.end(); It++)
		{
			HeeksObj* object = *It;
			object->m_visible = true;
		}
		theApp->m_hidden_for_drag.clear();
	}

	theApp->m_marked_list->gripping = false;
	theApp->EndHistory();
}

void GripperSelTransform::MakeMatrix(const Point3d &from, const Point3d &to, const Matrix& object_m, Matrix& mat)
{
	mat = Matrix();

	switch ( m_data.m_type )
	{
	case GripperTypeTranslate:
		mat.Translate(Point3d(from, to));
		break;
	case GripperTypeScale:
		{
			Point3d scale_centre_point = Point3d(0, 0, 0).Transformed(object_m);
			double dist = Point3d(from).Dist(scale_centre_point);
			if ( dist<0.00000001 )
			{
				return;
			}
			double scale = to.Dist( scale_centre_point ) /dist;
			mat.Translate(-scale_centre_point);
			mat.Scale(scale);
			mat.Translate(scale_centre_point);
	}
		break;
	case GripperTypeObjectScaleX:
		{
			Point3d object_x = Point3d(1, 0, 0).Transformed(object_m).Normalized();
			Point3d scale_centre_point = Point3d(0, 0, 0).Transformed(object_m);
			double old_x = from * object_x - scale_centre_point * object_x;
			double new_x = to * object_x - scale_centre_point * object_x;
			if(fabs(old_x) < 0.000000001)return;
			double scale = new_x/old_x;
			double m[16] = {scale, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
			Matrix invm = object_m.Inverse();
			mat = (invm * Matrix(m)) * object_m;
	}
		break;
	case GripperTypeObjectScaleY:
		{
			Point3d object_x = Point3d(0, 1, 0).Transformed(object_m).Normalized();
			Point3d scale_centre_point = Point3d(0, 0, 0).Transformed(object_m);
			double old_x = from * object_x - scale_centre_point * object_x;
			double new_x = to * object_x - scale_centre_point * object_x;
			if (fabs(old_x) < 0.000000001)return;
			double scale = new_x / old_x;
			double m[16] = { 1, 0, 0, 0, 0, scale, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
			Matrix invm = object_m.Inverse();
			mat = (invm * Matrix(m)) * object_m;
	}
		break;
	case GripperTypeObjectScaleZ:
		{
			Point3d object_x = Point3d(0, 0, 1).Transformed(object_m).Normalized();
			Point3d scale_centre_point = Point3d(0, 0, 0).Transformed(object_m);
			double old_x = from * object_x - scale_centre_point * object_x;
			double new_x = to * object_x - scale_centre_point * object_x;
			if (fabs(old_x) < 0.000000001)return;
			double scale = new_x / old_x;
			double m[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, scale, 0, 0, 0, 0, 1 };
			Matrix invm = object_m.Inverse();
			mat = (invm * Matrix(m)) * object_m;
	}
		break;
	case GripperTypeObjectScaleXY:
		{
			Point3d object_x = Point3d(1, 0, 0).Transformed(object_m).Normalized();
			Point3d scale_centre_point = Point3d(0, 0, 0).Transformed(object_m);
			double old_x = from * object_x - scale_centre_point * object_x;
			double new_x = to * object_x - scale_centre_point * object_x;
			if (fabs(old_x) < 0.000000001)return;
			double scale = new_x / old_x;
			double m[16] = { scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
			Matrix invm = object_m.Inverse();
			mat = (invm * Matrix(m)) * object_m;
	}
		break;
	case GripperTypeRotate:
	case GripperTypeRotateObject:
	case GripperTypeRotateObjectXY:
	case GripperTypeRotateObjectXZ:
	case GripperTypeRotateObjectYZ:
		{
			Point3d rotate_centre_point = Point3d(0, 0, 0).Transformed(object_m);
			Point3d start_to_end_vector(from, to);
			if ( start_to_end_vector.magnitude() <0.000001 ) return;
			Point3d start_vector(rotate_centre_point, from);
			Point3d end_vector(rotate_centre_point, to);
			if ( start_vector.magnitude() <0.000001 ) return;
			if ( end_vector.magnitude() <0.000001 ) return;
			mat.Translate(-rotate_centre_point);

			Point3d vx, vy;
			theApp->m_current_viewport->m_view_point.GetTwoAxes(vx, vy, false, 0);			
			Point3d rot_dir = vx ^ vy;
			rot_dir.Normalize();

			if(m_data.m_type == GripperTypeRotateObjectXY){
				// use object z axis
				Point3d object_x = Point3d(1, 0, 0).TransformedOnlyRotation(object_m).Normalized();
				Point3d object_y = Point3d(0, 1, 0).TransformedOnlyRotation(object_m).Normalized();
				Point3d object_z = Point3d(0, 0, 1).TransformedOnlyRotation(object_m).Normalized();
				rot_dir = object_z;
				vx = object_x;
				vy = object_y;
			}

			else if(m_data.m_type == GripperTypeRotateObjectXZ){
				// use object y axis
				Point3d object_x = Point3d(1, 0, 0).TransformedOnlyRotation(object_m).Normalized();
				Point3d object_y = Point3d(0, 1, 0).TransformedOnlyRotation(object_m).Normalized();
				Point3d object_z = Point3d(0, 0, 1).TransformedOnlyRotation(object_m).Normalized();
				rot_dir = object_y;
				vx = object_z;
				vy = object_x;
			}

			else if(m_data.m_type == GripperTypeRotateObjectYZ){
				// use object x axis
				Point3d object_x = Point3d(1, 0, 0).TransformedOnlyRotation(object_m).Normalized();
				Point3d object_y = Point3d(0, 1, 0).TransformedOnlyRotation(object_m).Normalized();
				Point3d object_z = Point3d(0, 0, 1).TransformedOnlyRotation(object_m).Normalized();
				rot_dir = object_x;
				vx = object_y;
				vy = object_z;
			}

			else if(m_data.m_type == GripperTypeRotateObject){
				// choose the closest object axis to use
				Point3d object_x = Point3d(1, 0, 0).TransformedOnlyRotation(object_m).Normalized();
				Point3d object_y = Point3d(0, 1, 0).TransformedOnlyRotation(object_m).Normalized();
				Point3d object_z = Point3d(0, 0, 1).TransformedOnlyRotation(object_m).Normalized();

				double dpx = fabs(rot_dir * object_x);
				double dpy = fabs(rot_dir * object_y);
				double dpz = fabs(rot_dir * object_z);
				if(dpx > dpy && dpx > dpz){
					// use object x axis
					rot_dir = object_x;
					vx = object_y;
					vy = object_z;
				}
				else if(dpy > dpz){
					// use object y axis
					rot_dir = object_y;
					vx = object_z;
					vy = object_x;
				}
				else{
					// use object z axis
					rot_dir = object_z;
					vx = object_x;
					vy = object_y;
				}
			}

			double sx = start_vector * vx;
			double sy = start_vector * vy;
			double ex = end_vector * vx;
			double ey = end_vector * vy;
			double angle = atan2(ey, ex) - atan2(sy, sx);
			Matrix t1;
			t1.Translate(-rotate_centre_point);
			mat = t1;
			Matrix t2;
			t2.Rotate(angle, rot_dir);
			mat.Multiply(t2);
			Matrix t3;
			t3.Translate(rotate_centre_point);
			mat.Multiply(t3);
	}
		break;
	default:
		break;
	}
}
