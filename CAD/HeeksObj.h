// HeeksObj.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "../Geom/Box.h"

#include <list>

class HeeksColor;
class Property;
class TiXmlNode;
class TiXmlElement;
class GripData;
class Matrix;
class Point3d;
class Line;
class CFilter;

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
	GroupType,
	AngularDimensionType,
	GearType,
	ImageType,
	XmlType,
	ObjectMaximumType,
};


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
	virtual void Transform(const Matrix &m){} // transform the object
	virtual bool GetStartPoint(Point3d &pos){ return false; }
	virtual void SetStartPoint(const Point3d &pos){}
	virtual bool GetEndPoint(Point3d &pos){ return false; }
	virtual void SetEndPoint(const Point3d &pos){}
	virtual bool GetCentrePoint(Point3d &pos){ return false; }
	virtual void SetCentrePoint(const Point3d &pos){}
	virtual bool GetAxis(Point3d &){ return false; }
	virtual bool GetRadius(double &){ return false; }
	virtual int GetCentrePoints(Point3d &pos, Point3d &pos2){ if (GetCentrePoint(pos))return 1; return 0; }
	virtual bool GetMidPoint(Point3d &pos){ return false; }
	virtual void Reverse(){}
	virtual bool GetScaleAboutMatrix(Matrix &m);
	virtual void GetProperties(std::list<Property *> *list);
	virtual void GetOnEdit(bool(**callback)(HeeksObj*)){ *callback = NULL; } // returns a function for doing edit with a dialog
	virtual bool ValidateProperties(){return true;}
	virtual const wchar_t* GetIconFilePath();  
	virtual int Intersects(const HeeksObj *object, std::list< double > *rl)const{return 0;}
	virtual bool FindNearPoint(const Line &ray, Point3d &point){ return false; }
	virtual bool FindPossTangentPoint(const Line &ray, Point3d &point){ return false; }
	virtual void GetGripperPositionsTransformed(std::list<GripData> *list, bool just_for_endof);
	virtual bool Stretch(const Point3d &p, const Point3d &shift, void* data){ return false; } // return true, if undo stretch is done with Add and Delete
	virtual bool StretchTemporary(const Point3d &p, const Point3d &shift, void* data){ Stretch(p, shift, data); return true; } // returns true, because Stretch was done.  If not done, then override and return false;
	virtual bool SetClickMarkPoint(const Point3d &ray_start, const Point3d &ray_direction){ return false; }
	virtual bool CanAdd(HeeksObj* object){return false;}
	virtual bool CanAddTo(HeeksObj* owner){return true;}
	virtual HeeksObj* PreferredPasteTarget(){return NULL;}
	virtual bool DescendForUndo(){return true;}
	virtual bool OneOfAKind(){return false;} // if true, then, instead of pasting, find the first object of the same type and copy object to it.
	virtual bool Add(HeeksObj* object, HeeksObj* prev_object);
	virtual void Remove(HeeksObj* object){object->OnRemove();}
	virtual void OnAdd(){}
	virtual void OnAdded(HeeksObj* object){}
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
	virtual void WriteXML(TiXmlNode *root);
	virtual const wchar_t* GetXMLTypeString()const{ return GetTypeString(); } // default is to use the GetTypeString, but Coordinate System's type doesn't has a space in it in the XML.
	virtual bool AddOnlyChildrenOnReadXML(){ return false; }
	virtual void WriteToXML(TiXmlElement *element);
	virtual void ReadFromXML(TiXmlElement* element);
	void SetID(int id);
	virtual unsigned int GetID(){return m_id;}
	virtual bool UsesID(){return true;}
	bool OnVisibleLayer();
	virtual bool IsList(){return false;}
	virtual HeeksObj *Find( const int type, const unsigned int id );
	virtual unsigned int GetIndex();
	virtual int PickPriority(){ return 1; } // 1 for faces, 2 for edges, 3 for points
protected:
	virtual void GetGripperPositions(std::list<GripData> *list, bool just_for_endof);
public:
	virtual void OnChangeViewUnits(const double units){}
	virtual void WriteDefaultValues(){}
	virtual void ReadDefaultValues(){}
	virtual bool NeverDelete(){ return false; }
};
