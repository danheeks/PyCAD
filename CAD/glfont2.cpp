//*******************************************************************
//glfont2.cpp -- glFont Version 2.0 implementation
//Copyright (c) 1998-2002 Brad Fish
//See glfont.html for terms of use
//May 14, 2002
//*******************************************************************

//STL headers
#include <string>
#include <utility>
#include <iostream>
#include <fstream>
using namespace std;

//OpenGL headers
#ifdef _WINDOWS
#include <windows.h>
#endif
#include <GL/gl.h>

//glFont header
#include "glfont2.h"
using namespace glfont;

#ifdef __BIG_ENDIAN__
inline void endian_swap(int& x)
{
    x = (x>>24) | 
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}
#endif

//*******************************************************************
//GLFont Class Implementation
//*******************************************************************
GLFont::GLFont ()
{
	//Initialize header to safe state
	header.tex_normal = -1;
	m_tex_sharp = -1;
	header.tex_width = 0;
	header.tex_height = 0;
	header.start_char = 0;
	header.end_char = 0;
	header.chars = NULL;
}
//*******************************************************************
GLFont::~GLFont ()
{
	//Destroy the font
	Destroy();
}
//*******************************************************************
bool GLFont::Create(const char *file_name, int tex_normal, int tex_sharp)
{
	ifstream input;
	int num_chars, num_tex_bytes;
	char *tex_bytes;

	//Destroy the old font if there was one, just to be safe
	Destroy();

	//Open input file
	input.open(file_name, ios::in | ios::binary);
	if (!input)
		return false;

	//Read the header from file
	//There is 4 byte space between the header data and the beginning of the
	//characters. this presumably was used to account for the 4 bytes taken
	//up by the pointer to the character array
	//on 64-bit the sizeof(header) is no longer correct and it reads to many
	//bytes
	input.read((char *)&header, sizeof(header) - (sizeof(void*) - 4));
	header.tex_normal = tex_normal;
	m_tex_sharp = tex_sharp;

	//Allocate space for character array
#ifdef __BIG_ENDIAN__
	endian_swap(header.end_char);
	endian_swap(header.start_char);
#endif
	num_chars = header.end_char - header.start_char + 1;
	if ((header.chars = new GLFontChar[num_chars]) == NULL)
		return false;

	//Read character array
	for(int i=0; i < num_chars; i++)
		input.read((char *)&header.chars[i], sizeof(GLFontChar));

	//Read texture pixel data
	num_tex_bytes = header.tex_width * header.tex_height * 2;
	tex_bytes = new char[num_tex_bytes];
	input.read(tex_bytes, num_tex_bytes);

	//Create OpenGL texture
	glBindTexture(GL_TEXTURE_2D, tex_normal);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 2, header.tex_width,
		header.tex_height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
		(void *)tex_bytes);

	for (int i = 0; i < num_tex_bytes; i += 2)
	{
		tex_bytes[i] = -1;
		if (tex_bytes[i + 1] != 0)
			tex_bytes[i + 1] = -1;
		else
			tex_bytes[i + 1] = 0;
	}

	glBindTexture(GL_TEXTURE_2D, tex_sharp);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexImage2D(GL_TEXTURE_2D, 0, 2, header.tex_width,
		header.tex_height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
		(void *)tex_bytes);

	//Free texture pixels memory
	delete[] tex_bytes;

	//Close input file
	input.close();

	//Return successfully

	//Uncomment to print out the data structure. Useful for finding differences between systems

/*	cout << "Start: " << header.start_char << ", End: " << header.end_char << ", Chars: " << header.chars << endl;
	cout << "Height: " << header.tex_height << ", Width: " << header.tex_width << endl;
	//Read character array
	for(int i=header.start_char; i < header.end_char; i++)
	{
		GLFontChar c = header.chars[i - header.start_char];
		cout << "Char: " << i << ", dx: " << c.dx << ", dy: " << c.dy << endl;
		cout << "ty1: " << c.ty1 << ", ty2: " << c.ty2 << ", tx1: " << c.tx1 << ", tx2: " << c.tx2 << endl;
	}*/

	return true;
}

//*******************************************************************
void GLFont::Destroy (void)
{
	//Delete the character array if necessary
	if (header.chars)
	{
		delete[] header.chars;
		header.chars = NULL;
	}
}
//*******************************************************************
void GLFont::GetTexSize (std::pair<int, int> *size)
{
	//Retrieve texture size
	size->first = header.tex_width;
	size->second = header.tex_height;
}
//*******************************************************************
int GLFont::GetTexWidth (void)
{
	//Return texture width
	return header.tex_width;
}
//*******************************************************************
int GLFont::GetTexHeight (void)
{
	//Return texture height
	return header.tex_height;
}
//*******************************************************************
void GLFont::GetCharInterval (std::pair<int, int> *interval)
{
	//Retrieve character interval
	interval->first = header.start_char;
	interval->second = header.end_char;
}
//*******************************************************************
int GLFont::GetStartChar (void)
{
	//Return start character
	return header.start_char;
}
//*******************************************************************
int GLFont::GetEndChar (void)
{
	//Return end character
	return header.end_char;
}
//*******************************************************************
void GLFont::GetCharSize (int c, std::pair<int, int> *size)
{
	//Make sure character is in range
	if (c < header.start_char || c > header.end_char)
	{
		//Not a valid character, so it obviously has no size
		size->first = 0;
		size->second = 0;
	}
	else
	{
		GLFontChar *glfont_char;

		//Retrieve character size
		glfont_char = &header.chars[c - header.start_char];
		size->first = (int)(glfont_char->dx * header.tex_width);
		size->second = (int)(glfont_char->dy *
			header.tex_height);
	}
}
//*******************************************************************
int GLFont::GetCharWidth (int c)
{
	//Make sure in range
	if (c < header.start_char || c > header.end_char)
		return 0;
	else
	{
		GLFontChar *glfont_char;
		
		//Retrieve character width
		glfont_char = &header.chars[c - header.start_char];
		return (int)(glfont_char->dx * header.tex_width);
	}
}
//*******************************************************************
int GLFont::GetCharHeight (int c)
{
	//Make sure in range
	if (c < header.start_char || c > header.end_char)
		return 0;
	else
	{
		GLFontChar *glfont_char;

		//Retrieve character height
		glfont_char = &header.chars[c - header.start_char];
		return (int)(glfont_char->dy * header.tex_height);
	}
}
//*******************************************************************
void GLFont::Begin (bool sharp)
{
	//Bind to font texture
	if (sharp)
	{
		glBindTexture(GL_TEXTURE_2D, m_tex_sharp);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, header.tex_normal);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
}
//*******************************************************************


wchar_t GLFont::get_unaccented_character( wchar_t c)
	{
		switch(c)
		{
		case 225:
		case 224:
			return 'a';
		case 193: 
		case 192:
			return 'A';
		case 233:
		case 232:
			return 'e';
		case 201:
		case 200:
			return 'E';
		case 237:
		case 236:
			return 'i';
		case 205:
		case 204:
			return 'I';
		case 243:
		case 242:
			return 'o';
		case 211:
		case 210:
			return 'O';
		case 250:
		case 249:
			return 'u';
		case 218:
		case 217:
			return 'U';
		case 241:
			return 'n';
		case 209:
			return 'N';
		case 252:
			return 'u';
		case 220:
			return 'U';
		default:
			return c;
		}
	}

//End of file

