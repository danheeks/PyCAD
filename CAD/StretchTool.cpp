// StretchTool.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "StretchTool.h"
#include "HeeksObj.h"

StretchTool::StretchTool(HeeksObj *object, const Point3d& p, const Point3d& shift, void* data){
	m_object = object;
	m_pos = p;
	m_shift = shift;
	m_undo_uses_add = false;
	m_data = data;
}

StretchTool::~StretchTool(void){
}

std::wstring stretch_function_string;

// Tool's virtual functions
const wchar_t* StretchTool::GetTitle(){
	stretch_function_string.assign(L"Stretch ");
	stretch_function_string.append(m_object->GetShortStringOrTypeString());
	return stretch_function_string.c_str();
}

void StretchTool::Run(bool redo){
	m_undo_uses_add = m_object->Stretch(m_pos, m_shift, m_data);
	m_new_pos= m_pos + m_shift;
}

void StretchTool::RollBack(){
	if(!m_undo_uses_add){
		Point3d unshift = -m_shift;
		m_object->Stretch(m_new_pos, unshift, m_data);
	}
}
