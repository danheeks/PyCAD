// HeeksFont.cpp

#include <stdafx.h>
#include "HeeksFont.h"
#include "../Geom/Geom.h"
#include "HeeksFontData.h"

float DrawHeeksFontString(const char* str, bool outline, bool fill)
{
	int i = 0;
	int index = 0;
	int num_c = 0;
	int curve = 0;
	int num_t = 0;
	int tri = 0;
	int point_index = 0;
	int num_p = 0;
	float x_spacing = 0.16f;
	float x_translated = 0.0f;

	while (str[i] != 0)
	{
		index = str[i] - 32;
		if (index >= 0 && index < 96)
		{
			if (outline)
			{
				num_c = num_curves[index];
				curve = start_curves[index];
				for (int j = 0; j < num_c; j++)
				{
					point_index = start_points[curve];
					num_p = num_points[curve];
					glBegin(GL_LINE_STRIP);
					for (int k = 0; k < num_p; k++)
					{
						float* p = points[point_index];
						glVertex2d(p[0], p[1]);
						point_index++;
					}
					glEnd();
					curve++;
				}
			}
			if (fill)
			{
				num_t = num_triangles[index];
				tri = start_tris[index];
				glBegin(GL_TRIANGLES);
				for (int j = 0; j < num_t; j++)
				{
					glVertex2d(tris[tri][0], tris[tri][1]);
					glVertex2d(tris[tri][2], tris[tri][3]);
					glVertex2d(tris[tri][4], tris[tri][5]);
					tri++;
				}
				glEnd();
			}
			float x = widths[index] + x_spacing;
			glTranslated(x, 0.0, 0.0);
			x_translated += x;
		}
		i++;
	}

	return x_translated;
}

#define NUM_DITHERS 7
static double dither_xy[NUM_DITHERS][2] = {
	{ 0.0, 0.0, },
	{ 0.866, 0.5, },
	{ 0.0, 1.0, },
	{ -0.866, 0.5, },
	{ -0.866, -0.5, },
	{ 0.0, -1.0, },
	{ 0.866, -0.5, },
};

void DrawHeeksFontStringAntialiased(const char* str, double blur_scale, bool outline, bool fill)
{
	float currentColor[4];
	glGetFloatv(GL_CURRENT_COLOR, currentColor);

	glColor4f(currentColor[0], currentColor[1], currentColor[2], 0.25f);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float x = 0.0f;
	for (int i = 0; i < NUM_DITHERS; i++)
	{
		double dx = dither_xy[i][0] * blur_scale;
		double dy = dither_xy[i][1] * blur_scale;
		glTranslated(dx, dy, 0.0);
		x = DrawHeeksFontString(str, outline, fill);
		glTranslatef(-x, 0, 0);
		glTranslated(-dx, -dy, 0.0);
	}
	glTranslatef(x, 0, 0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}
