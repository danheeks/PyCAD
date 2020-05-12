// Box.cpp
// Copyright 2020, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "Box.h"

std::ostream & operator<<(std::ostream &os, const CBox &b)
{
	return os << "Box3D " << b.m_x[0] << ", " << b.m_x[1] << ", " << b.m_x[2] << ", " << b.m_x[3] << ", " << b.m_x[4] << ", " << b.m_x[5];
}

