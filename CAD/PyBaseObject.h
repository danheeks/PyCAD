#include "PyWrapper.h"

class BaseObject : public ObjList, public cad_wrapper<ObjList>
{
public:
	static bool in_glCommands;
	static bool triangles_begun;
	static bool lines_begun;
	static TiXmlElement* m_cur_element;
	static bool m_no_colour; // from glCommands
	static bool m_marked; // from glCommands
	static bool m_select; // from glCommands
	static const HeeksObj* copy_from_object;

	BaseObject() :ObjList(){}
	BaseObject(int type) :ObjList(){}
	bool NeverDelete();
	int GetType()const;
	const wchar_t* GetIconFilePath();
	const wchar_t* GetShortString()const;
	const wchar_t* GetTypeString()const;
	const HeeksColor* GetColor()const;
	void SetColor(const HeeksColor &col);
	void glCommands(bool select, bool marked, bool no_color);
	void GetProperties(std::list<Property *> *list);
	void GetBox(CBox &box);
	void KillGLLists();
	void WriteToXML(TiXmlElement *element);
	void ReadFromXML(TiXmlElement *element);
	HeeksObj* MakeACopy()const;
	void CopyFrom(const HeeksObj* object);
	void ReloadPointers();
	bool OneOfAKind();
	bool OneOfAKind_default();
	void OnAdd();
	void OnRemove();
	bool SetClickMarkPoint(MarkedObject* marked_object, const Point3d &ray_start, const Point3d &ray_direction);
	boost::python::override get_override(char const* name) const;

	const HeeksObj* GetCopyFromObject(){ return copy_from_object; }
};


void AddProperty(Property* property);
