// DigitizeMode.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "DigitizedPoint.h"

DigitizedPoint digitize1(const IPoint &input_point);
DigitizedPoint Digitize(const Line &ray);

// calculate tangent points
bool GetLinePoints(const DigitizedPoint& d1, const DigitizedPoint& d2, Point3d &p1, Point3d &p2);
bool GetArcPoints(const DigitizedPoint& d1, const Point3d *initial_direction, const DigitizedPoint& d2, Point3d &p1, Point3d &p2, Point3d &centre, Point3d &axis);
bool GetTangentCircle(const DigitizedPoint& d1, const DigitizedPoint& d2, const DigitizedPoint& d3, Circle& c);
bool GetCircleBetween(const DigitizedPoint& d1, const DigitizedPoint& d2, Circle& c);
