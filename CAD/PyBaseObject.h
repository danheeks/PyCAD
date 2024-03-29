#include "PyWrapper.h"

class BaseObject : public ObjList, public cad_wrapper<ObjList>
{
public:
	static bool in_glCommands;
	static bool triangles_begun;
	static bool lines_begun;
	static bool m_no_colour; // from glCommands
	static bool m_marked; // from glCommands
	static bool m_select; // from glCommands
	static const HeeksObj* copy_from_object;

	int m_type;

	BaseObject() :ObjList(), m_type(0){}
	BaseObject(int type) :ObjList(), m_type(type){}
	bool NeverDelete();
	int GetType()const;
	const wchar_t* GetIconFilePath();
	const wchar_t* GetShortString()const;
	bool CanEditString(void)const;
	void OnEditString(const wchar_t* str);
	const wchar_t* GetTypeString()const;
	const HeeksColor* GetColor()const;
	void SetColor(const HeeksColor &col);
	void glCommands(bool select, bool marked, bool no_color);
	void GetProperties(std::list<Property *> *list);
	void GetGripperPositionsTransformed(std::list<GripData> *list, bool just_for_endof);
	bool StretchTemporary(const Point3d &p, const Point3d &shift, void* data){ return false; } // don't support dynamic stretching yet
	bool Stretch(const Point3d &p, const Point3d &shift, void* data);
	void Transform(const Matrix &m);
	void GetBox(CBox &box);
	void KillGLLists();
	void WriteToXML(TiXmlElement *element);
	void ReadFromXML(TiXmlElement *element);
	const wchar_t* GetXMLTypeString()const;
	bool UsesID();
	HeeksObj* MakeACopy()const;
	void CopyFrom(const HeeksObj* object);
	void ReloadPointers();
	bool OneOfAKind();
	bool OneOfAKind_default();
	void OnAdd();
	void OnRemove();
	bool CanAdd(HeeksObj* object);
	bool CanAddTo(HeeksObj* owner);
	HeeksObj* PreferredPasteTarget();
	bool SetClickMarkPoint(const Point3d &ray_start, const Point3d &ray_direction);
	void GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal = true);
	boost::python::override get_override(char const* name) const;

	const HeeksObj* GetCopyFromObject(){ return copy_from_object; }

	void AddTriangle(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2);
};

void AddGripper(GripData& gripper);
Point3d GetStretchPoint();
Point3d GetStretchShift();
