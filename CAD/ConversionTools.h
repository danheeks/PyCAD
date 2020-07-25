#pragma once

#include "../Geom/Area.h"

void ObjectsToArea(const std::list<HeeksObj*> &objects, CArea& area, std::list<HeeksObj*> *objects_to_delete = NULL);
CArea ObjectToArea(HeeksObj* object);
void CombineSelectedSketches();
