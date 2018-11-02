// HeeksObj.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../Geom/Box.h"

#include <list>

class HeeksColor;
class Property;
class Tool;
class MarkedObject;
class TiXmlNode;
class TiXmlElement;
class GripData;

// NOTE: If adding to this enumeration, please also add the verbose description to the HeeksCADType() routine
enum{
	UnknownType,
	DocumentType,
	GripperType,
	PointType,
	LineType,
	ArcType,
	ILineType,
	CircleType,
	SketchType,
	AreaType,
	StlSolidType,
	CoordinateSystemType,
	TextType,
	DimensionType,
	RulerType,
    EllipseType,
	GroupType,
	AngularDimensionType,
	GearType,
	ImageType,
	XmlType,
	PythonType,
	ObjectMaximumType,
};



#define MARKING_FILTER_LINE					0x00000001
#define MARKING_FILTER_ARC					0x00000002
#define MARKING_FILTER_ILINE				0x00000004
#define MARKING_FILTER_CIRCLE				0x00000008
#define MARKING_FILTER_POINT				0x00000010
#define MARKING_FILTER_STL_SOLID			0x00000040
#define MARKING_FILTER_SKETCH				0x00000400
#define MARKING_FILTER_IMAGE				0x00000800
#define MARKING_FILTER_COORDINATE_SYSTEM	0x00000800
#define MARKING_FILTER_TEXT					0x00001000
#define MARKING_FILTER_DIMENSION			0x00002000
#define MARKING_FILTER_RULER				0x00004000
#define MARKING_FILTER_GEAR					0x00100000
#define MARKING_FILTER_AREA					0x00200000
#define MARKING_FILTER_UNKNOWN				0x00800000

#define MARKING_FILTER_SKETCH_GROUP			(MARKING_FILTER_SKETCH | MARKING_FILTER_AREA | MARKING_FILTER_CIRCLE)

class HeeksObj{
public:
	HeeksObj* m_owner;
	unsigned int m_id;
	bool m_visible;
	unsigned int m_index;

	HeeksObj(void);
	HeeksObj(const HeeksObj& ho);
	virtual ~HeeksObj();

	virtual const HeeksObj& operator=(const HeeksObj &ho);

