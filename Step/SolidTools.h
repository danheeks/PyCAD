// SolidTools.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#if 0
#include "Tool.h"

void GetSolidMenuTools(std::list<Tool*>* t_list);

class SaveSolids : public Tool {
public:

	virtual void Run();
	const wchar_t* GetTitle(){ return L"Save Solids"; }
	std::wstring BitmapPath(){ return _T("saveas"); }
};

class OutlineSolids : public Tool {
public:

	virtual void Run();
	const wchar_t* GetTitle(){ return L"Make Outline Sketch Around Solids"; }
	std::wstring BitmapPath(){ return _T("outline"); }
};
#endif