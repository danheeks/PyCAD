#pragma once

#include "dxf.h"
#include "geometry.h"
#include "Area.h"
#include "Curve.h"

void AddToDxfFile(CDxfWrite& writer, const CArea& area);
void AddToDxfFile(CDxfWrite& writer, const CCurve& curve);
void AddToDxfFile(CDxfWrite& writer, const Span& span);
void AddToDxfFile(CDxfWrite& writer, const std::list<Line> &intof_list);

void WriteDxfFile(const CArea& area, const std::string& dxf_file_path);
void WriteDxfFile(const CCurve& curve, const std::string& dxf_file_path);
void WriteDxfFile(const std::string& dxf_file_path, const std::list<Line> &intof_list);

