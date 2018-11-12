// InputMode.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

//#include "HeeksObj.h"
//#include "Tool.h"

#pragma once

//class HeeksObj;
//class Tool;
class MouseEvent;
class KeyEvent;

class CInputMode{
public:
	virtual ~CInputMode() {}

	virtual const wchar_t* GetTitle() = 0;
	virtual bool TitleHighlighted(){return true;}
	virtual const wchar_t* GetHelpText(){ return NULL; }
	virtual void OnMouse( MouseEvent& event ){}
	virtual void OnKeyDown( KeyEvent& event){}
	virtual void OnKeyUp( KeyEvent& event){}
	virtual bool OnModeChange(void){return true;}
	virtual void OnFrontRender(){}
	virtual void OnRender(){}
	//virtual void GetProperties(std::list<Property *> *list){}
};
