// StretchTool.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#if !defined StretchTool_HEADER
#define StretchTool_HEADER

#include "Undoable.h"

class StretchTool: public Undoable{
private:
	Point3d m_pos;
	Point3d m_shift;
	Point3d m_new_pos;
	void* m_data;
	HeeksObj *m_object;
	bool m_undo_uses_add;

public:
	StretchTool(HeeksObj *object, const Point3d& p, const Point3d& shift, void* data);
	~StretchTool(void);

	// Tool's virtual functions
	const wchar_t* GetTitle();
	void Run(bool redo);
	void RollBack();
};

#endif
