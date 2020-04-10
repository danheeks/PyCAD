#pragma once

#include "HeeksObj.h"

template<typename T>
class ExtrudedObj : public T

{
public:
	double m_thickness;
	double m_extrusion_vector[3];

	~ExtrudedObj(void);
	ExtrudedObj(void)
	{
		m_thickness = 0.0;
		m_extrusion_vector[0] = 0.0;
		m_extrusion_vector[1] = 0.0;
		m_extrusion_vector[2] = 0.0;
	}
	ExtrudedObj(const ExtrudedObj& e);

	const ExtrudedObj& operator=(const ExtrudedObj &b);

	// HeeksObj's virtual functions
	void Transform(const Matrix& m);
	void CopyFrom(const HeeksObj* object){ operator=(*((ExtrudedObj*)object)); }
	HeeksObj* MakeACopyWithID();
	void WriteToXML(TiXmlElement *element);
	void ReadFromXML(TiXmlElement* element);
};

template<typename T>
ExtrudedObj<T>::ExtrudedObj(const ExtrudedObj& e)
{
	operator=(e);
}

template < typename T >
ExtrudedObj<T>::~ExtrudedObj(){
}

template < typename T >
const ExtrudedObj<T>& ExtrudedObj<T>::operator=(const ExtrudedObj<T> &b){
	T::operator = (b);
	m_thickness = b.m_thickness;
	m_extrusion_vector[0] = b.m_extrusion_vector[0];
	m_extrusion_vector[1] = b.m_extrusion_vector[1];
	m_extrusion_vector[2] = b.m_extrusion_vector[2];
	return *this;
}

template < typename T > HeeksObj* ExtrudedObj<T>::MakeACopyWithID()
{
	ExtrudedObj<T>* pnew = (ExtrudedObj<T>*)T::MakeACopyWithID();
	pnew->m_thickness = m_thickness;
	pnew->m_extrusion_vector[0] = m_extrusion_vector[0];
	pnew->m_extrusion_vector[1] = m_extrusion_vector[1];
	pnew->m_extrusion_vector[2] = m_extrusion_vector[2];
	return (HeeksObj*)pnew;
}

template < typename T > void ExtrudedObj<T>::Transform(const Matrix& m){
	Matrix mat(m);
	Point3d v(m_extrusion_vector[0], m_extrusion_vector[1], m_extrusion_vector[2]);
	v = v.Transformed(mat);
	v.get(m_extrusion_vector);
}

template < typename T > void ExtrudedObj<T>::WriteToXML(TiXmlElement *element)
{
	element->SetDoubleAttribute("thickness", m_thickness);
	element->SetDoubleAttribute("extruX", m_extrusion_vector[0]);
	element->SetDoubleAttribute("extruY", m_extrusion_vector[1]);
	element->SetDoubleAttribute("extruZ", m_extrusion_vector[2]);

	T::WriteToXML(element);
}

template < typename T > void ExtrudedObj<T>::ReadFromXML(TiXmlElement* pElem)
{
	pElem->Attribute("thickness", &m_thickness);
	pElem->Attribute("extruX", &m_extrusion_vector[0]);
	pElem->Attribute("extruY", &m_extrusion_vector[1]);
	pElem->Attribute("extruZ", &m_extrusion_vector[2]);

	T::ReadFromXML(pElem);
}
