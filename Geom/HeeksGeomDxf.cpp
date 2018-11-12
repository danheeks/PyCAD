#include "HeeksGeomDxf.h"

void AddToDxfFile(CDxfWrite& writer, const CArea& area)
{
	for (std::list<CCurve>::const_iterator It = area.m_curves.begin(); It != area.m_curves.end(); It++)
	{
		const CCurve& curve = *It;
		AddToDxfFile(writer, curve);
	}
}

void AddToDxfFile(CDxfWrite& writer, const CCurve& curve)
{
	std::list<Span> spans;
	curve.GetSpans(spans);
	for (std::list<Span>::iterator It = spans.begin(); It != spans.end(); It++)
	{
		Span& span = *It;
		AddToDxfFile(writer, span);
	}
}

void AddToDxfFile(CDxfWrite& writer, const Span& span)
{
	double s[3] = { 0, 0, 0 };
	double e[3] = { 0, 0, 0 };
	s[0] = span.m_p.x;
	s[1] = span.m_p.y;
	e[0] = span.m_v.m_p.x;
	e[1] = span.m_v.m_p.y;

	if (span.m_v.m_type == 0)
	{
		writer.WriteLine(s, e, "0");
	}
	else
	{
		double c[3] = { 0, 0, 0 };
		c[0] = span.m_v.m_c.x;
		c[1] = span.m_v.m_c.y;
		bool dir = (span.m_v.m_type > 0);
		writer.WriteArc(s, e, c, dir, "0");
	}
}

void AddToDxfFile(CDxfWrite& writer, const std::list<Line> &intof_list)
{
	for (std::list<Line>::const_iterator It = intof_list.begin(); It != intof_list.end(); It++)
	{
		const Line& line = *It;
		writer.WriteLine(line.p0.getBuffer(), (line.p0 + line.v).getBuffer(), "0");
	}
}

void WriteDxfFile(const CArea& area, const std::string& dxf_file_path)
{
	CDxfWrite dxf_writer(dxf_file_path.c_str());
	AddToDxfFile(dxf_writer, area);
}

void WriteDxfFile(const CCurve& curve, const std::string& dxf_file_path)
{
	CDxfWrite dxf_writer(dxf_file_path.c_str());
	AddToDxfFile(dxf_writer, curve);
}

void WriteDxfFile(const std::string& dxf_file_path, const std::list<Line> &intof_list)
{
	CDxfWrite dxf_writer(dxf_file_path.c_str());
	AddToDxfFile(dxf_writer, intof_list);
}
