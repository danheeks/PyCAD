// InputMode.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.


#pragma once

#include "KeyCode.h"

class MouseEvent;

class CInputMode{
public:
	virtual ~CInputMode() {}

	virtual const wchar_t* GetTitle() = 0;
	virtual bool TitleHighlighted(){return true;}
	virtual const wchar_t* GetHelpText(){ return NULL; }
	virtual void OnMouse( MouseEvent& event ){}
	virtual void OnKeyDown( KeyCode key_code ){}
	virtual void OnKeyUp( KeyCode key_code ){}
	virtual bool OnModeChange(void){return true;}
	virtual void OnFrontRender(){}
	virtual void OnRender(){}
	virtual void GetProperties(std::list<Property *> *list){}
};
