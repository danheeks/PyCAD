#include "stdafx.h"

#include "App.h"
#include "Grid.h"
#include "InputMode.h"
#include "SelectMode.h"
#include "Material.h"
#include "Viewport.h"
#include "MagDragWindow.h"
#include "ViewRotating.h"
#include "ViewZooming.h"
#include "ViewPanning.h"
#include "DigitizeMode.h"
#include "MarkedObject.h"
#include "MarkedList.h"
#include "StlSolid.h"
#include "CoordinateSystem.h"
#include "History.h"
#include "RemoveOrAddTool.h"
#include "Observer.h"
#include "Ruler.h"
#include "RegularShapesDrawing.h"
#include "HDxf.h"
#include "HXml.h"
#include "strconv.h"
#include "Group.h"
#include "RemoveOrAddTool.h"
#include "TransformTool.h"
#include "Picking.h"
#include "svg.h"

CApp theApp;


static unsigned int DecimalPlaces(const double value)
{
	double decimal_places = 1 / value;
	unsigned int required = 0;
	while (decimal_places >= 1.0)
	{
		required++;
		decimal_places /= 10.0;
	}

	return(required + 1);   // Always use 1 more decimal point than our accuracy requires.
}



CApp::CApp()
{
	m_version_number = std::wstring(HEEKSCAD_VERSION_MAIN) + L" " + HEEKSCAD_VERSION_SUB + L" 0";
	TiXmlBase::SetRequiredDecimalPlaces(DecimalPlaces(geoff_geometry::TOLERANCE));	 // Ensure we write XML in enough accuracy to be useful when re-read.

	m_sketch_reorder_tol = 0.01;
	m_view_units = 1.0;
	for (int i = 0; i<NUM_BACKGROUND_COLORS; i++)background_color[i] = HeeksColor(0, 0, 0);
	background_color[0] = HeeksColor(255, 255, 255);
	background_color[1] = HeeksColor(181, 230, 29);
	m_background_mode = BackgroundModeOneColor;
	current_color = HeeksColor(0, 0, 0);
	input_mode_object = NULL;
	cur_mouse_pos = IPoint(0, 0);
	drag_gripper = NULL;
	cursor_gripper = NULL;
	magnification = new MagDragWindow();
	viewrotating = new ViewRotating;
	viewzooming = new ViewZooming;
	viewpanning = new ViewPanning;
	m_select_mode = new CSelectMode();
	m_digitizing = new DigitizeMode();
	digitize_end = false;
	digitize_inters = false;
	digitize_centre = false;
	digitize_midpoint = false;
	digitize_nearest = false;
	digitize_tangent = false;
	digitize_coords = true;
	digitize_screen = false;
	digitizing_radius = 5.0;
	draw_to_grid = true;
	digitizing_grid = 1.0;
	grid_mode = 3;
	m_rotate_mode = 1;
	m_antialiasing = false;
	m_light_push_matrix = true;
	m_marked_list = new MarkedList;
	history = new MainHistory;
	m_doing_rollback = false;
	mouse_wheel_forward_away = true;
	m_mouse_move_highlighting = true;
	ctrl_does_rotate = false;
	m_ruler = new HRuler();
	m_show_ruler = false;
	m_show_datum_coords_system = true;
	m_datum_coords_system_solid_arrows = true;
	m_filepath = std::wstring(L"Untitled") + L".heeks";
	m_untitled = true;
	m_in_OpenFile = false;
	m_transform_gl_list = 0;
	m_current_coordinate_system = NULL;
	m_mark_newly_added_objects = false;
	m_show_grippers_on_drag = true;
	m_extrude_removes_sketches = false;
	m_loft_removes_sketches = false;
	m_font_tex_number[0] = 0;
	m_font_tex_number[1] = 0;
	m_graphics_text_mode = GraphicsTextModeNone;
	m_file_open_or_import_type = FileOpenOrImportTypeOther;
	m_inPaste = false;
	m_file_open_matrix = NULL;
	m_min_correlation_factor = 0.75;
	m_max_scale_threshold = 1.5;
	m_number_of_sample_points = 10;
	m_property_grid_validation = false;
	m_solid_view_mode = SolidViewFacesAndEdges;
	m_dragging_moves_objects = false;
	m_no_creation_mode = false;
	m_stl_solid_random_colors = false;
	m_svg_unite = false;
	m_stl_facet_tolerance = 0.1;
	m_icon_texture_number = 0;
	m_extrude_to_solid = true;
	m_revolve_angle = 360.0;
	m_fit_arcs_on_solid_outline = false;
	m_stl_save_as_binary = true;
	m_mouse_move_highlighting = true;
	m_highlight_color = HeeksColor(128, 255, 0);

	{
		std::list<std::wstring> extensions;
		extensions.push_back(L"svg");
		RegisterFileOpenHandler(extensions, OpenSVGFile);
	}
	{
		std::list<std::wstring> extensions;
		extensions.push_back(L"stl");
		RegisterFileOpenHandler(extensions, OpenSTLFile);
	}

	RegisterHeeksTypesConverter(HeeksCADType);

	m_settings_restored = false;

	m_icon_texture_number = 0;
	m_settings_restored = false;



	m_current_viewport = NULL;
}

CApp::~CApp()
{
	delete m_marked_list;
	m_marked_list = NULL;
	observers.clear();
	delete history;
	delete magnification;
	delete m_select_mode;
	delete m_digitizing;
	delete viewrotating;
	delete viewzooming;
	delete viewpanning;
	m_ruler->m_index = 0;
	delete m_ruler;
}

bool CApp::OnInit()
{
	m_gl_font_initialized = false;

	int width = 996;
	int height = 691;
	int posx = 200;
	int posy = 200;
	HeeksConfig config;
#if 0
	config.Read(_T("MainFrameWidth"), &width);
	config.Read(_T("MainFrameHeight"), &height);
	config.Read(_T("MainFramePosX"), &posx);
	config.Read(_T("MainFramePosY"), &posy);
	if (posx < 0)posx = 0;
	if (posy < 0)posy = 0;
	config.Read(_T("DrawEnd"), &digitize_end, false);
	config.Read(_T("DrawInters"), &digitize_inters, false);
	config.Read(_T("DrawCentre"), &digitize_centre, false);
	config.Read(_T("DrawMidpoint"), &digitize_midpoint, false);
	config.Read(_T("DrawNearest"), &digitize_nearest, false);
	config.Read(_T("DrawTangent"), &digitize_tangent, false);
	config.Read(_T("DrawCoords"), &digitize_coords, true);
	config.Read(_T("DrawScreen"), &digitize_screen, false);
	config.Read(_T("DrawToGrid"), &draw_to_grid, true);
	config.Read(_T("UseOldFuse"), &useOldFuse, false);
	config.Read(_T("DrawGrid"), &digitizing_grid);
	config.Read(_T("DrawRadius"), &digitizing_radius);
	{
		long default_color[NUM_BACKGROUND_COLORS] = {
			HeeksColor(187, 233, 255).COLORREF_color(),
			HeeksColor(255, 255, 255).COLORREF_color(),
			HeeksColor(247, 198, 243).COLORREF_color(),
			HeeksColor(193, 235, 236).COLORREF_color(),
			HeeksColor(255, 255, 255).COLORREF_color(),
			HeeksColor(255, 255, 255).COLORREF_color(),
			HeeksColor(255, 255, 255).COLORREF_color(),
			HeeksColor(255, 255, 255).COLORREF_color(),
			HeeksColor(255, 255, 255).COLORREF_color(),
			HeeksColor(255, 255, 255).COLORREF_color()
		};

		for (int i = 0; i<NUM_BACKGROUND_COLORS; i++)
		{
			wchar_t key[20] = _T("");
			wsprintf(key, _T("BackgroundColor%d"), i);
			int color = default_color[i];
			config.Read(key, &color);
			background_color[i] = HeeksColor((long)color);
		}
		int mode = (int)BackgroundModeTwoColors;
		config.Read(_T("BackgroundMode"), &mode);
		m_background_mode = (BackgroundMode)mode;
	}

	{
		wchar_t str[32];
		config.Read(_T("CurrentColor"), str, _T("0 0 0"));
		int r = 0, g = 0, b = 0;
		swscanf(str, _T("%d %d %d"), &r, &g, &b);
		current_color = HeeksColor((unsigned char)r, (unsigned char)g, (unsigned char)b);
	}
	config.Read(_T("RotateMode"), &m_rotate_mode);
	config.Read(_T("Antialiasing"), &m_antialiasing);
	config.Read(_T("GridMode"), &grid_mode);
	config.Read(_T("m_light_push_matrix"), &m_light_push_matrix);
	config.Read(_T("WheelForwardAway"), &mouse_wheel_forward_away);
	config.Read(_T("ZoomingReversed"), &ViewZooming::m_reversed);
	config.Read(_T("CtrlDoesRotate"), &ctrl_does_rotate);
	config.Read(_T("DrawDatum"), &m_show_datum_coords_system, true);
	config.Read(_T("DrawDatumSolid"), &m_datum_coords_system_solid_arrows, true);
	config.Read(_T("DatumSize"), &CoordinateSystem::size, 30);
	config.Read(_T("DatumSizeIsPixels"), &CoordinateSystem::size_is_pixels, true);
	config.Read(_T("DrawRuler"), &m_show_ruler, false);
	config.Read(_T("RegularShapesMode"), (int*)(&(regular_shapes_drawing.m_mode)));
	config.Read(_T("RegularShapesNSides"), &(regular_shapes_drawing.m_number_of_side_for_polygon));
	config.Read(_T("RegularShapesRectRad"), &(regular_shapes_drawing.m_rect_radius));
	config.Read(_T("RegularShapesObRad"), &(regular_shapes_drawing.m_obround_radius));
	config.Read(_T("ExtrudeRemovesSketches"), &m_extrude_removes_sketches, true);
	config.Read(_T("LoftRemovesSketches"), &m_loft_removes_sketches, true);
	config.Read(_T("GraphicsTextMode"), (int*)(&m_graphics_text_mode), GraphicsTextModeWithHelp);

	config.Read(_T("DxfMakeSketch"), &HeeksDxfRead::m_make_as_sketch, true);
	config.Read(_T("DxfIgnoreErrors"), &HeeksDxfRead::m_ignore_errors, false);

	config.Read(_T("ViewUnits"), &m_view_units);
	config.Read(_T("STLFacetTolerance"), &m_stl_facet_tolerance, 0.1);
	config.Read(_T("FitArcsOnSolidOutline"), &m_fit_arcs_on_solid_outline);
	config.Read(_T("SolidViewMode"), (int*)(&m_solid_view_mode), 0);

	config.Read(_T("DraggingMovesObjects"), &m_dragging_moves_objects, true);
	config.Read(_T("STLSaveBinary"), &m_stl_save_as_binary, true);
	config.Read(_T("MouseMoveHighlighting"), &m_mouse_move_highlighting, true);
	{
		int color = HeeksColor(128, 255, 0).COLORREF_color();
		config.Read(_T("HighlightColor"), &color);
		m_highlight_color = HeeksColor((long)color);
	}
	config.Read(_T("SketchReorderTolerance"), &m_sketch_reorder_tol, 0.01);
	config.Read(_T("StlSolidRandomColors"), &m_stl_solid_random_colors, false);
	config.Read(_T("SvgUnite"), &m_svg_unite, true);

	HDimension::ReadFromConfig(config);
#endif

	m_ruler->ReadFromConfig(config);

	m_current_viewport = NULL;

	// NOTE: A side-effect of calling the SetInputMode() method is
	// that the GetOptions() method is called.  To that end, all
	// configuration settings should be read BEFORE this point.
	SetInputMode(m_select_mode);

		{
			OnNewOrOpen(false, 0x08);
			ClearHistory();
			SetLikeNewFile();
//			SetFrameTitle();
		}

	return true;
}


