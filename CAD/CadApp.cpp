#include "stdafx.h"

#include "CadApp.h"
#include "Grid.h"
#include "InputMode.h"
#include "Material.h"
#include "Viewport.h"
#include "MagDragWindow.h"
#include "ViewRotating.h"
#include "ViewZooming.h"
#include "ViewPanning.h"
#include "DigitizeMode.h"
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
#include "Sketch.h"
#include "HLine.h"
#include "HArc.h"
#include "HPoint.h"
#include "CoordinateSystem.h"
#include "HCircle.h"
#include "HILine.h"
#include "HText.h"
#include "HeeksFont.h"
#include "Filter.h"

CCadApp *theApp = new CCadApp;

extern void PythonOnGLCommands();
extern bool PythonImportFile(const wchar_t* lowercase_extension, const wchar_t* filepath);
extern bool PythonExportFile(const wchar_t* lowercase_extension, const wchar_t* filepath);

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



CCadApp::CCadApp()
{
	m_version_number = std::wstring(HEEKSCAD_VERSION_MAIN) + L" " + HEEKSCAD_VERSION_SUB + L" 0";
	TiXmlBase::SetRequiredDecimalPlaces(DecimalPlaces(TOLERANCE));	 // Ensure we write XML in enough accuracy to be useful when re-read.

	m_sketch_reorder_tol = 0.01;
	m_view_units = 1.0;
	for (int i = 0; i<NUM_BACKGROUND_COLORS; i++)background_color[i] = HeeksColor(0, 0, 0);
	background_color[0] = HeeksColor(255, 255, 255);
	background_color[1] = HeeksColor(181, 230, 29);
	m_background_mode = BackgroundModeTwoColors;
	current_color = HeeksColor(0, 0, 0);
	input_mode_object = NULL;
	cur_mouse_pos = IPoint(0, 0);
	drag_gripper = NULL;
	cursor_gripper = NULL;
	magnification = new MagDragWindow();
	viewrotating = new ViewRotating;
	viewzooming = new ViewZooming;
	viewpanning = new ViewPanning;
	//m_select_mode = new CSelectMode();
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
	m_ruler = new HRuler();
	m_show_ruler = false;
	m_show_datum_coords_system = true;
	m_datum_coords_system_solid_arrows = true;
	m_in_OpenFile = false;
	m_transform_gl_list = 0;
	m_current_coordinate_system = NULL;
	m_mark_newly_added_objects = false;
	m_show_grippers_on_drag = true;
	m_extrude_removes_sketches = false;
	m_loft_removes_sketches = false;
	m_graphics_text_mode = GraphicsTextModeWithHelp;// GraphicsTextModeNone;
	m_file_open_or_import_type = FileOpenOrImportTypeOther;
	m_file_open_matrix = NULL;
	m_min_correlation_factor = 0.75;
	m_max_scale_threshold = 1.5;
	m_number_of_sample_points = 10;
	m_property_grid_validation = false;
	m_solid_view_mode = SolidViewFacesAndEdges;
	m_dragging_moves_objects = true;
	m_stl_solid_random_colors = false;
	m_svg_unite = false;
	m_stl_facet_tolerance = 0.1;
	m_icon_texture_number = 0;
	m_extrude_to_solid = true;
	m_revolve_angle = 360.0;
	m_fit_arcs_on_solid_outline = false;
	m_stl_save_as_binary = true;

	InitializeCreateFunctions();

	m_settings_restored = false;

	m_icon_texture_number = 0;
	m_settings_restored = false;

	m_current_viewport = NULL;
	m_observers_frozen = 0;
	frozen_selection_cleared = false;
	m_previous_input_mode = NULL;
	m_cur_xml_element = NULL;
	m_cur_xml_root = NULL;
}

CCadApp::~CCadApp()
{
	delete m_marked_list;
	m_marked_list = NULL;
	observers.clear();
	delete history;
	delete magnification;
	delete m_digitizing;
	delete viewrotating;
	delete viewzooming;
	delete viewpanning;
	m_ruler->m_index = 0;
	delete m_ruler;
}

void CCadApp::OnExit(){
	delete history;
	history = NULL;
}

extern void PythonOnSetInputMode();

void CCadApp::SetInputMode(CInputMode *new_mode){
	if(!new_mode)return;

	m_previous_input_mode = input_mode_object;
	new_mode->OnModeChange();
	input_mode_object = new_mode;

	RefreshInputCanvas();
}

void CCadApp::RestoreInputMode()
{
	if (m_previous_input_mode)
	{
		m_previous_input_mode->OnModeChange();
		input_mode_object = m_previous_input_mode;
		RefreshInputCanvas();
		m_previous_input_mode = NULL;
	}
}

void CCadApp::RefreshInputCanvas()
{
	PythonOnSetInputMode();
}

void CCadApp::CreateLights(void)
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

void CCadApp::DestroyLights(void)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_AUTO_NORMAL);
	glDisable(GL_NORMALIZE);
}

void CCadApp::Reset(){
	m_marked_list->Clear(true);
	m_marked_list->Reset();
	m_name_index.clear();
	Clear();
	ObserversClear();
	EndHistory();
	delete history;
	history = new MainHistory;
	m_current_coordinate_system = NULL;
	m_doing_rollback = false;
	Point3d vy(0, 1, 0), vz(0, 0, 1);
	m_current_viewport->m_view_point.SetView(vy, vz, 6);
	m_hidden_for_drag.clear();
	m_show_grippers_on_drag = true;
	if (m_ruler)delete m_ruler;
	m_ruler = new HRuler();
	RestoreInputMode();

	ResetIDs();
}

