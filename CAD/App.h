// not really an app, but a class with one global object to put global variables in and also is the list of objects to render
#pragma once

#include "HeeksColor.h"

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
class CSelectMode;

class CApp// : public ObjList
	/*! \class The Application Class:
	*
	* not really an app, but a class with one global object to put global variables in and also is the list of objects to render */
{
private:
#if 0 
	std::set<Observer*> observers;
	MainHistory *history;

	typedef std::map< int, std::list<HeeksObj*> > IdsToObjects_t;
	typedef int GroupId_t;
	typedef std::map< GroupId_t, IdsToObjects_t > UsedIds_t;

	UsedIds_t	used_ids;

	// std::map< int, std::map<int, HeeksObj*> > used_ids; // map of group type ( usually same as object type ) to "map of ID to object"
	std::map< int, int > next_id_map;
	std::map< std::string, HeeksObj*(*)(TiXmlElement* pElem) > xml_read_fn_map;

	void render_screen_text2(const wxChar* str, bool select);
	float get_text_scale();
	void RenderDatumOrCurrentCoordSys();
#endif

public:
	CApp();
	~CApp();

	IPoint cur_mouse_pos;
	HeeksColor current_color;
#define NUM_BACKGROUND_COLORS 10
	HeeksColor background_color[NUM_BACKGROUND_COLORS];
	BackgroundMode m_background_mode;
#if 0
	HeeksColor face_selection_color;
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
	double m_iges_sewing_tolerance;
	bool m_svg_unite;

	//gp_Trsf digitizing_matrix;
	CoordinateSystem *m_current_coordinate_system;
#endif
	CInputMode *input_mode_object;
#if 0
	MagDragWindow *magnification;
	ViewRotating *viewrotating;
	ViewZooming *viewzooming;
	ViewPanning *viewpanning;
#endif
	CSelectMode *m_select_mode;
#if 0
	DigitizeMode *m_digitizing;
	GripperMode* gripper_mode;
#endif
	int grid_mode;
#if 0
	Gripper *drag_gripper;
	geoff_geometry::Point3d grip_from, grip_to;
	Gripper *cursor_gripper;
	CHeeksFrame *m_frame;
#endif
	CViewport *m_current_viewport;
#if 0
	MarkedList *m_marked_list;
	bool m_doing_rollback;
	wxString m_filepath;
	bool m_untitled;
	bool m_light_push_matrix;
	std::list<HeeksObj*> m_hidden_for_drag;
	bool m_show_grippers_on_drag;
	double m_geom_tol;
	double m_sketch_reorder_tol;
	std::list<Plugin> m_loaded_libraries;
	std::list< void(*)() > m_on_glCommands_list;
	std::list< wxToolBarBase* > m_external_toolbars;
#ifdef USING_RIBBON
	std::list< void(*)(wxRibbonBar*, wxRibbonPage*) > m_AddRibbonPanels_list;
#else
	std::list< void(*)() > m_AddToolBars_list;
#endif
	std::list<wxWindow*> m_hideable_windows;
	HRuler* m_ruler;
	bool m_show_ruler;
	bool m_show_datum_coords_system;
	bool m_datum_coords_system_solid_arrows;
	std::list< wxString > m_recent_files;
	bool m_in_OpenFile;
	bool m_mark_newly_added_objects;
	wxString m_version_number;
	std::list< void(*)(IPointEvent&) > m_on_graphics_size_list;
	std::list< void(*)(wxMouseEvent&) > m_lbutton_up_callbacks;
	std::list< void(*)(bool) > m_on_save_callbacks;
	std::list< bool(*)() > m_is_modified_callbacks;
	std::list< void(*)() > m_on_build_texture_callbacks;
	std::list< void(*)(int, int) > m_beforeneworopen_callbacks;
	std::list< void(*)() > m_beforeframedelete_callbacks;
	std::list< void(*)(std::list<Tool*>&) > m_markedlisttools_callbacks;
	std::list< void(*)() > m_on_restore_defaults_callbacks;
	int m_transform_gl_list;
	gp_Trsf m_drag_matrix;
	bool m_extrude_removes_sketches;
	bool m_loft_removes_sketches;
	bool m_font_created;
	glfont::GLFont m_gl_font;
	unsigned int m_font_tex_number[2];
	GraphicsTextMode m_graphics_text_mode;
	bool m_print_scaled_to_page;
	wxPrintData *m_printData;
	wxPageSetupDialogData* m_pageSetupData;
	FileOpenOrImportType m_file_open_or_import_type;
	bool m_inPaste;
	double* m_file_open_matrix;
#endif
	double m_view_units; // units to display to the user ( but everything is stored as mm ), 1.0 for mm, 25.4 for inches
#if 0
	bool m_input_uses_modal_dialog;
	bool m_dragging_moves_objects;
	bool m_no_creation_mode; // set from a plugin, for making an exporter only application

	double m_min_correlation_factor;
	double m_max_scale_threshold;
	int m_number_of_sample_points;
	bool m_correlate_by_color;
	bool m_property_grid_validation;

#ifndef WIN32
	std::auto_ptr<VectorFonts>	m_pVectorFonts;	// QCAD format fonts that have been loaded.
	VectorFont   *m_pVectorFont;	// which font are we using? (NULL indicates the internal (OpenGL) font)
	wxString m_font_paths;	// SemiColon delimited list of directories that hold font files to load.
	double m_word_space_percentage;	// Font
	double m_character_space_percentage; // Font
#endif
	double m_stl_facet_tolerance;

	int m_auto_save_interval;	// In minutes
	std::auto_ptr<CAutoSave> m_pAutoSave;

	int m_icon_texture_number;
	bool m_extrude_to_solid;
	double m_revolve_angle;
	bool m_fit_arcs_on_solid_outline;

	typedef void(*FileOpenHandler_t)(const wxChar *path);
	typedef std::map<wxString, FileOpenHandler_t> FileOpenHandlers_t;

	FileOpenHandlers_t  m_fileopen_handlers;

	bool RegisterFileOpenHandler(const std::list<wxString> file_extensions, FileOpenHandler_t);
	bool UnregisterFileOpenHandler(void(*fileopen_handler)(const wxChar *path));

	typedef void(*UnitsChangedHandler_t)(const double value);
	typedef std::list<UnitsChangedHandler_t> UnitsChangedHandlers_t;

	UnitsChangedHandlers_t m_units_changed_handlers;

	void RegisterUnitsChangeHandler(UnitsChangedHandler_t);
	void UnregisterUnitsChangeHandler(UnitsChangedHandler_t);

	typedef wxString(*HeeksTypesConverter_t)(const int type);
	typedef std::list<HeeksTypesConverter_t> HeeksTypesConverters_t;

	HeeksTypesConverters_t m_heeks_types_converters;

	void RegisterHeeksTypesConverter(HeeksTypesConverter_t);
	void UnregisterHeeksTypesConverter(HeeksTypesConverter_t);

	wxString m_alternative_open_wild_card_string;

	bool m_settings_restored;
#endif

	// HeeksObj's virtual functions
	void GetBox(CBox &box);
	void glCommands(bool select, bool marked, bool no_color);
	//bool CanAdd(HeeksObj* object){ return true; }
	//int GetType()const{ return DocumentType; }

	bool OnInit();
#if 0
	int OnExit();
	void WriteConfig();
	void CreateLights(void);
	void DestroyLights(void);
	void FindMarkedObject(const IPoint &point, MarkedObject* marked_object);
#endif
	void SetInputMode(CInputMode *i);
#if 0
	void Repaint(bool soon = false);
	void RecalculateGLLists();
	void SetLikeNewFile(void);
	bool IsModified(void);
	void SetAsModified();
	void ClearHistory(void);
#endif
	void glCommandsAll(const CViewPoint &view_point);
	double GetPixelScale(void);
#if 0
	void DoMoveOrCopyDropDownMenu(wxWindow *wnd, const IPoint &point, MarkedObject* marked_object, HeeksObj* paste_into, HeeksObj* paste_before);
	void GetDropDownTools(std::list<Tool*> &f_list, const IPoint &point, MarkedObject* marked_object, bool dont_use_point_for_functions, bool control_pressed);
	void DoDropDownMenu(wxWindow *wnd, const IPoint &point, MarkedObject* marked_object, bool dont_use_point_for_functions, bool control_pressed);
	void GenerateIntersectionMenuOptions(std::list<Tool*> &f_list);
	void on_menu_event(wxCommandEvent& event);
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
	void Transform(std::list<HeeksObj*> objects, double *m);
	void Reset();
	HeeksObj* ReadXMLElement(TiXmlElement* pElem);
	void ObjectWriteBaseXML(HeeksObj *object, TiXmlElement *element);
	void ObjectReadBaseXML(HeeksObj *object, TiXmlElement* element);
	void InitializeXMLFunctions();
	void OpenXMLFile(const wxChar *filepath, HeeksObj* paste_into = NULL, HeeksObj* paste_before = NULL, bool undoably = false, bool show_error = true);
	static void OpenSVGFile(const wxChar *filepath);
	static void OpenSTLFile(const wxChar *filepath);
	static void OpenDXFFile(const wxChar *filepath);
	static void OpenRS274XFile(const wxChar *filepath);
	bool OpenImageFile(const wxChar *filepath);
	void OnNewButton();
	void OnOpenButton();
	bool OpenFile(const wxChar *filepath, bool import_not_open = false, HeeksObj* paste_into = NULL, HeeksObj* paste_before = NULL, bool retain_filename = true);
	void SaveDXFFile(const std::list<HeeksObj*>& objects, const wxChar *filepath);
	void SaveSTLFileBinary(const std::list<HeeksObj*>& objects, const wxChar *filepath, double facet_tolerance = -1.0, double* scale = NULL);
	void SaveSTLFileAscii(const std::list<HeeksObj*>& objects, const wxChar *filepath, double facet_tolerance = -1.0, double* scale = NULL);
	void SaveOBJFileAscii(const std::list<HeeksObj*>& objects, const wxChar *filepath, double facet_tolerance = -1.0, double* scale = NULL);
	void SaveSTLFile(const std::list<HeeksObj*>& objects, const wxChar *filepath, double facet_tolerance = -1.0, double* scale = NULL, bool binary = true);
	void SaveCPPFile(const std::list<HeeksObj*>& objects, const wxChar *filepath, double facet_tolerance = -1.0);
	void SavePyFile(const std::list<HeeksObj*>& objects, const wxChar *filepath, double facet_tolerance = -1.0);
	void SaveXMLFile(const std::list<HeeksObj*>& objects, const wxChar *filepath, bool for_clipboard = false);
	void SaveXMLFile(const wxChar *filepath){ SaveXMLFile(m_objects, filepath); }
	bool SaveFile(const wxChar *filepath, bool use_dialog = false, bool update_recent_file_list = true, bool set_app_caption = true);
	void AddUndoably(HeeksObj *object, HeeksObj* owner, HeeksObj* prev_object = NULL);
	void AddUndoably(const std::list<HeeksObj*>& list, HeeksObj* owner);
	void DeleteUndoably(HeeksObj* object);
	void DeleteUndoably(const std::list<HeeksObj*>& list);
	void CopyUndoably(HeeksObj* object, HeeksObj* copy_with_new_data);
	void TransformUndoably(HeeksObj *object, double *m);
	void TransformUndoably(const std::list<HeeksObj*>& list, double* m);
	void ReverseUndoably(HeeksObj *object);
	void EditUndoably(HeeksObj *object);
	void WasModified(HeeksObj *object);
	void WasAdded(HeeksObj *object);
	void WasRemoved(HeeksObj *object);
	void WereModified(const std::list<HeeksObj*>& list);
	void WereAdded(const std::list<HeeksObj*>& list);
	void WereRemoved(const std::list<HeeksObj*>& list);
#endif
	geoff_geometry::Matrix GetDrawMatrix(bool get_the_appropriate_orthogonal);
#if 0
	void GetOptions(std::list<Property *> *list);
	void DeleteMarkedItems();
	void glColorEnsuringContrast(const HeeksColor &c);
	void RegisterObserver(Observer* observer);
	void RemoveObserver(Observer* observer);
	void ObserversOnChange(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified);
	void ObserversMarkedListChanged(bool selection_cleared, const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed);
	void ObserversFreeze();
	void ObserversThaw();
	const wxChar* GetKnownFilesWildCardString(bool open, bool import_export)const;
	const wxChar* GetKnownFilesCommaSeparatedList(bool open, bool import_export)const;
	void GetTools(MarkedObject* marked_object, std::list<Tool*>& t_list, const IPoint& point, bool control_pressed);
	void GetTools2(MarkedObject* marked_object, std::list<Tool*>& t_list, const IPoint& point, bool control_pressed, bool make_tool_list_container);
	wxString GetExeFolder()const;
	wxString GetResFolder()const;
	void get_2d_arc_segments(double xs, double ys, double xe, double ye, double xc, double yc, bool dir, bool want_start, double pixels_per_mm, void(*callbackfunc)(const double* xy));
	int PickObjects(const wxChar* str, long marking_filter = -1, bool just_one = false);
	void StartPickObjects(const wxChar* str, long marking_filter = -1, bool just_one = false);
	int EndPickObjects();
	bool PickPosition(const wxChar* str, double* pos, void(*callback)(const double*) = NULL);
	void glSphere(double radius, const double* pos = NULL);
	void OnNewOrOpen(bool open, int res);
	void OnBeforeNewOrOpen(bool open, int res);
	void OnBeforeFrameDelete(void);
	void RegisterHideableWindow(wxWindow* w);
	void RemoveHideableWindow(wxWindow* w);
	void RegisterReadXMLfunction(const char* type_name, HeeksObj*(*read_xml_function)(TiXmlElement* pElem));
	void GetRecentFilesProfileString();
	void WriteRecentFilesProfileString(wxConfigBase &config);
	void InsertRecentFileItem(const wxChar* filepath);
	int CheckForModifiedDoc(); // returns wxCANCEL, if NOT OK to continue with file open etc.
	void SetFrameTitle();
	HeeksObj* GetIDObject(int type, int id);
	std::list<HeeksObj*> GetIDObjects(int type, int id);
	void SetObjectID(HeeksObj* object, int id);
	int GetNextID(int type);
	void RemoveID(HeeksObj* object); // only call this from ObjList::Remove()
	void ResetIDs();
	bool InputInt(const wxChar* prompt, const wxChar* value_name, int &value);
	bool InputDouble(const wxChar* prompt, const wxChar* value_name, double &value);
	bool InputAngleWithPlane(double &angle, double *axis = NULL, double *pos = NULL, int *number_of_copies = NULL, double *axial_shift = NULL);
	bool InputFromAndTo(double *from, double *to, int *number_of_copies = NULL);
	bool InputLength(const wxChar* prompt, const wxChar* value_name, double &value);
	void ShowModalOptions();
	void SectioningDialog();
	void RegisterOnGLCommands(void(*callbackfunc)());
	void RemoveOnGLCommands(void(*callbackfunc)());
	void RegisterOnGraphicsSize(void(*callbackfunc)(IPointEvent&));
	void RemoveOnGraphicsSize(void(*callbackfunc)(IPointEvent&));
	void RegisterOnMouseFn(void(*callbackfunc)(wxMouseEvent&));
	void RemoveOnMouseFn(void(*callbackfunc)(wxMouseEvent&));
	void RegisterOnSaveFn(void(*callbackfunc)(bool));
	void RegisterIsModifiedFn(bool(*callbackfunc)());
	void CreateTransformGLList(const std::list<HeeksObj*>& list, bool show_grippers_on_drag);
	void DestroyTransformGLList();
	bool IsPasteReady();
#endif
	void EnableBlend();
	void DisableBlend();
#if 0
	void Paste(HeeksObj* paste_into, HeeksObj* paste_before);
	bool CheckForNOrMore(const std::list<HeeksObj*> &list, int min_num, int type, const wxString& msg, const wxString& caption);
	bool CheckForNOrMore(const std::list<HeeksObj*> &list, int min_num, int type1, int type2, const wxString& msg, const wxString& caption);
	bool CheckForNOrMore(const std::list<HeeksObj*> &list, int min_num, int type1, int type2, int type3, const wxString& msg, const wxString& caption);
	void render_text(const wxChar* str, bool select);
	bool get_text_size(const wxChar* str, float* width, float* height);
	void render_screen_text(const wxChar* str1, const wxChar* str2, bool select);
	void render_screen_text_at(const wxChar* str1, double scale, double x, double y, double theta, bool select);
	void OnInputModeTitleChanged();
	void OnInputModeHelpTextChanged();
	void PlotSetColor(const HeeksColor &c);
	void PlotLine(const double* s, const double* e);
	void PlotArc(const double* s, const double* e, const double* c);
	void InitialiseLocale();
	void create_font();
#ifndef WIN32
	std::auto_ptr<VectorFonts>	& GetAvailableFonts(const bool force_read = false);
#endif
	void GetPluginsFromCommandLineParams(std::list<wxString> &plugins);
	void RegisterOnBuildTexture(void(*callbackfunc)());
	void RegisterOnBeforeNewOrOpen(void(*callbackfunc)(int, int));
	void RegisterOnBeforeFrameDelete(void(*callbackfunc)());

	typedef int ObjectType_t;
	typedef int ObjectId_t;
	typedef std::pair< ObjectType_t, ObjectId_t > ObjectReference_t;
	typedef std::map< ObjectReference_t, HeeksObj * > ObjectReferences_t;

	wxString HeeksType(const int type) const;
	unsigned int GetIndex(HeeksObj *object);
	void ReleaseIndex(unsigned int index);

	void GetExternalMarkedListTools(std::list<Tool*>& t_list);
	void RegisterMarkeListTools(void(*callbackfunc)(std::list<Tool*>& t_list));
	void RegisterOnRestoreDefaults(void(*callbackfunc)());
	void RestoreDefaults();
#ifdef USING_RIBBON
	void AddRibbonPanels(wxRibbonBar* ribbon, wxRibbonPage* main_page);
#endif

	// this code was in HeeksCNCApp
	CMachine m_machine;
	CNCCode* m_nc_code;
	COutputCanvas* m_output_canvas;
	CPrintCanvas* m_print_canvas;
#ifdef HAVE_TOOLBARS
	wxToolBarBase* m_machiningBar;
#endif
	wxMenu *m_menuMachining;
	std::list< void(*)() > m_OnRewritePython_list;
	std::set<int> m_external_op_types;

	void OnCNCStartUp();
	void OnCNCNewOrOpen(bool open, int res);
	void OnFrameDelete();
	wxString GetDllFolder() const;
	wxString GetResourceFilename(const wxString resource, const bool writableOnly = false) const;
	void BackplotGCode(const wxString& output_file);

#ifdef HAVE_TOOLBARS
	void StartToolBarFlyout(const wxString& title_and_bitmap);
	void AddFlyoutButton(const wxString& title, const wxBitmap& bitmap, const wxString& tooltip, void(*onButtonFunction)(wxCommandEvent&));
	void EndToolBarFlyout(wxToolBar* toolbar);
#endif
	void LinkXMLEndChild(TiXmlNode* root, TiXmlElement* pElem);
	TiXmlElement* FirstNamedXMLChildElement(TiXmlElement* pElem, const char* name);
	void RemoveXMLChild(TiXmlNode* pElem, TiXmlElement* child);
	bool Digitize(const IPoint &point, double* pos);
	bool GetLastDigitizePosition(double *pos);
	void ObjectAreaString(HeeksObj* object, wxString &s);
	const wxChar* GetFileFullPath();
	void SetViewUnits(double units, bool write_to_config);
	void Mark(HeeksObj* object);
	void Unmark(HeeksObj* object);

	// this code was in HeeksPython
	void OnPythonStartUp();
	void PythonOnNewOrOpen(bool open, int res);
#endif
};

extern CApp theApp;