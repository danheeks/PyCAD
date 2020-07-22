// Undoable.h
// Copyright (c) 2018, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class Undoable
{
public:
	virtual ~Undoable() {};
	virtual void Run(bool redo) = 0;
	virtual const wchar_t* GetTitle() = 0;
	virtual void RollBack(){};
	virtual bool CanBeDeleted(){ return true; }
};
