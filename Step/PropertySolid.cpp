#include "stdafx.h"
#include "PropertySolid.h"
#include "Shape.h"

void PropertySolidLength::Set(double value)
{
	PropertyLength::Set(value);
	((CShape*)m_object)->OnApplyProperties();
}

Property *PropertySolidLength::MakeACopy(void)const
{
	return new PropertySolidLength(*this);
}

void PropertySolidLengthScaled::Set(double value)
{
	PropertyLengthScaled::Set(value);
	((CShape*)m_object)->OnApplyProperties();
}

Property *PropertySolidLengthScaled::MakeACopy(void)const
{
	return new PropertySolidLengthScaled(*this);
}
