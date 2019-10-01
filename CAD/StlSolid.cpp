// StlSolid.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "StlSolid.h"
#include "MarkedObject.h"
#include "Picking.h"
#include "Material.h"
#include "strconv.h"
#include "GripData.h"
#include "Property.h"

using namespace std;

CStlTri::CStlTri(const float* t)
{
	memcpy(x[0], t, 9*sizeof(float));
}

CStlTri::CStlTri(const double* t)
{
	x[0][0] = (float)t[0];
	x[0][1] = (float)t[1];
	x[0][2] = (float)t[2];
	x[1][0] = (float)t[3];
	x[1][1] = (float)t[4];
	x[1][2] = (float)t[5];
	x[2][0] = (float)t[6];
	x[2][1] = (float)t[7];
	x[2][2] = (float)t[8];
}

CStlSolid::CStlSolid(const HeeksColor* col) :m_color(*col), m_gl_list(0), m_edge_gl_list(0), m_clicked_triangle(0){
	m_title.assign(GetTypeString());
}

CStlSolid::CStlSolid() : m_color(theApp->current_color), m_gl_list(0), m_edge_gl_list(0), m_clicked_triangle(0){
	m_title.assign(GetTypeString());
}

#ifdef UNICODE
// constructor for the Boost Python interface
CStlSolid::CStlSolid(const std::wstring& filepath) :m_color(theApp->current_color), m_gl_list(0), m_edge_gl_list(0), m_clicked_triangle(0){
	m_title.assign(GetTypeString());
	read_from_file(filepath.c_str());

	if(theApp->m_in_OpenFile && theApp->m_file_open_matrix)
	{
		Transform(*theApp->m_file_open_matrix);
	}
}
#endif

CStlSolid::CStlSolid(const wchar_t* filepath, const HeeksColor* col) :m_color(*col), m_gl_list(0), m_edge_gl_list(0), m_clicked_triangle(0){
	m_title.assign(GetTypeString());
	read_from_file(filepath);

	if(theApp->m_in_OpenFile && theApp->m_file_open_matrix)
	{
		Transform(*theApp->m_file_open_matrix);
	}
}

bool IsAsciiStlFile(ifstream *ifs)
{
	char solid_string[6] = "aaaaa";
	ifs->read(solid_string, 5);
	if (ifs->eof())return false;

	bool ascii = false;

	if (!strcmp(solid_string, "solid"))
	{
		ifs->seekg(-1, ios_base::end);

		bool found = false;
		bool started = false;
		int i = 0;
		while (!found && i < 512)
		{
			char c = ifs->get();


			if ((int)ifs->tellg() <= 1)
			{
				ifs->seekg(0);
				found = true;
			}
			else if (c == '\n')
			{
				if (started)
					found = true;
				else
				{
					ifs->seekg(-2, ios_base::cur);
				}
			}
			else
			{
				ifs->seekg(-2, ios_base::cur);
				i++;
				started = true;
			}
		}

		if (found)
		{
			char str[1024];
			ifs->getline(str, 1024);

			std::wstring wstr(Ctt(str));
			if (contains(wstr, L"endsolid"))
			{
				ascii = true;
			}
		}
	}

	ifs->seekg(0);

	return ascii;
}