void CApp::WriteConfig()
{
	HeeksConfig config;
#if 0
	config.Write(_T("DrawEnd"), digitize_end);
	config.Write(_T("DrawInters"), digitize_inters);
	config.Write(_T("DrawCentre"), digitize_centre);
	config.Write(_T("DrawMidpoint"), digitize_midpoint);
	config.Write(_T("DrawNearest"), digitize_nearest);
	config.Write(_T("DrawTangent"), digitize_tangent);
	config.Write(_T("DrawCoords"), digitize_coords);
	config.Write(_T("DrawScreen"), digitize_screen);
	config.Write(_T("DrawToGrid"), draw_to_grid);
	config.Write(_T("UseOldFuse"), useOldFuse);
	config.Write(_T("DrawGrid"), digitizing_grid);
	config.Write(_T("DrawRadius"), digitizing_radius);
	for (int i = 0; i<NUM_BACKGROUND_COLORS; i++)
	{
		wxString key = wxString::Format(_T("BackgroundColor%d"), i);
		config.Write(key, background_color[i].COLORREF_color());
	}
	config.Write(_T("BackgroundMode"), (int)m_background_mode);
	config.Write(_T("FaceSelectionColor"), face_selection_color.COLORREF_color());
	config.Write(_T("CurrentColor"), wxString::Format(_T("%d %d %d"), current_color.red, current_color.green, current_color.blue));
	config.Write(_T("RotateMode"), m_rotate_mode);
	config.Write(_T("Antialiasing"), m_antialiasing);
	config.Write(_T("GridMode"), grid_mode);
	config.Write(_T("m_light_push_matrix"), m_light_push_matrix);
	config.Write(_T("WheelForwardAway"), mouse_wheel_forward_away);
	config.Write(_T("ZoomingReversed"), ViewZooming::m_reversed);
	config.Write(_T("CtrlDoesRotate"), ctrl_does_rotate);
	config.Write(_T("DrawDatum"), m_show_datum_coords_system);
	config.Write(_T("DrawDatumSolid"), m_show_datum_coords_system);
	config.Write(_T("DatumSize"), CoordinateSystem::size);
	config.Write(_T("DatumSizeIsPixels"), CoordinateSystem::size_is_pixels);
	config.Write(_T("DrawRuler"), m_show_ruler);
	config.Write(_T("RegularShapesMode"), (int)(regular_shapes_drawing.m_mode));
	config.Write(_T("RegularShapesNSides"), regular_shapes_drawing.m_number_of_side_for_polygon);
	config.Write(_T("RegularShapesRectRad"), regular_shapes_drawing.m_rect_radius);
	config.Write(_T("RegularShapesObRad"), regular_shapes_drawing.m_obround_radius);
	config.Write(_T("ExtrudeRemovesSketches"), m_extrude_removes_sketches);
	config.Write(_T("LoftRemovesSketches"), m_loft_removes_sketches);
	config.Write(_T("GraphicsTextMode"), (int)m_graphics_text_mode);
	config.Write(_T("DxfMakeSketch"), HeeksDxfRead::m_make_as_sketch);
	config.Write(_T("DxfIgnoreErrors"), HeeksDxfRead::m_ignore_errors);
	config.Write(_T("FaceToSketchDeviation"), FaceToSketchTool::deviation);

	config.Write(_T("MinCorrelationFactor"), m_min_correlation_factor);
	config.Write(_T("MaxScaleThreshold"), m_max_scale_threshold);
	config.Write(_T("NumberOfSamplePoints"), m_number_of_sample_points);
	config.Write(_T("CorrelateByColor"), m_correlate_by_color);

#ifndef WIN32
	config.Write(_T("FontPaths"), m_font_paths);
#endif
	config.Write(_T("STLFacetTolerance"), m_stl_facet_tolerance);
	config.Write(_T("AutoSaveInterval"), m_auto_save_interval);
	config.Write(_T("ExtrudeToSolid"), m_extrude_to_solid);
	config.Write(_T("RevolveAngle"), m_revolve_angle);
	config.Write(_T("FitArcsOnSolidOutline"), m_fit_arcs_on_solid_outline);
	config.Write(_T("SolidViewMode"), (int)m_solid_view_mode);
	config.Write(_T("STLSaveBinary"), m_stl_save_as_binary);

	config.Write(_T("MouseMoveHighlighting"), m_mouse_move_highlighting);
	config.Write(_T("HighlightColor"), m_highlight_color.COLORREF_color());
	config.Write(_T("StlSolidRandomColors"), m_stl_solid_random_colors);

	config.Write(_T("IgesSewingTolerance"), m_iges_sewing_tolerance);
	config.Write(_T("SvgUnite"), m_svg_unite);

	HDimension::WriteToConfig(config);

	m_ruler->WriteToConfig(config);

	WriteRecentFilesProfileString(config);
#endif
}

void CApp::OnExit(){
	WriteConfig();

	delete history;
	history = NULL;
}

void CApp::SetInputMode(CInputMode *new_mode){
	if(!new_mode)return;
	//if(m_frame)m_current_viewport->EndDrawFront();
	if(new_mode->OnModeChange()){
		input_mode_object = new_mode;
	}
	else{
		input_mode_object = m_select_mode;
	}
	//if(m_frame && m_frame->m_input_canvas)m_frame->RefreshInputCanvas();
	//if(m_frame && m_frame->m_options)m_frame->RefreshOptions();
	//if(m_graphics_text_mode != GraphicsTextModeNone)Repaint();
}

void CApp::FindMarkedObject(const IPoint &point, MarkedObject* marked_object){
	m_current_viewport->FindMarkedObject(point, marked_object);
}

void CApp::CreateLights(void)
{
	GLfloat amb[4] =  {0.8f, 0.8f, 0.8f, 1.0f};
	GLfloat dif[4] =  {0.8f, 0.8f, 0.8f, 1.0f};
	GLfloat spec[4] = {0.8f, 0.8f, 0.8f, 1.0f};
	GLfloat pos[4] = {0.5f, 0.5f, 0.5f, 0.0f};
	GLfloat lmodel_amb[] = { 0.2f, 0.2f, 0.2f, 1.0 };
	GLfloat local_viewer[] = { 0.0 };
	if(m_light_push_matrix){
		glPushMatrix();
		glLoadIdentity();
	}
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_amb);
	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_viewer);
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,pos);
	if(m_light_push_matrix){
		glPopMatrix();
	}
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT2);
	glDisable(GL_LIGHT3);
	glDisable(GL_LIGHT4);
	glDisable(GL_LIGHT5);
	glDisable(GL_LIGHT6);
	glDisable(GL_LIGHT7);
}

void CApp::DestroyLights(void)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_AUTO_NORMAL);
	glDisable(GL_NORMALIZE);
}

void CApp::Reset(){
	m_marked_list->Clear(true);
	m_marked_list->Reset();
	std::set<Observer*>::iterator It;
	for(It = observers.begin(); It != observers.end(); It++){
		Observer *ov = *It;
		ov->Clear();
	}
	Clear();
	EndHistory();
	delete history;
	history = new MainHistory;
	m_current_coordinate_system = NULL;
	m_doing_rollback = false;
	geoff_geometry::Point3d vy(0, 1, 0), vz(0, 0, 1);
	m_current_viewport->m_view_point.SetView(vy, vz, 6);
	m_filepath = std::wstring(L"Untitled.heeks");
	m_untitled = true;
	m_hidden_for_drag.clear();
	m_show_grippers_on_drag = true;
	*m_ruler = HRuler();
	SetInputMode(m_select_mode);

	ResetIDs();
}

HeeksObj* ReadPyObjectFromXMLElement(TiXmlElement* pElem){ return NULL; } // dummy function

void CApp::InitializeXMLFunctions()
{
	// set up function map
	if(xml_read_fn_map.size() == 0)
	{
#if 0
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Line", HLine::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Arc", HArc::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "InfiniteLine", HILine::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Circle", HCircle::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Point", HPoint::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Image", HImage::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Sketch", CSketch::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "STEP_file", ReadSTEPFileFromXMLElement ) );
#endif
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "STLSolid", CStlSolid::ReadFromXMLElement ) );
#if 0
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "CoordinateSystem", CoordinateSystem::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Text", HText::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Dimension", HDimension::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Ellipse", HEllipse::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Spline", HSpline::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Group", CGroup::ReadFromXMLElement ) );
		xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( "Gear", HGear::ReadFromXMLElement ) );
		xml_read_fn_map.insert(std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) >("Area", HArea::ReadFromXMLElement));
#endif
	}
}

void CApp::RegisterReadXMLfunction(const char* type_name, HeeksObj*(*read_xml_function)(TiXmlElement* pElem))
{
	if (xml_read_fn_map.find(type_name) != xml_read_fn_map.end()){
//		wxMessageBox(_T("Error - trying to register an XML read function for an existing type"));
		return;
	}
	xml_read_fn_map.insert( std::pair< std::string, HeeksObj*(*)(TiXmlElement* pElem) > ( type_name, read_xml_function ) );
}

//HeeksObj* ReadPyObjectFromXMLElementWithName(const std::string& name, TiXmlElement* pElem);

HeeksObj* CApp::ReadXMLElement(TiXmlElement* pElem)
{
	std::string name(pElem->Value());

	std::map< std::string, HeeksObj*(*)(TiXmlElement* pElem) >::iterator FindIt = xml_read_fn_map.find( name );
	HeeksObj* object = NULL;
	if(FindIt != xml_read_fn_map.end())
	{
		HeeksObj*(*callback)(TiXmlElement* pElem) = FindIt->second;
		if (callback == ReadPyObjectFromXMLElement)
		{
			//object = ReadPyObjectFromXMLElementWithName(name, pElem);
		}
		else
		{
			object = (*(callback))(pElem);
		}
	}
	else
	{
		object = HXml::ReadFromXMLElement(pElem);
	}

	if (object != NULL)
	{
		// Check to see if we already have an object for this type/id pair.  If so, use the existing one instead.
		//
		// NOTE: This would be better if another ObjList pointer was passed in and we checked the objects in that
		// ObjList for pre-existing elements rather than going to the global one.  This would allow imported data
		// (i.e. data read from another file and used to augment the current data) to work as well as the scenario
		// where we are reading into an empty memory block. (new data)

		HeeksObj *existing = NULL;

		existing = GetIDObject(object->GetIDGroupType(), object->m_id);

		if ((existing != NULL) && (existing != object))
		{
			// There was a pre-existing version of this type/id pair.  Don't replace it with another one.
			delete object;
			return(existing);
		}
		else
		{
			// It's new.  Keep this new copy.
			return object;
		}
	}

	return(object);	//  NULL
}

void CApp::ObjectWriteBaseXML(HeeksObj *object, TiXmlElement *element)
{
	if (object->UsesID())element->SetAttribute("id", object->m_id);
	if (!object->m_visible)element->SetAttribute("vis", 0);
}

void CApp::ObjectReadBaseXML(HeeksObj *object, TiXmlElement* element)
{
	// get the attributes
	for (TiXmlAttribute* a = element->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if (!this->m_inPaste && object->UsesID() && name == "id"){ object->SetID(a->IntValue()); }
		if (name == "vis"){ object->m_visible = (a->IntValue() != 0); }
	}
}

