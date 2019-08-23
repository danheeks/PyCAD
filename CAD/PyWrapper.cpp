#include "stdafx.h"

#include <Python.h>

#include <boost/progress.hpp>
#include <boost/timer.hpp>
#include <boost/foreach.hpp>
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/class.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/python/call.hpp>

#include "PyWrapper.h"
#include "strconv.h"

PyObject *main_module = NULL;
PyObject *globals = NULL;

void GetPythonError(std::wstring& error_string)
{
	PyObject *errtype, *errvalue, *traceback;
	PyErr_Fetch(&errtype, &errvalue, &traceback);
	PyErr_NormalizeException(&errtype, &errvalue, &traceback);
	if (errtype != NULL && 0)/* error type not as useful, less is more */ {
		PyObject *s = PyObject_Str(errtype);
		PyObject* pStrObj = PyUnicode_AsUTF8String(s);
		char* c = PyBytes_AsString(pStrObj);
		if (c)
		{
			wchar_t wstr[1024];
			mbstowcs(wstr, c, 1024);
			error_string.append(wstr);
			error_string.append(L"\n\n");
		}
		Py_DECREF(s);
		Py_DECREF(pStrObj);
	}
	if (errvalue != NULL) {
		PyObject *s = PyObject_Str(errvalue);
		PyObject* pStrObj = PyUnicode_AsUTF8String(s);
		char* c = PyBytes_AsString(pStrObj);
		if (c)
		{
			wchar_t wstr[1024];
			mbstowcs(wstr, c, 1024);
			error_string.append(wstr);
			error_string.append(L"\n\n\n");
		}
		Py_DECREF(s);
		Py_DECREF(pStrObj);
	}

	PyObject *pModule = PyImport_ImportModule("traceback");

	if (traceback != NULL && pModule != NULL)
	{
		PyObject* pDict = PyModule_GetDict(pModule);
		PyObject* pFunc = PyDict_GetItemString(pDict, "format_tb");
		if (pFunc && PyCallable_Check(pFunc))
		{
			PyObject* pArgs = PyTuple_New(1);
			pArgs = PyTuple_New(1);
			PyTuple_SetItem(pArgs, 0, traceback);
			PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
			if (pValue != NULL)
			{
				int len = PyList_Size(pValue);
				if (len > 0) {
					PyObject *t, *tt;
					int i;
					char *buffer;
					for (i = 0; i < len; i++) {
						tt = PyList_GetItem(pValue, i);
						t = Py_BuildValue("(O)", tt);
						if (!PyArg_ParseTuple(t, "s", &buffer)){
							return;
						}

						wchar_t wstr[1024];
						mbstowcs(wstr, buffer, 1024);
						error_string.append(wstr);
						error_string.append(L"\n");
					}
				}
			}
			Py_DECREF(pValue);
			Py_DECREF(pArgs);
		}
	}
	Py_DECREF(pModule);

	Py_XDECREF(errvalue);
	Py_XDECREF(errtype);
	Py_XDECREF(traceback);
}

void HandlePythonCallError()
{
	std::wstring error_string;
	GetPythonError(error_string);

	wprintf(error_string.c_str());
	//theApp->MessageBox(error_string.c_str());
}

static int BeforePythonCall_level = 0;

void BeforePythonCall(PyObject **main_module, PyObject **globals)
{
	BeforePythonCall_level++;
	if (BeforePythonCall_level <= 1)
	{
		if (*main_module == NULL)
		{
			*main_module = PyImport_ImportModule("__main__");
			*globals = PyModule_GetDict(*main_module);
		}

		const char* str = "import sys\nold_stdout = sys.stdout\nold_stderr = sys.stderr\nclass CatchOutErr:\n  def __init__(self):\n    self.value = ''\n    self.count = ''\n  def write(self, txt):\n    self.value += str(self.count)+txt\n  def flush(self):\n    pass\ncatchOutErr = CatchOutErr()\nsys.stdout = catchOutErr\nsys.stderr = catchOutErr"; //this is python code to redirect stdouts/stderr
		PyRun_String(str, Py_file_input, *globals, *globals); //invoke code to redirect
	}
}

bool AfterPythonCall(PyObject *main_module)
{
	BeforePythonCall_level--;
	if (BeforePythonCall_level > 0)
	{
		if (PyErr_Occurred())
		{
			HandlePythonCallError();
			return false;
		}
		return true;
	}

	if (BeforePythonCall_level < 0)
	{
		theApp->DoMessageBox(L"too many internal calls to AfterPythonCall");
		return false;
	}

	PyObject *catcher = PyObject_GetAttrString(main_module, "catchOutErr"); //get our catchOutErr created above
	PyObject *output = PyObject_GetAttrString(catcher, "value"); //get the stdout and stderr from our catchOutErr object
#if PY_MAJOR_VERSION >= 3
	std::wstring s(Ctt(PyUnicode_AsUTF8(output)));
#else
	std::wstring s(Ctt(PyString_AsString(output)));
#endif
	if (s.size() > 0) {
		wprintf(s.c_str());
	}

	bool return_value = true;
	if (PyErr_Occurred())
	{
		HandlePythonCallError();
		return_value = false;
	}

	if (BeforePythonCall_level == 0)
	{
		const char* str = "sys.stdout = old_stdout\nsys.stderr = old_stderr";
		PyRun_String(str, Py_file_input, globals, globals); //invoke code
	}

	return return_value;
}

	void CallPythonCallback(PyObject* callback)
	{
		//PyObject *main_module, *globals;
		BeforePythonCall(&main_module, &globals);

		// Execute the python function
		PyObject* result = PyObject_CallFunction(callback, 0);

		AfterPythonCall(main_module);

	}

	static std::map<std::string, PyObject*> xml_read_callbacks;
	static std::map<std::string, int> custom_object_type_map;
	static int next_available_custom_object_type = ObjectMaximumType;

	HeeksObj* CreatePyObjectWithName(const std::string& name)
	{
		std::map< std::string, PyObject* >::iterator FindIt = xml_read_callbacks.find(name);
		HeeksObj* object = NULL;
		if (FindIt == xml_read_callbacks.end())
			return NULL;

		PyObject* python_callback = FindIt->second;

		//PyObject *main_module, *globals;
		BeforePythonCall(&main_module, &globals);

		// Execute the python function
		PyObject* result = PyObject_CallFunction(python_callback, 0);
		if (result)
		{
			object = boost::python::extract<HeeksObj*>(result);
		}

		AfterPythonCall(main_module);

		return object;
	}

	int RegisterObjectType(std::wstring name, PyObject *callback)
	{
		// registers the Create function to be called in python from CApp::CreateObjectOfType
		// returns the int type stored by CApp
		const char* name_c = Ttc(name.c_str());

		if (PyCallable_Check(callback))
		{
			// add an entry in map from name to 
			xml_read_callbacks.insert(std::make_pair(name_c, callback));
		}

		std::map<std::string, int>::iterator FindIt = custom_object_type_map.find(name_c);

		if (custom_object_type_map.find(name_c) == custom_object_type_map.end())
		{
			// not in map
			int value = next_available_custom_object_type;
			custom_object_type_map.insert(std::make_pair(std::string(name_c), value));
			next_available_custom_object_type++;
			return value;
		}

		return FindIt->second;
	}
