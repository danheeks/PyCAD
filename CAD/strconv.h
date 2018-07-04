// strconv.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include <vector>

#ifdef UNICODE
extern const char* Ttc(const wchar_t* str);

extern const wchar_t* Ctt(const char* narrow_utf8_source_string);
#else
#define Ttc(x) x
#define Ctt(x) x
#endif

std::vector<std::wstring> Tokens( const std::wstring wxLine, const std::wstring wxDelimiters );
bool AllNumeric( const std::wstring wxLine );

bool endsWith(std::wstring const &fullString, std::wstring const &ending);
bool startsWith(std::wstring const &fullString, std::wstring const &beginning);
void lowerCase(std::wstring &str);
void upperCase(std::wstring &str);
bool contains(std::wstring const &fullString, std::wstring const &substring);

