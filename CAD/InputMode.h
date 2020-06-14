// InputMode.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.


#pragma once

#include "KeyCode.h"

class MouseEvent;

enum InputModeType
{
	InputModeTypeDigitize = 1,
	InputModeTypeNumberOfTypes,
};

class CInputMode{
public:
	virtual ~CInputMode() {}

	virtual const wchar_t* GetTitle() = 0;
	virtual bool IsDrawing(){ return false; }
	virtual int GetType(){ return 0; }
	virtual bool TitleHighlighted(){return true;}
	virtual const wchar_t* GetHelpText(){ return NULL; }
	virtual void OnMouse( MouseEvent& event ){}
	virtual bool OnKeyDown(KeyCode key_code){ return false; } // return true if handled
	virtual bool OnKeyUp(KeyCode key_code){ return false; } // return true if handled
	virtual void OnModeChange(void){ }
	virtual void OnFrontRender(){}
	virtual void OnRender(){}
	virtual void GetProperties(std::list<Property *> *list){}
};