	// virtual functions
	virtual int GetType()const{return UnknownType;}
	virtual long GetMarkingMask()const{return MARKING_FILTER_UNKNOWN;}
	virtual int GetIDGroupType()const{return GetType();}
	virtual void glCommands(bool select, bool marked, bool no_color){};
	virtual bool DrawAfterOthers(){return false;}
	virtual void GetBox(CBox &box){}
	virtual const wchar_t* GetShortString(void)const{return NULL;}
	virtual const wchar_t* GetTypeString(void)const{ return L"Unknown"; }
	const wchar_t* GetShortStringOrTypeString(void)const{ if (GetShortString())return GetShortString(); return GetTypeString(); }
	virtual bool CanEditString(void)const{return false;}
	virtual void OnEditString(const wchar_t* str){}
	virtual void KillGLLists(void){};
	virtual HeeksObj *MakeACopy()const{ return NULL; }
	virtual HeeksObj *MakeACopyWithID();
	virtual void Clear(){}
	virtual void ReloadPointers(){}
	virtual void CopyFrom(const HeeksObj* object){}
	virtual void SetColor(const HeeksColor &col){}
	virtual const HeeksColor* GetColor()const{return NULL;}
	virtual void Transform(const geoff_geometry::Matrix &m){} // transform the object
	virtual bool GetStartPoint(geoff_geometry::Point3d &pos){ return false; }
	virtual bool GetEndPoint(geoff_geometry::Point3d &pos){ return false; }
	virtual bool GetCentrePoint(geoff_geometry::Point3d &pos){ return false; }
	virtual int GetCentrePoints(geoff_geometry::Point3d &pos, geoff_geometry::Point3d &pos2){ if (GetCentrePoint(pos))return 1; return 0; }
	virtual bool GetMidPoint(geoff_geometry::Point3d &pos){ return false; }
	virtual bool GetScaleAboutMatrix(geoff_geometry::Matrix &m);
	virtual void GetProperties(std::list<Property *> *list);
	virtual void GetOnEdit(bool(**callback)(HeeksObj*)){ *callback = NULL; } // returns a function for doing edit with a dialog
	bool Edit(){bool(*fn)(HeeksObj*) = NULL;GetOnEdit(&fn);if(fn){if((*fn)(this)){WriteDefaultValues();return true;}}return false;}  // do edit with a dialog
	virtual void OnApplyProperties(){}
	virtual bool ValidateProperties(){return true;}
	virtual const wchar_t* GetIconFilePath();  
	virtual int Intersects(const HeeksObj *object, std::list< double > *rl)const{return 0;}
	virtual bool FindNearPoint(const geoff_geometry::Line &ray, geoff_geometry::Point3d &point){ return false; }
	virtual bool FindPossTangentPoint(const geoff_geometry::Line &ray, geoff_geometry::Point3d &point){ return false; }
	virtual void GetGripperPositionsTransformed(std::list<GripData> *list, bool just_for_endof);
	virtual bool Stretch(const geoff_geometry::Point3d &p, const geoff_geometry::Point3d &shift, void* data){ return false; } // return true, if undo stretch is done with Add and Delete
	virtual bool StretchTemporary(const geoff_geometry::Point3d &p, const geoff_geometry::Point3d &shift, void* data){ Stretch(p, shift, data); return true; } // returns true, because Stretch was done.  If not done, then override and return false;
	virtual void SetClickMarkPoint(MarkedObject* marked_object, const geoff_geometry::Point3d &ray_start, const geoff_geometry::Point3d &ray_direction){}
	virtual bool CanAdd(HeeksObj* object){return false;}
	virtual bool CanAddTo(HeeksObj* owner){return true;}
	virtual HeeksObj* PreferredPasteTarget(){return NULL;}
	virtual bool DescendForUndo(){return true;}
	virtual bool OneOfAKind(){return false;} // if true, then, instead of pasting, find the first object of the same type and copy object to it.
	virtual bool Add(HeeksObj* object, HeeksObj* prev_object);
	virtual bool IsDifferent(HeeksObj* other){return false;}
	virtual void Remove(HeeksObj* object){object->OnRemove();}
	virtual void OnAdd(){}
	virtual void OnRemove();
	virtual bool CanBeRemoved(){return true;}
	virtual bool CanBeDragged(){return true;}
	virtual bool CanBeCopied(){return true;}
	virtual HeeksObj* GetFirstChild(){return NULL;}
	virtual HeeksObj* GetNextChild(){return NULL;}
	virtual HeeksObj* GetAtIndex(int index){return NULL;}
	virtual int GetNumChildren(){return 0;}
	virtual std::list<HeeksObj *> GetChildren() const { std::list<HeeksObj *> empty; return(empty); }
	virtual bool AutoExpand(){return false;}
	virtual void GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal = true){} // [nine doubles, three doubles],  or [nine doubles, nine doubles] if just_one_average_normal = false
	virtual double Area()const{ return 0.0; }
	virtual void GetSegments(void(*callbackfunc)(const double *p, bool start), double pixels_per_mm)const{};
	virtual void WriteXML(TiXmlNode *root){}
	virtual void WriteBaseXML(TiXmlElement *element);
	virtual void ReadBaseXML(TiXmlElement* element);
	void SetID(int id);
	virtual unsigned int GetID(){return m_id;}
	virtual bool UsesID(){return true;}
	bool OnVisibleLayer();
	virtual bool IsList(){return false;}
	virtual HeeksObj *Find( const int type, const unsigned int id );
	virtual unsigned int GetIndex();
protected:
	virtual void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
public:
	virtual void OnChangeViewUnits(const double units){}
	virtual void WriteDefaultValues(){}
	virtual void ReadDefaultValues(){}
	virtual bool NeverDelete(){ return false; }
};
