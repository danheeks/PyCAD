// Property.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

// Base class for all Properties

#include "HeeksColor.h"
#include "HeeksObj.h"

#if !defined Property_HEADER
#define Property_HEADER

enum{
	InvalidPropertyType,
	StringPropertyType,
	LongStringPropertyType,
	DoublePropertyType,
	LengthPropertyType,
	IntPropertyType,
	ChoicePropertyType,
	ColorPropertyType,
	CheckPropertyType,
	ListOfPropertyType,
	FilePropertyType
};

class Property{
public:
	std::wstring m_title;
	bool m_editable;
	bool m_highlighted;
	HeeksObj* m_object;

	Property(void) :m_title(L"Unknown"), m_editable(false), m_highlighted(false), m_object(NULL){} // default constructor for python
	Property(HeeksObj* object, const wchar_t* title) : m_title(title), m_editable(true), m_highlighted(false), m_object(object){}
	Property(const Property& ho);
	virtual ~Property(){}

	virtual const Property& operator=(const Property &ho);

	virtual int get_property_type(){return InvalidPropertyType;}
	virtual Property *MakeACopy(void)const{ return NULL; }
	virtual const wchar_t* GetShortString(void)const{ return m_title.c_str(); }
	virtual void Set(bool value){} // only called by Property Changer, set Property::m_editable to enable this
	virtual void Set(const HeeksColor& value){} // only called by Property Changer, set Property::m_editable to enable this
	virtual void Set(double value){} // only called by Property Changer, set Property::m_editable to enable this
	virtual void Set(const wchar_t*){} // only called by Property Changer, set Property::m_editable to enable this
	virtual void Set(int value){} // only called by Property Changer, set Property::m_editable to enable this
	virtual void Set(const Matrix& value){} // only called by Property Changer, set Property::m_editable to enable this
	virtual bool GetBool()const{ return false; }
	virtual void GetChoices(std::list< std::wstring > &choices)const{}
	virtual const HeeksColor &GetColor()const{ return *((const HeeksColor*)NULL); }
	virtual double GetDouble()const{ return 0.0; }
	virtual const wchar_t* GetString()const{ return NULL; }
	virtual int GetInt()const{return 66;}
	virtual const Matrix &GetTrsf()const{ return *((const Matrix*)NULL); }
	virtual void GetList(std::list< Property* > &list)const{}
};

class PropertyCheck :public Property{
protected:
	bool* m_pvar;
public:
	PropertyCheck(HeeksObj* object) :Property(object, NULL), m_pvar(NULL){}
	PropertyCheck(HeeksObj* object, const wchar_t* title, bool* pvar) :Property(object, title), m_pvar(pvar){ }
	PropertyCheck(HeeksObj* object, const wchar_t* title, const bool* pvar) :Property(object, title), m_pvar((bool*)pvar){ m_editable = false; }

	// Property's virtual functions
	int get_property_type(){ return CheckPropertyType; }
	Property *MakeACopy(void)const;
	void Set(bool value){ *m_pvar = value; }
	bool GetBool(void)const{ return *m_pvar; }
};

class PropertyChoice :public Property{
protected:
	int* m_pvar;
	std::list< std::wstring > m_choices;
public:
	PropertyChoice(HeeksObj* object, const wchar_t* title, const std::list< std::wstring > &choices, int* pvar) :Property(object, title), m_pvar(pvar), m_choices(choices){}
	// Property's virtual functions
	int get_property_type(){ return ChoicePropertyType; }
	Property *MakeACopy(void)const;
	void Set(int value){ *m_pvar = value; }
	int GetInt()const{ return *m_pvar; }
	void GetChoices(std::list< std::wstring > &choices)const{ choices = m_choices; }
};

class PropertyColor :public Property{
protected:
	HeeksColor* m_pvar;
public:
	PropertyColor(HeeksObj* object) :Property(object, NULL){}
	PropertyColor(HeeksObj* object, const wchar_t* title, HeeksColor* pvar) :Property(object, title), m_pvar(pvar){ }
	// Property's virtual functions
	int get_property_type(){ return ColorPropertyType; }
	Property *MakeACopy(void)const;
	void Set(const HeeksColor& value){ *m_pvar = value; }
	const HeeksColor &GetColor()const{ return *m_pvar; }
};

