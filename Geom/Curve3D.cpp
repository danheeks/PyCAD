#include "Curve3D.h"
#include "Box.h"

CVertex3D::CVertex3D(const Point3d& p)
{
	m_p = p;
}

void CVertex3D::Transform(const Matrix& matrix)
{
	m_p = m_p.Transformed(matrix);
}

void CCurve3D::append(const CVertex3D& vertex)
{
	m_vertices.push_back(vertex);
}

void CCurve3D::GetBox(CBox &box)const
{
	for (std::list<CVertex3D>::const_iterator It = m_vertices.begin(); It != m_vertices.end(); It++)
	{
		const CVertex3D& vertex = *It;
		box.Insert(vertex.m_p.getBuffer());
	}
}

void CCurve3D::Reverse()
{
	std::list<CVertex3D> copy = m_vertices;
	m_vertices.clear();
	for (std::list<CVertex3D>::iterator It = copy.begin(); It != copy.end(); It++)
	{
		CVertex3D& vertex = *It;
		m_vertices.push_back(vertex);
	}
}

void CCurve3D::Transform(const Matrix& matrix)
{
	for (std::list<CVertex3D>::iterator It = m_vertices.begin(); It != m_vertices.end(); It++)
	{
		CVertex3D& vertex = *It;
		vertex.Transform(matrix);
	}
}
