// HXml.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HXml.h"
#include "strconv.h"

HXml::HXml():m_element(TiXmlElement("")){
}

HXml::HXml(TiXmlElement* pElem) : m_element(*pElem){
}

HXml::~HXml(){
}

HeeksObj *HXml::MakeACopy(void)const{
	HXml *new_object = new HXml(*this);
	return new_object;
}

void HXml::GetProperties(std::list<Property *> *list){

	HeeksObj::GetProperties(list);
}

const wchar_t* HXml::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/xml.png";
	return iconpath.c_str();
}

void HXml::WriteToXML(TiXmlElement *element)
{
	*element = m_element;
}

void HXml::ReadFromXML(TiXmlElement* element)
{
	m_element = *element;
}

const wchar_t* HXml::GetShortString(void)const
{
	return Ctt(m_element.Value());
}

