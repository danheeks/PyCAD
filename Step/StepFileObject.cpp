// StepFileObject.cpp
#include "stdafx.h"
#include "StepFileObject.h"
#include "ShapeData.h"
#include "Shape.h"
#include "strconv.h"

// static variable
int StepFileObject::m_type = 0;

const wchar_t* StepFileObject::GetIconFilePath()
{
	static std::wstring iconpath = theApp->GetResFolder() + L"/icons/solids.png";
	return iconpath.c_str();
}

static std::wstring GetTempFolder()
{
	wchar_t temp_folder[1024] = L"/tmp/";
#ifdef WIN32
	GetTempPathW(1024, temp_folder);
#endif
	return temp_folder;
}

#define TEMP_FILE_NAME L"temp_HeeksCAD_STEP_file.step"

void StepFileObject::ReadFromXML(TiXmlElement* element)
{
	std::map<int, CShapeData> index_map;

	// get the children ( an index map)
	for (TiXmlElement* subElem = TiXmlHandle(element).FirstChildElement().Element(); subElem; subElem = subElem->NextSiblingElement())
	{
		std::string subname(subElem->Value());
		if (subname == std::string("index_map"))
		{
			// loop through all the child elements, looking for index_pair items
			for (TiXmlElement* subsubElem = TiXmlHandle(subElem).FirstChildElement().Element(); subsubElem; subsubElem = subsubElem->NextSiblingElement())
			{
				std::string subsubname(subsubElem->Value());
				if (subsubname == std::string("index_pair"))
				{
					int index = -1;
					CShapeData shape_data;

					// get the attributes
					for (TiXmlAttribute* a = subsubElem->FirstAttribute(); a; a = a->Next())
					{
						std::string attr_name(a->Name());
						if (attr_name == std::string("index")){ index = a->IntValue(); }
						else if (attr_name == std::string("id")){ shape_data.m_id = a->IntValue(); }
						else if (attr_name == std::string("title")){ shape_data.m_title.assign(a->Value()); }
						else if (attr_name == std::string("title_from_id")){ shape_data.m_title_made_from_id = (a->IntValue() != 0); }
						else if (attr_name == std::string("solid_type")){ shape_data.m_solid_type = (SolidTypeEnum)(a->IntValue()); }
						else if (attr_name == std::string("vis")){ shape_data.m_visible = (a->IntValue() != 0); }
						else shape_data.m_xml_element.SetAttribute(a->Name(), a->Value());
					}

					// get face ids
					for (TiXmlElement* faceElem = TiXmlHandle(subsubElem).FirstChildElement("face").Element(); faceElem; faceElem = faceElem->NextSiblingElement("face"))
					{
						int id = 0;
						faceElem->Attribute("id", &id);
						shape_data.m_face_ids.push_back(id);
					}

					// get edge ids
					for (TiXmlElement* edgeElem = TiXmlHandle(subsubElem).FirstChildElement("edge").Element(); edgeElem; edgeElem = edgeElem->NextSiblingElement("edge"))
					{
						int id = 0;
						edgeElem->Attribute("id", &id);
						shape_data.m_edge_ids.push_back(id);
					}

					// get vertex ids
					for (TiXmlElement* vertexElem = TiXmlHandle(subsubElem).FirstChildElement("vertex").Element(); vertexElem; vertexElem = vertexElem->NextSiblingElement("vertex"))
					{
						int id = 0;
						vertexElem->Attribute("id", &id);
						shape_data.m_vertex_ids.push_back(id);
					}

					if (index != -1)index_map.insert(std::pair<int, CShapeData>(index, shape_data));
				}
			}
		}
		else if (subname == std::string("file_text"))
		{
			const char* file_text = subElem->GetText();
			if (file_text)
			{
				std::wstring temp_file = GetTempFolder() + TEMP_FILE_NAME;
				{
					wofstream ofs(Ttc(temp_file.c_str()));
					ofs << file_text;
				}
				CShape::ImportSolidsFile(temp_file.c_str(), false, &index_map, this);
			}
		}
	}

	// get the attributes
	for (TiXmlAttribute* a = element->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if (name == "text")
		{
			std::wstring temp_file = GetTempFolder() + TEMP_FILE_NAME;
			{
				wofstream ofs(Ttc(temp_file.c_str()));
				ofs << a->Value();
			}
			CShape::ImportSolidsFile(temp_file.c_str(), false, &index_map, this);
		}
	}

}

