// Undo.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "HeeksObj.h"
#include "History.h"

History::History(const wchar_t* title, int Level)
{
	sub_history = NULL;
	level = Level;
	if (title != NULL)
		m_title.assign(title);
}

void History::Run(bool redo)
{
	if (sub_history)
	{
		sub_history->Run(redo);
		return;
	}
	for(std::list<Undoable *>::iterator It = m_undoables.begin(); It != m_undoables.end(); It++)
	{
		Undoable *u = *It;
		u->Run(redo);
	}
}

void History::RollBack(void)
{
	if (sub_history)
	{
		sub_history->RollBack();
		return;
	}
	for (std::list<Undoable *>::reverse_iterator It = m_undoables.rbegin(); It != m_undoables.rend(); It++)
	{
		Undoable *u = *It;
		u->RollBack();
	}
}

bool History::InternalRollBack(void)
{
	if (sub_history)
		return sub_history->InternalRollBack();
	if (!CanUndo())return false;
	Undoable *u;
	m_curpos--;
	u = *m_curpos;
	u->RollBack();
	return true;
}

bool History::InternalRollForward(void)
{
	if (sub_history)
		return sub_history->InternalRollForward();
	if (!CanRedo())return false;
	Undoable *u = *m_curpos;
	u->Run(true);
	m_curpos++;
	return true;
}

bool History::CanUndo(void)
{
	if (sub_history)
		return sub_history->CanUndo();
	if (m_undoables.size() == 0)return false;
	if(m_curpos == m_undoables.begin())return false;
	return true;
}

bool History::CanRedo(void)
{
	if (sub_history)
		return sub_history->CanRedo();
	if (m_undoables.size() == 0)return false;
	if(m_curpos == m_undoables.end())return false;
	return true;
}

void History::StartHistory(const wchar_t* title)
{
	if(sub_history)
	{
		sub_history->StartHistory(title);
	}
	else
	{
		sub_history = new History(title, level+1);
	}
}

bool History::EndHistory(void)
{
	if(sub_history)
	{
		if(sub_history->EndHistory())
		{
			if(sub_history->size()>0)Add(sub_history);
			else delete sub_history;
			sub_history = NULL;
		}
		return false;
	}
	else
	{
		return true;
	}
}

const wchar_t* History::GetUndoTitle()
{
	if (sub_history)
		return sub_history->GetUndoTitle();
	if (!CanUndo())return NULL;
	Undoable *u;
	m_curpos--;
	u = *m_curpos;
	return u->GetTitle();
}

const wchar_t* History::GetRedoTitle()
{
	if (sub_history)
		return sub_history->GetRedoTitle();
	if (!CanRedo())return NULL;
	Undoable *u = *m_curpos;
	m_curpos++;
	return u->GetTitle();
}

void History::DoUndoable(Undoable *u)
{
	if(u == NULL)return;
	if(sub_history)sub_history->DoUndoable(u);
	else
	{
		u->Run(false);
		Add(u);
	}
}

void History::Add(Undoable *u)
{
	if(m_undoables.size() > 0 && m_curpos != m_undoables.end())
	{
		ClearFromCurPos();
	}
	m_curpos = m_undoables.end();
	m_undoables.push_back(u);
	m_curpos = m_undoables.end();
}

void History::Clear(std::list<Undoable *>::iterator FromIt)
{
	if(m_undoables.size() == 0)return;
	std::list<Undoable *>::iterator It = m_undoables.end();
	It--;
	for(;; It--)
	{
		RemoveAsNewPosIfEqual(It);
		Undoable *u = *It;
		delete u;
		if(It == FromIt)break;
	}
	m_undoables.erase(FromIt, m_undoables.end());
}

void History::ClearFromFront(void)
{
	Clear(m_undoables.begin());
}

void History::ClearFromCurPos(void)
{
	Clear(m_curpos);
}

int History::GetLevel()const
{
	if (sub_history)
		return sub_history->GetLevel();
	return level;
}

History::~History(void)
{
	if(m_undoables.size() == 0)return;
	std::list<Undoable *>::iterator It;
	It = m_undoables.end();
	It--;
	for(;; It--)
	{
		Undoable *u = *It;
		if (u->CanBeDeleted())
			delete u;
		if(It == m_undoables.begin())break;
	}
}

bool MainHistory::IsModified(void)
{
	if(as_new_when_at_list_start)
	{
		if(size()==0)return false;
		if(m_curpos == m_undoables.begin())return false;
		return true;
	}
	if(size() == 0)return true;
	if(as_new_pos_exists)
	{
		std::list<Undoable *>::iterator TempIt = m_curpos;
		TempIt--;
		if(TempIt == as_new_pos)return false;
	}
	return true;
}

void MainHistory::RemoveAsNewPosIfEqual(std::list<Undoable *>::iterator &It)
{
	if(as_new_pos_exists && as_new_pos == It)
	{
		as_new_pos_exists = false;
	}
}

void MainHistory::SetLikeNewFile(void)
{
	if(size() == 0)
	{
		as_new_when_at_list_start = true;
		as_new_pos_exists = false;
	}
	else
	{
		std::list<Undoable *>::iterator TempIt = m_curpos;
		TempIt--;
		as_new_pos = TempIt;
		as_new_pos_exists = true;
		as_new_when_at_list_start = false;
	}
}

void MainHistory::DoUndoable(Undoable *u)
{
	History::DoUndoable(u);
}

void MainHistory::SetAsModified()
{
	as_new_when_at_list_start = false;
	as_new_pos_exists = false;
}

void MainHistory::StartHistory(const wchar_t* title, bool freeze_observers)
{
	if ((sub_history == NULL) && freeze_observers)
	{
		theApp->ObserversFreeze();
		observers_frozen = true;
	}
	History::StartHistory(title);
}

bool MainHistory::EndHistory(void)
{
	bool value = History::EndHistory();
	if ((sub_history == NULL) && observers_frozen)
	{
		theApp->ObserversThaw();
		observers_frozen = false;
	}
	return value;
}

