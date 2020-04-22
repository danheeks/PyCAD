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
				wchar_t temp_folder[1024];
				if(GetTempPathW(1024, temp_folder))// to do cross platform version
				{
					std::wstring temp_file = std::wstring(temp_folder) + L"temp_HeeksCAD_STEP_file.step";
					{
						ofstream ofs(temp_file.c_str());
						ofs << file_text;
					}
					CShape::ImportSolidsFile(temp_file.c_str(), false, &index_map, this);
				}
			}
		}
	}

	// get the attributes
	for (TiXmlAttribute* a = element->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if (name == "text")
		{
			wchar_t temp_folder[1024];
			if (GetTempPathW(1024, temp_folder))// to do cross platform version
			{
				std::wstring temp_file = std::wstring(temp_folder) + L"temp_HeeksCAD_STEP_file.step";
				{
					ofstream ofs(temp_file.c_str());
					ofs << a->Value();
				}
				CShape::ImportSolidsFile(temp_file.c_str(), false, &index_map, this);
			}
		}
	}

}
