// svg.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

struct TwoPoints
{
	geoff_geometry::Point3d ppnt;
	geoff_geometry::Point3d pcpnt;
};

class CArea;

// derive a class from this and implement it's virtual functions
class CSvgRead{
private:
	std::list<geoff_geometry::Matrix> m_transform_stack;
	bool m_fail;
	double m_stroke_width;
	double m_fill_opacity;
	CArea* m_current_area;
	bool m_usehspline;
	std::list<HeeksObj*> m_sketches_to_add;
#if 0
	CSketch* m_sketch;
#endif
	bool m_unite;

	std::string RemoveCommas(std::string input);
	void ReadSVGElement(TiXmlElement* pElem);
	void ReadStyle(TiXmlAttribute* a);
	void ReadG(TiXmlElement* pElem);
	void ReadTransform(TiXmlElement* pElem);
	void ReadPath(TiXmlElement* pElem);
	void ReadRect(TiXmlElement* pElem);
	void ReadCircle(TiXmlElement* pElem);
	void ReadEllipse(TiXmlElement* pElem);
	void ReadLine(TiXmlElement* pElem);
	void ReadPolyline(TiXmlElement* pElem, bool close);
	geoff_geometry::Point3d ReadStart(const char *text,geoff_geometry::Point3d ppnt,bool isupper);
	void ReadClose(geoff_geometry::Point3d ppnt, geoff_geometry::Point3d spnt);
	geoff_geometry::Point3d ReadLine(const char *text,geoff_geometry::Point3d ppnt,bool isupper);
	geoff_geometry::Point3d ReadHorizontal(const char *text,geoff_geometry::Point3d ppnt,bool isupper);
	geoff_geometry::Point3d ReadVertical(const char *text,geoff_geometry::Point3d ppnt,bool isupper);
	struct TwoPoints ReadCubic(const char *text,geoff_geometry::Point3d ppnt,bool isupper);
	struct TwoPoints ReadCubic(const char *text,geoff_geometry::Point3d ppnt,geoff_geometry::Point3d pcpnt, bool isupper);
	struct TwoPoints ReadQuadratic(const char *text,geoff_geometry::Point3d ppnt,bool isupper);
	struct TwoPoints ReadQuadratic(const char *text,geoff_geometry::Point3d ppnt,geoff_geometry::Point3d pcpnt,bool isupper);
	geoff_geometry::Point3d ReadEllipse(const char *text,geoff_geometry::Point3d ppnt,bool isupper);
	int JumpValues(const char *text, int number);
	void ProcessArea();
public:
	CSvgRead(const wchar_t* filepath, bool usehspline, bool unite); // this opens the file
	~CSvgRead(); // this closes the file

	geoff_geometry::Matrix m_transform;

	void Read(const wchar_t* filepath);

	void AddSketchIfNeeded();
	void ModifyByMatrix(HeeksObj* object);
	void OnReadStart();
	void OnReadLine(geoff_geometry::Point3d p1, geoff_geometry::Point3d p2);
	void OnReadCubic(geoff_geometry::Point3d s, geoff_geometry::Point3d c1, geoff_geometry::Point3d c2, geoff_geometry::Point3d e);
	void OnReadQuadratic(geoff_geometry::Point3d s, geoff_geometry::Point3d c, geoff_geometry::Point3d e);
	void OnReadEllipse(geoff_geometry::Point3d c, double maj_r, double min_r, double rot, double start, double end);
	void OnReadCircle(geoff_geometry::Point3d c, double r);

	bool Failed(){return m_fail;}
};

class CSketch;