void ImportSTEPFile(const std::wstring& filepath)
{
	CShape::ImportSolidsFile(filepath.c_str(), false);
}

void ExportSTEPFile(const std::wstring& filepath)
{
	CShape::ExportSolidsFile(theApp->GetObjListPointer()->GetChildren(), filepath.c_str());
}

void WriteSolids()
{
	// write a step file for all the solids
	if (CShape::m_solids_found){
		std::wstring temp_file = GetTempFolder() + TEMP_FILE_NAME;
		std::map<int, CShapeData> index_map;
		std::list<HeeksObj*> objects = theApp->GetObjPointer()->GetChildren();
		wprintf(temp_file.c_str());
		CShape::ExportSolidsFile(objects, temp_file.c_str(), &index_map);

		TiXmlElement *step_file_element = new TiXmlElement("STEP_file");
		theApp->GetXmlRoot()->LinkEndChild(step_file_element);

		// write the index map as a child of step_file
		{
			TiXmlElement *index_map_element = new TiXmlElement("index_map");
			step_file_element->LinkEndChild(index_map_element);
			for (std::map<int, CShapeData>::iterator It = index_map.begin(); It != index_map.end(); It++)
			{
				TiXmlElement *index_pair_element = new TiXmlElement("index_pair");
				index_map_element->LinkEndChild(index_pair_element);
				int index = It->first;
				CShapeData& shape_data = It->second;
				index_pair_element->SetAttribute("index", index);
				index_pair_element->SetAttribute("id", shape_data.m_id);
				index_pair_element->SetAttribute("title", shape_data.m_title.c_str());
				index_pair_element->SetAttribute("title_from_id", (shape_data.m_title_made_from_id ? 1 : 0));
				index_pair_element->SetAttribute("vis", shape_data.m_visible ? 1 : 0);
				if (shape_data.m_solid_type != SOLID_TYPE_UNKNOWN)index_pair_element->SetAttribute("solid_type", shape_data.m_solid_type);
				// get the CShapeData attributes
				for (TiXmlAttribute* a = shape_data.m_xml_element.FirstAttribute(); a; a = a->Next())
				{
					index_pair_element->SetAttribute(a->Name(), a->Value());
				}

				// write the face ids
				for (std::list<int>::iterator It = shape_data.m_face_ids.begin(); It != shape_data.m_face_ids.end(); It++)
				{
					int id = *It;
					TiXmlElement *face_id_element = new TiXmlElement("face");
					index_pair_element->LinkEndChild(face_id_element);
					face_id_element->SetAttribute("id", id);
				}

				// write the edge ids
				for (std::list<int>::iterator It = shape_data.m_edge_ids.begin(); It != shape_data.m_edge_ids.end(); It++)
				{
					int id = *It;
					TiXmlElement *edge_id_element = new TiXmlElement("edge");
					index_pair_element->LinkEndChild(edge_id_element);
					edge_id_element->SetAttribute("id", id);
				}

				// write the vertex ids
				for (std::list<int>::iterator It = shape_data.m_vertex_ids.begin(); It != shape_data.m_vertex_ids.end(); It++)
				{
					int id = *It;
					TiXmlElement *vertex_id_element = new TiXmlElement("vertex");
					index_pair_element->LinkEndChild(vertex_id_element);
					vertex_id_element->SetAttribute("id", id);
				}
			}
		}

		// write the step file as a string attribute of step_file
		ifstream ifs(Ttc(temp_file.c_str()));

		if (!(!ifs)){
			std::string fstr;
			char str[1024];
			while (!(ifs.eof())){
				ifs.getline(str, 1022);
				strcat(str, "\n");
				fstr.append(str);
				if (!ifs)break;
			}

			TiXmlElement *file_text_element = new TiXmlElement("file_text");
			step_file_element->LinkEndChild(file_text_element);
			TiXmlText *text = new TiXmlText(fstr.c_str());
			text->SetCDATA(true);
			file_text_element->LinkEndChild(text);
		}
	}
}