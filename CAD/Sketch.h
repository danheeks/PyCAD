// Sketch.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "IdNamedObjList.h"
#include "HeeksColor.h"
#include "SketchOrder.h"

class CoordinateSystem;

class CSketch:public IdNamedObjList
{
	HeeksColor color;
	bool IsClockwise()const{return GetArea()>0;}

public:
	static std::string m_sketch_order_str[MaxSketchOrderTypes];
	SketchOrderType m_order;

	CSketch();
	CSketch(const CSketch& c);
	virtual ~CSketch();

	const CSketch& operator=(const CSketch& c);

	bool operator== ( const CSketch & rhs ) const;
	bool operator!= ( const CSketch & rhs ) const { return(! (*this == rhs)); }

	bool IsDifferent( HeeksObj *other ) { return(*this != (*(CSketch *)other)); }

	int GetType()const{return SketchType;}
	long GetMarkingMask()const{return MARKING_FILTER_SKETCH;}
	const wchar_t* GetTypeString(void)const{return L"Sketch";}
	const wchar_t* GetIconFilePath();
	void GetProperties(std::list<Property *> *list);
	HeeksObj *MakeACopy(void)const;
	void CopyFrom(const HeeksObj* object){operator=(*((CSketch*)((IdNamedObjList*)object)));}
	bool UsesID(){return true;}
	void SetColor(const HeeksColor &col);
	const HeeksColor* GetColor()const;
	bool Add(HeeksObj* object, HeeksObj* prev_object);
	void Remove(HeeksObj* object);

	void CalculateSketchOrder();
	SketchOrderType GetSketchOrder();
	bool ReOrderSketch(SketchOrderType new_order); // returns true if done
	void ReLinkSketch();
	void ReverseSketch();
	void ExtractSeparateSketches(std::list<HeeksObj*> &new_separate_sketches, const bool allow_individual_objects = false);
	int Intersects(const HeeksObj *object, std::list< double > *rl) const;
	HeeksObj *Parallel( const double distance );
	bool FilletAtPoint(const Point3d& p, double rad);
	static void ReverseObject(HeeksObj* object);
	double GetArea()const;
	bool IsCircle()const;
	bool IsClosed();
	bool HasMultipleSketches();
};

class CSketchRelinker{
	const std::list<HeeksObj*> &m_old_list;
	std::set<HeeksObj*> m_added_from_old_set;
	std::list<HeeksObj*>::const_iterator m_old_front;
	HeeksObj* m_new_back;
	HeeksObj* m_new_front;
	bool AddNext();
	bool TryAdd(HeeksObj* object);

public:
	std::list< std::list<HeeksObj*> > m_new_lists;

	CSketchRelinker(const std::list<HeeksObj*>& old_list):m_old_list(old_list), m_new_back(NULL), m_new_front(NULL){}

	bool Do(); // makes m_new_lists
};
