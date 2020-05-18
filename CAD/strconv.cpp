// strconv.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include <locale>
#include <codecvt>
#include <string>
#include <vector>
#include <algorithm>

const char* Ttc(const wchar_t* str)
{
	std::wstring string_to_convert(str);

	//setup converter
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	static std::string converted_str;
	converted_str = converter.to_bytes(string_to_convert);
	return converted_str.c_str();
}

const wchar_t* Ctt(const char* narrow_utf8_source_string)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	static std::wstring wide;
	wide = converter.from_bytes(narrow_utf8_source_string);
	return wide.c_str();
}

static std::wstring::size_type find_first_of( const std::wstring line, const std::wstring delimiters )
{
	std::wstring::size_type offset = 0;
	bool offset_value_set = false;
	for (std::wstring::size_type delimiter = 0; delimiter < delimiters.size(); delimiter++)
	{
		std::wstring::size_type here = line.find( delimiters[delimiter] );
		if (here >= 0)
		{
			if (offset_value_set == false) { offset = here; offset_value_set = true; }
			if (here < offset) offset = here;
		}
	} // End for

	return(offset);
}

/**
	Breakup the line of text based on the delimiting characters passed
	in and return a vector of 'words'.
 */
std::vector<std::wstring> Tokens( const std::wstring wxLine, const std::wstring wxDelimiters )
{
	std::vector<std::wstring> tokens;
	std::wstring line(wxLine);	// non-const copy

	std::wstring::size_type offset;
	while ((offset = find_first_of( line, wxDelimiters )) != line.npos)
	{
		if (offset > 0)
		{
			tokens.push_back( line.substr(0, offset) );
		} // End if - then
		else
			break;

		line.erase(0, offset+1);
	} // End while

	if (line.size() > 0)
	{
		tokens.push_back( line );
	} // End if - then

	return(tokens);

} // End Tokens() method



bool AllNumeric( const std::wstring wxLine )
{
	if (wxLine.size() == 0) return(false);

	std::wstring line( wxLine );	// non-const copy
	std::wstring::size_type offset;

	for (offset=0; offset<line.size(); offset++)
	{
		if ((((line[offset] >= L'0') &&
		     (line[offset] <= L'9')) ||
		    (line[offset] == '+') ||
		    (line[offset] == '-') ||
		    (line[offset] == '.')) == false) return(false);
	} // End for
	return(true);
} // End Tokens() method


bool endsWith(std::wstring const &fullString, std::wstring const &ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
	}
}

void lowerCase(std::wstring &str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower); // lower case
}

void upperCase(std::wstring &str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::toupper); // upper case
}

bool startsWith(std::wstring const &fullString, std::wstring const &beginning)
{
	return fullString.compare(0, beginning.length(), beginning) == 0;
}

bool contains(std::wstring const &fullString, std::wstring const &substring)
{
	return fullString.find(substring) != std::wstring::npos;
}