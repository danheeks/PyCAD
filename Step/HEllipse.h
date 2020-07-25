// HEllipse.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "HeeksObj.h"
#include "HeeksColor.h"

class HEllipse: public HeeksObj{
private:
	HeeksColor color;

public:
	gp_Pnt C;
	double m_rot;
	double m_start; double m_end;
	double m_majr; double m_minr;
	gp_Dir m_zdir;
	gp_Dir m_xdir;
	static int m_type;

	~HEllipse(void);
	HEllipse();
	HEllipse(const gp_Elips &c, const HeeksColor* col);
	HEllipse(const gp_Elips &c, double start, double end, const HeeksColor* col);
	HEllipse(const HEllipse &c);

	const HEllipse& operator=(const HEllipse &c);

	double GetRotation()const;
	void SetRotation(double rot);
	void SetEllipse(gp_Elips e);
	gp_Elips GetEllipse() const;


	// HeeksObj's virtual functions
	int GetType()const{ return m_type; }
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	const wchar_t* GetTypeString(void)const{ return L"Ellipse"; }
	HeeksObj *MakeACopy(void)const;
	const wchar_t* GetIconFilePath();
	void Transform(const Matrix &m);
	void SetColor(const HeeksColor &col){color = col;}
	const HeeksColor* GetColor()const{return &color;}
	void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
	void GetProperties(std::list<Property *> *list);
	bool FindNearPoint(const double* ray_start, const double* ray_direction, double *point);
	bool FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point);
	bool Stretch(const double *p, const double* shift, void* data);
	void GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point = true)const;
	bool GetCentrePoint(double* pos);
	void WriteToXML(TiXmlElement *element);
	void ReadFromXML(TiXmlElement *element);
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	int PickPriority(){ return 2; } // lines
};