void CStlSolid::read_from_file(const wchar_t* filepath)
{
	// read the stl file
	ifstream ifs(filepath, ios::binary);
	if(!ifs)return;


	if (!IsAsciiStlFile(&ifs))
	{
		// try binary file read

		// read the header
		char header[81];
		header[80] = 0;
		ifs.read(header, 80);

		unsigned int num_facets = 0;
		ifs.read((char*)(&num_facets), 4);

		for (unsigned int i = 0; i<num_facets; i++)
		{
			CStlTri tri;
			float n[3];
			ifs.read((char*)(n), 12);
			ifs.read((char*)(tri.x[0]), 36);
			short attr;
			ifs.read((char*)(&attr), 2);
			m_list.push_back(tri);
		}
}
	else
	{
		// first 5 chars will be "solid"
		char str[1024] = "solid";
		ifs.getline(str, 1024);
		m_title.assign(Ctt(&(str[6])));

		CStlTri t;
		char five_chars[6] = "aaaaa";

		int vertex = 0;

		while (!ifs.eof() && !ifs.fail())
		{
			ifs.getline(str, 1024);

			int i = 0, j = 0;
			for (; i<5; i++, j++)
			{
				if (str[j] == 0)break;
				while (str[j] == ' ' || str[j] == '\t')j++;
				five_chars[i] = str[j];
			}
			if (i == 5)
			{
				if (!strcmp(five_chars, "verte"))
				{
#ifdef WIN32
					sscanf(str, " vertex %f %f %f", &(t.x[vertex][0]), &(t.x[vertex][1]), &(t.x[vertex][2]));
#else
					std::istringstream ss(str);
					ss.imbue(std::locale("C"));
					while (ss.peek() == ' ') ss.seekg(1, ios_base::cur);
					ss.seekg(std::string("vertex").size(), ios_base::cur);
					ss >> t.x[vertex][0] >> t.x[vertex][1] >> t.x[vertex][2];
#endif
					vertex++;
					if (vertex > 2)vertex = 2;
				}
				else if (!strcmp(five_chars, "facet"))
				{
					vertex = 0;
				}
				else if (!strcmp(five_chars, "endfa"))
				{
					if (vertex == 2)
					{
						m_list.push_back(t);
					}
				}
			}
		}
	}
}

CStlSolid::~CStlSolid(){
	KillGLLists();
}

const CStlSolid& CStlSolid::operator=(const CStlSolid& s)
{
    HeeksObj::operator = (s);

	// don't copy id
	m_box = s.m_box;
	m_title = s.m_title;
	KillGLLists();

	m_color = s.m_color;

	m_list.clear();
	std::copy( s.m_list.begin(), s.m_list.end(), std::inserter( m_list, m_list.begin() ));

	return *this;
}

bool CStlSolid::IsDifferent(HeeksObj* other)
{
	CStlSolid* shape = (CStlSolid*)other;
	if(shape->m_color.COLORREF_color() != m_color.COLORREF_color() || (shape->m_title == m_title) || shape->m_box != m_box)
		return true;

	return HeeksObj::IsDifferent(other);
}


void CStlSolid::KillGLLists()
{
	if (m_gl_list)
	{
		glDeleteLists(m_gl_list, 1);
		m_gl_list = 0;
	}
	if (m_edge_gl_list)
	{
		glDeleteLists(m_edge_gl_list, 1);
		m_edge_gl_list = 0;
	}
	m_box = CBox();
}

const wchar_t* CStlSolid::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/stlsolid.png";
	return iconpath.c_str();
}

void CStlSolid::GetProperties(std::list<Property *> *list)
{
	wchar_t buf[32];
	swprintf(buf, 32, L"%d", (int)(m_list.size()));
	list->push_back(new PropertyStringReadOnly(L"Number of Triangles", buf));
	HeeksObj::GetProperties(list);
}

