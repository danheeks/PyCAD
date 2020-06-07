// DigitizedPoint.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once


enum DigitizeType{
	DigitizeNoItemType,
	DigitizeEndofType,
	DigitizeIntersType,
	DigitizeMidpointType,
	DigitizeCentreType,
	DigitizeScreenType,
	DigitizeCoordsType,
	DigitizeNearestType,
	DigitizeTangentType,
	DigitizeInputType // typed into properties, for example
};

class DigitizedPoint{
public:
	Point3d m_point;
	DigitizeType m_type;
	HeeksObj* m_object1;
	HeeksObj* m_object2;

	DigitizedPoint();
	DigitizedPoint(Point3d point, DigitizeType t, HeeksObj* object1 = NULL, HeeksObj* object2 = NULL);

	int importance();
};

