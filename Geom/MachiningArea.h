#include "Area.h"

enum FaceFlatType{
	FaceFlatTypeFlat,
	FaceFlatTypeUpButNotFlat,
	FaceFlatTypeDown,
};

class CMachiningArea
{
public:
	CArea m_area;
	double m_top;
	double m_bottom;
	FaceFlatType m_face_type;
};
