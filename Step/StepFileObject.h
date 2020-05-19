#include "ObjList.h"

class StepFileObject :public ObjList
{
public:
	void ReadFromXML(TiXmlElement* element);
	const wchar_t* GetTypeString(void)const{ return L"STEP_file"; }
	const wchar_t* GetXMLTypeString(){ return L"STEP File";	}
	const wchar_t* GetIconFilePath();
	bool OnlyAddChildrenOnReadXML(){ return true; }

	static int m_type;
};

void WriteSolids();
void ImportSTEPFile(const std::wstring& filepath);