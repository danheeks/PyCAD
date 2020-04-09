
class StepFileObject :public HeeksObj
{
public:
	void ReadFromXML(TiXmlElement* element);
	virtual const wchar_t* GetTypeString(void)const{ return L"STEP_file"; }

	static int m_type;
};
