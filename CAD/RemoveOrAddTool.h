// RemoveOrAddTool.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#pragma once

#include "Undoable.h"

class RemoveOrAddTool:public Undoable{
protected:
	HeeksObj* m_prev_object;
	bool m_belongs_to_owner;

	void Add();
	void Remove();

public:
	HeeksObj* m_owner;
	HeeksObj* m_object;

	RemoveOrAddTool(HeeksObj *object, HeeksObj *owner, HeeksObj* prev_object);
	virtual ~RemoveOrAddTool();
};

class AddObjectTool:public RemoveOrAddTool{
private:

public:
	AddObjectTool(HeeksObj *object, HeeksObj* owner, HeeksObj* prev_object):RemoveOrAddTool(object, owner, prev_object){}

	// Tool's virtual functions
	const wchar_t* GetTitle();
	void Run(bool redo);
	void RollBack();
};

class RemoveObjectTool:public RemoveOrAddTool{
public:
	RemoveObjectTool(HeeksObj *object);

	// Tool's virtual functions
	const wchar_t* GetTitle() {return L"Remove";}
	void Run(bool redo);
	void RollBack();
	std::wstring BitmapPath(){return L"delete";}
};

class ManyRemoveOrAddTool:public Undoable{
protected:
	std::list<HeeksObj*> m_objects;
	HeeksObj* m_owner;
	bool m_belongs_to_owner;

	void Add();
	void Remove();

public:
	ManyRemoveOrAddTool(const std::list<HeeksObj*> &list, HeeksObj *owner): m_objects(list), m_owner(owner), m_belongs_to_owner(false){}
	virtual ~ManyRemoveOrAddTool();
};

class AddObjectsTool:public ManyRemoveOrAddTool{
public:
	AddObjectsTool(const std::list<HeeksObj*> &list, HeeksObj *owner):ManyRemoveOrAddTool(list, owner){}

	// Tool's virtual functions
	const wchar_t* GetTitle();
	void Run(bool redo);
	void RollBack();
};

class RemoveObjectsTool:public ManyRemoveOrAddTool{
public:
	RemoveObjectsTool(const std::list<HeeksObj*> &list, HeeksObj *owner):ManyRemoveOrAddTool(list, owner){}

	// Tool's virtual functions
	const wchar_t* GetTitle();
	void Run(bool redo);
	void RollBack();
};

class CopyObjectUndoable:public Undoable{
	// object will be changed on do and undo
	// copy is a new object that must be made and will live in this Undoable object
protected:
	HeeksObj* m_object;
	HeeksObj* m_new_copy;
	HeeksObj* m_old_copy;

	// Undoable's virtual functions
	const wchar_t* GetTitle(){return L"CopyObject";}
	void Run(bool redo);
	void RollBack();

public:
	CopyObjectUndoable(HeeksObj* object, HeeksObj* copy_object);
	virtual ~CopyObjectUndoable();
};