// not really an app, but a class with one global object to put global variables in and also is the list of objects to render
#pragma once

#include <list>
#include <string>

class HeeksColor;
class TiXmlElement;
class TiXmlNode;
class DigitizedPoint;
class CInputMode;
class IPoint;
class HeeksObj;
class Undoable;
class Property;
class ObjList;
class Matrix;
class Point3d;

enum FileOpenOrImportType
{
	FileOpenOrImportTypeOther,
	FileOpenTypeHeeks,
	FileImportTypeHeeks,
	FileOpenOrImportTypeDxf
};

enum SolidViewMode
{
	SolidViewFacesAndEdges,
	SolidViewEdgesOnly,
	SolidViewFacesOnly,
};

class CApp
	//	not really an app, but a class with one global object to put global variables in and also is the list of objects to render
	// this class is the base class with virtual functions that can be built into extension modules, the actual functionality happening in CCadApp
{
public:
	CApp(){}
	~CApp(){}
	virtual bool GetUndoableForOpenXML() = 0;
	virtual HeeksObj* GetPastIntoForOpenXML() = 0;
	virtual void Repaint(bool soon = false) = 0;
	virtual double GetPixelScale(void) = 0;
	virtual void StartHistory(const wchar_t* title, bool freeze_observers = true) = 0;
	virtual void EndHistory(void) = 0;
	virtual HeeksObj* ReadXMLElement(TiXmlElement* pElem) = 0;
	virtual void ObjectWriteToXML(HeeksObj *object, TiXmlElement *element) = 0;
	virtual void ObjectReadFromXML(HeeksObj *object, TiXmlElement* element) = 0;
	virtual void DoUndoable(Undoable *) = 0;
	virtual void AddUndoably(HeeksObj *object, HeeksObj* owner, HeeksObj* add_before = NULL) = 0;
	virtual void AddUndoably(const std::list<HeeksObj*>& list, HeeksObj* owner) = 0;
	virtual void DeleteUndoably(HeeksObj* object) = 0;
	virtual void DeleteUndoably(const std::list<HeeksObj*>& list) = 0;
	virtual Matrix* GetDrawMatrix(bool get_the_appropriate_orthogonal) = 0;
	virtual void DrawObjectsOnFront(const std::list<HeeksObj*> &list, bool do_depth_testing = false) = 0;
	virtual void glColorEnsuringContrast(const HeeksColor &c) = 0;
	virtual std::wstring GetResFolder()const = 0;
	virtual HeeksObj* GetIDObject(int type, int id) = 0;
	virtual void SetObjectID(HeeksObj* object, int id) = 0;
	virtual int GetNextID(int type) = 0;
	virtual void RemoveID(HeeksObj* object) = 0; // only call this from ObjList::Remove()
	virtual void DoMessageBox(const wchar_t* message) = 0;
	virtual unsigned int GetIndex(HeeksObj *object) = 0;
	virtual void ReleaseIndex(unsigned int index) = 0;
	virtual void ClearSelection(bool call_OnChanged) = 0;
	virtual void GetSelection(std::list<HeeksObj*> &objects) = 0;
	virtual bool ObjectMarked(HeeksObj* object) = 0;
	virtual void Mark(HeeksObj* object) = 0;
	virtual void Unmark(HeeksObj* object) = 0;
	virtual SolidViewMode GetSolidViewMode() = 0;
	virtual bool InOpenFile() = 0;
	virtual FileOpenOrImportType GetFileOpenOrImportType() = 0;
	virtual bool GetSetIdInAdd() = 0;
	virtual void SetSetIdInAdd(bool value) = 0;
	virtual TiXmlNode* GetXmlRoot() = 0;
	virtual TiXmlElement* GetXmlElement() = 0;
	virtual void SketchSplit(HeeksObj*, std::list<HeeksObj*> &) = 0;
	virtual HeeksObj* CreateNewLine(const Point3d& s, const Point3d& e) = 0;
	virtual HeeksObj* CreateNewArc(const Point3d& s, const Point3d& e, const Point3d& a, const Point3d& c) = 0;
	virtual HeeksObj* CreateNewCircle(const Point3d& c, const Point3d& a, double r) = 0;
	virtual HeeksObj* CreateNewPoint(const Point3d& p) = 0;
	virtual HeeksObj* CreateNewSketch() = 0;
	virtual const HeeksColor& GetCurrentColor() = 0;
	virtual void RegisterOnGLCommands(void(*callbackfunc)()) = 0;
	virtual HeeksObj* GetObjPointer() = 0;
	virtual ObjList* GetObjListPointer() = 0;
	virtual std::list<HeeksObj *> GetXmlWriteChildren() = 0;
	virtual bool GetDatumSolid() = 0;
	virtual void RenderArrow() = 0;
	virtual double GetViewUnits() = 0;
};
