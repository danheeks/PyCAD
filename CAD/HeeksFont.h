#pragma once

float DrawHeeksFontString(const char* str, bool outline, bool fill);
float GetHeeksFontStringWidth(const char* str);

void DrawHeeksFontStringAntialiased(const char* str, double blur_scale, bool outline, bool fill);