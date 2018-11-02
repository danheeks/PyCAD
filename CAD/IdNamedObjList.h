// IdNamedObjList.h
/*
 * Copyright (c) 2013, Dan Heeks
 * This program is released under the BSD license. See the file COPYING for
 * details.
 */

#pragma once

#include "ObjList.h"

class IdNamedObjList: public ObjList
{
public:
	std::wstring m_title;
	bool m_title_made_from_id;

	IdNamedObjList(const wchar_t* title = NULL)
	{
		if(title){m_title_made_from_id = false; m_title = std::wstring(title);}
		else {m_title_made_from_id = true;}
	}

	// HeeksObj's virtual functions
	void WriteBaseXML(TiXmlElement *element);
	void ReadBaseXML(TiXmlElement* element);
    const wchar_t* GetShortString(void)const;
    bool CanEditString(void)const{return true;}
    void OnEditString(const wchar_t* str);
};
