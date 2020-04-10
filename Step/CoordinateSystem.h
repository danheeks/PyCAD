// CoordinateSystem.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

void AxesToAngles(const gp_Dir &x, const gp_Dir &y, double &v_angle, double &h_angle, double &t_angle);
void AnglesToAxes(const double &v_angle, const double &h_angle, const double &t_angle, gp_Dir &x, gp_Dir &y);
void GetAx2Properties(std::list<Property *> *list, gp_Ax2& a, HeeksObj* object);
