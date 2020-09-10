#include "Property.h"
#include "Shape.h"

template <typename T>
class PropertyGpCoord :public Property{
protected:
	T* m_pnt;
public:
	PropertyGpCoord(HeeksObj* object, const wchar_t* title, T *pnt) : Property(object, title), m_pnt(pnt){ }
};

template <typename T>
class PropertyGpX :public PropertyGpCoord<T>
{
public:
	PropertyGpX(HeeksObj* object, T *pnt) :PropertyGpCoord<T>(object, L"x", pnt){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ this->m_pnt->SetX(value); ((CShape*)m_object)->OnApplyProperties(); }
	double GetDouble(void)const{ return this->m_pnt->X(); }
	Property* MakeACopy()const{ return new PropertyGpX<T>(*this); }
};

template <typename T>
class PropertyGpY :public PropertyGpCoord<T>
{
public:
	PropertyGpY(HeeksObj* object, T *pnt) :PropertyGpCoord<T>(object, L"y", pnt){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ this->m_pnt->SetY(value); ((CShape*)m_object)->OnApplyProperties(); }
	double GetDouble(void)const{ return this->m_pnt->Y(); }
	Property* MakeACopy()const{ return new PropertyGpY<T>(*this); }
};

template <typename T>
class PropertyGpZ :public PropertyGpCoord<T>
{
public:
	PropertyGpZ(HeeksObj* object, T *pnt) :PropertyGpCoord<T>(object, L"z", pnt){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ this->m_pnt->SetZ(value); ((CShape*)m_object)->OnApplyProperties(); }
	double GetDouble(void)const{ return this->m_pnt->Z(); }
	Property* MakeACopy()const{ return new PropertyGpZ<T>(*this); }
};

template <class T>
void PropertyGp(std::list<Property *> *list, HeeksObj* object, T* pnt)
{
	list->push_back(new PropertyGpX<T>(object, pnt));
	list->push_back(new PropertyGpY<T>(object, pnt));
	list->push_back(new PropertyGpZ<T>(object, pnt));
}

template <class T>
PropertyList* PropertyGp(HeeksObj* object, const wchar_t* title, T* pnt)
{
	PropertyList* p = new PropertyList(title);
	PropertyGp<T>(&(p->m_list), object, pnt);
	return p;
}

