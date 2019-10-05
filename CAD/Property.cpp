// Property.cpp
// Copyright (c) 2018, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"

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

PropertyCheck::~PropertyCheck()
{
	int a = 0;
	a = 3;
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

PropertyList* PropertyVertex(HeeksObj* object, const wchar_t* title, double* x)
{
	PropertyList* p = new PropertyList(title);
	p->m_list.push_back(new PropertyDouble(object, L"x", &x[0]));
	p->m_list.push_back(new PropertyDouble(object, L"y", &x[1]));
	p->m_list.push_back(new PropertyDouble(object, L"z", &x[2]));
	return p;
}

PropertyList* PropertyVertex(HeeksObj* object, const wchar_t* title, const double* x)
{
	PropertyList* p = new PropertyList(title);
	p->m_list.push_back(new PropertyDouble(object, L"x", &x[0]));
	p->m_list.push_back(new PropertyDouble(object, L"y", &x[1]));
	p->m_list.push_back(new PropertyDouble(object, L"z", &x[2]));
	return p;
}



class PropertyPntCoord :public Property{
protected:
	Point3d* m_pnt;
public:
	PropertyPntCoord(HeeksObj* object, const wchar_t* title, Point3d *pnt) :Property(object, title), m_pnt(pnt){ }
};

class PropertyPntX :public PropertyPntCoord
{
public:
	PropertyPntX(HeeksObj* object, Point3d *pnt) :PropertyPntCoord(object, L"x", pnt){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ m_pnt->x = (value); }
	double GetDouble(void)const{ return m_pnt->x; }
	Property* MakeACopy()const{ return new PropertyPntX(*this); }
};

class PropertyPntY :public PropertyPntCoord
{
public:
	PropertyPntY(HeeksObj* object, Point3d *pnt) :PropertyPntCoord(object, L"y", pnt){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ m_pnt->y = (value); }
	double GetDouble(void)const{ return m_pnt->y; }
	Property* MakeACopy()const{ return new PropertyPntY(*this); }
};

class PropertyPntZ :public PropertyPntCoord
{
public:
	PropertyPntZ(HeeksObj* object, Point3d *pnt) :PropertyPntCoord(object, L"z", pnt){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ m_pnt->z = (value); }
	double GetDouble(void)const{ return m_pnt->z; }
	Property* MakeACopy()const{ return new PropertyPntZ(*this); }
};

PropertyList* PropertyPnt(HeeksObj* object, const wchar_t* title, Point3d* pnt)
{
	PropertyList* p = new PropertyList(title);
	p->m_list.push_back(new PropertyPntX(object, pnt));
	p->m_list.push_back(new PropertyPntY(object, pnt));
	p->m_list.push_back(new PropertyPntZ(object, pnt));
	return p;
}

void PropertyPnt(std::list<Property *> *list, HeeksObj* object, Point3d* pnt)
{
	list->push_back(new PropertyPntX(object, pnt));
	list->push_back(new PropertyPntY(object, pnt));
	list->push_back(new PropertyPntZ(object, pnt));
}


#if 0
to do
class PropertyTrsfBase :public Property{
protected:
	Matrix* m_trsf;
public:
	PropertyTrsfBase(HeeksObj* object, const wchar_t* title, Matrix *trsf) :Property(object, title), m_trsf(trsf){ }
};

class PropertyLengthTrsfPosX :public PropertyTrsfBase
{
public:
	PropertyLengthTrsfPosX(HeeksObj* object, Matrix *trsf) :PropertyTrsfBase(object, _("x"), trsf){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ gp_XYZ t = m_trsf->TranslationPart(); t.SetX(t.x + value); m_trsf->SetTranslationPart(t); }
	double GetDouble(void)const{ return  m_trsf->TranslationPart().x; }
	Property* MakeACopy()const{ return new PropertyLengthTrsfPosX(*this); }
};

class PropertyLengthTrsfPosY :public PropertyTrsfBase
{
public:
	PropertyLengthTrsfPosY(HeeksObj* object, Matrix *trsf) :PropertyTrsfBase(object, _("y"), trsf){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ gp_XYZ t = m_trsf->TranslationPart(); t.SetY(t.y + value); m_trsf->SetTranslationPart(t); }
	double GetDouble(void)const{ return  m_trsf->TranslationPart().y; }
	Property* MakeACopy()const{ return new PropertyLengthTrsfPosY(*this); }
};

class PropertyLengthTrsfPosZ :public PropertyTrsfBase
{
public:
	PropertyLengthTrsfPosZ(HeeksObj* object, Matrix *trsf) :PropertyTrsfBase(object, _("z"), trsf){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ gp_XYZ t = m_trsf->TranslationPart(); t.SetZ(t.z + value); m_trsf->SetTranslationPart(t); }
	double GetDouble(void)const{ return  m_trsf->TranslationPart().z; }
	Property* MakeACopy()const{ return new PropertyLengthTrsfPosZ(*this); }
};

class PropertyLengthTrsfXDirX :public PropertyTrsfBase
{
public:
	PropertyLengthTrsfXDirX(HeeksObj* object, Matrix *trsf) :PropertyTrsfBase(object, _("x"), trsf){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ Point3d d; d.Transform(*m_trsf); Point3d x(1, 0, 0);  x.Transform(*m_trsf); Point3d y(1, 0, 0); y.Transform(*m_trsf); x.SetX(value); *m_trsf = Matrix(d, x, y);  }
	double GetDouble(void)const{ Point3d x(1, 0, 0);  x.Transform(*m_trsf); return x.x; }
	Property* MakeACopy()const{ return new PropertyLengthTrsfXDirX(*this); }
};

class PropertyLengthTrsfXDirY :public PropertyTrsfBase
{
public:
	PropertyLengthTrsfXDirY(HeeksObj* object, Matrix *trsf) :PropertyTrsfBase(object, _("y"), trsf){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ Point3d d; d.Transform(*m_trsf); Point3d x(1, 0, 0);  x.Transform(*m_trsf); Point3d y(1, 0, 0); y.Transform(*m_trsf); x.SetY(value); *m_trsf = Matrix(d, x, y); }
	double GetDouble(void)const{ Point3d x(1, 0, 0);  x.Transform(*m_trsf); return x.y; }
	Property* MakeACopy()const{ return new PropertyLengthTrsfXDirY(*this); }
};

class PropertyLengthTrsfXDirZ :public PropertyTrsfBase
{
public:
	PropertyLengthTrsfXDirZ(HeeksObj* object, Matrix *trsf) :PropertyTrsfBase(object, _("z"), trsf){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ Point3d d; d.Transform(*m_trsf); Point3d x(1, 0, 0);  x.Transform(*m_trsf); Point3d y(1, 0, 0); y.Transform(*m_trsf); x.SetZ(value); *m_trsf = Matrix(d, x, y); }
	double GetDouble(void)const{ Point3d x(1, 0, 0);  x.Transform(*m_trsf); return x.z; }
	Property* MakeACopy()const{ return new PropertyLengthTrsfXDirZ(*this); }
};

class PropertyLengthTrsfYDirX :public PropertyTrsfBase
{
public:
	PropertyLengthTrsfYDirX(HeeksObj* object, Matrix *trsf) :PropertyTrsfBase(object, _("x"), trsf){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ Point3d d; d.Transform(*m_trsf); Point3d x(1, 0, 0);  x.Transform(*m_trsf); Point3d y(1, 0, 0); y.Transform(*m_trsf); y.SetX(value); *m_trsf = Matrix(d, x, y); }
	double GetDouble(void)const{ Point3d y(0,1,0);  y.Transform(*m_trsf); return y.x; }
	Property* MakeACopy()const{ return new PropertyLengthTrsfYDirX(*this); }
};

class PropertyLengthTrsfYDirY :public PropertyTrsfBase
{
public:
	PropertyLengthTrsfYDirY(HeeksObj* object, Matrix *trsf) :PropertyTrsfBase(object, _("y"), trsf){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ Point3d d; d.Transform(*m_trsf); Point3d x(1, 0, 0);  x.Transform(*m_trsf); Point3d y(1, 0, 0); y.Transform(*m_trsf); y.SetY(value); *m_trsf = Matrix(d, x, y); }
	double GetDouble(void)const{ Point3d y(0, 1, 0);  y.Transform(*m_trsf); return y.y; }
	Property* MakeACopy()const{ return new PropertyLengthTrsfYDirY(*this); }
};

class PropertyLengthTrsfYDirZ :public PropertyTrsfBase
{
public:
	PropertyLengthTrsfYDirZ(HeeksObj* object, Matrix *trsf) :PropertyTrsfBase(object, _("z"), trsf){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ Point3d d; d.Transform(*m_trsf); Point3d x(1, 0, 0);  x.Transform(*m_trsf); Point3d y(1, 0, 0); y.Transform(*m_trsf); y.SetZ(value); *m_trsf = Matrix(d, x, y); }
	double GetDouble(void)const{ Point3d y(0, 1, 0);  y.Transform(*m_trsf); return y.z; }
	Property* MakeACopy()const{ return new PropertyLengthTrsfYDirZ(*this); }
};

static const std::wstring angle_titles[3] = { _("vertical angle"), _("horizontal angle"), _("twist angle") };

class PropertyDoubleTrsfAngle :public PropertyTrsfBase
{
	int m_type;
public:
	PropertyDoubleTrsfAngle(HeeksObj* object, Matrix* trsf, int type) :PropertyTrsfBase(object, angle_titles[type], trsf), m_type(type){}
	// Property's virtual functions
	int get_property_type(){ return DoublePropertyType; }
	Property* MakeACopy()const{ return new PropertyDoubleTrsfAngle(*this); }
	void Set(double value){
		double vertical_angle, horizontal_angle, twist_angle;
		Point3d x(1, 0, 0);
		Point3d y(0, 1, 0);
		Point3d d(0, 0, 0);
		x.Transform(*m_trsf);
		y.Transform(*m_trsf);
		d.Transform(*m_trsf);
		CoordinateSystem::AxesToAngles(x, y, vertical_angle, horizontal_angle, twist_angle);
		switch (m_type)
		{
		case 0:
			vertical_angle = value * M_PI / 180;
			break;
		case 1:
			horizontal_angle = value * M_PI / 180;
			break;
		default:
			twist_angle = value * M_PI / 180;
			break;
		}
		Point3d dx, dy;
		CoordinateSystem::AnglesToAxes(vertical_angle, horizontal_angle, twist_angle, dx, dy);
		*m_trsf = Matrix(d, dx, dy);
	}
	double GetDouble(void)const{
		double vertical_angle, horizontal_angle, twist_angle;
		Point3d x(1, 0, 0);
		Point3d y(0, 1, 0);
		x.Transform(*m_trsf);
		y.Transform(*m_trsf);
		CoordinateSystem::AxesToAngles(x, y, vertical_angle, horizontal_angle, twist_angle);
		switch (m_type)
		{
		case 0:
			return vertical_angle / M_PI * 180;
		case 1:
			return horizontal_angle / M_PI * 180;
		default:
			return twist_angle / M_PI * 180;
		}
	}
};


PropertyList* PropertyTrsfPnt(HeeksObj* object, const wchar_t* title, Matrix* trsf)
{
	PropertyList* p = new PropertyList(title);
	p->m_list.push_back(new PropertyLengthTrsfPosX(object, trsf));
	p->m_list.push_back(new PropertyLengthTrsfPosY(object, trsf));
	p->m_list.push_back(new PropertyLengthTrsfPosZ(object, trsf));
	return p;
}

PropertyList* PropertyTrsfXDir(HeeksObj* object, const wchar_t* title, Matrix* trsf)
{
	PropertyList* p = new PropertyList(title);
	p->m_list.push_back(new PropertyLengthTrsfXDirX(object, trsf));
	p->m_list.push_back(new PropertyLengthTrsfXDirY(object, trsf));
	p->m_list.push_back(new PropertyLengthTrsfXDirZ(object, trsf));
	return p;
}

PropertyList* PropertyTrsfYDir(HeeksObj* object, const wchar_t* title, Matrix* trsf)
{
	PropertyList* p = new PropertyList(title);
	p->m_list.push_back(new PropertyLengthTrsfYDirX(object, trsf));
	p->m_list.push_back(new PropertyLengthTrsfYDirY(object, trsf));
	p->m_list.push_back(new PropertyLengthTrsfYDirZ(object, trsf));
	return p;
}


PropertyList* PropertyTrsf(HeeksObj* object, const wchar_t* title, Matrix* trsf)
{
	PropertyList* p = new PropertyList(title);
	p->m_list.push_back(PropertyTrsfPnt(object, _("position"), trsf));
	p->m_list.push_back(PropertyTrsfXDir(object, _("x axis"), trsf));
	p->m_list.push_back(PropertyTrsfYDir(object, _("y axis"), trsf));
	p->m_list.push_back(new PropertyDoubleTrsfAngle(object, trsf, 0));
	p->m_list.push_back(new PropertyDoubleTrsfAngle(object, trsf, 1));
	p->m_list.push_back(new PropertyDoubleTrsfAngle(object, trsf, 2));
	return p;
}

#else

PropertyList* PropertyTrsf(HeeksObj* object, const wchar_t* title, Matrix* trsf)
{
	return new PropertyList(L"to do ");
}

#endif
