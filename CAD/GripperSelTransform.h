// GripperSelTransform.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#if !defined GripperSelTransform_HEADER
#define GripperSelTransform_HEADER

#include "Gripper.h"

class GripperSelTransform:public Gripper{
public:
	Point3d m_from;
	Point3d m_last_from;
	Point3d m_initial_grip_pos;

	GripperSelTransform(const GripData& data, HeeksObj* parent);

	// HeeksObj's virtual functions
	HeeksObj *MakeACopy(void)const{ return new GripperSelTransform(*this);}

	// virtual functions
	void MakeMatrix(const Point3d &from, const Point3d &to, const Matrix& object_m, Matrix& mat);

	//Gripper's virtual functions
	void OnGripperMoved(Point3d & from, const Point3d & to);
	bool OnGripperGrabbed(bool show_grippers_on_drag, Point3d & from);
	void OnGripperReleased(const Point3d & from, const Point3d & to);
};

#endif
