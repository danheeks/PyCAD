// ObjList.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "HeeksObj.h"
#include "Undoable.h"

#include <list>
#include <vector>
#include <set>

class ObjList : public HeeksObj
{
	friend class ReorderTool;

protected:
	std::list<HeeksObj*> m_objects;
	std::list<HeeksObj*>::iterator LoopIt;
	std::list<std::list<HeeksObj*>::iterator> LoopItStack;
	std::vector<HeeksObj*> m_index_list; // for quick performance of GetAtIndex();
	bool m_index_list_valid;

	void recalculate_index_list();

public:
	ObjList():m_index_list_valid(true){}
	ObjList(const ObjList& objlist);
	virtual ~ObjList(){}

	const ObjList& operator=(const ObjList& objlist);

	bool operator==( const ObjList & rhs ) const;
	bool operator!=( const ObjList & rhs ) const { return(! (*this == rhs)); }
	bool IsDifferent(HeeksObj *other) { return( *this != (*(ObjList *)other) ); }

	void ClearUndoably(void);
	void Clear(std::set<HeeksObj*> &to_delete);

	HeeksObj* MakeACopy(void) const;
	void CopyFrom(const HeeksObj* object);
	void GetBox(CBox &box);
	void glCommands(bool select, bool marked, bool no_color);
	HeeksObj* GetFirstChild();
	HeeksObj* GetNextChild();
	HeeksObj* GetAtIndex(int index);
	int GetNumChildren();
	std::list<HeeksObj *> GetChildren() const;
	bool CanAdd(HeeksObj* object){return true;}
	virtual bool Add(HeeksObj* object, HeeksObj* prev_object);
	virtual void Add(std::list<HeeksObj*> objects);
	virtual void Remove(HeeksObj* object);
	virtual void Remove(std::list<HeeksObj*> objects);
	void Clear();
	void KillGLLists(void);
	void WriteToXML(TiXmlElement *element);
	void ReadFromXML(TiXmlElement *element);
	void Transform(const Matrix& m);
	void GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal = true);
	void GetSegments(void(*callbackfunc)(const double *p, bool start), double pixels_per_mm)const;
	bool IsList(){ return true; }
	void GetProperties(std::list<Property *> *list);
	void ReloadPointers();
	void OnChangeViewUnits(const double units);
	bool SetClickMarkPoint(const Point3d &ray_start, const Point3d &ray_direction);

	HeeksObj *Find( const int type, const unsigned int id );	// Search for an object by type/id from this or any child objects.
};


class ReorderTool: public Undoable
{
	ObjList* m_object;
	std::list<HeeksObj *> m_original_order;
	std::list<HeeksObj *> m_new_order;

public:
	ReorderTool(ObjList* object, std::list<HeeksObj *> &new_order);
	const wchar_t* GetTitle(){return L"Reorder";}
	void Run(bool redo);
	void RollBack();
};
