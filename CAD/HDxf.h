// HDxf.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "dxf.h"
#include "strconv.h"

class CSketch;

// a class just to be used while doing dxf import
class HInsert: public HeeksObj
{
	std::string m_block_name;
	double m_insert_point[3];
	double m_rotation_angle;

	wchar_t title[1024]; // to be removed

public:
	HInsert(const char* block_name, const double* insert_point, double rotation_angle)
	{
		m_block_name.assign(block_name);
		memcpy(m_insert_point, insert_point, 3*sizeof(double));
		m_rotation_angle = rotation_angle;

		swprintf(title, L"Insert of %s at x%lf y%lf %lf rot%lf", Ctt(block_name), insert_point[0], insert_point[1], insert_point[2], rotation_angle);
	}

	HeeksObj *MakeACopy(void)const{ return new HInsert(*this);}

	const wchar_t* GetShortString(void)const{ return title; } // to be removed
};

class HeeksDxfRead : public CDxfRead{
private:
    typedef std::wstring LayerName_t;
	typedef std::map< LayerName_t, CSketch * > Sketches_t;
	Sketches_t m_sketches;
    typedef std::wstring BlockName_t;
	typedef std::map< BlockName_t, CSketch * > Blocks_t;
	Blocks_t m_blocks;
	std::set<BlockName_t> inserted_blocks;
	CSketch* m_current_block;
	Matrix m_ucs_matrix;
	bool m_undoable;

	HeeksColor DecodeACI(const int aci);
#if 0
	void OnReadSpline(TColgp_Array1OfPnt &control, TColStd_Array1OfReal &weight, TColStd_Array1OfReal &knot,TColStd_Array1OfInteger &mult, int degree, bool periodic, bool rational);
#endif
	bool IsValidLayerName( const std::wstring layer_name ) const;

protected:
	HeeksColor *ActiveColorPtr(Aci_t & aci);

public:
	HeeksDxfRead(const wchar_t* filepath, bool undoable);

	static bool m_make_as_sketch;
	static bool m_ignore_errors;
	static bool m_read_points;
	static std::wstring m_layer_name_suffixes_to_discard;
	static bool m_add_uninstanced_blocks;

	// CDxfRead's virtual functions
	void OnReadUCS(const double* ucs_point);
	void OnReadBlock(const char* block_name, const double* base_point);
	void OnReadEndBlock();
	void OnReadInsert(const char* block_name, const double* insert_point, double rotation_angle);
	void OnReadLine(const double* s, const double* e, bool hidden);
	void OnReadPoint(const double* s);
	void OnReadText(const double* point, const double height,  const char* text, int hj, int vj);
	void OnReadArc(const double* s, const double* e, const double* c, bool dir, bool hidden);
	void OnReadCircle(const double* s, const double* c, bool dir, bool hidden);
    void OnReadEllipse(const double* c, double major_radius, double minor_radius, double rotation, double start_angle, double end_angle, bool dir);
	void OnReadSpline(struct SplineData& sd);
	void OnReadDimension(int, double, double, double, double, const double *, const double *, const double *, const double *, const double *, const double *, const double *);

	void AddObject(HeeksObj *object);
	void AddGraphics();
};