void CStlSolid::glCommands(bool select, bool marked, bool no_color){
	bool draw_faces = (theApp->m_solid_view_mode == SolidViewFacesAndEdges || theApp->m_solid_view_mode == SolidViewFacesOnly);
	bool draw_edges = (theApp->m_solid_view_mode == SolidViewFacesAndEdges || theApp->m_solid_view_mode == SolidViewEdgesOnly);

	if (draw_faces)
	{
		if (!no_color)
		{
			glEnable(GL_LIGHTING);
			Material(m_color).glMaterial(1.0);
		}

		if (m_gl_list)
		{
			glCallList(m_gl_list);
		}

		
		else{
			m_gl_list = glGenLists(1);
			glNewList(m_gl_list, GL_COMPILE_AND_EXECUTE);

			// render all the triangles
			int i = 0;
			for (std::list<CStlTri>::iterator It = m_list.begin(); It != m_list.end(); It++, i++)
			{
				CStlTri &t = *It;
				//SetPickingColor(i);
				if (theApp->m_stl_solid_random_colors)
				{
					HeeksColor col(rand() >> 7, rand() >> 7, rand() >> 7);
					Material(col).glMaterial(1.0);
				}
				Point3d p0(t.x[0][0], t.x[0][1], t.x[0][2]);
				Point3d p1(t.x[1][0], t.x[1][1], t.x[1][2]);
				Point3d p2(t.x[2][0], t.x[2][1], t.x[2][2]);
				Point3d v1(p0, p1);
				Point3d v2(p0, p2);
				try
				{
					Point3d norm = (v1 ^ v2).Normalized();
					float n[3];
					n[0] = (float)(norm.x);
					n[1] = (float)(norm.y);
					n[2] = (float)(norm.z);
					glBegin(GL_TRIANGLES);
					glNormal3fv(n);
					glVertex3fv(t.x[0]);
					glVertex3fv(t.x[1]);
					glVertex3fv(t.x[2]);
			glEnd();
				}
				catch (...)
				{
				}
			}

			glEndList();
		}

		if (!no_color)
			glDisable(GL_LIGHTING);
	}
	
	if (draw_edges)
	{
		if (m_edge_gl_list)
		{
			glCallList(m_edge_gl_list);
		}
		else{
			m_edge_gl_list = glGenLists(1);
			glNewList(m_edge_gl_list, GL_COMPILE_AND_EXECUTE);

			glBegin(GL_LINES);
			glColor3ub(0, 0, 0);
			for (std::list<CStlTri>::iterator It = m_list.begin(); It != m_list.end(); It++)
			{
				CStlTri &t = *It;
				glVertex3fv(t.x[0]);
				glVertex3fv(t.x[1]);
				glVertex3fv(t.x[1]);
				glVertex3fv(t.x[2]);
				glVertex3fv(t.x[2]);
				glVertex3fv(t.x[0]);
			}
			glEnd();

			glEndList();
		}
	}
}

void CStlSolid::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	if (just_for_endof && theApp->m_stl_solid_random_colors)
	{
		for (std::list<CStlTri>::iterator It = m_list.begin(); It != m_list.end(); It++)
		{
			CStlTri &t = *It;
			list->push_back(GripData(GripperTypeTranslate, t.x[0][0], t.x[0][1], t.x[0][2]));
			list->push_back(GripData(GripperTypeTranslate, t.x[1][0], t.x[1][1], t.x[1][2]));
			list->push_back(GripData(GripperTypeTranslate, t.x[2][0], t.x[2][1], t.x[2][2]));
		}
	}
	else
	{
		HeeksObj::GetGripperPositions(list, just_for_endof);
	}
}

void CStlSolid::GetBox(CBox &box){
	if(!m_box.m_valid)
	{
		// calculate the box for all the triangles
		for(std::list<CStlTri>::iterator It = m_list.begin(); It != m_list.end(); It++)
		{
			CStlTri &t = *It;
			m_box.Insert(t.x[0][0], t.x[0][1], t.x[0][2]);
			m_box.Insert(t.x[1][0], t.x[1][1], t.x[1][2]);
			m_box.Insert(t.x[2][0], t.x[2][1], t.x[2][2]);
		}
	}

	box.Insert(m_box);
}

void CStlSolid::Transform(const Matrix &m){
	Matrix mat = Matrix(m);
	for(std::list<CStlTri>::iterator It = m_list.begin(); It != m_list.end(); It++)
	{
		CStlTri &t = *It;
		for(int i = 0; i<3; i++){
			Point3d vx;
			vx = Point3d(t.x[i][0], t.x[i][1], t.x[i][2]);
			vx = vx.Transformed(mat);
			t.x[i][0] = (float)vx.x;
			t.x[i][1] = (float)vx.y;
			t.x[i][2] = (float)vx.z;
		}
	}

	KillGLLists();
}

CStlSolid::CStlSolid(const CStlSolid & rhs) : m_gl_list(0), m_edge_gl_list(0), m_clicked_triangle(0)
{
    *this = rhs;    // Call the assignment operator.
}

HeeksObj *CStlSolid::MakeACopy(void)const{
	CStlSolid *new_object = new CStlSolid(*this);
	return new_object;
}

void CStlSolid::CopyFrom(const HeeksObj* object)
{
	operator=(*((CStlSolid*)object));
}

void CStlSolid::OnEditString(const wchar_t* str){
	m_title.assign(str);
	// to do, use undoable property changes
}

