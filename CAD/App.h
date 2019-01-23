// not really an app, but a class with one global object to put global variables in and also is the list of objects to render
#pragma once

#include "HeeksColor.h"
#include "ObjList.h"
#include "glfont2.h"
#include <map>

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

enum FileOpenOrImportType
{
	FileOpenOrImportTypeOther,
	FileOpenTypeHeeks,
	FileImportTypeHeeks,
	FileOpenOrImportTypeDxf
};

enum BackgroundMode
{
	BackgroundModeOneColor,
	BackgroundModeTwoColors,
	BackgroundModeTwoColorsLeftToRight,
	BackgroundModeFourColors,
	BackgroundModeSkyDome
};

enum SolidViewMode
{
	SolidViewFacesAndEdges,
	SolidViewEdgesOnly,
	SolidViewFacesOnly,
};

class CViewport;
class CViewPoint;
class CInputMode;
class CSelectMode;
class CoordinateSystem;
class HRuler;
class Property;
class Tool;
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

class CApp : public ObjList
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
	std::map< std::string, HeeksObj*(*)(TiXmlElement* pElem) > xml_read_fn_map;

	void render_screen_text2(const wchar_t* str, bool select);
	float get_text_scale();
	void RenderDatumOrCurrentCoordSys();

public:
	CApp();
	~CApp();

	IPoint cur_mouse_pos;
	HeeksColor current_color;
