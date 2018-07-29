// PropertyChange.h

#include "Property.h"
#include "Undoable.h"

class PropertyChangeString: public Undoable
{
	Property* m_property;
public:
	std::wstring m_value;
	std::wstring m_old;

	PropertyChangeString(const std::wstring& value, Property* property);

	void Run(bool redo);
	void RollBack();
	const wchar_t* GetTitle(){return L"Property Change String";}
};

class PropertyChangeDouble: public Undoable
{
	Property* m_property;
public:
	double m_value;
	double m_old;

	PropertyChangeDouble(const double& value, Property* property);

	void Run(bool redo);
	void RollBack();
	const wchar_t* GetTitle(){return L"Property Change Double";}
};

class PropertyChangeLength: public Undoable
{
	Property* m_property;
public:
	double m_value;
	double m_old;

	PropertyChangeLength(const double& value, Property* property);

	void Run(bool redo);
	void RollBack();
	const wchar_t* GetTitle(){return L"Property Change Length";}
};

class PropertyChangeInt: public Undoable
{
	Property* m_property;
public:
	int m_value;
	int m_old;

	PropertyChangeInt(const int& value, Property* property);

	void Run(bool redo);
	void RollBack();
	const wchar_t* GetTitle(){return L"Property Change Int";}
};

class PropertyChangeColor: public Undoable
{
	Property* m_property;
public:
	HeeksColor m_value;
	HeeksColor m_old;

	PropertyChangeColor(const HeeksColor& value, Property* property);

	void Run(bool redo);
	void RollBack();
	const wchar_t* GetTitle(){return L"Property Change Color";}
};

class PropertyChangeChoice: public Undoable
{
	Property* m_property;
public:
	int m_value;
	int m_old;

	PropertyChangeChoice(const int& value, Property* property);

	void Run(bool redo);
	void RollBack();
	const wchar_t* GetTitle(){return L"Property Change Choice";}
};

class PropertyChangeCheck: public Undoable
{
	Property* m_property;
public:
	bool m_value;
	bool m_old;

	PropertyChangeCheck(const bool& value, Property* property);

	void Run(bool redo);
	void RollBack();
	const wchar_t* GetTitle(){return L"Property Change Check";}
};
