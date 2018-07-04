// TransformTool.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Undoable.h"

class TransformTool: public Undoable{
private:
	geoff_geometry::Matrix modify_matrix;
	geoff_geometry::Matrix revert_matrix;
	HeeksObj *object;

public:
	TransformTool(HeeksObj *o, const geoff_geometry::Matrix &t, const geoff_geometry::Matrix &i);
	~TransformTool(void);

	// Tool's virtual functions
	const wchar_t* GetTitle();
	void Run(bool redo);
	void RollBack();
};

class TransformObjectsTool: public Undoable{
private:
	geoff_geometry::Matrix modify_matrix;
	geoff_geometry::Matrix revert_matrix;
	std::list<HeeksObj*> m_list;

public:
	TransformObjectsTool(const std::list<HeeksObj*>& list, const geoff_geometry::Matrix &t, const geoff_geometry::Matrix &i);
	~TransformObjectsTool(void);

	// Tool's virtual functions
	const wchar_t* GetTitle();
	void Run(bool redo);
	void RollBack();
};
