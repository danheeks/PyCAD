// PropertyChange.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "PropertyChange.h"

PropertyChangeString::PropertyChangeString(const std::wstring& value, PropertyString* property) :m_property(property)
{
	m_value = value;
	m_old = m_property->GetString();
}

void PropertyChangeString::Run(bool redo)
{
	m_property->Set(m_value.c_str());
	theApp.WasModified(m_property->m_object);
}

void PropertyChangeString::RollBack()
{
	m_property->Set(m_old.c_str());
	theApp.WasModified(m_property->m_object);
}

PropertyChangeDouble::PropertyChangeDouble(const double& value, PropertyDouble* property) :m_property(property)
{
	m_value = value;
	m_old = m_property->GetDouble();
}

void PropertyChangeDouble::Run(bool redo)
{
	m_property->Set(m_value);
	theApp.WasModified(m_property->m_object);
}

void PropertyChangeDouble::RollBack()
{
	m_property->Set(m_old);
	theApp.WasModified(m_property->m_object);
}

PropertyChangeLength::PropertyChangeLength(const double& value, PropertyLength* property) :m_property(property)
{
	m_value = value;
	m_old = m_property->GetDouble();
}

void PropertyChangeLength::Run(bool redo)
{
	m_property->Set(m_value);
	theApp.WasModified(m_property->m_object);
}

void PropertyChangeLength::RollBack()
{
	m_property->Set(m_old);
	theApp.WasModified(m_property->m_object);
}

PropertyChangeInt::PropertyChangeInt(const int& value, PropertyInt* property) :m_property(property)
{
	m_value = value;
	m_old = m_property->GetInt();
}

void PropertyChangeInt::Run(bool redo)
{
	m_property->Set(m_value);
	theApp.WasModified(m_property->m_object);
}

void PropertyChangeInt::RollBack()
{
	m_property->Set(m_old);
	theApp.WasModified(m_property->m_object);
}

PropertyChangeColor::PropertyChangeColor(const HeeksColor& value, PropertyColor* property) :m_property(property)
{
	m_value = value;
	m_old = ((PropertyColor*)property)->GetColor();
}

void PropertyChangeColor::Run(bool redo)
{
	m_property->Set(m_value);
	theApp.WasModified(m_property->m_object);
}

void PropertyChangeColor::RollBack()
{
	m_property->Set(m_old);
	theApp.WasModified(m_property->m_object);
}

PropertyChangeChoice::PropertyChangeChoice(const int& value, PropertyChoice* property) :m_property(property)
{
	m_value = value;
	m_old = m_property->GetInt();
}

void PropertyChangeChoice::Run(bool redo)
{
	m_property->Set(m_value);
	theApp.WasModified(m_property->m_object);
}

void PropertyChangeChoice::RollBack()
{
	m_property->Set(m_old);
	theApp.WasModified(m_property->m_object);
}

PropertyChangeCheck::PropertyChangeCheck(const bool& value, PropertyCheck* property) :m_property(property)
{
	m_value = value;
	m_old = m_property->GetBool();
}

void PropertyChangeCheck::Run(bool redo)
{
	m_property->Set(m_value);
	theApp.WasModified(m_property->m_object);
}

void PropertyChangeCheck::RollBack()
{
	m_property->Set(m_old);
	theApp.WasModified(m_property->m_object);
}

