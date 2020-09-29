// not really an app, but a class with one global object to put global variables in and also is the list of objects to render
#pragma once

#include "HeeksColor.h"
#include "App.h"
#include "Index.h"
#include "ObjList.h"
#include <map>
#include <set>

#define HEEKSCAD_VERSION_MAIN L"2"
#define HEEKSCAD_VERSION_SUB L"0"

extern std::wstring HeeksCADType(const int type);

#define MAX_RECENT_FILES 20

enum GraphicsTextMode
{
	GraphicsTextModeNone,
	GraphicsTextModeInputTitle,
	GraphicsTextModeWithHelp
};

enum BackgroundMode
{
	BackgroundModeOneColor,
	BackgroundModeTwoColors,
	BackgroundModeTwoColorsLeftToRight,
	BackgroundModeFourColors,
	BackgroundModeSkyDome
};

class CViewport;
class CViewPoint;
class CInputMode;
class CoordinateSystem;
class HRuler;
class Property;
class Observer;
class MainHistory;
class MagDragWindow;
class ViewRotating;
class ViewZooming;
class ViewPanning;
class DigitizeMode;
class MarkedList;
class GripperMode;
class Gripper;
class MainHistory;
class MouseEvent;

class CCadApp : public CApp, public ObjList
//	not really an app, but a class with one global object to put global variables in and also is the list of objects to render
{
private:
	std::set<Observer*> observers;
	MainHistory *history;

	typedef std::map< int, std::list<HeeksObj*> > IdsToObjects_t;
	typedef int GroupId_t;
	typedef std::map< GroupId_t, IdsToObjects_t > UsedIds_t;
	UsedIds_t	used_ids;
	std::map< int, int > next_id_map;
	std::map< std::string, HeeksObj*(*)() > object_create_fn_map;
	int m_observers_frozen;
	std::list<HeeksObj*> frozen_added;
	std::list<HeeksObj*> frozen_removed;
	std::list<HeeksObj*> frozen_modified;
	bool frozen_selection_cleared;
	std::list<HeeksObj*> frozen_selection_added;
	std::list<HeeksObj*> frozen_selection_removed;
	Index<unsigned, HeeksObj*> m_name_index;

	void render_screen_text2(const wchar_t* str, double scale);
	void RenderDatumOrCurrentCoordSys();

public:
	CCadApp();
	~CCadApp();

	IPoint cur_mouse_pos;
	HeeksColor current_color;
#define NUM_BACKGROUND_COLORS 10
	HeeksColor background_color[NUM_BACKGROUND_COLORS];
	BackgroundMode m_background_mode;
	int m_rotate_mode;
	bool m_antialiasing;
	bool digitize_end;
	bool digitize_inters;
	bool digitize_centre;
	bool digitize_midpoint;
	bool digitize_nearest;
	bool digitize_coords;
	bool digitize_screen;
	bool digitize_tangent;
	double digitizing_radius; // for ambiguous arcs and circles
	bool draw_to_grid;
	double digitizing_grid;
	bool m_allow_opengl_stippling;
	SolidViewMode m_solid_view_mode;
	bool m_stl_save_as_binary;
	bool m_stl_solid_random_colors;
	bool m_svg_unite;

	CoordinateSystem *m_current_coordinate_system;
	CInputMode *input_mode_object;
	MagDragWindow *magnification;
	ViewRotating *viewrotating;
	ViewZooming *viewzooming;
	ViewPanning *viewpanning;
	DigitizeMode *m_digitizing;
	GripperMode* gripper_mode;
	int grid_mode;
	Gripper *drag_gripper;
	Point3d grip_from, grip_to;
	Gripper *cursor_gripper;
	CViewport *m_current_viewport;
	MarkedList *m_marked_list;
	bool m_doing_rollback;
	bool m_light_push_matrix;
	std::list<HeeksObj*> m_hidden_for_drag;
	bool m_show_grippers_on_drag;
	double m_sketch_reorder_tol;
	std::list< void(*)() > m_on_glCommands_list;
	HRuler* m_ruler;
	bool m_show_ruler;
	bool m_show_datum_coords_system;
	bool m_datum_coords_system_solid_arrows;
	bool m_in_OpenFile;
	bool m_mark_newly_added_objects;
	std::wstring m_version_number;
	std::list< void(*)(MouseEvent&) > m_lbutton_up_callbacks;
	std::list< bool(*)() > m_is_modified_callbacks;
	std::list< void(*)() > m_on_build_texture_callbacks;
	std::list< void(*)(int, int) > m_beforeneworopen_callbacks;
	std::list< void(*)() > m_beforeframedelete_callbacks;
	int m_transform_gl_list;
	Matrix m_drag_matrix;
	bool m_extrude_removes_sketches;
	bool m_loft_removes_sketches;
	GraphicsTextMode m_graphics_text_mode;
	bool m_print_scaled_to_page;
	FileOpenOrImportType m_file_open_or_import_type;
	bool m_set_id_in_add;
	Matrix* m_file_open_matrix;
	double m_view_units; // units to display to the user ( but everything is stored as mm ), 1.0 for mm, 25.4 for inches
	bool m_dragging_moves_objects;
	std::wstring m_res_folder;

	double m_min_correlation_factor;
	double m_max_scale_threshold;
	int m_number_of_sample_points;
	bool m_correlate_by_color;
	bool m_property_grid_validation;
	double m_stl_facet_tolerance;

	int m_icon_texture_number;
	bool m_extrude_to_solid;
	double m_revolve_angle;
	bool m_fit_arcs_on_solid_outline;
	bool undoable_in_OpenXMLFile;
	virtual bool GetUndoableForOpenXML(){ return undoable_in_OpenXMLFile; }
	HeeksObj* paste_into_for_OpenXMLFile;
	virtual HeeksObj* GetPastIntoForOpenXML(){ return paste_into_for_OpenXMLFile; }

	CInputMode* m_previous_input_mode;
	TiXmlElement* m_cur_xml_element;
	TiXmlNode* m_cur_xml_root;

	typedef void(*UnitsChangedHandler_t)(const double value);
	typedef std::list<UnitsChangedHandler_t> UnitsChangedHandlers_t;

	UnitsChangedHandlers_t m_units_changed_handlers;

	void RegisterUnitsChangeHandler(UnitsChangedHandler_t);
	void UnregisterUnitsChangeHandler(UnitsChangedHandler_t);

	bool m_settings_restored;

	// HeeksObj's virtual functions
	void glCommands(bool select, bool marked, bool no_color);
	bool CanAdd(HeeksObj* object){ return true; }
	int GetType()const{ return DocumentType; }

	void OnExit();
	void CreateLights(void);
	void DestroyLights(void);
	void SetInputMode(CInputMode *i);
	void RestoreInputMode();
	virtual void Repaint(bool soon = false);
	void RecalculateGLLists();
	void SetLikeNewFile(void);
	bool IsModified(void);
	void SetAsModified();
	void ClearHistory(void);
	void glCommandsAll(const CViewPoint &view_point);
	virtual double GetPixelScale(void);
	void DoUndoable(Undoable *);
	bool RollBack(void);
	bool RollForward(void);
	bool CanUndo(void);
	bool CanRedo(void);
	virtual void StartHistory();
	virtual void EndHistory(void);
	void ClearRollingForward(void);
	bool Add(HeeksObj* object, HeeksObj* prev_object);
	void Remove(HeeksObj* object);
	void Remove(std::list<HeeksObj*> objects);
	void Transform(std::list<HeeksObj*> objects, const Matrix& m);
	void Reset();
	HeeksObj* CreateObjectOfType(const std::string& name);
	virtual HeeksObj* ReadXMLElement(TiXmlElement* pElem);
	virtual void ObjectWriteToXML(HeeksObj *object, TiXmlElement *element);
	virtual void ObjectReadFromXML(HeeksObj *object, TiXmlElement* element);
	void InitializeCreateFunctions();
	void OpenXMLFile(const wchar_t *filepath, HeeksObj* paste_into = NULL, HeeksObj* paste_before = NULL, bool call_was_added = false, bool show_error = true, bool undoable = false);
	static void OpenSVGFile(const wchar_t *filepath);
	static void OpenSTLFile(const wchar_t *filepath);
	static void OpenDXFFile(const wchar_t *filepath);
	static void OpenRS274XFile(const wchar_t *filepath);
	bool OpenImageFile(const wchar_t *filepath);
	void OnNewButton();
	void OnOpenButton();
	bool OpenFile(const wchar_t *filepath, bool import_not_open = false, HeeksObj* paste_into = NULL, HeeksObj* paste_before = NULL, bool retain_filename = true);
	void SaveDXFFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath);
	void SaveSTLFileBinary(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance = -1.0, double* scale = NULL);
	void SaveSTLFileAscii(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance = -1.0, double* scale = NULL);
	void SaveOBJFileAscii(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance = -1.0, double* scale = NULL);
	void SaveSTLFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance = -1.0, double* scale = NULL, bool binary = true);
	void SaveCPPFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance = -1.0);
	void SavePyFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance = -1.0);
	void SaveXMLFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath, bool for_clipboard = false);
	void SaveXMLFile(const wchar_t *filepath){ SaveXMLFile(m_objects, filepath); }
	bool SaveFile(const wchar_t *filepath, const std::list<HeeksObj*>* objects = NULL);
	virtual void AddUndoably(HeeksObj *object, HeeksObj* owner, HeeksObj* prev_object = NULL);
	void AddUndoably(const std::list<HeeksObj*>& list, HeeksObj* owner);
	virtual void DeleteUndoably(HeeksObj* object);
	virtual void DeleteUndoably(const std::list<HeeksObj*>& list);
	void CopyUndoably(HeeksObj* object, HeeksObj* copy_with_new_data);
	void TransformUndoably(HeeksObj *object, const Matrix &m);
	void TransformUndoably(const std::list<HeeksObj*>& list, const Matrix &m);
	void ReverseUndoably(HeeksObj *object);
	void WasModified(HeeksObj *object);
	void WasAdded(HeeksObj *object);
	void WasRemoved(HeeksObj *object);
	void WereModified(const std::list<HeeksObj*>& list);
	void WereAdded(const std::list<HeeksObj*>& list);
	void WereRemoved(const std::list<HeeksObj*>& list);
	virtual Matrix* GetDrawMatrix(bool get_the_appropriate_orthogonal);
	void DrawObjectsOnFront(const std::list<HeeksObj*> &list, bool do_depth_testing);
	CInputMode* GetDigitizing();
	void GetOptions(std::list<Property *> *list);
	void DeleteMarkedItems();
	virtual void glColorEnsuringContrast(const HeeksColor &c);
	void RegisterObserver(Observer* observer);
	void RemoveObserver(Observer* observer);
	void ObserversOnChange(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified);
	void ObserversMarkedListChanged(bool selection_cleared, const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed);
	void ObserversFreeze();
	void ObserversThaw();
	void ObserversClear();
	std::wstring GetExeFolder()const;
	virtual std::wstring GetResFolder()const;
	void get_2d_arc_segments(double xs, double ys, double xe, double ye, double xc, double yc, bool dir, bool want_start, double pixels_per_mm, void(*callbackfunc)(const double* xy));
	IRect PointToPickBox(const IPoint& point);
	void GetObjectsInWindow(const IRect &window, bool only_if_fully_in, bool one_of_each, const CFilter &filter, std::list<HeeksObj*> &objects, bool just_top_level_item = true, bool sort_by_pick_priority = true);
	void ColorPickLowestObjects(IRect window, bool single_picking, std::list<HeeksObj*> &objects);
	bool PickPosition(const wchar_t* str, double* pos, void(*callback)(const double*) = NULL);
	void glSphere(double radius, const double* pos = NULL);
	void OnNewOrOpen(bool open, int res);
	void OnBeforeNewOrOpen(bool open, int res);
	void OnBeforeFrameDelete(void);
	virtual HeeksObj* GetIDObject(int type, int id);
	std::list<HeeksObj*> GetIDObjects(int type, int id);
	virtual void SetObjectID(HeeksObj* object, int id);
	virtual int GetNextID(int type);
	virtual void RemoveID(HeeksObj* object); // only call this from ObjList::Remove()
	void ResetIDs();
	bool InputInt(const wchar_t* prompt, const wchar_t* value_name, int &value);
	bool InputDouble(const wchar_t* prompt, const wchar_t* value_name, double &value);
	bool InputAngleWithPlane(double &angle, double *axis = NULL, double *pos = NULL, int *number_of_copies = NULL, double *axial_shift = NULL);
	//bool InputFromAndTo(double *from, double *to, int *number_of_copies = NULL);
	bool InputLength(const wchar_t* prompt, const wchar_t* value_name, double &value);
	void ShowModalOptions();
	void SectioningDialog();
	void RegisterOnGLCommands(void(*callbackfunc)());
	void RemoveOnGLCommands(void(*callbackfunc)());
	void RegisterIsModifiedFn(bool(*callbackfunc)());
	void CreateTransformGLList(const std::list<HeeksObj*>& list, bool show_grippers_on_drag);
	void DestroyTransformGLList();
	bool IsPasteReady();
	void EnableBlend();
	void DisableBlend();
	void Paste(HeeksObj* paste_into, HeeksObj* paste_before);
	bool CheckForNOrMore(const std::list<HeeksObj*> &list, int min_num, int type, const std::wstring& msg, const std::wstring& caption);
	bool CheckForNOrMore(const std::list<HeeksObj*> &list, int min_num, int type1, int type2, const std::wstring& msg, const std::wstring& caption);
	bool CheckForNOrMore(const std::list<HeeksObj*> &list, int min_num, int type1, int type2, int type3, const std::wstring& msg, const std::wstring& caption);
	void render_text(const wchar_t* str, bool select, double scale, double blur_scale);
	bool get_text_size(const wchar_t* str, float* width, float* height);
	void render_screen_text(const wchar_t* str1, const wchar_t* str2);
	void render_screen_text_at(const wchar_t* str1, double scale, double x, double y, double theta);
	void OnInputModeTitleChanged();
	void OnInputModeHelpTextChanged();
	void RefreshInputCanvas();
	CInputMode* GetInputMode(){ return input_mode_object; }
	void PlotSetColor(const HeeksColor &c);
	void PlotLine(const double* s, const double* e);
	void PlotArc(const double* s, const double* e, const double* c);
	void InitialiseLocale();
	void GetPluginsFromCommandLineParams(std::list<std::wstring> &plugins);
	void RegisterOnBuildTexture(void(*callbackfunc)());
	void RegisterOnBeforeNewOrOpen(void(*callbackfunc)(int, int));
	void RegisterOnBeforeFrameDelete(void(*callbackfunc)());
	virtual void DoMessageBox(const wchar_t* message);

	typedef int ObjectType_t;
	typedef int ObjectId_t;
	typedef std::pair< ObjectType_t, ObjectId_t > ObjectReference_t;
	typedef std::map< ObjectReference_t, HeeksObj * > ObjectReferences_t;

	std::wstring HeeksType(const int type) const;
	virtual unsigned int GetIndex(HeeksObj *object);
	virtual void ReleaseIndex(unsigned int index);

	void RegisterOnRestoreDefaults(void(*callbackfunc)());
	void RestoreDefaults();

	void LinkXMLEndChild(TiXmlNode* root, TiXmlElement* pElem);
	TiXmlElement* FirstNamedXMLChildElement(TiXmlElement* pElem, const char* name);
	void RemoveXMLChild(TiXmlNode* pElem, TiXmlElement* child);
	DigitizedPoint& Digitize(const IPoint&);
	const DigitizedPoint& GetLastDigitizePoint();
	void SetLastDigitizedPoint(const DigitizedPoint&);
	void UseDigitiedPointAsReference();
	void ObjectAreaString(HeeksObj* object, std::wstring &s);
	void SetViewUnits(double units, bool write_to_config);
	virtual void ClearSelection(bool call_OnChanged);
	void GetSelection(std::list<HeeksObj*> &objects);
	virtual bool ObjectMarked(HeeksObj* object);
	virtual void Mark(HeeksObj* object);
	virtual void Unmark(HeeksObj* object);
	virtual bool InOpenFile(){ return m_in_OpenFile; }
	virtual FileOpenOrImportType GetFileOpenOrImportType(){ return m_file_open_or_import_type; }
	virtual bool GetSetIdInAdd(){return m_set_id_in_add;}
	virtual void SetSetIdInAdd(bool value){ m_set_id_in_add = value; }
	virtual SolidViewMode GetSolidViewMode(){ return m_solid_view_mode; }
	TiXmlNode* GetXmlRoot(){ return m_cur_xml_root; }
	TiXmlElement* GetXmlElement(){ return m_cur_xml_element; }
	void SketchSplit(HeeksObj*, std::list<HeeksObj*> &);
	HeeksObj* CreateNewLine(const Point3d& s, const Point3d& e);
	HeeksObj* CreateNewArc(const Point3d& s, const Point3d& e, const Point3d& a, const Point3d& c);
	HeeksObj* CreateNewCircle(const Point3d& c, const Point3d& a, double r);
	HeeksObj* CreateNewPoint(const Point3d& p);
	HeeksObj* CreateNewSketch();
	const HeeksColor& GetCurrentColor(){ return current_color; }
	void DrawFront();
	void EndDrawFront();
	HeeksObj* GetObjPointer();
	ObjList* GetObjListPointer();
	std::list<HeeksObj *> GetXmlWriteChildren();
	bool GetDatumSolid(){ return m_datum_coords_system_solid_arrows; }
	void RenderArrow();
};

extern CCadApp* theApp;
