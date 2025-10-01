// AreaCavC.cpp
#include "CavalierContours/include/cavc/polylineoffset.hpp"
#include "CavalierContours/include/cavc/polylineoffsetislands.hpp"

// implements CArea methods using "Cavalier Contours" https://github.com/jbuckmccready/CavalierContours

#include "Area.h"

bool CArea::HolesLinked(){ return false; }

static const double M_PI = 3.1415926535897932;

void CArea::Subtract(const CArea& a2)
{
}

void CArea::Intersect(const CArea& a2)
{
}

void CArea::Union(const CArea& a2)
{
}

// static
CArea CArea::UniteCurves(std::list<CCurve> &curves)
{
	CArea area;
	return area;
}

void CArea::Xor(const CArea& a2)
{
}

cavc::Polyline<double> convertCurveToPolyline(const CCurve& curve) {
    cavc::Polyline<double> polyline;

    if (curve.m_vertices.empty())
        return polyline;

    auto it = curve.m_vertices.begin();
    Point prev = it->m_p;   // start point
    //polyline.addVertex(prev.x, prev.y, 0.0); // start has no bulge

    ++it;
    for (; it != curve.m_vertices.end(); ++it) {
        const CVertex& v = *it;

        double bulge = 0.0;
        if (v.m_type != 0) {
            // arc segment
            Point center = v.m_c;
            Point start = prev;
            Point end = v.m_p;

            double dxs = start.x - center.x;
            double dys = start.y - center.y;
            double dxe = end.x - center.x;
            double dye = end.y - center.y;

            double ang1 = std::atan2(dys, dxs);
            double ang2 = std::atan2(dye, dxe);
            double delta = ang2 - ang1;

            if (v.m_type == 1 && delta < 0) delta += 2 * M_PI;    // CCW
            if (v.m_type == -1 && delta > 0) delta -= 2 * M_PI;   // CW

            bulge = std::tan(delta / 4.0);
        }

        polyline.addVertex(v.m_p.x, v.m_p.y, bulge);
        prev = v.m_p;
    }

    polyline.isClosed() = true;

    return polyline;
}

void convertAreaToLoopSet(const CArea& area, cavc::OffsetLoopSet<double> &loopSet)
{
    for (std::list<CCurve>::const_iterator It = area.m_curves.begin(); It != area.m_curves.end(); It++)
    {
        const CCurve& curve = *It;
        cavc::Polyline<double> polyline = convertCurveToPolyline(curve);
        if (curve.IsClockwise())
        {
            loopSet.cwLoops.push_back({ 0, polyline, cavc::createApproxSpatialIndex(polyline) });
        }
        else
        {
            loopSet.ccwLoops.push_back({ 0, polyline, cavc::createApproxSpatialIndex(polyline) });
        }
    }
}


Point arcCenter(const Point& P0, const Point& P1, double bulge)
{
    if (bulge == 0.0) return { 0.0, 0.0 }; // degenerate = line

    double dx = P1.x - P0.x;
    double dy = P1.y - P0.y;
    double chord = std::sqrt(dx * dx + dy * dy);

    // Midpoint of chord
    double mx = (P0.x + P1.x) / 2.0;
    double my = (P0.y + P1.y) / 2.0;

    // Central angle
    double theta = 4.0 * std::atan(bulge);

    // Radius
    double radius = chord / (2.0 * std::sin(theta / 2.0));

    // Offset distance from midpoint to center
    double h = std::sqrt(radius * radius - (chord / 2.0) * (chord / 2.0));

    // Perpendicular to chord
    double px = -dy / chord;
    double py = dx / chord;

    // Flip direction for CW bulge
    if (bulge < 0.0) { px = -px; py = -py; }

    return { mx + h * px, my + h * py };
}



CArea convertOffsetResultToArea(const cavc::OffsetLoopSet<double>& offsetResult)
{
    CArea area;

    // Helper lambda to convert a CavalierContours polyline to a CCurve
    auto polyToCurve = [](const cavc::Polyline<double>& poly) -> CCurve
    {
        CCurve curve;
        const cavc::PlineVertex<double>* prev_v = NULL;

        for (const auto& v : poly.vertexes())
        {
            CVertex cv;
            cv.m_p.x = v.x();
            cv.m_p.y = v.y();

            if (prev_v != NULL)
            {
                if (!prev_v->bulgeIsZero())
                {
                    cv.m_c = arcCenter(curve.m_vertices.back().m_p, cv.m_p, prev_v->bulge());
                    cv.m_type = (prev_v->bulge() > 0) ? 1 : -1;
                }
            }
            curve.m_vertices.push_back(cv);

            prev_v = &v;
        }

        CVertex cv;
        cv.m_p = curve.m_vertices.front().m_p;

        if (prev_v != NULL)
        {
            if (!prev_v->bulgeIsZero())
            {
                cv.m_c = arcCenter(curve.m_vertices.back().m_p, cv.m_p, prev_v->bulge());
                cv.m_type = (prev_v->bulge() > 0) ? 1 : -1;
            }
        }
        curve.m_vertices.push_back(cv);

        return curve;
    };

    // Convert all counter-clockwise loops (outer boundaries)
    for (const auto& ccw : offsetResult.ccwLoops)
    {
        area.m_curves.push_back(polyToCurve(ccw.polyline));
    }

    // Convert all clockwise loops (islands)
    for (const auto& cw : offsetResult.cwLoops)
    {
        area.m_curves.push_back(polyToCurve(cw.polyline));
    }

    return area;
}

void CArea::Offset(double inwards_value)
{
      cavc::OffsetLoopSet<double> loopSet;

#if 0
      cavc::Polyline<double> outerCCWLoop;
      outerCCWLoop.addVertex(0, 0, 0);
      outerCCWLoop.addVertex(10, 0, 0);
      outerCCWLoop.addVertex(10, 10, 0);
      outerCCWLoop.addVertex(0, 10, 0);
      outerCCWLoop.isClosed() = true;

      loopSet.ccwLoops.push_back({ 0, outerCCWLoop, cavc::createApproxSpatialIndex(outerCCWLoop) });
#endif


    convertAreaToLoopSet(*this, loopSet);

    cavc::ParallelOffsetIslands<double> alg;
    double offsetDelta = inwards_value;
    cavc::OffsetLoopSet<double> offsetResult = alg.compute(loopSet, offsetDelta);

    *this = convertOffsetResultToArea(offsetResult);
}




void CArea::Thicken(double value)
{
}
