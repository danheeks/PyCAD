#include <stdafx.h>

void SetPickingColor(unsigned int name)
{
	unsigned char r = name & 0xff;
	unsigned char g = (name & 0xff00) >> 8;
	unsigned char b = (name & 0xff0000) >> 16;
	glColor3ub(r, g, b); 
}