class PropertyDouble :public Property{
protected:
	double* m_pvar;
public:
	PropertyDouble(HeeksObj* object) :Property(object, NULL), m_pvar(NULL){}
	PropertyDouble(HeeksObj* object, const wchar_t* title, double* pvar) :Property(object, title), m_pvar(pvar){ }
	PropertyDouble(HeeksObj* object, const wchar_t* title, const double* pvar) :Property(object, title), m_pvar((double*)pvar){ m_editable = false; }
	// Property's virtual functions
	int get_property_type(){ return DoublePropertyType; }
	Property *MakeACopy(void)const;
	void Set(double value){ *m_pvar = value; }
	double GetDouble(void)const{ return *m_pvar; }
};

class PropertyDoubleReadOnly : public Property{
protected:
	double m_value;
public:
	PropertyDoubleReadOnly(HeeksObj* object) :Property(object, NULL), m_value(0.0){}
	PropertyDoubleReadOnly(HeeksObj* object, const wchar_t* title, double value) :Property(object, title), m_value(value){ m_editable = false; }
	// Property's virtual functions
	int get_property_type(){ return DoublePropertyType; }
	Property *MakeACopy(void)const{ return new PropertyDoubleReadOnly(*this); }
	double GetDouble(void)const{ return m_value; }

};
class PropertyDoubleScaled :public Property{
	double* m_pvar;
	double m_scale;
public:
	PropertyDoubleScaled(HeeksObj* object, const wchar_t* title, double* pvar, double scale) :Property(object, title), m_pvar(pvar), m_scale(scale){ }
	PropertyDoubleScaled(HeeksObj* object, const wchar_t* title, const double* pvar, double scale) :Property(object, title), m_pvar((double*)pvar), m_scale(scale){ m_editable = false; }
	// Property's virtual functions
	int get_property_type(){ return DoublePropertyType; }
	Property *MakeACopy(void)const{ return new PropertyDoubleScaled(*this); }
	void Set(double value){ *m_pvar = value / m_scale; }
	double GetDouble(void)const{ return *m_pvar * m_scale; }
};

class PropertyLengthScaled :public PropertyDoubleScaled{
public:
	PropertyLengthScaled(HeeksObj* object, const wchar_t* title, double* pvar, double scale) :PropertyDoubleScaled(object, title, pvar, scale){}
	PropertyLengthScaled(HeeksObj* object, const wchar_t* title, const double* pvar, double scale) :PropertyDoubleScaled(object, title, pvar, scale){}
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	Property *MakeACopy(void)const{	return new PropertyLengthScaled(*this);	}
};

class PropertyDoubleLimited :public PropertyDouble{
	bool m_l, m_u;
	double m_upper, m_lower;
public:
	PropertyDoubleLimited(HeeksObj* object, const wchar_t* title, double* pvar, bool l, double lower, bool u, double upper) :PropertyDouble(object, title, pvar), m_l(l), m_u(u), m_upper(upper), m_lower(lower){}
	// Property's virtual functions
	int get_property_type(){ return DoublePropertyType; }
	Property *MakeACopy(void)const;
	void Set(double value){ if (m_l && value < m_lower)value = m_lower; if (m_u && value > m_upper)value = m_upper; PropertyDouble::Set(value); }
};

class PropertyString :public Property{
	std::wstring *m_pvar;
public:
	PropertyString(HeeksObj* object, const wchar_t* title, std::wstring* pvar) :Property(object, title), m_pvar(pvar){}
	// Property's virtual functions
	int get_property_type(){ return StringPropertyType; }
	Property *MakeACopy(void)const;
	void Set(const wchar_t* value){ *m_pvar = value; }
	const wchar_t* GetString()const{ return m_pvar->c_str(); }
};

