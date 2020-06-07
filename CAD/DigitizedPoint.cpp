// DigitizedPoint.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "DigitizedPoint.h"

DigitizedPoint::DigitizedPoint()
{
	m_type = DigitizeNoItemType;
	m_point = Point3d(0, 0, 0);
	m_object1 = NULL;
	m_object2 = NULL;
}

DigitizedPoint::DigitizedPoint(Point3d point, DigitizeType t, HeeksObj* object1, HeeksObj* object2)
{
	m_point = point;
	m_type = t;
	m_object1 = object1;
	m_object2 = object2;
}

int DigitizedPoint::importance(){
	switch (m_type){
	case DigitizeEndofType:
		return 10;

	case DigitizeIntersType:
		return 5;

	case DigitizeMidpointType:
		return 7;

	case DigitizeCentreType:
		return 7;

	case DigitizeNearestType:
		return 4;

	case DigitizeTangentType:
		return 3;

	default:
		return 0;
	}
}
