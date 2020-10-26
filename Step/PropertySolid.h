#include "Property.h"

class PropertySolidLength :public PropertyLength{
public:
	PropertySolidLength(HeeksObj* object, const wchar_t* title, double* pvar) :PropertyLength(object, title, pvar){}
	void Set(double value);
	Property *MakeACopy(void)const;
};


class PropertySolidLengthScaled :public PropertyLengthScaled{
public:
	PropertySolidLengthScaled(HeeksObj* object, const wchar_t* title, double* pvar, double scale) :PropertyLengthScaled(object, title, pvar, scale){}
	void Set(double value);
	Property *MakeACopy(void)const;
};



