// HXml.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "HeeksObj.h"

class HXml: public HeeksObj{
public:
	TiXmlElement m_element;

	~HXml(void);
	HXml();
	HXml(TiXmlElement* pElem);

	// HeeksObj's virtual functions
	int GetType()const{return XmlType;}
	const wchar_t* GetTypeString(void)const{return L"Xml";}
	const wchar_t* GetShortString(void)const;
	HeeksObj *MakeACopy(void)const;
	const wchar_t* GetIconFilePath();
	void GetProperties(std::list<Property *> *list);
	void CopyFrom(const HeeksObj* object){operator=(*((HXml*)object));}
	void WriteToXML(TiXmlElement *element);
	void ReadFromXML(TiXmlElement *element);
};
