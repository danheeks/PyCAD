// History.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class HeeksObj;

#include "Undoable.h"

class History: public Undoable{
protected:
	std::list<Undoable *>::iterator m_curpos;
	std::list<Undoable *> m_undoables;
	std::wstring m_title;

protected:
	History *sub_history;
	int level;

	virtual void SetAsNewPos(std::list<Undoable *>::iterator &){}
	virtual void RemoveAsNewPosIfEqual(std::list<Undoable *>::iterator &){}

public:
	History(const wchar_t* title, int Level);
	virtual ~History(void);

	// Undoable's virtual functions
	const wchar_t* GetTitle(){ return m_title.c_str(); }
	void Run(bool redo);
	void RollBack();

	bool InternalRollBack(void);
	bool InternalRollForward(void);
	bool CanUndo(void);
	bool CanRedo(void);
	void DoUndoable(Undoable *);
	void Add(Undoable *);
	void StartHistory(const wchar_t* title);
	bool EndHistory(void);
	unsigned int size(void){return m_undoables.size();}
	void Clear(std::list<Undoable *>::iterator FromIt);
	void ClearFromFront(void);
	void ClearFromCurPos(void);
	int GetLevel()const;
	const wchar_t* GetUndoTitle();
	const wchar_t* GetRedoTitle();
};

class MainHistory: public History{
private:
	std::list<Undoable *>::iterator as_new_pos;
	bool as_new_pos_exists;
	bool as_new_when_at_list_start;
	bool observers_frozen;

	// History virtual function
	void SetAsNewPos(std::list<Undoable *>::iterator &It){as_new_pos = It; as_new_pos_exists = true;}
	void RemoveAsNewPosIfEqual(std::list<Undoable *>::iterator &It);

public:
	MainHistory(void) : History(NULL, 0){ as_new_pos_exists = false; as_new_when_at_list_start = true; observers_frozen = false; }
	~MainHistory(void){}

	bool IsModified(void);
	void SetLikeNewFile(void);
	void DoUndoable(Undoable *);
	void SetAsModified();

	void StartHistory(const wchar_t* title, bool freeze_observers = true);
	bool EndHistory(void);
};
