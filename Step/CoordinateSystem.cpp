// CordinateSystem.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "CoordinateSystem.h"
#include "PropertySolid.h"
#include "Shape.h"

class PropertyAx2 :public Property{
protected:
	gp_Ax2* m_ax2;
public:
	PropertyAx2(HeeksObj* object, const wchar_t* title, gp_Ax2 *ax2) :Property(object, title), m_ax2(ax2){ }
};

class PropertyLengthAx2PosX :public PropertyAx2
{
public:
	PropertyLengthAx2PosX(HeeksObj* object, gp_Ax2* ax2) :PropertyAx2(object, L"x", ax2){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){	gp_Pnt p = m_ax2->Location(); p.SetX(value); m_ax2->SetLocation(p); ((CShape*)m_object)->OnApplyProperties(); }
	double GetDouble(void)const{ return m_ax2->Location().X(); }
	Property* MakeACopy()const{ return new PropertyLengthAx2PosX(*this); }
};

class PropertyLengthAx2PosY :public PropertyAx2
{
public:
	PropertyLengthAx2PosY(HeeksObj* object, gp_Ax2* ax2) :PropertyAx2(object, L"y", ax2){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ gp_Pnt p = m_ax2->Location(); p.SetY(value); m_ax2->SetLocation(p); ((CShape*)m_object)->OnApplyProperties(); }
	double GetDouble(void)const{ return m_ax2->Location().Y(); }
	Property* MakeACopy()const{ return new PropertyLengthAx2PosY(*this); }
};

class PropertyLengthAx2PosZ :public PropertyAx2
{
public:
	PropertyLengthAx2PosZ(HeeksObj* object, gp_Ax2* ax2) :PropertyAx2(object, L"z", ax2){ }
	// Property's virtual functions
	int get_property_type(){ return LengthPropertyType; }
	void Set(double value){ gp_Pnt p = m_ax2->Location(); p.SetZ(value); m_ax2->SetLocation(p); ((CShape*)m_object)->OnApplyProperties(); }
	double GetDouble(void)const{ return m_ax2->Location().Z(); }
	Property* MakeACopy()const{ return new PropertyLengthAx2PosZ(*this); }
};

static const wchar_t* angle_titles[3] = { L"vertical angle", L"horizontal angle", L"twist angle" };

class PropertyDoubleAx2Angle :public PropertyAx2
{
	int m_type;
public:
	PropertyDoubleAx2Angle(HeeksObj* object, gp_Ax2* ax2, int type) :PropertyAx2(object, angle_titles[type], ax2), m_type(type){}
	// Property's virtual functions
	int get_property_type(){ return DoublePropertyType; }
	Property* MakeACopy()const{ return new PropertyDoubleAx2Angle(*this); }
	void Set(double value){
		double vertical_angle, horizontal_angle, twist_angle;
		AxesToAngles(m_ax2->XDirection(), m_ax2->YDirection(), vertical_angle, horizontal_angle, twist_angle);
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
		gp_Dir dx, dy;
		AnglesToAxes(vertical_angle, horizontal_angle, twist_angle, dx, dy);
		gp_Trsf mat = make_matrix(m_ax2->Location(), dx, dy);
		*m_ax2 = gp_Ax2(m_ax2->Location(), gp_Dir(0, 0, 1).Transformed(mat), gp_Dir(1, 0, 0).Transformed(mat));
		((CShape*)m_object)->OnApplyProperties();
	}
	double GetDouble(void)const{
		double vertical_angle, horizontal_angle, twist_angle;
		AxesToAngles(m_ax2->XDirection(), m_ax2->YDirection(), vertical_angle, horizontal_angle, twist_angle);
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


// code for AxesToAngles copied from http://tog.acm.org/GraphicsGems/gemsiv/euler_angle/EulerAngles.c

#define EulSafe	     "\000\001\002\000"
#define EulNext	     "\001\002\000\001"
#define EulGetOrd(ord,i,j,k,h,n,s,f) {unsigned o=ord;f=o&1;o>>=1;s=o&1;o>>=1;n=o&1;o>>=1;i=EulSafe[o&3];j=EulNext[i+n];k=EulNext[i+1-n];h=s?k:i;}
#define EulOrd(i,p,r,f)	   (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))
#define EulOrdZXZs    EulOrd(2,0,1,0)

void AxesToAngles(const gp_Dir &x, const gp_Dir &y, double &v_angle, double &h_angle, double &t_angle)
{
	double M[4][4];
	extract(make_matrix(gp_Pnt(0, 0, 0), x, y), M[0]);
	int order = EulOrdZXZs;

    int i,j,k,h,n,s,f;
    EulGetOrd(order,i,j,k,h,n,s,f);
    if (s==1) {
	double sy = sqrt(M[i][j]*M[i][j] + M[i][k]*M[i][k]);
	if (sy > 16*FLT_EPSILON) {
	    t_angle = atan2(M[i][j], M[i][k]);
	    v_angle = atan2(sy, M[i][i]);
	    h_angle = atan2(M[j][i], -M[k][i]);
	} else {
	    t_angle = atan2(-M[j][k], M[j][j]);
	    v_angle = atan2(sy, M[i][i]);
	    h_angle = 0;
	}
    } else {
	double cy = sqrt(M[i][i]*M[i][i] + M[j][i]*M[j][i]);
	if (cy > 16*DBL_EPSILON) {
	    t_angle = atan2(M[k][j], M[k][k]);
	    v_angle = atan2(-M[k][i], cy);
	    h_angle = atan2(M[j][i], M[i][i]);
	} else {
	    t_angle = atan2(-M[j][k], M[j][j]);
	    v_angle = atan2(-M[k][i], cy);
	    h_angle = 0;
	}
    }
    if (n==1) {t_angle = -t_angle; v_angle = - v_angle; h_angle = -h_angle;}
    if (f==1) {double t = t_angle; t_angle = h_angle; h_angle = t;}
}

//static
void AnglesToAxes(const double &v_angle, const double
&h_angle, const double &t_angle, gp_Dir &x, gp_Dir &y)
{
	gp_Trsf zmat1;
	zmat1.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), t_angle);

	gp_Trsf xmat;
	xmat.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0)), v_angle);

	gp_Trsf zmat2;
	zmat2.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), h_angle);

	gp_Trsf mat = zmat2 * xmat * zmat1;

	x = gp_Dir(1, 0, 0).Transformed(mat);
	y = gp_Dir(0, 1, 0).Transformed(mat);
} 

void GetAx2Properties(std::list<Property *> *list, gp_Ax2& a, HeeksObj* object)
{
	//ax2_for_GetProperties = &a;

	PropertyList* p = new PropertyList(L"position");
	p->m_list.push_back(new PropertyLengthAx2PosX(object, &a));
	p->m_list.push_back(new PropertyLengthAx2PosY(object, &a));
	p->m_list.push_back(new PropertyLengthAx2PosZ(object, &a));
	list->push_back(p);

	list->push_back(new PropertyDoubleAx2Angle(object, &a, 0));
	list->push_back(new PropertyDoubleAx2Angle(object, &a, 1));
	list->push_back(new PropertyDoubleAx2Angle(object, &a, 2));
}