static HeeksObj* CreateHLine(){ HeeksColor c; return new HLine(Point3d(), Point3d(), &c); }
static HeeksObj* CreateHArc(){ HeeksColor c; return new HArc(Point3d(), Point3d(), Point3d(), Point3d(), &c); }
static HeeksObj* CreateHCircle(){ HeeksColor c; return new HCircle(); }
static HeeksObj* CreateHILine(){ HeeksColor c; return new HILine(Point3d(), Point3d(), &c); }
static HeeksObj* CreateHPoint(){ HeeksColor c; return new HPoint(Point3d(), &c); }
static HeeksObj* CreateCSketch(){ return new CSketch; }
static HeeksObj* CreateCStlSolid(){ return new CStlSolid; }
static HeeksObj* CreateCoordinateSystem(){ return new CoordinateSystem; }
static HeeksObj* CreateHText(){ HeeksColor c; return new HText(Matrix(), L"", &c, 0, 0); }

void CCadApp::InitializeCreateFunctions()
{
	// set up function map
	if(object_create_fn_map.size() == 0)
	{
		object_create_fn_map.insert(std::pair< std::string, HeeksObj*(*)() >("Line", CreateHLine));
		object_create_fn_map.insert(std::pair< std::string, HeeksObj*(*)() >("Arc", CreateHArc));
		object_create_fn_map.insert(std::pair< std::string, HeeksObj*(*)() >("Circle", CreateHCircle));
		object_create_fn_map.insert(std::pair< std::string, HeeksObj*(*)() >("InfiniteLine", CreateHILine));
		object_create_fn_map.insert(std::pair< std::string, HeeksObj*(*)() >("Point", CreateHPoint));
		object_create_fn_map.insert(std::pair< std::string, HeeksObj*(*)() >("Sketch", CreateCSketch));
		object_create_fn_map.insert(std::pair< std::string, HeeksObj*(*)() >("STLSolid", CreateCStlSolid));
		object_create_fn_map.insert(std::pair< std::string, HeeksObj*(*)() >("CoordinateSystem", CreateCoordinateSystem));
		object_create_fn_map.insert(std::pair< std::string, HeeksObj*(*)() >("Text", CreateHText));
	}
}

HeeksObj* CreatePyObjectWithName(const std::string& name);

HeeksObj* CCadApp::CreateObjectOfType(const std::string& name)
{
	std::map< std::string, HeeksObj*(*)() >::iterator FindIt = object_create_fn_map.find(name);
	if (FindIt != object_create_fn_map.end())
		return (*(FindIt->second))();

	HeeksObj* object = CreatePyObjectWithName(name);

	if (object)
		return object;

	return new HXml;
}

HeeksObj* CCadApp::ReadXMLElement(TiXmlElement* pElem)
{
	std::string name(pElem->Value());

	HeeksObj* object = CreateObjectOfType(name);

	if (object == NULL)
		return NULL;

	object->ReadFromXML(pElem);
	object->ReloadPointers();

	return object;
}

void CCadApp::ObjectWriteToXML(HeeksObj *object, TiXmlElement *element)
{
	if (object->UsesID())element->SetAttribute("id", object->m_id);
	if (!object->m_visible)element->SetAttribute("vis", 0);
}

void CCadApp::ObjectReadFromXML(HeeksObj *object, TiXmlElement* element)
{
	// get the attributes
	for (TiXmlAttribute* a = element->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if (object->UsesID() && name == "id"){ object->SetID(a->IntValue()); }
		if (name == "vis"){ object->m_visible = (a->IntValue() != 0); }
	}
}


static bool ImportObjectInto(HeeksObj* object, HeeksObj* add_to, HeeksObj* paste_before, bool undoable)
{
	if (object->OneOfAKind())
	{
		for (HeeksObj* child = add_to->GetFirstChild(); child; child = add_to->GetNextChild())
		{
			if (child->GetType() == object->GetType())
			{
				// there is already a child of the same type, so remove that one and add this one
				HeeksObj* next_child = add_to->GetNextChild();
				if (undoable)
				{
					theApp->DeleteUndoably(child);
					theApp->AddUndoably(object, add_to, next_child);
				}
				else
				{
					add_to->Remove(child);
					add_to->Add(object, next_child);
				}

				return false;
			}
		}
	}

	if (undoable)
	{
		theApp->AddUndoably(object, add_to, paste_before);
	}
	else
	{
		add_to->Add(object, paste_before);
	}
	return true;
}

