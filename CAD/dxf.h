// dxf.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#pragma once

#include <algorithm>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <math.h>

//Following is required to be defined on Ubuntu with OCC 6.3.1
#ifndef HAVE_IOSTREAM
#define HAVE_IOSTREAM
#endif

typedef int Aci_t; // AutoCAD color index

typedef enum
{
	eUnspecified = 0,	// Unspecified (No units)
	eInches,
	eFeet,
	eMiles,
	eMillimeters,
	eCentimeters,
	eMeters,
	eKilometers,
	eMicroinches,
	eMils,
	eYards,
	eAngstroms,
	eNanometers,
	eMicrons,
	eDecimeters,
	eDekameters,
	eHectometers,
	eGigameters,
	eAstronomicalUnits,
	eLightYears,
	eParsecs
} eDxfUnits_t;


struct SplineData
{
	double norm[3];
	int degree;
	int knots;
	int control_points;
	int fit_points;
	int flag;
	std::list<double> starttanx;
	std::list<double> starttany;
	std::list<double> starttanz;
	std::list<double> endtanx;
	std::list<double> endtany;
	std::list<double> endtanz;
	std::list<double> knot;
	std::list<double> weight;
	std::list<double> controlx;
	std::list<double> controly;
	std::list<double> controlz;
	std::list<double> fitx;
	std::list<double> fity;
	std::list<double> fitz;
};

class CDxfWrite{
private:
	std::ofstream* m_ofs;
	bool m_fail;

	void WriteExtrusion(double thickness, const double* extru);

public:
	CDxfWrite(const char* filepath);
	~CDxfWrite();

	bool Failed(){return m_fail;}

	void WriteLine(const double* s, const double* e, const char* layer_name, double thickness, const double* extru = NULL);
	void WritePoint(const double*, const char*);
	void WriteArc(const double* s, const double* e, const double* c, bool dir, const char* layer_name, double thickness, const double* extru = NULL);
	void WriteEllipse(const double* c, double major_radius, double minor_radius, double rotation, double start_angle, double end_angle, bool dir, const char* layer_name, double thickness, const double* extru = NULL);
	void WriteCircle(const double* c, double radius, const char* layer_name, double thickness, const double* extru = NULL);
};

#define STORE_LINE_NUMBERS

class CPolyLinePoint
{
public:
	double x;
	double y;
	double z;
	bool bulge_found;
	double bulge;

	CPolyLinePoint(double px, double py, double pz, bool pbulge_found, double pbulge){x = px; y = py; z = pz; bulge_found = pbulge_found; bulge = pbulge;}
};

// derive a class from this and implement it's virtual functions
class CDxfRead{
private:
	std::ifstream* m_ifs;

	bool m_fail;
	char m_str[1024];
	char m_unused_line[1024];
	eDxfUnits_t m_eUnits;
	bool m_measurement_inch;
	char m_layer_name[1024];
	char m_section_name[1024];
	bool m_ignore_errors;
#ifdef STORE_LINE_NUMBERS
	long m_line_number;
#endif

	typedef std::map< std::string,Aci_t > LayerAciMap_t;
	LayerAciMap_t m_layer_aci;  // layer names -> layer color aci map

	bool ReadUCS();
	bool ReadUnits();
	bool ReadLayer();
	bool ReadSection();
	bool ReadBlock();
	bool ReadEndBlock();
	bool ReadInsert();
	bool ReadLine();
	bool ReadText();
	bool ReadMText();
	bool ReadRText();
	bool ReadArc();
	bool ReadCircle();
	bool ReadEllipse();
	bool ReadPoint();
	bool ReadSpline();
	bool ReadLwPolyLine();
	bool ReadPolyLine();
	bool ReadVertex(double *pVertex, bool *bulge_found, double *bulge);
	void OnReadArc(double start_angle, double end_angle, double radius, const double* c, bool hidden);
	void OnReadCircle(const double* c, double radius, bool hidden);
    void OnReadEllipse(const double* c, const double* m, double ratio, double start_angle, double end_angle);
	bool ReadLeader();
	bool ReadMLine();
	bool ReadXLine();
	bool ReadDimension();

	void get_line();
	void put_line(const char *value);
	void DerefACI();
	void StorePolyLinePoint(double x, double y, double z, bool bulge_found, double bulge);
	void AddPolyLinePoints(bool mirrored, bool closed);
	void ResetExtrusionAndThickness();
	bool ReadExtrusionOrThickness(int n);

protected:
	Aci_t m_aci; // manifest color name or 256 for layer color
	double m_extrusion_vector[3];
	double m_thickness;

public:
	CDxfRead(const char* filepath); // this opens the file
	~CDxfRead(); // this closes the file

	bool Failed(){return m_fail;}
	void DoRead(const bool ignore_errors = false); // this reads the file and calls the following functions

	double mm( double value ) const;

	bool IgnoreErrors() const { return(m_ignore_errors); }

	virtual void OnReadUCS(const double* /*ucs point*/){}
	virtual void OnReadBlock(const char* /*block_name*/, const double* /*base_point*/){}
	virtual void OnReadInsert(const char* /*block_name*/, const double* /*insert_point*/, double /*rotation_angle*/){}
	virtual void OnReadEndBlock(){}
	virtual void OnReadLine(const double* /*s*/, const double* /*e*/, bool /*hidden*/){}
	virtual void OnReadPoint(const double* /*s*/){}
	virtual void OnReadText(const double* /*point*/, const double /*height*/, const char* /*text*/, int /*hj*/, int /*vj*/){}
	virtual void OnReadArc(const double* /*s*/, const double* /*e*/, const double* /*c*/, bool /*dir*/, bool /*hidden*/){}
	virtual void OnReadCircle(const double* /*s*/, const double* /*c*/, bool /*dir*/, bool /*hidden*/){}
	virtual void OnReadEllipse(const double* /*c*/, double /*major_radius*/, double /*minor_radius*/, double /*rotation*/, double /*start_angle*/, double /*end_angle*/, bool /*dir*/){}
	virtual void OnReadSpline(struct SplineData& /*sd*/){}
	virtual void OnReadDimension(int /*dimension_type*/, double /*angle*/, double /*angle2*/, double /*angle3*/, double /*radius_leader_length*/, const double * /*def_point*/, const double * /*mid*/, const double * /*p1*/, const double * /*p2*/, const double * /*p3*/, const double * /*p4*/, const double * /*p5*/){}
	virtual void AddGraphics() { }

    std::string LayerName() const;

};
