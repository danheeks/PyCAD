// IdNamedObjList.cpp

#include "stdafx.h"

#include "IdNamedObjList.h"
#include "strconv.h"

static std::wstring temp_pattern_string;

void IdNamedObjList::WriteToXML(TiXmlElement *element)
{
	element->SetAttribute("title_from_id", m_title_made_from_id ? 1:0);
	if(!m_title_made_from_id)element->SetAttribute("title", Ttc(m_title.c_str()));
	ObjList::WriteToXML(element);
}

void IdNamedObjList::ReadFromXML(TiXmlElement* element)
{
	if(const char* pstr = element->Attribute("title"))m_title = Ctt(pstr);
	int i;
	if(element->Attribute("title_from_id", &i))m_title_made_from_id = (i != 0);
	ObjList::ReadFromXML(element);
}

const wchar_t* IdNamedObjList::GetShortString(void)const
{
	if(m_title_made_from_id)
	{
		wchar_t str[128];
		swprintf(str, L"%s %d", GetTypeString(), m_id);
		temp_pattern_string = str;
		return temp_pattern_string.c_str();
	}
	return m_title.c_str();
}

void IdNamedObjList::OnEditString(const wchar_t* str)
{
    m_title.assign(str);
	m_title_made_from_id = false;
}
