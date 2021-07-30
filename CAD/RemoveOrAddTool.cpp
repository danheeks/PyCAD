// RemoveOrAddTool.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

#include "RemoveOrAddTool.h"
#include "HeeksObj.h"
#include "MarkedList.h"

RemoveOrAddTool::RemoveOrAddTool(HeeksObj *object, HeeksObj *owner, HeeksObj *add_before) : m_belongs_to_owner(false)
{
	m_object = object;
	m_owner = owner;
	m_add_before = add_before;
}

RemoveOrAddTool::~RemoveOrAddTool()
{
	if(m_owner == NULL)return;
	if(!m_belongs_to_owner)if(!(m_object->NeverDelete()))delete m_object;
}

static std::wstring string_for_GetTitle;

const wchar_t* AddObjectTool::GetTitle()
{
	string_for_GetTitle.assign(L"Add ");
	string_for_GetTitle.append(m_object->GetShortStringOrTypeString());
	return string_for_GetTitle.c_str();
}

void RemoveOrAddTool::Add()
{

	if (theApp->m_doing_rollback && (m_owner == NULL))
	{
		m_owner = NULL;
	}

	if (m_owner == NULL)
	{
		theApp->DoMessageBox(L"Can't Have NULL owner!");
		return;
	}

	m_owner->Add(m_object, m_add_before);
	m_object->m_owner = m_owner;

	theApp->WasAdded(m_object);
	theApp->WasModified(m_owner);

	m_belongs_to_owner = true;
}

void RemoveOrAddTool::Remove()
{
	if (m_object->m_owner)
	{
		// to do multiple owners
		m_owner = m_object->m_owner;
		m_owner->Remove(m_object);
		theApp->WasRemoved(m_object);
		theApp->WasModified(m_owner);
		m_object->m_owner = NULL;
	}
	m_belongs_to_owner = false;
}

void AddObjectTool::Run(bool redo)
{
	Add();
}

void AddObjectTool::RollBack()
{
	Remove();
}

RemoveObjectTool::RemoveObjectTool(HeeksObj *object) :RemoveOrAddTool(object, object->m_owner, GetAddBefore(object))
{
	if(object)m_owner = object->m_owner;
	else m_owner = NULL;
}

void RemoveObjectTool::Run(bool redo)
{
	Remove();
}

void RemoveObjectTool::RollBack()
{
	Add();
}

HeeksObj* RemoveObjectTool::GetAddBefore(HeeksObj* object)
{
	for (HeeksObj* child = object->m_owner->GetFirstChild(); child; child = object->m_owner->GetNextChild())
	{
		if (child == object)
			return object->m_owner->GetNextChild();
	}
	return NULL;
}

AddObjectsTool::AddObjectsTool(const std::list<HeeksObj*> &list, HeeksObj *owner) :m_objects(list), m_owner(owner){}

AddObjectsTool::~AddObjectsTool()
{
	if(!m_belongs_to_owner){
		std::list<HeeksObj*>::iterator It;
		for(It = m_objects.begin(); It != m_objects.end(); It++){
			HeeksObj* object = *It;
            delete object;
		}
	}
}

void AddObjectsTool::Add()
{
	if (m_owner == NULL)
	{
		theApp->DoMessageBox(L"Can't have NULL owner!");
		return;
	}

	std::list<HeeksObj*>::iterator It;
	for(It = m_objects.begin(); It != m_objects.end(); It++){
		HeeksObj* object = *It;
		m_owner->Add(object, NULL);
		object->m_owner = m_owner;
	}

	theApp->WereAdded(m_objects);
	theApp->WasModified(m_owner);

	m_belongs_to_owner = true;
}

void AddObjectsTool::Remove()
{
	std::list<HeeksObj*>::iterator It;
	for(It = m_objects.begin(); It != m_objects.end(); It++){
		HeeksObj* object = *It;
		m_owner->Remove(object);
		theApp->m_marked_list->Remove(object, false);
	}

	theApp->WereRemoved(m_objects);
	theApp->WasModified(m_owner);
	for(It = m_objects.begin(); It != m_objects.end(); It++){
		HeeksObj* object = *It;
		object->m_owner = NULL;
	}

	m_belongs_to_owner = false;
}

const wchar_t* AddObjectsTool::GetTitle()
{
	return L"Add Objects";
}

void AddObjectsTool::Run(bool redo)
{
	Add();
}

void AddObjectsTool::RollBack()
{
	Remove();
}

CopyObjectUndoable::CopyObjectUndoable(HeeksObj* object, HeeksObj* copy_object): m_object(object), m_new_copy(copy_object)
{
	m_old_copy = m_object->MakeACopy();
	if(m_old_copy->IsList())((ObjList*)m_old_copy)->Clear();
	if(m_new_copy->IsList())((ObjList*)m_new_copy)->Clear();
}

CopyObjectUndoable::~CopyObjectUndoable()
{
	delete m_new_copy;
	delete m_old_copy;
}

void CopyObjectUndoable::Run(bool redo)
{
	m_object->CopyFrom(m_new_copy);
	theApp->WasModified(m_object);
}

void CopyObjectUndoable::RollBack()
{
	m_object->CopyFrom(m_old_copy);
	theApp->WasModified(m_object);
}
