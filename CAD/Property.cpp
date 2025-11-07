// Property.cpp
// Copyright (c) 2018, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include <stdafx.h>

#include "Property.h"

Property::Property(const Property& rhs) : m_editable(false), m_highlighted(false), m_object(NULL)
{
	operator=(rhs);
}

const Property& Property::operator=(const Property &rhs)
{
	m_editable = rhs.m_editable;
	m_highlighted = rhs.m_highlighted;
	m_object = rhs.m_object;
	m_title = rhs.m_title;

	return *this;
}

Property *PropertyCheck::MakeACopy(void)const{
	PropertyCheck* new_object = new PropertyCheck(*this);
	return new_object;
}

Property *PropertyChoice::MakeACopy(void)const{
	PropertyChoice* new_object = new PropertyChoice(*this);
	return new_object;
}

Property *PropertyColor::MakeACopy(void)const{
	PropertyColor* new_object = new PropertyColor(*this);
	return new_object;
}

Property *PropertyDouble::MakeACopy(void)const{
	PropertyDouble* new_object = new PropertyDouble(*this);
	return new_object;
}

Property *PropertyDoubleLimited::MakeACopy(void)const{
	PropertyDoubleLimited* new_object = new PropertyDoubleLimited(*this);
	return new_object;
}

Property *PropertyFile::MakeACopy(void)const{
	PropertyFile* new_object = new PropertyFile(*this);
	return new_object;
}

Property *PropertyInt::MakeACopy(void)const{
	PropertyInt* new_object = new PropertyInt(*this);
	return new_object;
}

Property *PropertyLength::MakeACopy(void)const{
	PropertyLength* new_object = new PropertyLength(*this);
	return new_object;
}

Property *PropertyList::MakeACopy(void)const{
	PropertyList* new_object = new PropertyList(*this);
	return new_object;
}

Property *PropertyString::MakeACopy(void)const{
	PropertyString* new_object = new PropertyString(*this);
	return new_object;
}

Property *PropertyStringReadOnly::MakeACopy(void)const{
	PropertyStringReadOnly* new_object = new PropertyStringReadOnly(*this);
	return new_object;
}