void CStlSolid::GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal){
	double x[9];
	double n[9];
	for(std::list<CStlTri>::iterator It = m_list.begin(); It != m_list.end(); It++)
	{
		CStlTri &t = *It;
		x[0] = t.x[0][0];
		x[1] = t.x[0][1];
		x[2] = t.x[0][2];
		x[3] = t.x[1][0];
		x[4] = t.x[1][1];
		x[5] = t.x[1][2];
		x[6] = t.x[2][0];
		x[7] = t.x[2][1];
		x[8] = t.x[2][2];
		Point3d p0(t.x[0][0], t.x[0][1], t.x[0][2]);
		Point3d p1(t.x[1][0], t.x[1][1], t.x[1][2]);
		Point3d p2(t.x[2][0], t.x[2][1], t.x[2][2]);
		Point3d v1(p0, p1);
		Point3d v2(p0, p2);
		try
		{
			Point3d norm = (v1 ^ v2).Normalized();
			n[0] = norm.x;
			n[1] = norm.y;
			n[2] = norm.z;
			if(!just_one_average_normal)
			{
				n[3] = n[0];
				n[4] = n[1];
				n[5] = n[2];
				n[6] = n[0];
				n[7] = n[1];
				n[8] = n[2];
			}
			(*callbackfunc)(x, n);
		}
		catch(...)
		{
		}
	}
}

void CStlSolid::WriteToXML(TiXmlElement *element)
{
	element->SetAttribute("col", m_color.COLORREF_color());

	for(std::list<CStlTri>::iterator It = m_list.begin(); It != m_list.end(); It++)
	{
		CStlTri &t = *It;
		TiXmlElement * child_element;
		child_element = new TiXmlElement( "tri" );
		element->LinkEndChild( child_element );
		child_element->SetDoubleAttribute("p1x", t.x[0][0]);
		child_element->SetDoubleAttribute("p1y", t.x[0][1]);
		child_element->SetDoubleAttribute("p1z", t.x[0][2]);
		child_element->SetDoubleAttribute("p2x", t.x[1][0]);
		child_element->SetDoubleAttribute("p2y", t.x[1][1]);
		child_element->SetDoubleAttribute("p2z", t.x[1][2]);
		child_element->SetDoubleAttribute("p3x", t.x[2][0]);
		child_element->SetDoubleAttribute("p3y", t.x[2][1]);
		child_element->SetDoubleAttribute("p3z", t.x[2][2]);
	}

	HeeksObj::WriteToXML(element);
}

void CStlSolid::ReadFromXML(TiXmlElement *element)
{
	int int_value;
	if (element->Attribute("col", &int_value))m_color = HeeksColor((long)int_value);

	// loop through all the "tri" objects
	double x[3][3];

	for (TiXmlElement* pTriElem = TiXmlHandle(element).FirstChildElement().Element(); pTriElem; pTriElem = pTriElem->NextSiblingElement())
	{
		// get the attributes
		pTriElem->Attribute("p1x", &x[0][0]);
		pTriElem->Attribute("p1y", &x[0][1]);
		pTriElem->Attribute("p1z", &x[0][2]);
		pTriElem->Attribute("p2x", &x[1][0]);
		pTriElem->Attribute("p2y", &x[1][1]);
		pTriElem->Attribute("p2z", &x[1][2]);
		pTriElem->Attribute("p3x", &x[2][0]);
		pTriElem->Attribute("p3y", &x[2][1]);
		pTriElem->Attribute("p3z", &x[2][2]);
		m_list.push_back(CStlTri(&x[0][0]));
	}

	HeeksObj::ReadFromXML(element);
}

void CStlSolid::AddTriangle(float* t)
{
	CStlTri tri(t);
	m_list.push_back(tri);
}


bool CStlSolid::SetClickMarkPoint(MarkedObject* marked_object, const Point3d &ray_start, const Point3d &ray_direction)
{
	// set picked triangle
	if (marked_object->GetNumCustomNames() > 0)
	{
 		m_clicked_triangle = marked_object->GetCustomNames()[0];
	}
	return true; // click was handled by this object
}
