#pragma once
#include "Property.h"

PropertyList* PropertyVertex(HeeksObj* object, const wchar_t* title, double* p);
PropertyList* PropertyVertex(HeeksObj* object, const wchar_t* title, const double* p);
PropertyList* PropertyPnt(HeeksObj* object, const wchar_t* title, Point3d* pnt);
void PropertyPnt(std::list<Property*>* list, HeeksObj* object, Point3d* pnt);
PropertyList* PropertyTrsf(HeeksObj* object, const wchar_t* title, Matrix* trsf);
