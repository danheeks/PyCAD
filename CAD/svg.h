// svg.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

struct TwoPoints
{
	Point3d ppnt;
	Point3d pcpnt;
};

class CArea;

// derive a class from this and implement it's virtual functions
class CSvgRead{
private:
	std::list<Matrix> m_transform_stack;
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
	Point3d ReadStart(const char *text,Point3d ppnt,bool isupper);
	void ReadClose(Point3d ppnt, Point3d spnt);
	Point3d ReadLine(const char *text,Point3d ppnt,bool isupper);
	Point3d ReadHorizontal(const char *text,Point3d ppnt,bool isupper);
	Point3d ReadVertical(const char *text,Point3d ppnt,bool isupper);
	struct TwoPoints ReadCubic(const char *text,Point3d ppnt,bool isupper);
	struct TwoPoints ReadCubic(const char *text,Point3d ppnt,Point3d pcpnt, bool isupper);
	struct TwoPoints ReadQuadratic(const char *text,Point3d ppnt,bool isupper);
	struct TwoPoints ReadQuadratic(const char *text,Point3d ppnt,Point3d pcpnt,bool isupper);
	Point3d ReadEllipse(const char *text,Point3d ppnt,bool isupper);
	int JumpValues(const char *text, int number);
	void ProcessArea();
public:
	CSvgRead(const wchar_t* filepath, bool usehspline, bool unite); // this opens the file
	~CSvgRead(); // this closes the file

	Matrix m_transform;

	void Read(const wchar_t* filepath);

	void AddSketchIfNeeded();
	void ModifyByMatrix(HeeksObj* object);
	void OnReadStart();
	void OnReadLine(Point3d p1, Point3d p2);
	void OnReadCubic(Point3d s, Point3d c1, Point3d c2, Point3d e);
	void OnReadQuadratic(Point3d s, Point3d c, Point3d e);
	void OnReadEllipse(Point3d c, double maj_r, double min_r, double rot, double start, double end);
	void OnReadCircle(Point3d c, double r);

	bool Failed(){return m_fail;}
};

class CSketch;
