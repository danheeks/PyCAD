#pragma once

#include <boost/python/wrapper.hpp>

void HandlePythonCallError();


/*
This RAII structure ensures that threads created on the native C side
adhere to the laws of Python and ensure they grab the GIL lock when
calling into python
*/
struct PyLockGIL
{

	PyLockGIL()
		: gstate(PyGILState_Ensure())
	{
	}

	~PyLockGIL()
	{
		PyGILState_Release(gstate);
	}

	PyLockGIL(const PyLockGIL&) = delete;
	PyLockGIL& operator=(const PyLockGIL&) = delete;

	PyGILState_STATE gstate;
};

void BeforePythonCall(PyObject **main_module, PyObject **globals);
bool AfterPythonCall(PyObject *main_module);
extern PyObject *main_module;
extern PyObject *globals;

template <class T>
class cad_wrapper : public boost::python::wrapper<T>
{
public:
	bool CallVoidReturn(const char* func)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func)){

			if (PyErr_Occurred()){
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else{

				BeforePythonCall(&main_module, &globals);
				PyLockGIL lock;
				try{
					PyObject_CallFunction(f.ptr(), const_cast<char*>("()"));
					success = AfterPythonCall(main_module);
					return success;
				}
				catch (const boost::python::error_already_set&){}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return false;
	}

	bool CallVoidReturn(const char* func, CBox& box)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func)){

			if (PyErr_Occurred()){
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else{

				BeforePythonCall(&main_module, &globals);
				PyLockGIL lock;
				try{

					boost::python::detail::method_result result = f(box);
					success = AfterPythonCall(main_module);
					return success;
				}
				catch (const boost::python::error_already_set&){}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return false;
	}

	bool CallVoidReturn(const char* func, const Matrix& mat)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func)){

			if (PyErr_Occurred()){
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else{

				BeforePythonCall(&main_module, &globals);
				PyLockGIL lock;
				try{

					boost::python::detail::method_result result = f(mat);
					success = AfterPythonCall(main_module);
					return success;
				}
				catch (const boost::python::error_already_set&){}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return false;
	}

	bool CallVoidReturn(const char* func, const HeeksColor& c)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func)){

			if (PyErr_Occurred()){
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else{

				BeforePythonCall(&main_module, &globals);
				PyLockGIL lock;
				try{

					boost::python::detail::method_result result = f(
						c);
					success = AfterPythonCall(main_module);
					return success;
				}
				catch (const boost::python::error_already_set&){}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return false;
	}

	bool CallVoidReturn(const char* func, const HeeksObj* object)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func)){

			if (PyErr_Occurred()){
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else{

				BeforePythonCall(&main_module, &globals);
				PyLockGIL lock;
				try{

					boost::python::detail::method_result result = f(boost::ref(object));
					success = AfterPythonCall(main_module);
					return success;
				}
				catch (const boost::python::error_already_set&){
					if (PyErr_Occurred())
					{
						HandlePythonCallError();
					}
				}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return false;
	}

	bool CallVoidReturn(const char* func, HeeksObj* object)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func)){

			if (PyErr_Occurred()){
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else{

				BeforePythonCall(&main_module, &globals);
				PyLockGIL lock;
				try{

					boost::python::detail::method_result result = f(object);
					success = AfterPythonCall(main_module);
					return success;
				}
				catch (const boost::python::error_already_set&){
					if (PyErr_Occurred())
					{
						HandlePythonCallError();
					}
				}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return false;
	}

	bool CallVoidReturn(const char* func, bool value)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func)){

			if (PyErr_Occurred()){
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else{

				BeforePythonCall(&main_module, &globals);
				PyLockGIL lock;
				try{

					boost::python::detail::method_result result = f(value);
					success = AfterPythonCall(main_module);
					return success;
				}
				catch (const boost::python::error_already_set&){}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return false;
	}

	bool CallVoidReturn(const char* func, bool b1, bool b2, bool b3)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func)){

			if (PyErr_Occurred()){
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else{

				BeforePythonCall(&main_module, &globals);
				PyLockGIL lock;
				try{

					boost::python::detail::method_result result = f(b1, b2, b3);
					success = AfterPythonCall(main_module);
					return success;
				}
				catch (const boost::python::error_already_set&){}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return false;
	}

	bool CallVoidReturn(const char* func, int value)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func)){

			if (PyErr_Occurred()){
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else{

				BeforePythonCall(&main_module, &globals);
				PyLockGIL lock;
				try{

					boost::python::detail::method_result result = f(value);
					success = AfterPythonCall(main_module);
					return success;
				}
				catch (const boost::python::error_already_set&){}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return false;
	}

	bool CallVoidReturn(const char* func, double value)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func)){

			if (PyErr_Occurred()){
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else{

				BeforePythonCall(&main_module, &globals);
				PyLockGIL lock;
				try{

					boost::python::detail::method_result result = f(value);
					success = AfterPythonCall(main_module);
					return success;
				}
				catch (const boost::python::error_already_set&){}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return false;
	}

	bool CallVoidReturn(const char* func, std::wstring value)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func)){

			if (PyErr_Occurred()){
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else{

				BeforePythonCall(&main_module, &globals);
				PyLockGIL lock;
				try{

					boost::python::detail::method_result result = f(value);
					success = AfterPythonCall(main_module);
					return success;
				}
				catch (const boost::python::error_already_set&){}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return false;
	}

	bool CallVoidReturn(const char* func, boost::python::list &value)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func)){

			if (PyErr_Occurred()){
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else{

				BeforePythonCall(&main_module, &globals);
				PyLockGIL lock;
				try{

					boost::python::detail::method_result result = f(value);
					success = AfterPythonCall(main_module);
					return success;
				}
				catch (const boost::python::error_already_set&){}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return false;
	}

	bool CallVoidReturn(const char* func, MouseEvent& value)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func)){

			if (PyErr_Occurred()){
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else{

				BeforePythonCall(&main_module, &globals);
				PyLockGIL lock;
				try{

					boost::python::detail::method_result result = f(value);
					success = AfterPythonCall(main_module);
					return success;
				}
				catch (const boost::python::error_already_set&){}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return false;
	}

	bool CallVoidReturn(const char* func, boost::python::list &value1, boost::python::list &value2)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func)){

			if (PyErr_Occurred()){
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else{

				BeforePythonCall(&main_module, &globals);
				PyLockGIL lock;
				try{

					boost::python::detail::method_result result = f(value1, value2);
					success = AfterPythonCall(main_module);
					return success;
				}
				catch (const boost::python::error_already_set&){}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return false;
	}
	
	std::pair<bool, bool> CallReturnBool(const char* func)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func))
		{
			if (PyErr_Occurred())
			{
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else
			{
				BeforePythonCall(&main_module, &globals);

				// Execute the python function
				PyLockGIL lock;
				try
				{
					boost::python::detail::method_result result = f();
					success = AfterPythonCall(main_module);
					return std::make_pair(success, (bool)result);
				}
				catch (const boost::python::error_already_set&)
				{
				}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return std::make_pair(false, false);
	}

	std::pair<bool, bool> CallReturnBool(const char* func, HeeksObj* object)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func))
		{
			if (PyErr_Occurred())
			{
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else
			{
				BeforePythonCall(&main_module, &globals);

				// Execute the python function
				PyLockGIL lock;
				try
				{
					boost::python::detail::method_result result = f(boost::ref(object));
					success = AfterPythonCall(main_module);
					return std::make_pair(success, (bool)result);
				}
				catch (const boost::python::error_already_set&)
				{
				}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return std::make_pair(false, false);
	}

	std::pair<bool, bool> CallReturnBool(const char* func, const Point3d &ray_start, const Point3d &ray_direction)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func))
		{
			if (PyErr_Occurred())
			{
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else
			{
				BeforePythonCall(&main_module, &globals);

				// Execute the python function
				PyLockGIL lock;
				try
				{
					boost::python::detail::method_result result = f(ray_start, ray_direction);
					success = AfterPythonCall(main_module);
					return std::make_pair(success, (bool)result);
				}
				catch (const boost::python::error_already_set&)
				{
				}
				success = AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return std::make_pair(false, false);
	}

	std::pair<bool, int> CallReturnInt(const char* func)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func))
		{
			if (PyErr_Occurred())
			{
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else
			{
				BeforePythonCall(&main_module, &globals);

				// Execute the python function
				PyLockGIL lock;
				try
				{
					boost::python::detail::method_result result = f();
					success = AfterPythonCall(main_module);
					return std::make_pair(success, (int)result);
				}
				catch (const boost::python::error_already_set&)
				{
				}
				AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return std::make_pair(false, 0);
	}

	std::pair<bool, double> CallReturnDouble(const char* func)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func))
		{
			if (PyErr_Occurred())
			{
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else
			{
				BeforePythonCall(&main_module, &globals);

				// Execute the python function
				PyLockGIL lock;
				try
				{
					boost::python::detail::method_result result = f();
					success = AfterPythonCall(main_module);
					return std::make_pair(success, (double)result);
				}
				catch (const boost::python::error_already_set&)
				{
				}
				AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return std::make_pair(false, 0.0);
	}

	std::pair<bool, std::string> CallReturnString(const char* func)const
	{
		bool success = false;
		bool after_called = false;
		if (boost::python::override f = this->get_override(func))
		{
			if (PyErr_Occurred())
			{
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else
			{
				BeforePythonCall(&main_module, &globals);

				// Execute the python function
				PyLockGIL lock;
				try
				{
					boost::python::detail::method_result result = f();
					success = AfterPythonCall(main_module);
					after_called = true;
					const char* result_string = (const char*)result;
					if (result_string == NULL)result_string = "";
					return std::make_pair(success, (std::string)result_string);
				}
				catch (const boost::python::error_already_set&)
				{
				}
				if(!after_called)AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return std::make_pair(false, std::string());
	}

	std::pair<bool, std::wstring> CallReturnWString(const char* func)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func))
		{
			if (PyErr_Occurred())
			{
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else
			{
				BeforePythonCall(&main_module, &globals);

				// Execute the python function
				PyLockGIL lock;
				try
				{
					boost::python::detail::method_result result = f();
					success = AfterPythonCall(main_module);
					const wchar_t* result_string = (const wchar_t*)result;
					if (result_string == NULL)result_string = L"";
					return std::make_pair(success, (std::wstring)result_string);
				}
				catch (const boost::python::error_already_set&)
				{
				}
				AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return std::make_pair(false, std::wstring());
	}

	std::pair<bool, HeeksObj*> CallReturnHeeksObj(const char* func)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func))
		{
			if (PyErr_Occurred())
			{
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else
			{
				BeforePythonCall(&main_module, &globals);

				// Execute the python function
				PyLockGIL lock;
				try
				{
					boost::python::detail::method_result result = f();
					success = AfterPythonCall(main_module);
					return std::make_pair(success, (HeeksObj*)result);
				}
				catch (const boost::python::error_already_set&)
				{
				}
				AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return std::make_pair(false, (HeeksObj*)NULL);
	}

	std::pair<bool, HeeksColor> CallReturnColor(const char* func)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func))
		{
			if (PyErr_Occurred())
			{
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else
			{
				BeforePythonCall(&main_module, &globals);

				// Execute the python function
				PyLockGIL lock;
				try
				{
					boost::python::detail::method_result result = f();
					success = AfterPythonCall(main_module);
					return std::make_pair(success, (HeeksColor)(long)result);
				}
				catch (const boost::python::error_already_set&)
				{
				}
				AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return std::make_pair(false, HeeksColor(0, 0, 0));
	}

	std::pair<bool, CBox> CallReturnBox(const char* func)const
	{
		bool success = false;
		if (boost::python::override f = this->get_override(func))
		{
			if (PyErr_Occurred())
			{
				PyErr_Clear();// clear message saying 'object has no attribute' and don't call function recursively
			}
			else
			{
				BeforePythonCall(&main_module, &globals);

				// Execute the python function
				PyLockGIL lock;
				try
				{
					boost::python::detail::method_result result = f();
					success = AfterPythonCall(main_module);
					return std::make_pair(success, (const CBox&)result);
				}
				catch (const boost::python::error_already_set&)
				{
				}
				AfterPythonCall(main_module);
			}
		}
		PyErr_Clear();
		return std::make_pair(false, CBox());
	}

};

void CallPythonCallback(PyObject* callback);
HeeksObj* CreatePyObjectWithName(const std::string& name);
int RegisterObjectType(std::wstring name, PyObject *callback = NULL, bool add_to_filter = true);
boost::python::list GetObjectNamesAndTypes(void);

