// TransformTool.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "TransformTool.h"
#include "HeeksObj.h"

TransformTool::TransformTool(HeeksObj *o, const Matrix &t, const Matrix &i){
	object = o;
	modify_matrix = t;
	revert_matrix = i;
}

TransformTool::~TransformTool(void){
}

std::wstring global_string;

// Tool's virtual functions
const wchar_t* TransformTool::GetTitle(){
//	global_string.assign(std::wstring::Format(_T("%s %s"), _("Transform"), object->GetShortStringOrTypeString()));
	global_string = L"Transform";
	return global_string.c_str();
}

void TransformTool::Run(bool redo){
	object->Transform(modify_matrix);
	theApp.WasModified(object);
}

void TransformTool::RollBack(){
	object->Transform(revert_matrix);
	theApp.WasModified(object);
}

TransformObjectsTool::TransformObjectsTool(const std::list<HeeksObj*> &list, const Matrix &t, const Matrix &i){
	m_list = list;
	modify_matrix = t;
	revert_matrix = i;
}

TransformObjectsTool::~TransformObjectsTool(void){
}

// Tool's virtual functions
const wchar_t* TransformObjectsTool::GetTitle(){
	return L"Transform Objects";
}

void TransformObjectsTool::Run(bool redo){
	std::list<HeeksObj*>::iterator It;
	for(It = m_list.begin(); It != m_list.end(); It++){
		HeeksObj* object = *It;
		object->Transform(modify_matrix);
	}
	theApp.WereModified(m_list);
}

void TransformObjectsTool::RollBack(){
	std::list<HeeksObj*>::const_iterator It;
	for(It = m_list.begin(); It != m_list.end(); It++){
		HeeksObj* object = *It;
		object->Transform(revert_matrix);
	}
	theApp.WereModified(m_list);
}
