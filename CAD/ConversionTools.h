#pragma once

#include "../Geom/Area.h"

void ObjectsToArea(const std::list<HeeksObj*> &objects, std::list<HeeksObj*> objects_to_delete, CArea& area);
CArea ObjectToArea(HeeksObj* object);