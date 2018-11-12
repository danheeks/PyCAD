// GripperSelTransform.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "GripperSelTransform.h"
#include "MarkedList.h"
#include "DigitizeMode.h"
#include "Viewport.h"
//#include "Sketch.h"
//#include "EndedObject.h"

GripperSelTransform::GripperSelTransform(const GripData& data, HeeksObj* parent):Gripper(data, parent){
}

bool GripperSelTransform::OnGripperGrabbed(const std::list<HeeksObj*>& list, bool show_grippers_on_drag, double* from){
	m_initial_grip_pos[0] = m_data.m_x;
	m_initial_grip_pos[1] = m_data.m_y;
	m_initial_grip_pos[2] = m_data.m_z;

	memcpy(m_from, from, 3*sizeof(double));
	memcpy(m_last_from, from, 3*sizeof(double));
	theApp.m_marked_list->gripping = true;
	m_items_marked_at_grab.clear();
	std::list<HeeksObj *>::const_iterator It;
	for(It = list.begin(); It != list.end(); It++){
		m_items_marked_at_grab.push_back(*It);
		theApp.m_marked_list->set_ignore_onoff(*It, true);
	}
	if ( m_data.m_type <= GripperTypeObjectScaleXY )
	{
		theApp.CreateTransformGLList(list, show_grippers_on_drag);
		theApp.m_drag_matrix = Matrix();
		for(It = list.begin(); It != list.end(); It++){
			HeeksObj* object = *It;
			if(object->m_visible)theApp.m_hidden_for_drag.push_back(object);
			object->m_visible = false;
		}
	}
	return true;
}


void GripperSelTransform::OnGripperMoved( double* from, const double* to ){
	if ( m_data.m_type == GripperTypeStretch)
	{
		bool stretch_done = false;

		double shift[3];
		if(m_data.m_move_relative){
			shift[0] = to[0] - from[0];
			shift[1] = to[1] - from[1];
			shift[2] = to[2] - from[2];
		}
		else{
			shift[0] = to[0] - m_initial_grip_pos[0];
			shift[1] = to[1] - m_initial_grip_pos[1];
			shift[2] = to[2] - m_initial_grip_pos[2];
		}

		{
			std::list<HeeksObj *>::iterator It;
			for ( It = m_items_marked_at_grab.begin(); It != m_items_marked_at_grab.end(); It++ )
			{
				HeeksObj* object = *It;
				if(object)
				{
					double p[3] = {m_data.m_x, m_data.m_y, m_data.m_z};
					stretch_done = object->StretchTemporary(p, shift,m_data.m_data);
				}
			}
		}
		
		if(stretch_done)
		{
			m_data.m_x += shift[0];
			m_data.m_y += shift[1];
			m_data.m_z += shift[2];
			from[0] += shift[0];
			from[1] += shift[1];
			from[2] += shift[2];
			m_initial_grip_pos[0] += shift[0];
			m_initial_grip_pos[1] += shift[1];
			m_initial_grip_pos[2] += shift[2];
		}

		theApp.Repaint(true);
		return;
	}

	Matrix object_m;

	if(m_items_marked_at_grab.size() > 0)m_items_marked_at_grab.front()->GetScaleAboutMatrix(object_m);

	MakeMatrix ( from, to, object_m, theApp.m_drag_matrix );

	theApp.Repaint();
}

