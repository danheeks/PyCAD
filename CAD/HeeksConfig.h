// HeeksConfig.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

class HeeksConfig
{
public:
	bool m_disabled;
	HeeksConfig();
	~HeeksConfig(){}

	// not sure what I'm going to do with config yet...
	// probably allow the python to derive a class, so that wxPython can do this instead
	void Write(const wchar_t* key_name, double value){}

	void Read(const wchar_t* key_name, int* value, int default_value)const{}
	void Read(const wchar_t* key_name, int* value)const{}
	void Read(const wchar_t* key_name, wchar_t* value, const wchar_t* default_value)const{}
	void Read(const wchar_t* key_name, wchar_t* value)const{}
	void Read(const wchar_t* key_name, std::wstring* value, const wchar_t* default_value)const{}
	void Read(const wchar_t* key_name, std::wstring* value)const{}
	void Read(const wchar_t* key_name, double* value, double default_value)const{}
	void Read(const wchar_t* key_name, double* value)const{}
	void Read(const wchar_t* key_name, float* value, float default_value)const{}
	void Read(const wchar_t* key_name, float* value)const{}
	void Read(const wchar_t* key_name, bool* value, bool default_value)const{}
	void Read(const wchar_t* key_name, bool* value)const{}

	void DeleteAll(){}
};
