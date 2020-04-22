// HeeksFont.cpp

#include "stdafx.h"
#include "HeeksFont.h"
#include "../Geom/Geom.h"
#include "HeeksFontData.h"

void DrawHeeksFontString(const char* str, double scale, bool outline, bool fill)
{
	glPushMatrix();
	glTranslated(0, -scale * 1.6, 0);
	glScaled(scale * 1.6, scale * 1.6, 1.0);
	int i = 0;
	int index = 0;
	int num_c = 0;
	int curve = 0;
	int num_t = 0;
	int tri = 0;
	int point_index = 0;
	int num_p = 0;
	float x_spacing = 0.1f;

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
			glTranslated(widths[index] + x_spacing, 0.0, 0.0);
		}
		i++;
	}
	glPopMatrix();
}

static double dither_xy[8][2] = {
	{ 1.0, 0.0, },
	{ 0.7, 0.7, },
	{ 0.0, 1.0, },
	{ -0.7, 0.7, },
	{ -1.0, 0.0, },
	{ -0.7, -0.7, },
	{ 0.0, -1.0, },
	{ 0.7, -0.7, },
};

void DrawHeeksFontStringAntialiased(const char* str, double scale, double blur_scale, bool outline, bool fill)
{
	glColor4ub(0x00, 0x00, 0x00, 0x40);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (int i = 0; i < 8; i++)
	{
		glPushMatrix();
		glTranslated(dither_xy[i][0] * blur_scale, dither_xy[i][1] * blur_scale, 0.0);
		DrawHeeksFontString(str, scale, outline, fill);
		glPopMatrix();
	}
	glDisable(GL_BLEND);
}