void GripperSelTransform::OnGripperReleased ( const double* from, const double* to )
{
	theApp.DestroyTransformGLList();

	theApp.StartHistory();

	for ( std::list<HeeksObj *>::iterator It = m_items_marked_at_grab.begin(); It != m_items_marked_at_grab.end(); It++ )
	{
		HeeksObj* object = *It;
		if ( object == m_gripper_parent && m_data.m_type > GripperTypeScale )
		{
			double shift[3];
			if(m_data.m_move_relative){
				shift[0] = to[0] - from[0];
				shift[1] = to[1] - from[1];
				shift[2] = to[2] - from[2];
			}
			else{
				shift[0] = to[0] - m_initial_grip_pos[0];
				shift[1] = to[1] - m_initial_grip_pos[1];
				shift[2] = to[2] - m_initial_grip_pos[2];
			}

			{
#if 0
				to do
				if(object)theApp.DoUndoable(new StretchTool(object, m_initial_grip_pos, shift, m_data.m_data));
#endif
			}
			m_data.m_x += shift[0];
			m_data.m_y += shift[1];
			m_data.m_z += shift[2];
		}
		else
		{
			Matrix mat;
			Matrix object_m;
			if(m_items_marked_at_grab.size() > 0)m_items_marked_at_grab.front()->GetScaleAboutMatrix(object_m);
			MakeMatrix ( from, to, object_m, mat );
			theApp.TransformUndoably(object, mat);
		}
	}

	m_items_marked_at_grab.clear();

	if ( m_data.m_type <= GripperTypeObjectScaleXY )
	{
		for(std::list<HeeksObj*>::iterator It = theApp.m_hidden_for_drag.begin(); It != theApp.m_hidden_for_drag.end(); It++)
		{
			HeeksObj* object = *It;
			object->m_visible = true;
		}
		theApp.m_hidden_for_drag.clear();
	}

	{
		std::list<HeeksObj *>::iterator It;
		for ( It = m_items_marked_at_grab.begin(); It != m_items_marked_at_grab.end(); It++ )
		{
			theApp.m_marked_list->set_ignore_onoff ( *It, false );
		}
	}
	theApp.m_marked_list->gripping = false;
	theApp.EndHistory();
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
			Matrix invm = object_m;
			invm.Inverse();
			mat = object_m * Matrix(m) * invm;
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
			Matrix invm = object_m;
			invm.Inverse();
			mat = object_m * Matrix(m) * invm;
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
			Matrix invm = object_m;
			invm.Inverse();
			mat = object_m * Matrix(m) * invm;
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
			double m[16] = { scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, scale };
			Matrix invm = object_m;
			invm.Inverse();
			mat = object_m * Matrix(m) * invm;
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
			theApp.m_current_viewport->m_view_point.GetTwoAxes(vx, vy, false, 0);			
			Point3d rot_dir = vx ^ vy;
			rot_dir.Normalize();

			if(m_data.m_type == GripperTypeRotateObjectXY){
				// use object z axis
				Point3d object_x = Point3d(1, 0, 0).Transformed(object_m).Normalized();
				Point3d object_y = Point3d(0, 1, 0).Transformed(object_m).Normalized();
				Point3d object_z = Point3d(0, 0, 1).Transformed(object_m).Normalized();
				rot_dir = object_z;
				vx = object_x;
				vy = object_y;
			}

			else if(m_data.m_type == GripperTypeRotateObjectXZ){
				// use object y axis
				Point3d object_x = Point3d(1, 0, 0).Transformed(object_m).Normalized();
				Point3d object_y = Point3d(0, 1, 0).Transformed(object_m).Normalized();
				Point3d object_z = Point3d(0, 0, 1).Transformed(object_m).Normalized();
				rot_dir = object_y;
				vx = object_z;
				vy = object_x;
			}

			else if(m_data.m_type == GripperTypeRotateObjectYZ){
				// use object x axis
				Point3d object_x = Point3d(1, 0, 0).Transformed(object_m).Normalized();
				Point3d object_y = Point3d(0, 1, 0).Transformed(object_m).Normalized();
				Point3d object_z = Point3d(0, 0, 1).Transformed(object_m).Normalized();
				rot_dir = object_x;
				vx = object_y;
				vy = object_z;
			}

			else if(m_data.m_type == GripperTypeRotateObject){
				// choose the closest object axis to use
				Point3d object_x = Point3d(1, 0, 0).Transformed(object_m).Normalized();
				Point3d object_y = Point3d(0, 1, 0).Transformed(object_m).Normalized();
				Point3d object_z = Point3d(0, 0, 1).Transformed(object_m).Normalized();

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

#if 0 
			to do
			gp_Ax1 rot_axis(rotate_centre_point, rot_dir);
			double sx = start_vector * vx;
			double sy = start_vector * vy;
			double ex = end_vector * vx;
			double ey = end_vector * vy;
			double angle = Point3d(sx, sy, 0).AngleWithRef(Point3d(ex, ey, 0), Point3d(0,0,1));
			mat.SetRotation(rot_axis, angle);
#endif
		}
		break;
	default:
		break;
	}
}