void CApp::OpenXMLFile(const wchar_t *filepath, HeeksObj* paste_into, HeeksObj* paste_before, bool undoably, bool show_error)
{
	TiXmlDocument doc(Ttc(filepath));
	if (!doc.LoadFile())
	{
		if (show_error && doc.Error())
		{
#if 0
			std::wstring msg(filepath);
			msg << wxT(": ") << Ctt(doc.ErrorDesc());
			wxMessageBox(msg);
#endif
		}
		return;
	}

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlNode* root = &doc;

	pElem = hDoc.FirstChildElement().Element();
	if (!pElem) return;
	std::string name(pElem->Value());
	if (name == "HeeksCAD_Document")
	{
		root = pElem;
	}

	ObjectReferences_t unique_set;

	char oldlocale[1000];
	strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

	std::list<HeeksObj*> objects;
	for (pElem = root->FirstChildElement(); pElem; pElem = pElem->NextSiblingElement())
	{
		HeeksObj* object = ReadXMLElement(pElem);
		if (object)
		{
			objects.push_back(object);
		}
	}

	if (objects.size() > 0)
	{
		HeeksObj* add_to = this;
		if (paste_into)add_to = paste_into;
		for (std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
		{
			HeeksObj* object = *It;
			object->ReloadPointers();

			while (1)
			{
				if (add_to->CanAdd(object) && object->CanAddTo(add_to))
				{
					if (object->OneOfAKind())
					{
						bool one_found = false;
						for (HeeksObj* child = add_to->GetFirstChild(); child; child = add_to->GetNextChild())
						{
							if (child->GetType() == object->GetType())
							{
								child->CopyFrom(object);
								one_found = true;
								break;
							}
						}
						if (!one_found)
						{
							add_to->Add(object, paste_before);
							if (m_inPaste)WasAdded(object);
						}
					}
					else
					{
						add_to->Add(object, paste_before);
						if (m_inPaste)WasAdded(object);
					}
					break;
				}
				else if (paste_into == NULL)
				{
					// can't add normally, look for preferred paste target
					add_to = object->PreferredPasteTarget();
					if (add_to == NULL || add_to == this)// already tried
						break;
				}
				else break;
			}
		}
	}
	setlocale(LC_NUMERIC, oldlocale);

	//CGroup::MoveSolidsToGroupsById(this);
}

/* static */ void CApp::OpenSVGFile(const wchar_t *filepath)
{
	char oldlocale[1000];
	strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

	CSvgRead svgread(filepath, true, theApp.m_svg_unite);
	setlocale(LC_NUMERIC, oldlocale);
}

/* static */ void CApp::OpenSTLFile(const wchar_t *filepath)
{
	char oldlocale[1000];
	strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

	HeeksColor c(128, 128, 128);
	CStlSolid* new_object = new CStlSolid(filepath, &c);
	theApp.AddUndoably(new_object, NULL, NULL);
	setlocale(LC_NUMERIC, oldlocale);
}

/* static */ void CApp::OpenDXFFile(const wchar_t *filepath )
{
	char oldlocale[1000];
	strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

		HeeksDxfRead dxf_file(filepath, true);
	setlocale(LC_NUMERIC, oldlocale);
}

bool CApp::OpenImageFile(const wchar_t *filepath)
{
#if 0
	std::wstring wf(filepath);
	wf.LowerCase();

	wxList handlers = wxImage::GetHandlers();
	for(wxList::iterator It = handlers.begin(); It != handlers.end(); It++)
	{
		wxImageHandler* handler = (wxImageHandler*)(*It);
		std::wstring ext = _T(".") + handler->GetExtension();
		if (wf.EndsWith(ext))
		{
			HImage* new_object = new HImage(filepath);
			Add(new_object, NULL);
			Repaint();
			return true;
		}
}
#endif

	return false;
}

void CApp::OnNewButton()
{
	int res = CheckForModifiedDoc();
	if (res != 0x10)
	{
		OnBeforeNewOrOpen(false, res);
		Reset();
		OnNewOrOpen(false, res);
		ClearHistory();
		SetLikeNewFile();
//		SetFrameTitle();
		Repaint();
	}
}

void CApp::OnOpenButton()
{
#if 0
	std::wstring default_directory = wxGetCwd();

	if (m_recent_files.size() > 0)
	{
#ifdef WIN32
		std::wstring delimiter(_T("\\"));
#else
		std::wstring delimiter(_T("/"));
#endif // WIN32

		default_directory = *(m_recent_files.begin());
		int last_directory_delimiter = default_directory.Find(delimiter[0], true);
		if (last_directory_delimiter > 0)
		{
			default_directory.Remove(last_directory_delimiter);
		}
	}

	wxFileDialog dialog(m_frame, _("Open file"), default_directory, wxEmptyString, GetKnownFilesWildCardString(true, false));
	dialog.CentreOnParent();

	if (dialog.ShowModal() == wxID_OK)
	{
		int res = CheckForModifiedDoc();
		if (res != wxCANCEL)
		{
			OnBeforeNewOrOpen(true, res);
			Reset();
			if (!OpenFile(dialog.GetPath().c_str()))
			{
				std::wstring str = std::wstring(_("Invalid file type chosen")) + _T("  ") + _("expecting") + _T(" ") + GetKnownFilesCommaSeparatedList(true, false);
				wxMessageBox(str);
			}
			else
			{
				m_frame->m_graphics->OnMagExtents(true, true, 25);
				OnNewOrOpen(true, res);
				ClearHistory();
				SetLikeNewFile();
			}
		}
	}
#endif
}

bool CApp::OpenFile(const wchar_t *filepath, bool import_not_open, HeeksObj* paste_into, HeeksObj* paste_before, bool retain_filename /* = true */)
{
	bool history_started = false;
	if (import_not_open && paste_into == NULL)
	{
		StartHistory();
		history_started = true;
	}

	m_in_OpenFile = true;
	m_file_open_or_import_type = FileOpenOrImportTypeOther;
	geoff_geometry::Matrix file_open_matrix;
	if (import_not_open && m_current_coordinate_system)
	{
		file_open_matrix = m_current_coordinate_system->GetMatrix();
		m_file_open_matrix = &file_open_matrix;
	}

	// returns true if file open was successful
	std::wstring wf(filepath);
	lowerCase(wf);

	std::wstring extension(filepath);
	int offset = extension.rfind('.');
	if (offset > 0) extension.erase(0, offset + 1);
	lowerCase(extension);

	bool open_succeeded = true;

	if (endsWith(wf, L".heeks"))
	{
		m_file_open_or_import_type = FileOpenTypeHeeks;
		if (import_not_open)
			m_file_open_or_import_type = FileImportTypeHeeks;
		OpenXMLFile(filepath, paste_into, paste_before, history_started);
	}
	else if (m_fileopen_handlers.find(extension) != m_fileopen_handlers.end())
	{
		(m_fileopen_handlers[extension])(filepath);
	}
	else if (endsWith(wf, L".dxf"))
	{
		m_file_open_or_import_type = FileOpenOrImportTypeDxf;
		OpenDXFFile(filepath);
	}
	// check for images
	else if (OpenImageFile(filepath))
	{
	}
	else
	{
		// error
		std::wstring str = std::wstring(L"Invalid file type chosen") + L"  " + L"expecting" + L" " + GetKnownFilesCommaSeparatedList(true, import_not_open);
		MessageBox(str.c_str());
		open_succeeded = false;
	}

	if (open_succeeded && !import_not_open)
	{
		if (retain_filename)
		{
			m_filepath.assign(filepath);
			m_untitled = false;
//			SetFrameTitle();
		}
		SetLikeNewFile();
	}

	m_file_open_matrix = NULL;
	m_in_OpenFile = false;

	if (history_started)EndHistory();

	return open_succeeded;
}

static void WriteDXFEntity(HeeksObj* object, CDxfWrite& dxf_file, const std::wstring parent_layer_name)
{
#if 0
	// to do when we have lines and arcs
	std::wstring layer_name;

	if (parent_layer_name.size() == 0)
	{
		layer_name << object->m_id;
	}
	else
	{
		layer_name = parent_layer_name;
	}

	switch (object->GetType())
	{
	case LineType:
	{
		HLine* l = (HLine*)object;
		double s[3], e[3];
		extract(l->A, s);
		extract(l->B, e);
		dxf_file.WriteLine(s, e, Ttc(layer_name.c_str()), l->m_thickness, l->m_extrusion_vector);
	}
	break;
	case PointType:
	{
		HPoint* p = (HPoint*)object;
		double s[3];
		extract(p->m_p, s);
		dxf_file.WritePoint(s, Ttc(layer_name.c_str()));
	}
	break;
	case ArcType:
	{
		HArc* a = (HArc*)object;
		double s[3], e[3], c[3];
		extract(a->A, s);
		extract(a->B, e);
		extract(a->C, c);
		bool dir = a->m_axis.Direction().z > 0;
		dxf_file.WriteArc(s, e, c, dir, Ttc(layer_name.c_str()), a->m_thickness, a->m_extrusion_vector);
	}
	break;
	case EllipseType:
	{
		HEllipse* e = (HEllipse*)object;
		double c[3];
		extract(e->C, c);
		bool dir = e->m_zdir.z > 0;
		double maj_r = e->m_majr;
		double min_r = e->m_minr;
		double rot = e->GetRotation();
		dxf_file.WriteEllipse(c, maj_r, min_r, rot, 0, 2 * M_PI, dir, Ttc(layer_name.c_str()), 0.0);
	}
	break;
	case CircleType:
	{
		HCircle* cir = (HCircle*)object;
		double c[3];
		extract(cir->m_axis.Location(), c);
		double radius = cir->m_radius;
		dxf_file.WriteCircle(c, radius, Ttc(layer_name.c_str()), cir->m_thickness, cir->m_extrusion_vector);
	}
	break;
	default:
	{
		if (parent_layer_name.Len() == 0)
		{
			layer_name.Clear();
			if ((object->GetShortString() != NULL) && (std::wstring(object->GetTypeString()) != std::wstring(object->GetShortString())))
			{
				layer_name << object->GetShortString();
			}
			else
			{
				layer_name << object->m_id;   // Use the ID as a layer name so that it's unique.
			}
		}
		else
		{
			layer_name = parent_layer_name;
		}

		for (HeeksObj* child = object->GetFirstChild(); child; child = object->GetNextChild())
		{

			// recursive
			WriteDXFEntity(child, dxf_file, layer_name);
		}
	}
	}
#endif
}

void CApp::SaveDXFFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath)
{
	CDxfWrite dxf_file(Ttc(filepath));
	if (dxf_file.Failed())
	{
		std::wstring str = std::wstring(L"couldn't open file") + filepath;
		MessageBox(str.c_str());
		return;
	}

	// write all the objects
	for (std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
	{
		HeeksObj* object = *It;
		// At this level, don't assign each element to its own layer.  We only want sketch objects
		// to be located on their own layer.  This will be done from within the WriteDXFEntity() method.
		WriteDXFEntity(object, dxf_file, L"");
	}

	// when dxf_file goes out of scope it writes the file, see ~CDxfWrite
}

static ofstream* ofs_for_write_stl_triangle = NULL;
static double* scale_for_write_triangle = NULL;

static void write_stl_triangle(const double* x, const double* n)
{
	(*ofs_for_write_stl_triangle) << " facet normal " << n[0] << " " << n[1] << " " << n[2] << endl;
	(*ofs_for_write_stl_triangle) << "   outer loop" << endl;
	if (scale_for_write_triangle)
	{
		(*ofs_for_write_stl_triangle) << "     vertex " << x[0] * (*scale_for_write_triangle) << " " << x[1] * (*scale_for_write_triangle) << " " << x[2] * (*scale_for_write_triangle) << endl;
		(*ofs_for_write_stl_triangle) << "     vertex " << x[3] * (*scale_for_write_triangle) << " " << x[4] * (*scale_for_write_triangle) << " " << x[5] * (*scale_for_write_triangle) << endl;
		(*ofs_for_write_stl_triangle) << "     vertex " << x[6] * (*scale_for_write_triangle) << " " << x[7] * (*scale_for_write_triangle) << " " << x[8] * (*scale_for_write_triangle) << endl;
		(*ofs_for_write_stl_triangle) << "   endloop" << endl;
		(*ofs_for_write_stl_triangle) << " endfacet" << endl;
	}
	else
	{
		(*ofs_for_write_stl_triangle) << "     vertex " << x[0] << " " << x[1] << " " << x[2] << endl;
		(*ofs_for_write_stl_triangle) << "     vertex " << x[3] << " " << x[4] << " " << x[5] << endl;
		(*ofs_for_write_stl_triangle) << "     vertex " << x[6] << " " << x[7] << " " << x[8] << endl;
		(*ofs_for_write_stl_triangle) << "   endloop" << endl;
		(*ofs_for_write_stl_triangle) << " endfacet" << endl;
	}
}

static void write_py_triangle(const double* x, const double* n)
{
	(*ofs_for_write_stl_triangle) << "s.addTriangle(ocl.Triangle(ocl.Point(" << x[0] << ", " << x[1] << ", " << x[2] << "), ocl.Point(" << x[3] << ", " << x[4] << ", " << x[5] << "), ocl.Point(" << x[6] << ", " << x[7] << ", " << x[8] << ")))" << endl;
}

static void write_cpp_triangle(const double* x, const double* n)
{
	for (int i = 0; i<3; i++)
	{
		(*ofs_for_write_stl_triangle) << "glNormal3d(" << n[i * 3 + 0] << ", " << n[i * 3 + 1] << ", " << n[i * 3 + 2] << ");" << endl;
		(*ofs_for_write_stl_triangle) << "glVertex3d(" << x[i * 3 + 0] << ", " << x[i * 3 + 1] << ", " << x[i * 3 + 2] << ");" << endl;
	}
}

class NineFloatsThreeFloats
{
public:
	float x[9];
	float n[3];
};
static std::list<NineFloatsThreeFloats> binary_triangles;

static void write_binary_triangle(const double* x, const double* n)
{
	NineFloatsThreeFloats t;
	for (int i = 0; i<9; i++)t.x[i] = (float)(x[i]);
	for (int i = 0; i<3; i++)t.n[i] = (float)(n[i]);
	binary_triangles.push_back(t);
}

void CApp::SaveSTLFileBinary(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance, double* scale)
{
#if 0
	// to do
#ifdef __WXMSW__
	ofstream ofs(filepath, ios::binary);
#else
	ofstream ofs(Ttc(filepath), ios::binary);
#endif

	// write 80 characters ( could be anything )
	char header[80] = "Binary STL file made with HeeksCAD                                     ";
	ofs.write(header, 80);

	// get all the triangles
	for (std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
	{
		HeeksObj* object = *It;
		object->GetTriangles(write_binary_triangle, facet_tolerance < 0 ? m_stl_facet_tolerance : facet_tolerance);
	}

	// write the number of facets
	unsigned int num_facets = binary_triangles.size();
	ofs.write((char*)(&num_facets), 4);

	for (std::list<NineFloatsThreeFloats>::iterator It = binary_triangles.begin(); It != binary_triangles.end(); It++)
	{
		NineFloatsThreeFloats t = *It;

		geoff_geometry::Point3d p0(t.x[0], t.x[1], t.x[2]);
		geoff_geometry::Point3d p1(t.x[3], t.x[4], t.x[5]);
		geoff_geometry::Point3d p2(t.x[6], t.x[7], t.x[8]);
		geoff_geometry::Point3d v1(p0, p1);
		geoff_geometry::Point3d v2(p0, p2);
		float n[3] = { 0.0f, 0.0f, 1.0f };
		try
		{
			geoff_geometry::Point3d norm = (v1 ^ v2).Normalized();
			n[0] = (float)(norm.x);
			n[1] = (float)(norm.y);
			n[2] = (float)(norm.z);
		}
		catch (...)
		{
		}

		ofs.write((char*)(n), 12);
		ofs.write((char*)(t.x), 36);
		short attr = 0;
		ofs.write((char*)(&attr), 2);
	}

	binary_triangles.clear();
#endif
}

void CApp::SaveSTLFileAscii(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance, double* scale)
{
#ifdef __WXMSW__
	ofstream ofs(filepath);
#else
	ofstream ofs(Ttc(filepath));
#endif
	if (!ofs)
	{
		std::wstring str = std::wstring(L"couldn't open file") + L" - " + filepath;
		MessageBox(str.c_str());
		return;
	}
	ofs.imbue(std::locale("C"));

	ofs << "solid" << endl;

	// write all the objects
	ofs_for_write_stl_triangle = &ofs;
	scale_for_write_triangle = scale;
	for (std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
	{
		HeeksObj* object = *It;
		object->GetTriangles(write_stl_triangle, facet_tolerance < 0 ? m_stl_facet_tolerance : facet_tolerance);
	}
	scale_for_write_triangle = NULL;

	ofs << "endsolid" << endl;
}

class ObjFileVertex
{
public:
	int m_index;
	geoff_geometry::Point3d m_p;
	ObjFileVertex(int index, const geoff_geometry::Point3d& p) :m_index(index), m_p(p){}
};

class ObjFileTriangle
{
public:
	int m_v[3];
	ObjFileTriangle(int v0, int v1, int v2){ m_v[0] = v0; m_v[1] = v1; m_v[2] = v2; }
};

class ObjFileVertexManager
{
	std::map<double, std::list<ObjFileVertex*> > m_map;
	int m_next_index;
	std::list<ObjFileTriangle> m_tris;
	std::list<ObjFileVertex*> m_verts;

public:
	ObjFileVertexManager() :m_next_index(1){}
	~ObjFileVertexManager()
	{
		for (std::list<ObjFileVertex*>::iterator It = m_verts.begin(); It != m_verts.end(); It++)
		{
			ObjFileVertex* v = *It;
			delete v;
		}
	}

	int InsertVertex(const geoff_geometry::Point3d& p)
	{
		ObjFileVertex* v = NULL;

		std::map<double, std::list<ObjFileVertex*> >::iterator FindIt = m_map.find(p.x);
		if (FindIt == m_map.end())
		{
			v = new ObjFileVertex(m_next_index, p);
			std::list<ObjFileVertex*> list;
			list.push_back(v);
			m_verts.push_back(v);
			m_map.insert(std::make_pair(p.x, list));
			m_next_index++;
		}
		else
		{
			std::list<ObjFileVertex*> &list = FindIt->second;
			for (std::list<ObjFileVertex*>::iterator It = list.begin(); It != list.end(); It++)
			{
				ObjFileVertex* v = *It;
				if (v->m_p == p)
					return v->m_index;
			}

			v = new ObjFileVertex(m_next_index, p);
			list.push_back(v);
			m_verts.push_back(v);
			m_next_index++;
		}
		return v->m_index;
	}

	void InsertTriangle(const double* t)
	{
		int v0 = InsertVertex(geoff_geometry::Point3d(t[0], t[1], t[2]));
		int v1 = InsertVertex(geoff_geometry::Point3d(t[3], t[4], t[5]));
		int v2 = InsertVertex(geoff_geometry::Point3d(t[6], t[7], t[8]));
		m_tris.push_back(ObjFileTriangle(v0, v1, v2));
	}

	int GetIndex(const geoff_geometry::Point3d& p)
	{
		std::map<double, std::list<ObjFileVertex*> >::iterator FindIt = m_map.find(p.x);
		if (FindIt != m_map.end())
		{
			std::list<ObjFileVertex*> &list = FindIt->second;
			for (std::list<ObjFileVertex*>::iterator It = list.begin(); It != list.end(); It++)
			{
				ObjFileVertex* v = *It;
				if (v->m_p ==p)
					return v->m_index;
			}
		}
		return 0;
	}

	void WriteObjFile(const std::wstring& filepath)
	{
#ifdef __WXMSW__
		ofstream ofs(filepath);
#else
		ofstream ofs(Ttc(filepath.c_str()));
#endif
		if (!ofs)
		{
			std::wstring str = std::wstring(L"couldn't open file") + L" - " + filepath;
			theApp.MessageBox(str.c_str());
			return;
		}
		ofs.imbue(std::locale("C"));

		// write the vertices
		for (std::list<ObjFileVertex*>::iterator It = m_verts.begin(); It != m_verts.end(); It++)
		{
			ObjFileVertex* v = *It;
			ofs << "v " << v->m_p.x << " " << v->m_p.y << " " << v->m_p.z << endl;
		}

		// write the triangles
		for (std::list<ObjFileTriangle>::iterator It = m_tris.begin(); It != m_tris.end(); It++)
		{
			ObjFileTriangle& tri = *It;
			ofs << "f " << tri.m_v[0] << " " << tri.m_v[1] << " " << tri.m_v[2] << endl;
		}
	}
};

ObjFileVertexManager* vertex_manager_for_write_triangle = NULL;


static void add_obj_triangles(const double* x, const double* n)
{
	vertex_manager_for_write_triangle->InsertTriangle(x);
}

void CApp::SaveOBJFileAscii(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance, double* scale)
{
	ObjFileVertexManager vertex_manager;
	vertex_manager_for_write_triangle = &vertex_manager;

	for (std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
	{
		HeeksObj* object = *It;
		object->GetTriangles(add_obj_triangles, facet_tolerance < 0 ? m_stl_facet_tolerance : facet_tolerance);
	}

	vertex_manager.WriteObjFile(filepath);
}

void CApp::SaveSTLFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance, double* scale, bool binary)
{
	if (binary)SaveSTLFileBinary(objects, filepath, facet_tolerance, scale);
	else SaveSTLFileAscii(objects, filepath, facet_tolerance, scale);
}

void CApp::SaveCPPFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance)
{
#ifdef __WXMSW__
	ofstream ofs(filepath);
#else
	ofstream ofs(Ttc(filepath));
#endif
	if (!ofs)
	{
		std::wstring str = std::wstring(L"couldn't open file") + L" - " + filepath;
		MessageBox(str.c_str());
		return;
	}
	ofs.imbue(std::locale("C"));

	ofs << "glBegin(GL_TRIANGLES);" << endl;

	// write all the objects
	ofs_for_write_stl_triangle = &ofs;
	for (std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
	{
		HeeksObj* object = *It;
		object->GetTriangles(write_cpp_triangle, facet_tolerance < 0 ? m_stl_facet_tolerance : facet_tolerance, false);
	}

	ofs << "glEnd();" << endl;
}

void CApp::SavePyFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance)
{
#ifdef __WXMSW__
	ofstream ofs(filepath);
#else
	ofstream ofs(Ttc(filepath));
#endif
	if (!ofs)
	{
		std::wstring str = std::wstring(L"couldn't open file") + L" - " + filepath;
		MessageBox(str.c_str());
		return;
	}
	ofs.imbue(std::locale("C"));

	ofs << "s = ocl.STLSurf()" << endl;

	// write all the objects
	ofs_for_write_stl_triangle = &ofs;
	for (std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
	{
		HeeksObj* object = *It;
		object->GetTriangles(write_py_triangle, facet_tolerance < 0 ? m_stl_facet_tolerance : facet_tolerance, false);
	}
}

void CApp::SaveXMLFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath, bool for_clipboard)
{
	// write an xml file
	TiXmlDocument doc;
	const char *l_pszVersion = "1.0";
	const char *l_pszEncoding = "UTF-8";
	const char *l_pszStandalone = "";

	TiXmlDeclaration* decl = new TiXmlDeclaration(l_pszVersion, l_pszEncoding, l_pszStandalone);
	doc.LinkEndChild(decl);

	TiXmlNode* root = &doc;
	if (!for_clipboard)
	{
		root = new TiXmlElement("HeeksCAD_Document");
		doc.LinkEndChild(root);
	}

	// loop through all the objects writing them
	for (std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
	{
		HeeksObj* object = *It;
		object->WriteXML(root);
	}


	doc.SaveFile(Ttc(filepath));
}

bool CApp::SaveFile(const wchar_t *filepath, bool use_dialog, bool update_recent_file_list, bool set_app_caption)
{
#if 0
	if (use_dialog){
		wxFileDialog fd(m_frame, _("Save graphical data file"), wxEmptyString, filepath, GetKnownFilesWildCardString(false, false), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		fd.SetFilterIndex(1);
		if (fd.ShowModal() == wxID_CANCEL)return false;
		return SaveFile(fd.GetPath().c_str(), false, update_recent_file_list);
	}

	std::wstring wf(filepath);
	wf.LowerCase();

	if (wf.EndsWith(_T(".heeks")))
	{
		// call external OnSave functions
		for (std::list< void(*)(bool) >::iterator It = m_on_save_callbacks.begin(); It != m_on_save_callbacks.end(); It++)
		{
			void(*callbackfunc)(bool) = *It;
			(*callbackfunc)(false);
		}

		SaveXMLFile(filepath);
	}
	else if (wf.EndsWith(_T(".dxf")))
	{
		SaveDXFFile(m_objects, filepath);
	}
	else if (wf.EndsWith(_T(".stl")))
	{
		SaveSTLFile(m_objects, filepath, -1.0, NULL, m_stl_save_as_binary);
	}
	else if (wf.EndsWith(_T(".cpp")))
	{
		SaveCPPFile(m_objects, filepath);
	}
	else if (wf.EndsWith(_T(".obj")))
	{
		SaveOBJFileAscii(m_objects, filepath);
	}
	else if (wf.EndsWith(_T(".py")))
	{
		SavePyFile(m_objects, filepath);
	}
	else if (CShape::ExportSolidsFile(m_objects, filepath))
	{
	}
	else
	{
		std::wstring str = std::wstring(_("Invalid file type chosen")) + _T("  ") + _("expecting") + _T(" ") + GetKnownFilesCommaSeparatedList(false, false);
		wxMessageBox(str);
		return false;
	}

	m_filepath.assign(filepath);
	m_untitled = false;

	if (update_recent_file_list)InsertRecentFileItem(filepath);
	if (set_app_caption)SetFrameTitle();
	SetLikeNewFile();
#endif

	return true;
}


void CApp::Repaint(bool soon)
{
#if 0
	if (soon)m_frame->m_graphics->RefreshSoon();
	else m_frame->m_graphics->Refresh();
#endif
}

void CApp::RecalculateGLLists()
{
	for (HeeksObj* object = GetFirstChild(); object; object = GetNextChild()){
		object->KillGLLists();
	}
}

void CApp::RenderDatumOrCurrentCoordSys()
{
	if (m_show_datum_coords_system || m_current_coordinate_system)
	{
		bool bright_datum = (m_current_coordinate_system == NULL);
		if (m_datum_coords_system_solid_arrows)
		{
			// make the datum appear at the front of everything, by clearing the depth buffer
			if (m_show_datum_coords_system)
			{
				glClear(GL_DEPTH_BUFFER_BIT);
				CoordinateSystem::RenderDatum(bright_datum, true);
			}
			if (m_current_coordinate_system)
			{
				glClear(GL_DEPTH_BUFFER_BIT);
				CoordinateSystem::rendering_current = true;
				m_current_coordinate_system->glCommands(false, m_marked_list->ObjectMarked(m_current_coordinate_system), false);
				CoordinateSystem::rendering_current = false;
			}
		}
		else
		{
			// make the datum appear at the front of everything, by setting the depth range
			GLfloat save_depth_range[2];
			glGetFloatv(GL_DEPTH_RANGE, save_depth_range);
			glDepthRange(0, 0);

			if (m_current_coordinate_system)
			{
				CoordinateSystem::rendering_current = true;
				m_current_coordinate_system->glCommands(false, m_marked_list->ObjectMarked(m_current_coordinate_system), false);
				CoordinateSystem::rendering_current = false;
			}
			if (m_show_datum_coords_system)
			{
				CoordinateSystem::RenderDatum(bright_datum, false);
			}

			// restore the depth range
			glDepthRange(save_depth_range[0], save_depth_range[1]);
		}
	}
}

void CApp::glCommandsAll(const CViewPoint &view_point)
{
	CreateLights();
	glDisable(GL_LIGHTING);
	Material().glMaterial(1.0);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glLineWidth(1);
	glDepthMask(1);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glShadeModel(GL_FLAT);
	view_point.SetPolygonOffset();

	std::list<HeeksObj*> after_others_objects;

	for (std::list<HeeksObj*>::iterator It = m_objects.begin(); It != m_objects.end(); It++)
	{
		HeeksObj* object = *It;
		if (object->OnVisibleLayer() && object->m_visible)
		{
			if (object->DrawAfterOthers())after_others_objects.push_back(object);
			else
			{
				object->glCommands(false, m_marked_list->ObjectMarked(object), false);
			}
		}
	}

	// draw any last_objects
	for (std::list<HeeksObj*>::iterator It = after_others_objects.begin(); It != after_others_objects.end(); It++)
	{
		HeeksObj* object = *It;
		object->glCommands(false, m_marked_list->ObjectMarked(object), false);
	}

	glDisable(GL_POLYGON_OFFSET_FILL);
	for (std::list< void(*)() >::iterator It = m_on_glCommands_list.begin(); It != m_on_glCommands_list.end(); It++)
	{
		void(*callbackfunc)() = *It;
		(*callbackfunc)();
	}
	glEnable(GL_POLYGON_OFFSET_FILL);

	input_mode_object->OnRender();
	if (m_transform_gl_list)
	{
		glPushMatrix();
		double m[16];
		m_drag_matrix.Get(m);
		glMultMatrixd(m);
		glCallList(m_transform_gl_list);
		glPopMatrix();
	}

	// draw the ruler
	if (m_show_ruler && m_ruler->m_visible)
	{
		m_ruler->glCommands(false, false, false);
	}
	// draw the grid
	glDepthFunc(GL_LESS);
	RenderGrid(&view_point);
	glDepthFunc(GL_LEQUAL);

	// draw the datum
	RenderDatumOrCurrentCoordSys();

	DestroyLights();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (m_hidden_for_drag.size() == 0 || !m_show_grippers_on_drag)m_marked_list->GrippersGLCommands(false, false);

	// draw the input mode text on the top
	if (m_graphics_text_mode != GraphicsTextModeNone)
	{
		std::wstring screen_text1, screen_text2;

		if (input_mode_object && input_mode_object->GetTitle())
		{
			screen_text1.append(input_mode_object->GetTitle());
			screen_text1.append(L"\n");
		}
		if (m_graphics_text_mode == GraphicsTextModeWithHelp && input_mode_object)
		{
			const wchar_t* help_str = input_mode_object->GetHelpText();
			if (help_str)
			{
				screen_text2.append(help_str);
			}
		}
		render_screen_text(screen_text1.c_str(), screen_text2.c_str(), false);
	}
}

void CApp::OnInputModeTitleChanged()
{
	if (m_graphics_text_mode != GraphicsTextModeNone)
	{
		Repaint();
	}
}

void CApp::OnInputModeHelpTextChanged()
{
	if (m_graphics_text_mode == GraphicsTextModeWithHelp)
	{
		Repaint();
	}
}

void CApp::glCommands(bool select, bool marked, bool no_color)
{
	// this is called when select is true
	ObjList::glCommands(select, marked, no_color);

	// draw the ruler
	if (m_show_ruler)
	{
		if (select)SetPickingColor(m_ruler->GetIndex());
		m_ruler->glCommands(select, false, no_color);
	}
}

void CApp::GetBox(CBox &box){
	CBox temp_box;
	ObjList::GetBox(temp_box);
	if (temp_box.m_valid && temp_box.Radius() > 0.000001)
		box.Insert(temp_box);
}

double CApp::GetPixelScale(void){
	return m_current_viewport->m_view_point.m_pixel_scale;
}

bool CApp::IsModified(void){
	if (history->IsModified())return true;

	for (std::list< bool(*)() >::iterator It = m_is_modified_callbacks.begin(); It != m_is_modified_callbacks.end(); It++)
	{
		bool(*callbackfunc)() = *It;
		bool is_modified = (*callbackfunc)();
		if (is_modified)
		{
			return true;
		}
	}

	return false;
}

void CApp::SetAsModified(){
	history->SetAsModified();
}

void CApp::SetLikeNewFile(void){
	history->SetLikeNewFile();
}

void CApp::ClearHistory(void){
	history->ClearFromFront();
	history->SetLikeNewFile();
}

void CApp::DoUndoable(Undoable *u)
{
	history->DoUndoable(u);
}

bool CApp::RollBack(void)
{
	m_doing_rollback = true;
	bool result = history->InternalRollBack();
	m_doing_rollback = false;
	return result;
}

bool CApp::CanUndo(void)
{
	return history->CanUndo();
}

bool CApp::CanRedo(void)
{
	return history->CanRedo();
}

bool CApp::RollForward(void)
{
	m_doing_rollback = true;
	bool result = history->InternalRollForward();
	m_doing_rollback = false;
	return result;
}

void CApp::StartHistory()
{
	history->StartHistory();
}

void CApp::EndHistory(void)
{
	history->EndHistory();
}

void CApp::ClearRollingForward(void)
{
	history->ClearFromCurPos();
}

void CApp::RegisterObserver(Observer* observer)
{
	if (observer == NULL) return;
	observers.insert(observer);
	observer->OnChanged(&m_objects, NULL, NULL);
}

void CApp::RemoveObserver(Observer* observer){
	observers.erase(observer);
}

void CApp::ObserversOnChange(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified){
	std::set<Observer*>::iterator It;
	for (It = observers.begin(); It != observers.end(); It++){
		Observer *ov = *It;
		ov->OnChanged(added, removed, modified);
	}
}

void CApp::ObserversMarkedListChanged(bool selection_cleared, const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed){
	std::set<Observer*>::iterator It;
	for (It = observers.begin(); It != observers.end(); It++){
		Observer *ov = *It;
		ov->WhenMarkedListChanges(selection_cleared, added, removed);
	}
}

void CApp::ObserversFreeze()
{
	std::set<Observer*>::iterator It;
	for (It = observers.begin(); It != observers.end(); It++){
		Observer *ov = *It;
		ov->Freeze();
	}
}

void CApp::ObserversThaw()
{
	std::set<Observer*>::iterator It;
	for (It = observers.begin(); It != observers.end(); It++){
		Observer *ov = *It;
		ov->Thaw();
	}
}

bool CApp::Add(HeeksObj *object, HeeksObj* prev_object)
{
	if (!ObjList::Add(object, prev_object)) return false;

	if (object->GetType() == CoordinateSystemType && (!m_in_OpenFile || (m_file_open_or_import_type != FileOpenTypeHeeks && m_file_open_or_import_type != FileImportTypeHeeks)))
	{
		m_current_coordinate_system = (CoordinateSystem*)object;
	}

	if (m_mark_newly_added_objects)
	{
		m_marked_list->Add(object, true);
	}

	return true;
}

void CApp::Remove(HeeksObj* object)
{
#ifdef MULTIPLE_OWNERS
	HeeksObj* owner = object->GetFirstOwner();
	while (owner)
	{
		if (owner != this)
		{
			owner->Remove(object);
			owner->ReloadPointers();
		}
		else
			ObjList::Remove(object);
		owner = object->GetNextOwner();
	}
#else
	if (object->m_owner)
	{
		if (object->m_owner != this)
		{
			object->m_owner->Remove(object);
			object->m_owner->ReloadPointers();
		}
		else
			ObjList::Remove(object);
	}
#endif
	if (object == m_current_coordinate_system)m_current_coordinate_system = NULL;
}

void CApp::Remove(std::list<HeeksObj*> objects)
{
	ObjList::Remove(objects);
}

void CApp::AddUndoably(HeeksObj *object, HeeksObj* owner, HeeksObj* prev_object)
{
	if (object == NULL)return;
	if (owner == NULL)owner = this;
	AddObjectTool *undoable = new AddObjectTool(object, owner, prev_object);
	DoUndoable(undoable);
}

void CApp::AddUndoably(const std::list<HeeksObj*> &list, HeeksObj* owner)
{
	if (list.size() == 0)return;
	if (owner == NULL)owner = this;
	AddObjectsTool *undoable = new AddObjectsTool(list, owner);
	DoUndoable(undoable);
}

void CApp::DeleteUndoably(HeeksObj *object){
	if (object == NULL)return;
	if (!object->CanBeRemoved())return;
	RemoveObjectTool *undoable = new RemoveObjectTool(object);
	DoUndoable(undoable);
}

void CApp::DeleteUndoably(const std::list<HeeksObj*>& list)
{
	if (list.size() == 0)return;
	std::list<HeeksObj*> list2;
	for (std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++)
	{
		HeeksObj* object = *It;
		if (object->CanBeRemoved())list2.push_back(object);
	}
	if (list2.size() == 0)return;
	RemoveObjectsTool *undoable = new RemoveObjectsTool(list2, list2.front()->m_owner);
	DoUndoable(undoable);
}

void CApp::CopyUndoably(HeeksObj* object, HeeksObj* copy_with_new_data)
{
	DoUndoable(new CopyObjectUndoable(object, copy_with_new_data));
}

void CApp::TransformUndoably(HeeksObj *object, const geoff_geometry::Matrix &m)
{
	if (!object)return;
	geoff_geometry::Matrix mat = geoff_geometry::Matrix(m);
	geoff_geometry::Matrix im = mat.Inverse();
	TransformTool *undoable = new TransformTool(object, mat, im);
	DoUndoable(undoable);
}

void CApp::TransformUndoably(const std::list<HeeksObj*> &list, const geoff_geometry::Matrix &m)
{
	if (list.size() == 0)return;
	geoff_geometry::Matrix mat = geoff_geometry::Matrix(m);
	geoff_geometry::Matrix im = mat.Inverse();
	TransformObjectsTool *undoable = new TransformObjectsTool(list, mat, im);
	DoUndoable(undoable);
}

#if 0
// to do
class ReverseUndoable : public Undoable{
	HeeksObj* m_object;

public:
	ReverseUndoable(HeeksObj* object) :m_object(object){}
	void Run(bool redo){ CSketch::ReverseObject(m_object); }
	const wchar_t* GetTitle(){ return _("Reverse Object"); }
	void RollBack(){ CSketch::ReverseObject(m_object); }
};
#endif

void CApp::ReverseUndoably(HeeksObj *object)
{
#if 0
	DoUndoable(new ReverseUndoable(object));
#endif
}

void CApp::EditUndoably(HeeksObj *object)
{
	HeeksObj* copy_object = object->MakeACopyWithID();
	if (copy_object)
	{
		if (copy_object->Edit())
		{
			theApp.CopyUndoably(object, copy_object);
		}
		else
			delete copy_object;
	}
}

void CApp::Transform(std::list<HeeksObj*> objects, const geoff_geometry::Matrix& m)
{
	std::list<HeeksObj*>::iterator it;
	for (it = objects.begin(); it != objects.end(); ++it)
	{
		(*it)->Transform(m);
	}
}

void CApp::WasModified(HeeksObj *object)
{
	std::list<HeeksObj*> list;
	list.push_back(object);
	WereModified(list);
}

void CApp::WasAdded(HeeksObj *object)
{
	std::list<HeeksObj*> list;
	list.push_back(object);
	WereAdded(list);
}

void CApp::WasRemoved(HeeksObj *object)
{
	std::list<HeeksObj*> list;
	list.push_back(object);
	WereRemoved(list);
}

void CApp::WereModified(const std::list<HeeksObj*>& list)
{
	if (list.size() == 0) return;
	HeeksObj* object = *(list.begin());
	if (object == NULL) return;
	ObserversOnChange(NULL, NULL, &list);
	SetAsModified();
}

void CApp::WereAdded(const std::list<HeeksObj*>& list)
{
	if (list.size() == 0) return;
	HeeksObj* object = *(list.begin());
	if (object == NULL) return;
	ObserversOnChange(&list, NULL, NULL);
	SetAsModified();
}

void CApp::WereRemoved(const std::list<HeeksObj*>& list)
{
	if (list.size() == 0) return;
	HeeksObj* object = *(list.begin());
	if (object == NULL) return;

	std::list<HeeksObj*> marked_remove;
	for (std::list<HeeksObj*>::const_iterator It = list.begin(); It != list.end(); It++)
	{
		object = *It;
		if (m_marked_list->ObjectMarked(object))marked_remove.push_back(object);
	}
	if (marked_remove.size() > 0)m_marked_list->Remove(marked_remove, false);

	ObserversOnChange(NULL, &list, NULL);
	SetAsModified();
}

geoff_geometry::Matrix CApp::GetDrawMatrix(bool get_the_appropriate_orthogonal)
{
#if 0
	if (get_the_appropriate_orthogonal){
		// choose from the three orthoganal possibilities, the one where it's z-axis closest to the camera direction
		geoff_geometry::Point3d vx, vy;
		m_current_viewport->m_view_point.GetTwoAxes(vx, vy, false, 0);
		{
			geoff_geometry::Point3d o(0, 0, 0);
			if (m_current_coordinate_system)o.Transform(m_current_coordinate_system->GetMatrix());
			return make_matrix(o, vx, vy);
		}
	}

	geoff_geometry::Matrix mat;
	if (m_current_coordinate_system)mat = m_current_coordinate_system->GetMatrix();
	return mat;
#endif
	return geoff_geometry::Matrix();
}

void CApp::MessageBox(const wchar_t* message)
{
	// this will pass the message out to the GUI via some kind of callback function
}

void CApp::GetOptions(std::list<Property *> *list)
{
	// maybe not
}


void CApp::DeleteMarkedItems()
{
	std::list<HeeksObj *> list;
	for (std::list<HeeksObj*>::iterator It = m_marked_list->list().begin(); It != m_marked_list->list().end(); It++)
	{
		HeeksObj* object = *It;
		if (object->CanBeRemoved())list.push_back(object);
	}

	// clear first, so properties cancel happens first
	m_marked_list->Clear(false);

	if (list.size() == 1){
		DeleteUndoably(*(list.begin()));
	}
	else if (list.size()>1){
		DeleteUndoably(list);
	}
	Repaint(0);
}

void CApp::glColorEnsuringContrast(const HeeksColor &c)
{
	if (c == HeeksColor(0, 0, 0) || c == HeeksColor(255, 255, 255))background_color[0].best_black_or_white().glColor();
	else c.glColor();
}

static std::wstring known_file_ext;

const wchar_t* CApp::GetKnownFilesWildCardString(bool open, bool import_export)const
{
	if (!import_export)
	{
		known_file_ext = std::wstring(L"Heeks files") + L" |*.heeks;*.HEEKS";
		return known_file_ext.c_str();
	}

	if (open){
		if (m_alternative_open_wild_card_string.size() > 0)
			return m_alternative_open_wild_card_string.c_str();

#if 0
		wxList handlers = wxImage::GetHandlers();
		std::wstring imageExtStr;
		std::wstring imageExtStr2;
		for (wxList::iterator It = handlers.begin(); It != handlers.end(); It++)
		{
			wxImageHandler* handler = (wxImageHandler*)(*It);
			std::wstring ext = handler->GetExtension();
			if (It != handlers.begin())imageExtStr.Append(_T(";"));
			imageExtStr.Append(_T("*."));
			imageExtStr.Append(ext);
			if (It != handlers.begin())imageExtStr2.Append(_T(" "));
			imageExtStr2.Append(_T("*."));
			imageExtStr2.Append(ext);
		}
#endif

		std::wstring registeredExtensions;
#if 0
		for (FileOpenHandlers_t::const_iterator itHandler = m_fileopen_handlers.begin(); itHandler != m_fileopen_handlers.end(); itHandler++)
		{
			registeredExtensions << L";*." << itHandler->first;
		}
#endif

		known_file_ext = std::wstring(L"Known Files") + L" |*.heeks;*.HEEKS;*.igs;*.IGS;*.iges;*.IGES;*.stp;*.STP;*.step;*.STEP;*.dxf;*.DXF"/* + imageExtStr*/ + registeredExtensions + L"|" + L"Heeks files" + L" (*.heeks)|*.heeks;*.HEEKS|" + L"IGES files" + L" (*.igs *.iges)|*.igs;*.IGS;*.iges;*.IGES|" + L"STEP files" + L" (*.stp *.step)|*.stp;*.STP;*.step;*.STEP|" + L"STL files" + L" (*.stl)|*.stl;*.STL|" + L"Scalar Vector Graphics files" + L" (*.svg)|*.svg;*.SVG|" + L"DXF files" + L" (*.dxf)|*.dxf;*.DXF|" + L"RS274X/Gerber files" + L" (*.gbr,*.rs274x)|*.gbr;*.GBR;*.rs274x;*.RS274X;*.pho;*.PHO|"/* + L"Picture files" + L" (" + imageExtStr2 + L")|" + imageExtStr*/;
		return known_file_ext.c_str();
	}
	else{
		// file save
		known_file_ext = std::wstring(L"Known Files") + L" |*.heeks;*.igs;*.iges;*.stp;*.step;*.stl;*.dxf;*.cpp;*.py;*.obj|" + L"Heeks files" + L" (*.heeks)|*.heeks|" + L"IGES files" + L" (*.igs *.iges)|*.igs;*.iges|" + L"STEP files" + L" (*.stp *.step)|*.stp;*.step|" + L"STL files" + L" (*.stl)|*.stl|" + L"DXF files" + L" (*.dxf)|*.dxf|" + L"CPP files" + L" (*.cpp)|*.cpp|" + L"OpenCAMLib python files" + L" (*.py)|*.py|" + L"Wavefront .obj files" + L" (*.obj)|*.obj";
		return known_file_ext.c_str();
	}
}

const wchar_t* CApp::GetKnownFilesCommaSeparatedList(bool open, bool import_export)const
{
	if (!import_export)
		return L"heeks";

#if 0
	if (open){
		wxList handlers = wxImage::GetHandlers();
		std::wstring known_ext_str = _T("heeks, igs, iges, stp, step, dxf");
		for (wxList::iterator It = handlers.begin(); It != handlers.end(); It++)
		{
			wxImageHandler* handler = (wxImageHandler*)(*It);
			std::wstring ext = handler->GetExtension();
			known_ext_str.Append(_T(", "));
			known_ext_str.Append(ext);
		}

		for (FileOpenHandlers_t::const_iterator itHandler = m_fileopen_handlers.begin(); itHandler != m_fileopen_handlers.end(); itHandler++)
		{
			known_ext_str << _T(", ") << itHandler->first;
		}


		return known_ext_str;
	}
	else{
		// file save
#endif
		return L"heeks, igs, iges, stp, step, stl, dxf";
#if 0
	}
#endif
}

std::wstring CApp::GetExeFolder()const{
	return L"";
}
std::wstring CApp::GetResFolder()const{ return L""; }


// do your own glBegin and glEnd
void CApp::get_2d_arc_segments(double xs, double ys, double xe, double ye, double xc, double yc, bool dir, bool want_start, double pixels_per_mm, void(*callbackfunc)(const double* xy)){
	double ax = xs - xc;
	double ay = ys - yc;
	double bx = xe - xc;
	double by = ye - yc;

	double start_angle = atan2(ay, ax);
	double end_angle = atan2(by, bx);

	if (dir){
		if (start_angle > end_angle)end_angle += 6.28318530717958;
	}
	else{
		if (end_angle > start_angle)start_angle += 6.28318530717958;
	}

	double dxc = xs - xc;
	double dyc = ys - yc;
	double radius = sqrt(dxc*dxc + dyc*dyc);
	double d_angle = end_angle - start_angle;
	int segments = (int)(pixels_per_mm * radius * fabs(d_angle) / 6.28318530717958 + 1);

	double theta = d_angle / (double)segments;
	while (theta>1.0){ segments *= 2; theta = d_angle / (double)segments; }
	double tangetial_factor = tan(theta);
	double radial_factor = 1 - cos(theta);

	double x = radius * cos(start_angle);
	double y = radius * sin(start_angle);

	for (int i = 0; i < segments + 1; i++)
	{
		if (i != 0 || want_start){
			double xy[2] = { xc + x, yc + y };
			(*callbackfunc)(xy);
		}

		double tx = -y;
		double ty = x;

		x += tx * tangetial_factor;
		y += ty * tangetial_factor;

		double rx = -x;
		double ry = -y;

		x += rx * radial_factor;
		y += ry * radial_factor;
	}
}

static long save_filter_for_StartPickObjects = 0;
static bool save_just_one_for_EndPickObjects = false;
static CInputMode* save_mode_for_EndPickObjects = NULL;

void CApp::StartPickObjects(const wchar_t* str, long marking_filter, bool just_one)
{
	save_mode_for_EndPickObjects = input_mode_object;
//	m_select_mode->m_prompt_when_doing_a_main_loop.assign(str);
	m_select_mode->m_doing_a_main_loop = true;
	save_just_one_for_EndPickObjects = m_select_mode->m_just_one;
	m_select_mode->m_just_one = just_one;
	SetInputMode(m_select_mode);

	// set marking filter
	save_filter_for_StartPickObjects = m_marked_list->m_filter;
	m_marked_list->m_filter = marking_filter;
}

int CApp::EndPickObjects()
{
	// restore marking filter
	m_marked_list->m_filter = save_filter_for_StartPickObjects;

	m_select_mode->m_just_one = save_just_one_for_EndPickObjects;
	m_select_mode->m_doing_a_main_loop = false;
	SetInputMode(save_mode_for_EndPickObjects); // update tool bar

	return 1;
}

int CApp::PickObjects(const wchar_t* str, long marking_filter, bool just_one)
{
	StartPickObjects(str, marking_filter, just_one);

	// stay in an input loop until finished picking
#if 0
	OnRun();
#endif

	return EndPickObjects();
}

bool CApp::PickPosition(const wchar_t* str, double* pos, void(*callback)(const double*))
{
	// not sure
	return false;


#if 0
	CInputMode* save_mode = input_mode_object;
	m_digitizing->m_prompt_when_doing_a_main_loop.assign(str);
	m_digitizing->m_doing_a_main_loop = true;
	m_digitizing->m_callback = callback;
	SetInputMode(m_digitizing);

	OnRun();

	bool return_found = false;
	if (m_digitizing->digitized_point.m_type != DigitizeNoItemType){
		extract(m_digitizing->digitized_point.m_point, pos);
		return_found = true;
	}

	m_digitizing->m_doing_a_main_loop = false;
	m_digitizing->m_callback = NULL;
	SetInputMode(save_mode);
	return return_found;
#endif
}

static int sphere_display_list = 0;

void CApp::glSphere(double radius, const double* pos)
{
	glPushMatrix();

	if (pos)
	{
		glTranslated(pos[0], pos[1], pos[2]);
	}

	glScaled(radius, radius, radius);

	if (sphere_display_list)
	{
		glCallList(sphere_display_list);
	}
	else{
		sphere_display_list = glGenLists(1);
		glNewList(sphere_display_list, GL_COMPILE_AND_EXECUTE);

		glBegin(GL_QUADS);

		double v_ang = 0.0;
		double pcosv = 0.0;
		double psinv = 0.0;

		for (int i = 0; i<11; i++, v_ang += 0.15707963267948966)
		{
			double cosv = cos(v_ang);
			double sinv = sin(v_ang);

			if (i > 0)
			{
				double h_ang = 0.0;
				double pcosh = 0.0;
				double psinh = 0.0;

				for (int j = 0; j<21; j++, h_ang += 0.314159265358979323)
				{
					double cosh = cos(h_ang);
					double sinh = sin(h_ang);

					if (j > 0)
					{
						// top quad
						glNormal3d(pcosh * pcosv, psinh * pcosv, psinv);
						glVertex3d(pcosh * pcosv, psinh * pcosv, psinv);
						glNormal3d(cosh * pcosv, sinh * pcosv, psinv);
						glVertex3d(cosh * pcosv, sinh * pcosv, psinv);
						glNormal3d(cosh * cosv, sinh * cosv, sinv);
						glVertex3d(cosh * cosv, sinh * cosv, sinv);
						glNormal3d(pcosh * cosv, psinh * cosv, sinv);
						glVertex3d(pcosh * cosv, psinh * cosv, sinv);

						// bottom quad
						glNormal3d(pcosh * pcosv, psinh * pcosv, -psinv);
						glVertex3d(pcosh * pcosv, psinh * pcosv, -psinv);
						glNormal3d(pcosh * cosv, psinh * cosv, -sinv);
						glVertex3d(pcosh * cosv, psinh * cosv, -sinv);
						glNormal3d(cosh * cosv, sinh * cosv, -sinv);
						glVertex3d(cosh * cosv, sinh * cosv, -sinv);
						glNormal3d(cosh * pcosv, sinh * pcosv, -psinv);
						glVertex3d(cosh * pcosv, sinh * pcosv, -psinv);
					}
					pcosh = cosh;
					psinh = sinh;
				}
			}
			pcosv = cosv;
			psinv = sinv;
		}

		glEnd();

		glEndList();
	}

	glPopMatrix();
}

void CApp::OnNewOrOpen(bool open, int res)
{
	//PythonOnNewOrOpen(open, res);

	ObserversOnChange(&m_objects, NULL, NULL);
}

void CApp::OnBeforeNewOrOpen(bool open, int res)
{
	for (std::list< void(*)(int, int) >::iterator It = m_beforeneworopen_callbacks.begin(); It != m_beforeneworopen_callbacks.end(); It++)
	{
		void(*callbackfunc)(int, int) = *It;
		(*callbackfunc)(open ? 1 : 0, res);
	}
}

void CApp::OnBeforeFrameDelete(void)
{
	for (std::list< void(*)() >::iterator It = m_beforeframedelete_callbacks.begin(); It != m_beforeframedelete_callbacks.end(); It++)
	{
		void(*callbackfunc)() = *It;
		(*callbackfunc)();
	}
}


int CApp::CheckForModifiedDoc()
{
	// returns wxCANCEL if not OK to continue opening file
	if (!m_no_creation_mode && IsModified())
	{
		std::wstring str = std::wstring(L"Save changes to file") + L" " + m_filepath;
		MessageBox(str.c_str());

#if 0
		int res = wxMessageBox(str, wxMessageBoxCaptionStr, wxCANCEL | wxYES_NO | wxCENTRE);
		if (res == wxCANCEL || res == wxNO) return res;
		if (res == wxYES)
		{
			return (int)SaveFile(m_filepath.c_str(), true);
		}
#endif
	}
	return 0x04;
}


HeeksObj* CApp::GetIDObject(int type, int id)
{
	UsedIds_t::iterator FindIt1 = used_ids.find(type);
	if (FindIt1 == used_ids.end()) return(NULL);

	IdsToObjects_t &ids = FindIt1->second;
	IdsToObjects_t::iterator FindIt2 = ids.find(id);

	if (FindIt2 == ids.end())return NULL;

	std::list<HeeksObj*> &list = FindIt2->second;
	return list.back();
}

std::list<HeeksObj*> CApp::GetIDObjects(int type, int id)
{
	std::list<HeeksObj *> results;

	UsedIds_t::iterator FindIt1 = used_ids.find(type);
	if (FindIt1 == used_ids.end()) return results;

	IdsToObjects_t &ids = FindIt1->second;
	IdsToObjects_t::iterator FindIt2 = ids.find(id);

	if (FindIt2 == ids.end()) return results;
	return FindIt2->second;
}

void CApp::SetObjectID(HeeksObj* object, int id)
{
	if (object->UsesID())
	{
		object->m_id = id;
		GroupId_t id_group_type = object->GetIDGroupType();

		UsedIds_t::iterator FindIt1 = used_ids.find(id_group_type);
		if (FindIt1 == used_ids.end())
		{
			// add a new map
			std::list<HeeksObj*> empty_list;
			empty_list.push_back(object);
			IdsToObjects_t empty_map;
			empty_map.insert(std::make_pair(id, empty_list));
			FindIt1 = used_ids.insert(std::make_pair(id_group_type, empty_map)).first;
		}
		else
		{

			IdsToObjects_t &map = FindIt1->second;
			IdsToObjects_t::iterator FindIt2 = map.find(id);
			if (FindIt2 == map.end())
			{
				std::list<HeeksObj*> empty_list;
				empty_list.push_back(object);
				map.insert(std::make_pair(id, empty_list));
			}
			else
			{
				std::list<HeeksObj*> &list = FindIt2->second;
				list.push_back(object);
			}
		}
	}
}

int CApp::GetNextID(int id_group_type)
{
	UsedIds_t::iterator FindIt1 = used_ids.find(id_group_type);
	if (FindIt1 == used_ids.end())return 1;

	std::map< int, int >::iterator FindIt2 = next_id_map.find(id_group_type);

	IdsToObjects_t &map = FindIt1->second;

	if (FindIt2 == next_id_map.end())
	{
		// add a new int
		int next_id = map.begin()->first + 1;
		FindIt2 = next_id_map.insert(std::make_pair(id_group_type, next_id)).first;
	}

	int &next_id = FindIt2->second;

	while (map.find(next_id) != map.end())next_id++;
	return next_id;
}

void CApp::RemoveID(HeeksObj* object)
{
	int id_group_type = object->GetIDGroupType();

	UsedIds_t::iterator FindIt1 = used_ids.find(id_group_type);
	if (FindIt1 == used_ids.end())return;
	std::map< int, int >::iterator FindIt2 = next_id_map.find(id_group_type);

	IdsToObjects_t &map = FindIt1->second;
	if (FindIt2 != next_id_map.end())
	{
		IdsToObjects_t::iterator FindIt3 = map.find(object->m_id);
		if (FindIt3 != map.end())
		{
			std::list<HeeksObj*> &list = FindIt3->second;
			list.remove(object);
			if (list.size() == 0)map.erase(object->m_id);
		}
	} // End if - then
}

void CApp::ResetIDs()
{
	used_ids.clear();
	next_id_map.clear();
}

void CApp::RegisterOnGLCommands(void(*callbackfunc)())
{
	m_on_glCommands_list.push_back(callbackfunc);
}

void CApp::RemoveOnGLCommands(void(*callbackfunc)())
{
	m_on_glCommands_list.remove(callbackfunc);
}

void CApp::RegisterOnSaveFn(void(*callbackfunc)(bool from_changed_prompt))
{
	m_on_save_callbacks.push_back(callbackfunc);
}

void CApp::RegisterIsModifiedFn(bool(*callbackfunc)())
{
	m_is_modified_callbacks.push_back(callbackfunc);
}

void CApp::CreateTransformGLList(const std::list<HeeksObj*>& list, bool show_grippers_on_drag){
	DestroyTransformGLList();
	m_transform_gl_list = glGenLists(1);
	glNewList(m_transform_gl_list, GL_COMPILE);
	std::list<HeeksObj *>::const_iterator It;
	for (It = list.begin(); It != list.end(); It++){
		(*It)->glCommands(false, true, false);
	}
	glDisable(GL_DEPTH_TEST);
	if (show_grippers_on_drag)m_marked_list->GrippersGLCommands(false, false);
	glEnable(GL_DEPTH_TEST);
	glEndList();
}

void CApp::DestroyTransformGLList(){
	if (m_transform_gl_list)
	{
		glDeleteLists(m_transform_gl_list, 1);
	}
	m_transform_gl_list = 0;
}


void CApp::create_font()
{
	if (m_gl_font_initialized)
		return;
	std::wstring fstr = GetResFolder() + L"/bitmaps/font.glf";

	glGenTextures(2, m_font_tex_number);

	//Create our glFont from verdana.glf, using texture 1
	m_gl_font.Create((char*)Ttc(fstr.c_str()), m_font_tex_number[0], m_font_tex_number[1]);
	m_gl_font_initialized = true;
}

void CApp::render_text(const wchar_t* str, bool select)
{
	//Needs to be called before text output
	create_font();
	EnableBlend();
	glEnable(GL_TEXTURE_2D);
	glDepthMask(0);
	glDisable(GL_POLYGON_OFFSET_FILL);
	m_gl_font.Begin(select);

	//Draws text with a glFont
	m_gl_font.DrawString(str, get_text_scale(), 0.0f, 0.0f);

	glDepthMask(1);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_TEXTURE_2D);
	DisableBlend();
}

bool CApp::get_text_size(const wchar_t* str, float* width, float* height)
{
	create_font();

	std::pair<int, int> size;
	m_gl_font.GetStringSize(str, &size);
	*width = (float)(size.first) * get_text_scale();
	*height = (float)(size.second) * get_text_scale();

	return true;
}

float CApp::get_text_scale()
{
	return 21.0f / m_gl_font.GetTexHeight();
}

void CApp::render_screen_text2(const wchar_t* str, bool select)
{
	size_t n = wcslen(str);

	wchar_t buffer[1024];


	int j = 0;
	const wchar_t* newlinestr = L"\n";
	wchar_t newline = newlinestr[0];

	for (size_t i = 0; i<n; i++)
	{
		buffer[j] = str[i];
		j++;
		if (str[i] == newline || i == n - 1 || j == 1023){
			buffer[j] = 0;
			render_text(buffer, select);
			if (str[i] == newline)glTranslated(0.0, -2.2, 1.0);
			j = 0;
		}
	}
}

void CApp::render_screen_text(const wchar_t* str1, const wchar_t* str2, bool select)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	m_current_viewport->SetIdentityProjection();
	background_color[0].best_black_or_white().glColor();
	int w, h;
	m_current_viewport->GetViewportSize(&w, &h);
	glTranslated(2.0, h - 1.0, 0.0);

	glScaled(10.0, 10.0, 0);
	render_screen_text2(str1, select);

	glScaled(0.612, 0.612, 0);
	render_screen_text2(str2, select);

	//Even though this is in reverse order, the different matrices have different stacks, and we want to exit here in the modelview
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void CApp::EnableBlend()
{
	if (!m_antialiasing)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	}
}

void CApp::DisableBlend()
{
	if (!m_antialiasing)glDisable(GL_BLEND);
}

void CApp::render_screen_text_at(const wchar_t* str1, double scale, double x, double y, double theta, bool select)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	m_current_viewport->SetIdentityProjection();
	background_color[0].best_black_or_white().glColor();
	int w, h;
	m_current_viewport->GetViewportSize(&w, &h);
	glTranslated(x, y, 0.0);

	glScaled(scale, scale, 0);
	glRotated(theta, 0, 0, 1);
	render_screen_text2(str1, select);

	//Even though this is in reverse order, the different matrices have different stacks, and we want to exit here in the modelview
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void CApp::RegisterOnBuildTexture(void(*callbackfunc)())
{
	m_on_build_texture_callbacks.push_back(callbackfunc);
}

void CApp::RegisterOnBeforeNewOrOpen(void(*callbackfunc)(int, int))
{
	m_beforeneworopen_callbacks.push_back(callbackfunc);
}

bool CApp::RegisterFileOpenHandler(const std::list<std::wstring> file_extensions, FileOpenHandler_t fileopen_handler)
{
	std::set<std::wstring> valid_extensions;

	// For Linux, where the file system supports case-sensitive file names, we should expand
	// the extensions to include uppercase and lowercase and add them to our set.

	for (std::list<std::wstring>::const_iterator l_itExtension = file_extensions.begin(); l_itExtension != file_extensions.end(); l_itExtension++)
	{
		std::wstring extension(*l_itExtension);

		// Make sure the calling routine didn't add the '.'
		if (startsWith(extension, L"."))
		{
			extension.erase(0, 1);
		}

#ifndef WIN32
		lowerCase(extension);
		valid_extensions.insert(extension);

		upperCase(extension);
		valid_extensions.insert(extension);
#else
		lowerCase(extension);
		valid_extensions.insert(extension);
#endif // WIN32
	} // End for

	for (std::set<std::wstring>::iterator itExtension = valid_extensions.begin(); itExtension != valid_extensions.end(); itExtension++)
	{
		if (m_fileopen_handlers.find(*itExtension) != m_fileopen_handlers.end())
		{
//			MessageBox(L"Aborting file-open handler registration for extension %s as it has already been registered\n", Ttc(itExtension->c_str()));
			return(false);
		}
	}

	// We must not have seen these extensions before.  Go ahead and register them.
	for (std::set<std::wstring>::iterator itExtension = valid_extensions.begin(); itExtension != valid_extensions.end(); itExtension++)
	{
		m_fileopen_handlers.insert(std::make_pair(*itExtension, fileopen_handler));
	}

	return(true);
}

bool CApp::UnregisterFileOpenHandler(void(*fileopen_handler)(const wchar_t *path))
{
	std::list<FileOpenHandlers_t::iterator> remove;
	for (FileOpenHandlers_t::iterator itHandler = m_fileopen_handlers.begin(); itHandler != m_fileopen_handlers.end(); itHandler++)
	{
		if (itHandler->second == fileopen_handler) remove.push_back(itHandler);
	}

	for (std::list<FileOpenHandlers_t::iterator>::iterator itRemove = remove.begin(); itRemove != remove.end(); itRemove++)
	{
		m_fileopen_handlers.erase(*itRemove);
	}

	return(remove.size() > 0);
}


void CApp::RegisterUnitsChangeHandler(void(*units_changed_handler)(const double value))
{
	m_units_changed_handlers.push_back(units_changed_handler);
}

void CApp::UnregisterUnitsChangeHandler(void(*units_changed_handler)(const double value))
{
	for (UnitsChangedHandlers_t::iterator itHandler = m_units_changed_handlers.begin(); itHandler != m_units_changed_handlers.end(); /* increment within loop */)
	{
		if (units_changed_handler == *itHandler)
		{
			itHandler = m_units_changed_handlers.erase(itHandler);
		}
		else
		{
			itHandler++;
		}
	}
}


void CApp::RegisterHeeksTypesConverter(std::wstring(*converter)(const int type))
{
	m_heeks_types_converters.push_back(converter);
}

void CApp::UnregisterHeeksTypesConverter(std::wstring(*converter)(const int type))
{
	for (HeeksTypesConverters_t::iterator itConverter = m_heeks_types_converters.begin(); itConverter != m_heeks_types_converters.end(); /* increment within loop */)
	{
		if (converter == *itConverter)
		{
			itConverter = m_heeks_types_converters.erase(itConverter);
		}
		else
		{
			itConverter++;
		}
	}
}

std::wstring HeeksCADType(const int type)
{
	switch (type)
	{
	case UnknownType:   return(L"Unknown");
	case DocumentType:   return(L"Document");
	case PointType:   return(L"Point");
	case LineType:   return(L"Line");
	case ArcType:   return(L"Arc");
	case ILineType:   return(L"ILine");
	case CircleType:   return(L"Circle");
	case GripperType:   return(L"Gripper");
	case StlSolidType:   return(L"StlSolid");
	case SketchType:   return(L"Sketch");
	case ImageType:   return(L"Image");
	case CoordinateSystemType:   return(L"CoordinateSystem");
	case TextType:   return(L"Text");
	case DimensionType:   return(L"Dimension");
	case RulerType:   return(L"Ruler");
	case XmlType:   return(L"Xml");
	case EllipseType:   return(L"Ellipse");
	case GroupType:   return(L"Group");
	case AngularDimensionType:   return(L"AngularDimension");
	case ObjectMaximumType:   return(L"ObjectMaximum");
	default:    return(L""); // Indicate that this routine could not find a conversion.
	} // End switch
} // End HeeksCADType() routine


std::wstring CApp::HeeksType(const int type) const
{

	for (HeeksTypesConverters_t::const_iterator itConverter = m_heeks_types_converters.begin(); itConverter != m_heeks_types_converters.end(); itConverter++)
	{
		std::wstring result = (*itConverter)(type);
		if (result.size() > 0)
		{
			return(result);
		}
	}

	wchar_t buf[32];
	swprintf(buf, 32, L"%d", type);
	return std::wstring(buf);
}


unsigned int CApp::GetIndex(HeeksObj *object)
{
	return m_marked_list->GetIndex(object);
}


void CApp::ReleaseIndex(unsigned int index)
{
	if (m_marked_list)m_marked_list->ReleaseIndex(index);
}

void CApp::RegisterOnRestoreDefaults(void(*callbackfunc)())
{
	m_on_restore_defaults_callbacks.push_back(callbackfunc);
}

void CApp::RestoreDefaults()
{
	HeeksConfig config;
	config.DeleteAll();
	for (std::list< void(*)() >::iterator It = m_on_restore_defaults_callbacks.begin(); It != m_on_restore_defaults_callbacks.end(); It++)
	{
		void(*callbackfunc)() = *It;
		(*callbackfunc)();
	}
	theApp.m_settings_restored = true;
}