class PropertyStringReadOnly :public Property{
	std::wstring m_value;
public:
	PropertyStringReadOnly(const wchar_t* title, const wchar_t* value) :Property(NULL, title), m_value(value){ m_editable = false; }
	// Property's virtual functions
	int get_property_type(){ return StringPropertyType; }
	Property *MakeACopy(void)const;
	const wchar_t* GetString()const{ return m_value.c_str(); }
};

class PropertyFile :public PropertyString{
public:
	PropertyFile(HeeksObj* object, const wchar_t* title, std::wstring* pvar) :PropertyString(object, title, pvar){}
	// Property's virtual functions
	int get_property_type(){ return FilePropertyType; }
	Property *MakeACopy(void)const;
};

class PropertyInt :public Property{
protected:
	int* m_pvar;
public:
	PropertyInt(HeeksObj* object) :Property(object, NULL){}
	PropertyInt(HeeksObj* object, const wchar_t* title, int* pvar) :Property(object, title), m_pvar(pvar){ }
	PropertyInt(HeeksObj* object, const wchar_t* title, const int* pvar) :Property(object, title), m_pvar((int*)pvar){ m_editable = false; }
	// Property's virtual functions
	int get_property_type(){ return IntPropertyType; }
	Property *MakeACopy(void)const;
	void Set(int value){ *m_pvar = value; }
	int GetInt(void)const{ return *m_pvar; }
};

class PropertyLength :public PropertyDouble{
public:
	PropertyLength(HeeksObj* object) :PropertyDouble(object){}

	PropertyLength(HeeksObj* object, const wchar_t* title, double* pvar) :PropertyDouble(object, title, pvar){}
	PropertyLength(HeeksObj* object, const wchar_t* title, const double* pvar) :PropertyDouble(object, title, pvar){} // to do, replace with PropertyLengthReadOnly
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	Property *MakeACopy(void)const;
};

class PropertyLengthReadOnly : public PropertyDoubleReadOnly{
public:
	PropertyLengthReadOnly(HeeksObj* object) :PropertyDoubleReadOnly(object){}

	PropertyLengthReadOnly(HeeksObj* object, const wchar_t* title, double value) :PropertyDoubleReadOnly(object, title, value){}
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	Property *MakeACopy(void)const{ return new PropertyLengthReadOnly(*this); }
};

class PropertyLengthWithKillGLLists : public PropertyLength
{
public:
	PropertyLengthWithKillGLLists(HeeksObj* object, const wchar_t* title, double* pvar) :PropertyLength(object, title, pvar){}
	void Set(double value){ PropertyLength::Set(value); m_object->KillGLLists(); }
	Property *MakeACopy(void)const{ return new PropertyLengthWithKillGLLists(*this); }
};

class PropertyList :public Property{
public:
	std::list< Property* > m_list;
	PropertyList(const wchar_t* title) :Property(NULL, title){}
	// Property's virtual functions
	int get_property_type(){ return ListOfPropertyType; }
	Property *MakeACopy(void)const;
	void GetList(std::list< Property* > &list)const{ list = m_list; }
};

class HeeksObj;

class PropertyObjectTitle :public Property{
public:
	PropertyObjectTitle(HeeksObj* object);
	// Property's virtual functions
	int get_property_type(){ return StringPropertyType; }
	Property *MakeACopy(void)const;
	const wchar_t* GetString()const;
	void Set(const wchar_t* value);
};


class PropertyObjectColor :public Property{
public:
	PropertyObjectColor(HeeksObj* object);
	// Property's virtual functions
	int get_property_type(){ return ColorPropertyType; }
	Property *MakeACopy(void)const;
	const HeeksColor &GetColor()const;
	void Set(const HeeksColor& value);
};


PropertyList* PropertyVertex(HeeksObj* object, const wchar_t* title, double* p);
PropertyList* PropertyVertex(HeeksObj* object, const wchar_t* title, const double* p);
PropertyList* PropertyPnt(HeeksObj* object, const wchar_t* title, Point3d* pnt);
void PropertyPnt(std::list<Property *> *list, HeeksObj* object, Point3d* pnt);
PropertyList* PropertyTrsf(HeeksObj* object, const wchar_t* title, Matrix* trsf);


#endif