void CCadApp::OpenXMLFile(const wchar_t *filepath, HeeksObj* paste_into, HeeksObj* paste_before, bool call_was_added, bool show_error, bool undoable)
{
	TiXmlDocument doc(Ttc(filepath));
	if (!doc.LoadFile())
	{
		if (show_error && doc.Error())
		{
			std::wstring msg(filepath);
			msg.append(L": ");
			msg.append(Ctt(doc.ErrorDesc()));
			DoMessageBox(msg.c_str());
		}
		return;
	}

	undoable_in_OpenXMLFile = undoable;
	paste_into_for_OpenXMLFile = paste_into;

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	m_cur_xml_root = &doc;

	pElem = hDoc.FirstChildElement().Element();
	if (!pElem) return;
	std::string name(pElem->Value());
	if (name == "HeeksCAD_Document")
	{
		m_cur_xml_root = pElem;
	}

	ObjectReferences_t unique_set;

	char oldlocale[1000];
	strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

	std::list<HeeksObj*> objects;
	for (pElem = m_cur_xml_root->FirstChildElement(); pElem; pElem = pElem->NextSiblingElement())
	{
		HeeksObj* object = ReadXMLElement(pElem);
		if (object)
		{
			if (object->AddOnlyChildrenOnReadXML())
			{
				std::list<HeeksObj*> children = object->GetChildren();
				for (std::list<HeeksObj*>::iterator It = children.begin(); It != children.end(); It++)
				{
					HeeksObj* child = *It;
					objects.push_back(child);
				}
			}
			else
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
					if (ImportObjectInto(object, add_to, paste_before, undoable))
						if (call_was_added)WasAdded(object);
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

/* static */ void CCadApp::OpenSVGFile(const wchar_t *filepath)
{
	char oldlocale[1000];
	strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

	CSvgRead svgread(filepath, true, theApp->m_svg_unite);
	setlocale(LC_NUMERIC, oldlocale);
}

/* static */ void CCadApp::OpenSTLFile(const wchar_t *filepath)
{
	char oldlocale[1000];
	strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

	HeeksColor c(128, 128, 128);
	CStlSolid* new_object = new CStlSolid(filepath, &c);
	theApp->AddUndoably(new_object, NULL, NULL);
	setlocale(LC_NUMERIC, oldlocale);
}

/* static */ void CCadApp::OpenDXFFile(const wchar_t *filepath )
{
	char oldlocale[1000];
	strcpy(oldlocale, setlocale(LC_NUMERIC, "C"));

	HeeksDxfRead dxf_file(filepath, true);
	dxf_file.DoRead();
	setlocale(LC_NUMERIC, oldlocale);
}

bool CCadApp::OpenImageFile(const wchar_t *filepath)
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

void CCadApp::OnNewButton()
{
#if 0
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
#endif
}

void CCadApp::OnOpenButton()
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

bool CCadApp::OpenFile(const wchar_t *filepath, bool import_not_open, HeeksObj* paste_into, HeeksObj* paste_before, bool retain_filename /* = true */)
{
	bool history_started = false;
	if (import_not_open)
	{
		StartHistory();
		history_started = true;
	}

	m_in_OpenFile = true;
	m_file_open_or_import_type = FileOpenOrImportTypeOther;
	Matrix file_open_matrix;
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

	if (endsWith(wf, L".heeks") || endsWith(wf, L".xml"))
	{
		m_file_open_or_import_type = FileOpenTypeHeeks;
		if (import_not_open)
			m_file_open_or_import_type = FileImportTypeHeeks;
		OpenXMLFile(filepath, paste_into, paste_before, import_not_open, false, import_not_open);
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
	else if (endsWith(wf, L".svg"))
	{
		OpenSVGFile(filepath);
	}
	else if (endsWith(wf, L".stl"))
	{
		OpenSTLFile(filepath);
	}
	else if(!PythonImportFile(extension.c_str(), filepath))
	{
		// error
		std::wstring str = std::wstring(L"Invalid file type chosen");
		DoMessageBox(str.c_str());
		open_succeeded = false;
	}

	m_file_open_matrix = NULL;
	m_in_OpenFile = false;

	if (history_started)EndHistory();
	else this->ObserversOnChange(&m_objects, NULL, NULL);

	return open_succeeded;
}

static void WriteDXFEntity(HeeksObj* object, CDxfWrite& dxf_file, const std::wstring parent_layer_name)
{
	std::wstring layer_name;

	if (parent_layer_name.size() == 0)
	{
		wchar_t ln[1024];
		swprintf(ln,1024, L"%d", object->m_id);
		layer_name.append(ln);
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
		dxf_file.WriteLine(l->A.getBuffer(), l->B.getBuffer(), Ttc(layer_name.c_str()), l->m_thickness, l->m_extrusion_vector);
	}
	break;
	case PointType:
	{
		HPoint* p = (HPoint*)object;
		dxf_file.WritePoint(p->m_p.getBuffer(), Ttc(layer_name.c_str()));
	}
	break;
	case ArcType:
	{
		HArc* a = (HArc*)object;
		bool dir = a->m_axis.z > 0;
		dxf_file.WriteArc(a->A.getBuffer(), a->B.getBuffer(), a->C.getBuffer(), dir, Ttc(layer_name.c_str()), a->m_thickness, a->m_extrusion_vector);
	}
	break;
	// to do
#if 0
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
#endif
	case CircleType:
	{
		HCircle* cir = (HCircle*)object;
		dxf_file.WriteCircle(cir->m_c.getBuffer(), cir->m_radius, Ttc(layer_name.c_str()), cir->m_thickness, cir->m_extrusion_vector);
	}
	break;
	default:
	{
		if (parent_layer_name.length() == 0)
		{
			layer_name.clear();
			if ((object->GetShortString() != NULL) && (std::wstring(object->GetTypeString()) != std::wstring(object->GetShortString())))
			{
				layer_name.append( object->GetShortString());
			}
			else
			{
				wchar_t ln[1024];
				swprintf(ln,1024, L"%d", object->m_id);
				layer_name.append(ln);  // Use the ID as a layer name so that it's unique.
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
}

void CCadApp::SaveDXFFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath)
{
	CDxfWrite dxf_file(Ttc(filepath));
	if (dxf_file.Failed())
	{
		std::wstring str = std::wstring(L"couldn't open file") + filepath;
		DoMessageBox(str.c_str());
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

static wofstream* ofs_for_write_stl_triangle = NULL;
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

void CCadApp::SaveSTLFileBinary(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance, double* scale)
{
#ifdef WIN32
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

		Point3d p0(t.x[0], t.x[1], t.x[2]);
		Point3d p1(t.x[3], t.x[4], t.x[5]);
		Point3d p2(t.x[6], t.x[7], t.x[8]);
		Point3d v1(p0, p1);
		Point3d v2(p0, p2);
		float n[3] = { 0.0f, 0.0f, 1.0f };
		try
		{
			Point3d norm = (v1 ^ v2).Normalized();
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
}

void CCadApp::SaveSTLFileAscii(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance, double* scale)
{
#ifdef WIN32
	wofstream ofs(filepath);
#else
	wofstream ofs(Ttc(filepath));
#endif

	if (!ofs)
	{
		std::wstring str = std::wstring(L"couldn't open file") + L" - " + filepath;
		DoMessageBox(str.c_str());
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
	Point3d m_p;
	ObjFileVertex(int index, const Point3d& p) :m_index(index), m_p(p){}
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

	int InsertVertex(const Point3d& p)
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
		int v0 = InsertVertex(Point3d(t[0], t[1], t[2]));
		int v1 = InsertVertex(Point3d(t[3], t[4], t[5]));
		int v2 = InsertVertex(Point3d(t[6], t[7], t[8]));
		m_tris.push_back(ObjFileTriangle(v0, v1, v2));
	}

	int GetIndex(const Point3d& p)
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
#ifdef WIN32
		wofstream ofs(filepath);
#else
		wofstream ofs(Ttc(filepath.c_str()));
#endif

		if (!ofs)
		{
			std::wstring str = std::wstring(L"couldn't open file") + L" - " + filepath;
			theApp->DoMessageBox(str.c_str());
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

void CCadApp::SaveOBJFileAscii(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance, double* scale)
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

void CCadApp::SaveSTLFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance, double* scale, bool binary)
{
	if (binary)SaveSTLFileBinary(objects, filepath, facet_tolerance, scale);
	else SaveSTLFileAscii(objects, filepath, facet_tolerance, scale);
}

void CCadApp::SaveCPPFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance)
{
#ifdef WIN32
	wofstream ofs(filepath);
#else
	wofstream ofs(Ttc(filepath));
#endif

	if (!ofs)
	{
		std::wstring str = std::wstring(L"couldn't open file") + L" - " + filepath;
		DoMessageBox(str.c_str());
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

void CCadApp::SavePyFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath, double facet_tolerance)
{
#ifdef WIN32
	wofstream ofs(filepath);
#else
	wofstream ofs(Ttc(filepath));
#endif

	if (!ofs)
	{
		std::wstring str = std::wstring(L"couldn't open file") + L" - " + filepath;
		DoMessageBox(str.c_str());
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

extern void PythonOnEndXmlWrite();

void CCadApp::SaveXMLFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath, bool for_clipboard)
{
	// write an xml file
	TiXmlDocument doc;
	const char *l_pszVersion = "1.0";
	const char *l_pszEncoding = "UTF-8";
	const char *l_pszStandalone = "";

	TiXmlDeclaration* decl = new TiXmlDeclaration(l_pszVersion, l_pszEncoding, l_pszStandalone);
	doc.LinkEndChild(decl);

	m_cur_xml_root = &doc;
	if (!for_clipboard)
	{
		m_cur_xml_root = new TiXmlElement("HeeksCAD_Document");
		doc.LinkEndChild(m_cur_xml_root);
	}

	// loop through all the objects writing them
	for (std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
	{
		HeeksObj* object = *It;
		object->WriteXML(m_cur_xml_root);
	}

	PythonOnEndXmlWrite();

	if(!doc.SaveFile(Ttc(filepath)))
		this->DoMessageBox((std::wstring(L"failed to open file for saving: ") + filepath).c_str());
}

bool CCadApp::SaveFile(const wchar_t *filepath, const std::list<HeeksObj*>* objects)
{
	if (objects == NULL)
		objects = &m_objects;

	std::wstring wf(filepath);
	lowerCase(wf);

	if (endsWith(wf, L".heeks") || endsWith(wf, L".xml"))
	{
		SaveXMLFile(*objects, filepath);
	}
	else if (endsWith(wf, L".dxf"))
	{
		SaveDXFFile(*objects, filepath);
	}
	else if (endsWith(wf, L".stl"))
	{
		SaveSTLFile(*objects, filepath, -1.0, NULL, m_stl_save_as_binary);
	}
	else if (endsWith(wf, L".cpp"))
	{
		SaveCPPFile(*objects, filepath);
	}
	else if (endsWith(wf, L".obj"))
	{
		SaveOBJFileAscii(*objects, filepath);
	}
	else if (endsWith(wf, L".py"))
	{
		SavePyFile(*objects, filepath);
	}
	else
	{
		std::wstring str = std::wstring(L"Invalid file type chosen ") + filepath;
		DoMessageBox(str.c_str());
		return false;
	}

	return true;
}

extern void PythonOnRepaint(bool soon);

void CCadApp::Repaint(bool soon)
{
	PythonOnRepaint(soon);
}

#if 0
	if (soon)m_frame->m_graphics->RefreshSoon();
	else m_frame->m_graphics->Refresh();
#endif

void CCadApp::RecalculateGLLists()
{
	for (HeeksObj* object = GetFirstChild(); object; object = GetNextChild()){
		object->KillGLLists();
	}
}

void CCadApp::RenderDatumOrCurrentCoordSys()
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

void CCadApp::glCommandsAll(const CViewPoint &view_point)
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
	PythonOnGLCommands();
	glEnable(GL_POLYGON_OFFSET_FILL);

	input_mode_object->OnRender();
	if (m_transform_gl_list)
	{
		glPushMatrix();
		double m[16];
		m_drag_matrix.GetTransposed(m);
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
		render_screen_text(screen_text1.c_str(), screen_text2.c_str());
	}
}

void CCadApp::OnInputModeTitleChanged()
{
	if (m_graphics_text_mode != GraphicsTextModeNone)
	{
		Repaint();
	}
}

void CCadApp::OnInputModeHelpTextChanged()
{
	if (m_graphics_text_mode == GraphicsTextModeWithHelp)
	{
		Repaint();
	}
}

void CCadApp::glCommands(bool select, bool marked, bool no_color)
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

double CCadApp::GetPixelScale(void){
	return m_current_viewport->m_view_point.m_pixel_scale;
}

bool CCadApp::IsModified(void){
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

void CCadApp::SetAsModified(){
	history->SetAsModified();
}

void CCadApp::SetLikeNewFile(void){
	history->SetLikeNewFile();
}

void CCadApp::ClearHistory(void){
	history->ClearFromFront();
	history->SetLikeNewFile();
}

void CCadApp::DoUndoable(Undoable *u)
{
	history->DoUndoable(u);
}

bool CCadApp::RollBack(void)
{
	ObserversFreeze();
	m_doing_rollback = true;
	bool result = history->InternalRollBack();
	m_doing_rollback = false;
	ObserversThaw();
	return result;
}

bool CCadApp::CanUndo(void)
{
	return history->CanUndo();
}

bool CCadApp::CanRedo(void)
{
	return history->CanRedo();
}

bool CCadApp::RollForward(void)
{
	ObserversFreeze();
	m_doing_rollback = true;
	bool result = history->InternalRollForward();
	m_doing_rollback = false;
	ObserversThaw();
	return result;
}

void CCadApp::StartHistory()
{
	history->StartHistory();
}

void CCadApp::EndHistory(void)
{
	history->EndHistory();
}

void CCadApp::ClearRollingForward(void)
{
	history->ClearFromCurPos();
}

void CCadApp::RegisterObserver(Observer* observer)
{
	if (observer == NULL) return;
	observers.insert(observer);
	observer->OnChanged(&m_objects, NULL, NULL);
}

void CCadApp::RemoveObserver(Observer* observer){
	observers.erase(observer);
}

void CCadApp::ObserversOnChange(const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed, const std::list<HeeksObj*>* modified){
	if (m_observers_frozen > 0)
	{
		if (added)for (std::list<HeeksObj*>::const_iterator It = added->begin(); It != added->end(); It++)frozen_added.push_back(*It);
		if (removed)for (std::list<HeeksObj*>::const_iterator It = removed->begin(); It != removed->end(); It++)frozen_removed.push_back(*It);
		if (modified)for (std::list<HeeksObj*>::const_iterator It = modified->begin(); It != modified->end(); It++)frozen_modified.push_back(*It);
	}
	else
	{
		std::set<Observer*>::iterator It;
		for (It = observers.begin(); It != observers.end(); It++){
			Observer *ov = *It;
			ov->OnChanged(added, removed, modified);
		}
	}
}

void CCadApp::ObserversMarkedListChanged(bool selection_cleared, const std::list<HeeksObj*>* added, const std::list<HeeksObj*>* removed){
	if (m_observers_frozen > 0)
	{
		if (selection_cleared)
		{
			frozen_selection_cleared = true;
			frozen_selection_added.clear();
			frozen_selection_removed.clear();
		}
		if (added)for (std::list<HeeksObj*>::const_iterator It = added->begin(); It != added->end(); It++)frozen_selection_added.push_back(*It);
		if (removed)for (std::list<HeeksObj*>::const_iterator It = removed->begin(); It != removed->end(); It++)frozen_selection_removed.push_back(*It);
	}
	else
	{
		std::set<Observer*>::iterator It;
		for (It = observers.begin(); It != observers.end(); It++){
			Observer *ov = *It;
			ov->WhenMarkedListChanges(selection_cleared, added, removed);
		}
	}
}

void CCadApp::ObserversFreeze()
{
	m_observers_frozen++;
}

void CCadApp::ObserversThaw()
{
	m_observers_frozen--;
	if (m_observers_frozen == 0)
	{
		// call observers with any stored changes
		if ((frozen_added.size() > 0) || (frozen_removed.size() > 0) || (frozen_modified.size() > 0))
		{
			ObserversOnChange(&frozen_added, &frozen_removed, &frozen_modified);
		}
		frozen_added.clear();
		frozen_removed.clear();
		frozen_modified.clear();
		if (frozen_selection_cleared || (frozen_selection_added.size() > 0) || (frozen_selection_removed.size() > 0))
		{
			ObserversMarkedListChanged(frozen_selection_cleared, &frozen_selection_added, &frozen_selection_removed);
		}
		frozen_selection_cleared = false;
		frozen_selection_added.clear();
		frozen_selection_removed.clear();
	}
	else if (m_observers_frozen < 0)
	{
		// this shouldn't happen
		m_observers_frozen = 0;
	}
}

void CCadApp::ObserversClear()
{
	std::set<Observer*>::iterator It;
	for (It = observers.begin(); It != observers.end(); It++){
		Observer *ov = *It;
		ov->Clear();
	}
	frozen_added.clear();
	frozen_removed.clear();
	frozen_modified.clear();
	frozen_selection_cleared = false;
	frozen_selection_added.clear();
	frozen_selection_removed.clear();
	m_observers_frozen = 0;
}

bool CCadApp::Add(HeeksObj *object, HeeksObj* prev_object)
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

void CCadApp::Remove(HeeksObj* object)
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

void CCadApp::Remove(std::list<HeeksObj*> objects)
{
	ObjList::Remove(objects);
}

void CCadApp::AddUndoably(HeeksObj *object, HeeksObj* owner, HeeksObj* prev_object)
{
	if (object == NULL)return;
	if (owner == NULL)owner = this;
	AddObjectTool *undoable = new AddObjectTool(object, owner, prev_object);
	DoUndoable(undoable);
}

void CCadApp::AddUndoably(const std::list<HeeksObj*> &list, HeeksObj* owner)
{
	if (list.size() == 0)return;
	if (owner == NULL)owner = this;
	AddObjectsTool *undoable = new AddObjectsTool(list, owner);
	DoUndoable(undoable);
}

void CCadApp::DeleteUndoably(HeeksObj *object){
	if (object == NULL)return;
	if (!object->CanBeRemoved())return;
	RemoveObjectTool *undoable = new RemoveObjectTool(object);
	DoUndoable(undoable);
}

void CCadApp::DeleteUndoably(const std::list<HeeksObj*>& list)
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

void CCadApp::CopyUndoably(HeeksObj* object, HeeksObj* copy_with_new_data)
{
	DoUndoable(new CopyObjectUndoable(object, copy_with_new_data));
}

void CCadApp::TransformUndoably(HeeksObj *object, const Matrix &m)
{
	if (!object)return;
	Matrix mat = Matrix(m);
	Matrix im = mat.Inverse();
	TransformTool *undoable = new TransformTool(object, mat, im);
	DoUndoable(undoable);
}

void CCadApp::TransformUndoably(const std::list<HeeksObj*> &list, const Matrix &m)
{
	if (list.size() == 0)return;
	Matrix mat = Matrix(m);
	Matrix im = mat.Inverse();
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

void CCadApp::ReverseUndoably(HeeksObj *object)
{
#if 0
	DoUndoable(new ReverseUndoable(object));
#endif
}

void CCadApp::Transform(std::list<HeeksObj*> objects, const Matrix& m)
{
	std::list<HeeksObj*>::iterator it;
	for (it = objects.begin(); it != objects.end(); ++it)
	{
		(*it)->Transform(m);
	}
}

void CCadApp::WasModified(HeeksObj *object)
{
	std::list<HeeksObj*> list;
	list.push_back(object);
	WereModified(list);
}

void CCadApp::WasAdded(HeeksObj *object)
{
	std::list<HeeksObj*> list;
	list.push_back(object);
	WereAdded(list);
}

void CCadApp::WasRemoved(HeeksObj *object)
{
	std::list<HeeksObj*> list;
	list.push_back(object);
	WereRemoved(list);
}

void CCadApp::WereModified(const std::list<HeeksObj*>& list)
{
	if (list.size() == 0) return;
	HeeksObj* object = *(list.begin());
	if (object == NULL) return;
	ObserversOnChange(NULL, NULL, &list);
	SetAsModified();
}

void CCadApp::WereAdded(const std::list<HeeksObj*>& list)
{
	if (list.size() == 0) return;
	HeeksObj* object = *(list.begin());
	if (object == NULL) return;
	ObserversOnChange(&list, NULL, NULL);
	SetAsModified();
}

void CCadApp::WereRemoved(const std::list<HeeksObj*>& list)
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

Matrix matrix_for_GetDrawMatrix;

Matrix* CCadApp::GetDrawMatrix(bool get_the_appropriate_orthogonal)
{
	if (get_the_appropriate_orthogonal){
		// choose from the three orthoganal possibilities, the one where it's z-axis closest to the camera direction
		Point3d vx, vy;
		m_current_viewport->m_view_point.GetTwoAxes(vx, vy, false, 0);
		Point3d o(0, 0, 0);
		if (m_current_coordinate_system)o = o.Transformed(m_current_coordinate_system->GetMatrix());
		matrix_for_GetDrawMatrix = Matrix(o, vx, vy);
	}
	else
	{
		matrix_for_GetDrawMatrix = Matrix();
		if (m_current_coordinate_system)matrix_for_GetDrawMatrix = m_current_coordinate_system->GetMatrix();
	}

	return &matrix_for_GetDrawMatrix;
}

void CCadApp::DrawObjectsOnFront(const std::list<HeeksObj*> &list, bool do_depth_testing)
{
	m_current_viewport->DrawObjectsOnFront(list, do_depth_testing);
}

CInputMode* CCadApp::GetDigitizing(){ return m_digitizing; }


extern void PythonOnMessageBox(const wchar_t* message);

void CCadApp::DoMessageBox(const wchar_t* message)
{
#ifdef WIN32
	::MessageBox(NULL, message, L"Message", MB_OK);
#else
	PythonOnMessageBox(message);
#endif
}

void CCadApp::GetOptions(std::list<Property *> *list)
{
	// maybe not
}


void CCadApp::DeleteMarkedItems()
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

void CCadApp::glColorEnsuringContrast(const HeeksColor &c)
{
	if (c == HeeksColor(0, 0, 0) || c == HeeksColor(255, 255, 255))background_color[0].best_black_or_white().glColor();
	else c.glColor();
}

std::wstring CCadApp::GetExeFolder()const{
	return L"";
}

std::wstring CCadApp::GetResFolder()const{ return m_res_folder; }


// do your own glBegin and glEnd
void CCadApp::get_2d_arc_segments(double xs, double ys, double xe, double ye, double xc, double yc, bool dir, bool want_start, double pixels_per_mm, void(*callbackfunc)(const double* xy)){
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

bool CCadApp::PickPosition(const wchar_t* str, double* pos, void(*callback)(const double*))
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

void CCadApp::glSphere(double radius, const double* pos)
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

void CCadApp::OnNewOrOpen(bool open, int res)
{
	//PythonOnNewOrOpen(open, res);

	ObserversOnChange(&m_objects, NULL, NULL);
}

void CCadApp::OnBeforeNewOrOpen(bool open, int res)
{
	for (std::list< void(*)(int, int) >::iterator It = m_beforeneworopen_callbacks.begin(); It != m_beforeneworopen_callbacks.end(); It++)
	{
		void(*callbackfunc)(int, int) = *It;
		(*callbackfunc)(open ? 1 : 0, res);
	}
}

void CCadApp::OnBeforeFrameDelete(void)
{
	for (std::list< void(*)() >::iterator It = m_beforeframedelete_callbacks.begin(); It != m_beforeframedelete_callbacks.end(); It++)
	{
		void(*callbackfunc)() = *It;
		(*callbackfunc)();
	}
}

HeeksObj* CCadApp::GetIDObject(int type, int id)
{
	UsedIds_t::iterator FindIt1 = used_ids.find(type);
	if (FindIt1 == used_ids.end()) return(NULL);

	IdsToObjects_t &ids = FindIt1->second;
	IdsToObjects_t::iterator FindIt2 = ids.find(id);

	if (FindIt2 == ids.end())return NULL;

	std::list<HeeksObj*> &list = FindIt2->second;
	return list.back();
}

std::list<HeeksObj*> CCadApp::GetIDObjects(int type, int id)
{
	std::list<HeeksObj *> results;

	UsedIds_t::iterator FindIt1 = used_ids.find(type);
	if (FindIt1 == used_ids.end()) return results;

	IdsToObjects_t &ids = FindIt1->second;
	IdsToObjects_t::iterator FindIt2 = ids.find(id);

	if (FindIt2 == ids.end()) return results;
	return FindIt2->second;
}

void CCadApp::SetObjectID(HeeksObj* object, int id)
{
	if (object->UsesID())
	{
		object->m_id = id;
		GroupId_t id_group_type = object->GetIDGroupType();
		if (id_group_type != 0)
		{
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
}

int CCadApp::GetNextID(int id_group_type)
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

void CCadApp::RemoveID(HeeksObj* object)
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

void CCadApp::ResetIDs()
{
	used_ids.clear();
	next_id_map.clear();
}

void CCadApp::RegisterOnGLCommands(void(*callbackfunc)())
{
	m_on_glCommands_list.push_back(callbackfunc);
}

void CCadApp::RemoveOnGLCommands(void(*callbackfunc)())
{
	m_on_glCommands_list.remove(callbackfunc);
}

void CCadApp::RegisterIsModifiedFn(bool(*callbackfunc)())
{
	m_is_modified_callbacks.push_back(callbackfunc);
}

void CCadApp::CreateTransformGLList(const std::list<HeeksObj*>& list, bool show_grippers_on_drag){
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

void CCadApp::DestroyTransformGLList(){
	if (m_transform_gl_list)
	{
		glDeleteLists(m_transform_gl_list, 1);
	}
	m_transform_gl_list = 0;
}


void CCadApp::render_text(const wchar_t* str, bool select, double scale, double blur_scale)
{
	glPushMatrix();
	double s = scale * 1.6;
	glScaled(s, s, 1.0);
	glTranslated(0, -1.0, 0);
	DrawHeeksFontStringAntialiased(Ttc(str), blur_scale / scale, false, true);
	glPopMatrix();
}

bool CCadApp::get_text_size(const wchar_t* str, float* width, float* height)
{
	return false;
#if 0
	std::pair<int, int> size;
	m_gl_font.GetStringSize(str, &size);
	*width = (float)(size.first) * get_text_scale();
	*height = (float)(size.second) * get_text_scale();

	return true;
#endif
}

void CCadApp::render_screen_text2(const wchar_t* str, double scale)
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
			render_text(buffer, false, scale, 0.2);
			if (str[i] == newline)
				glTranslated(0.0, -2.2 * scale, 0.0);
			j = 0;
		}
	}
}

void CCadApp::render_screen_text(const wchar_t* str1, const wchar_t* str2)
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

	render_screen_text2(str1, 10.0);

	render_screen_text2(str2, 6.12);

	//Even though this is in reverse order, the different matrices have different stacks, and we want to exit here in the modelview
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void CCadApp::EnableBlend()
{
	if (!m_antialiasing)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	}
}

void CCadApp::DisableBlend()
{
	if (!m_antialiasing)glDisable(GL_BLEND);
}

void CCadApp::render_screen_text_at(const wchar_t* str1, double scale, double x, double y, double theta)
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

	glRotated(theta, 0, 0, 1);
	render_screen_text2(str1, scale);

	//Even though this is in reverse order, the different matrices have different stacks, and we want to exit here in the modelview
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void CCadApp::RegisterOnBuildTexture(void(*callbackfunc)())
{
	m_on_build_texture_callbacks.push_back(callbackfunc);
}

void CCadApp::RegisterOnBeforeNewOrOpen(void(*callbackfunc)(int, int))
{
	m_beforeneworopen_callbacks.push_back(callbackfunc);
}

void CCadApp::RegisterUnitsChangeHandler(void(*units_changed_handler)(const double value))
{
	m_units_changed_handlers.push_back(units_changed_handler);
}

void CCadApp::UnregisterUnitsChangeHandler(void(*units_changed_handler)(const double value))
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

IRect CCadApp::PointToPickBox(const IPoint& point)
{
	return IRect(point.x - 5, theApp->m_current_viewport->GetViewportSize().GetHeight() - point.y - 5, 10, 10);
}

void CCadApp::GetObjectsInWindow(const IRect &window, bool only_if_fully_in, bool one_of_each, const CFilter &filter, std::list<HeeksObj*> &objects, bool just_top_level_item)
{
	if (only_if_fully_in){
		// only select objects which are completely within the window
		std::list<HeeksObj*> objects_in_window;
		GetObjectsInWindow(window, false, one_of_each, filter, objects_in_window, just_top_level_item);
		std::set<HeeksObj*> obj_set;
		for (std::list<HeeksObj*>::iterator It = objects_in_window.begin(); It != objects_in_window.end(); It++)
			obj_set.insert(*It);

		int bottom = window.y;
		int top = window.y + window.height;
		int height = abs(window.height);
		if (top < bottom)
		{
			int temp = bottom;
			bottom = top;
			top = temp;
		}

		IRect strip_boxes[4];
		// top
		strip_boxes[0] = IRect(window.x - 1, top, window.width + 2, 1);
		// bottom
		strip_boxes[1] = IRect(window.x - 1, bottom - 1, window.width + 2, 1);
		// left
		strip_boxes[2] = IRect(window.x - 1, bottom, 1, height);
		// right
		strip_boxes[3] = IRect(window.x + window.width, bottom, 1, height);

		for (int i = 0; i < 4; i++)
		{
			std::list<HeeksObj*> objects_in_strip;
			GetObjectsInWindow(strip_boxes[i], false, one_of_each, filter, objects_in_strip, just_top_level_item);
			for (std::list<HeeksObj*>::iterator It = objects_in_strip.begin(); It != objects_in_strip.end(); It++)
				obj_set.erase(*It);
		}

		for (std::set<HeeksObj*>::iterator It = obj_set.begin(); It != obj_set.end(); It++)
		{
			objects.push_back(*It);
		}
	}
	else{
		// select all the objects in the window, even if only partly in the window
		std::list<HeeksObj*> lowest_objects;
		ColorPickLowestObjects(window, false, lowest_objects);

		std::set<int> types_already_added;

		for (std::list<HeeksObj*>::iterator It = lowest_objects.begin(); It != lowest_objects.end(); It++)
		{
			HeeksObj* object = *It;
			HeeksObj* object_to_use = NULL;
			while (true)
			{
				if (!just_top_level_item)
					object_to_use = NULL;

				if (filter.CanTypeBePicked(object->GetType())){
					object_to_use = object;
				}

				object = object->m_owner; // move up to object's parent
				bool continue_in_loop = (object && (object != theApp));

				if (object_to_use && (!just_top_level_item || !continue_in_loop))
				{
					if (one_of_each)
					{
						int t = object_to_use->GetType();
						if (types_already_added.find(t) == types_already_added.end())
						{
							objects.push_back(object_to_use);
							types_already_added.insert(t);
						}
					}
					else
					{
						objects.push_back(object_to_use);
					}
				}

				if (!continue_in_loop)
					break;
			}
		}
	}
}

void CCadApp::ColorPickLowestObjects(IRect window, bool single_picking, std::list<HeeksObj*> &objects)
{
	// render everything with unique colors
	// window, width and height must be positive
	// single picking means we only want one object in the list; the best object

	//IRect is in window coordinates, so y = 0 is a the top of the screen
	window.y = m_current_viewport->GetViewportSize().y - (window.y + window.height);

	glDrawBuffer(GL_BACK);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);

	m_current_viewport->SetViewport();
	m_current_viewport->m_view_point.SetProjection(true);
	m_current_viewport->m_view_point.SetModelview();

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glLineWidth(1);
	glDepthMask(1);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glShadeModel(GL_FLAT);
	m_current_viewport->m_view_point.SetPolygonOffset();

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	theApp->glCommands(true, false, true);

	m_marked_list->GrippersGLCommands(true, true);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_COLOR_MATERIAL);

	unsigned int pixel_size = 4 * window.width * window.height;
	unsigned char* pixels = (unsigned char*)malloc(pixel_size);
	memset((void*)pixels, 0, pixel_size);
	glReadPixels(window.x, window.y, window.width, window.height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	int half_window_width = 0;
	IPoint window_centre;
	if (single_picking){
		half_window_width = (window.width) / 2;
		window_centre.x = window.x + window.width / 2;
		window_centre.y = window.y + window.height / 2;
	}
	int window_mode = 0;
	while (1){
		if (single_picking){
			int window_size = half_window_width - 1;
			if (window_mode == 0)window_size = 0;
			if (window_mode == 1)window_size = half_window_width / 2;
			window.x = window_centre.x - window_size;
			window.y = window_centre.y - window_size;
			window.width = 1 + window_size * 2;
			window.height = 1 + window_size * 2;
		}

		std::set<unsigned int> color_list;

		for (unsigned int i = 0; i < pixel_size; i += 4)
		{
			unsigned int color = pixels[i] | (pixels[i + 1] << 8) | (pixels[i + 2] << 16);
			if (color != 0)
				color_list.insert(color);
		}

		for (std::set<unsigned int>::iterator It = color_list.begin(); It != color_list.end(); It++)
		{
			unsigned int name = *It;
			HeeksObj *object = m_name_index.find(name);
			objects.push_back(object);
		}

		window_mode++;
		if (!single_picking)break;
		if (window_mode > 2)break;
	}

	free(pixels);
}

unsigned int CCadApp::GetIndex(HeeksObj *object) {
	return m_name_index.insert(object);
}

void CCadApp::ReleaseIndex(unsigned int index) {
	return m_name_index.erase(index);
}

void CCadApp::ClearSelection(bool call_OnChanged)
{
	m_marked_list->Clear(call_OnChanged);
}

void CCadApp::GetSelection(std::list<HeeksObj*> &objects)
{
	objects = m_marked_list->list();
}

bool CCadApp::ObjectMarked(HeeksObj* object)
{
	return m_marked_list->ObjectMarked(object);
}

void CCadApp::Mark(HeeksObj* object)
{
	m_marked_list->Add(object, true);
}

void CCadApp::Unmark(HeeksObj* object)
{
	m_marked_list->Remove(object, true);
}

void CCadApp::SketchSplit(HeeksObj* object, std::list<HeeksObj*> &new_separate_sketches)
{
	if (object->GetType() != SketchType)
		return;

	((CSketch*)object)->ExtractSeparateSketches(new_separate_sketches);
}

HeeksObj* CCadApp::CreateNewLine(const Point3d& s, const Point3d& e){ return new HLine(s, e, &current_color); }
HeeksObj* CCadApp::CreateNewArc(const Point3d& s, const Point3d& e, const Point3d& a, const Point3d& c){ return new HArc(s, e, a, c, &current_color); }
HeeksObj* CCadApp::CreateNewCircle(const Point3d& c, const Point3d& a, double r){ return new HCircle(c, a, r, &current_color); }
HeeksObj* CCadApp::CreateNewPoint(const Point3d& p){ return new HPoint(p, &current_color); }

void CCadApp::DrawFront()
{
	m_current_viewport->DrawFront();
}

void CCadApp::EndDrawFront()
{
	m_current_viewport->EndDrawFront();
}

static DigitizedPoint digitized_point_for_return;

DigitizedPoint& CCadApp::Digitize(const IPoint& p)
{
	digitized_point_for_return = m_digitizing->digitize(p);
	return digitized_point_for_return;
}

const DigitizedPoint& CCadApp::GetLastDigitizePoint()
{
	digitized_point_for_return = m_digitizing->digitized_point;
	return digitized_point_for_return;
}

void CCadApp::SetLastDigitizedPoint(const DigitizedPoint& p)
{
	m_digitizing->digitized_point = p;
}

void CCadApp::UseDigitiedPointAsReference()
{
	m_digitizing->reference_point = m_digitizing->digitized_point;
}

HeeksObj* CCadApp::GetObjPointer()
{
	return this;
}

ObjList* CCadApp::GetObjListPointer()
{
	return this;
}