#define NUM_BACKGROUND_COLORS 10
	HeeksColor background_color[NUM_BACKGROUND_COLORS];
	BackgroundMode m_background_mode;
	bool m_gl_font_initialized;
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
	bool useOldFuse;
	double digitizing_grid;
	bool mouse_wheel_forward_away; // true for forwards/backwards = zoom out / zoom in, false for reverse
	bool ctrl_does_rotate; // true - rotate on Ctrl, pan when not Ctrl      false - rotate when not Ctrl, pan when Ctrl
	bool m_allow_opengl_stippling;
	SolidViewMode m_solid_view_mode;
	bool m_stl_save_as_binary;
	bool m_mouse_move_highlighting;
	HeeksColor m_highlight_color;
	bool m_stl_solid_random_colors;
	bool m_svg_unite;

	CoordinateSystem *m_current_coordinate_system;
	CInputMode *input_mode_object;
	MagDragWindow *magnification;
	ViewRotating *viewrotating;
	ViewZooming *viewzooming;
	ViewPanning *viewpanning;
	CSelectMode *m_select_mode;
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
	std::list< void(*)(bool) > m_on_save_callbacks;
	std::list< bool(*)() > m_is_modified_callbacks;
	std::list< void(*)() > m_on_build_texture_callbacks;
	std::list< void(*)(int, int) > m_beforeneworopen_callbacks;
	std::list< void(*)() > m_beforeframedelete_callbacks;
	std::list< void(*)(std::list<Tool*>&) > m_markedlisttools_callbacks;
	int m_transform_gl_list;
	Matrix m_drag_matrix;
	bool m_extrude_removes_sketches;
	bool m_loft_removes_sketches;
	bool m_font_created;
	glfont::GLFont m_gl_font;
	unsigned int m_font_tex_number[2];
	GraphicsTextMode m_graphics_text_mode;
	bool m_print_scaled_to_page;
	FileOpenOrImportType m_file_open_or_import_type;
	bool m_inPaste;
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

	typedef void(*FileOpenHandler_t)(const wchar_t *path);
	typedef std::map<std::wstring, FileOpenHandler_t> FileOpenHandlers_t;

	FileOpenHandlers_t  m_fileopen_handlers;

	bool RegisterFileOpenHandler(const std::list<std::wstring> file_extensions, FileOpenHandler_t);
	bool UnregisterFileOpenHandler(void(*fileopen_handler)(const wchar_t *path));

	typedef void(*UnitsChangedHandler_t)(const double value);
	typedef std::list<UnitsChangedHandler_t> UnitsChangedHandlers_t;

	UnitsChangedHandlers_t m_units_changed_handlers;

	void RegisterUnitsChangeHandler(UnitsChangedHandler_t);
	void UnregisterUnitsChangeHandler(UnitsChangedHandler_t);

	bool m_settings_restored;

	// HeeksObj's virtual functions
	void GetBox(CBox &box);
	void glCommands(bool select, bool marked, bool no_color);
	bool CanAdd(HeeksObj* object){ return true; }
	int GetType()const{ return DocumentType; }

	void OnExit();
	void CreateLights(void);
	void DestroyLights(void);
	void FindMarkedObject(const IPoint &point, MarkedObject* marked_object);
	void SetInputMode(CInputMode *i);
	void Repaint(bool soon = false);
	void RecalculateGLLists();
	void SetLikeNewFile(void);
	bool IsModified(void);
	void SetAsModified();
	void ClearHistory(void);
	void glCommandsAll(const CViewPoint &view_point);
	double GetPixelScale(void);
	void GetDropDownTools(std::list<Tool*> &f_list, const IPoint &point, MarkedObject* marked_object, bool dont_use_point_for_functions, bool control_pressed);
	void GenerateIntersectionMenuOptions(std::list<Tool*> &f_list);
	void DoUndoable(Undoable *);
	bool RollBack(void);
	bool RollForward(void);
	bool CanUndo(void);
	bool CanRedo(void);
	void StartHistory();
	void EndHistory(void);
	void ClearRollingForward(void);
	bool Add(HeeksObj* object, HeeksObj* prev_object);
	void Remove(HeeksObj* object);
	void Remove(std::list<HeeksObj*> objects);
	void Transform(std::list<HeeksObj*> objects, const Matrix& m);
	void Reset();
	HeeksObj* ReadXMLElement(TiXmlElement* pElem);
	void ObjectWriteBaseXML(HeeksObj *object, TiXmlElement *element);
	void ObjectReadBaseXML(HeeksObj *object, TiXmlElement* element);
	void InitializeXMLFunctions();
	void OpenXMLFile(const wchar_t *filepath, HeeksObj* paste_into = NULL, HeeksObj* paste_before = NULL, bool undoably = false, bool show_error = true);
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
	void AddUndoably(HeeksObj *object, HeeksObj* owner, HeeksObj* prev_object = NULL);
	void AddUndoably(const std::list<HeeksObj*>& list, HeeksObj* owner);
	void DeleteUndoably(HeeksObj* object);
	void DeleteUndoably(const std::list<HeeksObj*>& list);
	void CopyUndoably(HeeksObj* object, HeeksObj* copy_with_new_data);
	void TransformUndoably(HeeksObj *object, const Matrix &m);
	void TransformUndoably(const std::list<HeeksObj*>& list, const Matrix &m);
	void ReverseUndoably(HeeksObj *object);
	void EditUndoably(HeeksObj *object);
	void WasModified(HeeksObj *object);
	void WasAdded(HeeksObj *object);
	void WasRemoved(HeeksObj *object);
	void WereModified(const std::list<HeeksObj*>& list);
	void WereAdded(const std::list<HeeksObj*>& list);
	void WereRemoved(const std::list<HeeksObj*>& list);
	Matrix GetDrawMatrix(bool get_the_appropriate_orthogonal);
	void GetOptions(std::list<Property *> *list);
	void DeleteMarkedItems();
	void glColorEnsuringContrast(const HeeksColor &c);
	void RegisterObserver(Observer* observer);
	void RemoveObserver(Observer* observer);
	void ObserversOnChange(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified);
	void ObserversMarkedListChanged(bool selection_cleared, const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed);
	void ObserversFreeze();
	void ObserversThaw();
	std::wstring GetExeFolder()const;
	std::wstring GetResFolder()const;
	void get_2d_arc_segments(double xs, double ys, double xe, double ye, double xc, double yc, bool dir, bool want_start, double pixels_per_mm, void(*callbackfunc)(const double* xy));
	int PickObjects(const wchar_t* str, long marking_filter = -1, bool just_one = false);
	void StartPickObjects(const wchar_t* str, long marking_filter = -1, bool just_one = false);
	int EndPickObjects();
	bool PickPosition(const wchar_t* str, double* pos, void(*callback)(const double*) = NULL);
	void glSphere(double radius, const double* pos = NULL);
	void OnNewOrOpen(bool open, int res);
	void OnBeforeNewOrOpen(bool open, int res);
	void OnBeforeFrameDelete(void);
	void RegisterReadXMLfunction(const char* type_name, HeeksObj*(*read_xml_function)(TiXmlElement* pElem));
	HeeksObj* GetIDObject(int type, int id);
	std::list<HeeksObj*> GetIDObjects(int type, int id);
	void SetObjectID(HeeksObj* object, int id);
	int GetNextID(int type);
	void RemoveID(HeeksObj* object); // only call this from ObjList::Remove()
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
	void RegisterOnSaveFn(void(*callbackfunc)(bool));
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
	void render_text(const wchar_t* str, bool select);
	bool get_text_size(const wchar_t* str, float* width, float* height);
	void render_screen_text(const wchar_t* str1, const wchar_t* str2, bool select);
	void render_screen_text_at(const wchar_t* str1, double scale, double x, double y, double theta, bool select);
	void OnInputModeTitleChanged();
	void OnInputModeHelpTextChanged();
	void PlotSetColor(const HeeksColor &c);
	void PlotLine(const double* s, const double* e);
	void PlotArc(const double* s, const double* e, const double* c);
	void InitialiseLocale();
	void create_font();
	void GetPluginsFromCommandLineParams(std::list<std::wstring> &plugins);
	void RegisterOnBuildTexture(void(*callbackfunc)());
	void RegisterOnBeforeNewOrOpen(void(*callbackfunc)(int, int));
	void RegisterOnBeforeFrameDelete(void(*callbackfunc)());
	void MessageBox(const wchar_t* message);

	typedef int ObjectType_t;
	typedef int ObjectId_t;
	typedef std::pair< ObjectType_t, ObjectId_t > ObjectReference_t;
	typedef std::map< ObjectReference_t, HeeksObj * > ObjectReferences_t;

	std::wstring HeeksType(const int type) const;
	unsigned int GetIndex(HeeksObj *object);
	void ReleaseIndex(unsigned int index);

	void GetExternalMarkedListTools(std::list<Tool*>& t_list);
	void RegisterMarkeListTools(void(*callbackfunc)(std::list<Tool*>& t_list));
	void RegisterOnRestoreDefaults(void(*callbackfunc)());
	void RestoreDefaults();

	void LinkXMLEndChild(TiXmlNode* root, TiXmlElement* pElem);
	TiXmlElement* FirstNamedXMLChildElement(TiXmlElement* pElem, const char* name);
	void RemoveXMLChild(TiXmlNode* pElem, TiXmlElement* child);
	bool Digitize(const IPoint &point, double* pos);
	bool GetLastDigitizePosition(double *pos);
	void ObjectAreaString(HeeksObj* object, std::wstring &s);
	void SetViewUnits(double units, bool write_to_config);
	void Mark(HeeksObj* object);
	void Unmark(HeeksObj* object);
};

extern CApp theApp;