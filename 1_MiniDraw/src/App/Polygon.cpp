#include "Polygon.h"

CPolygon::CPolygon(QPoint start)
{
	m_Polygon = new QPolygon();
	m_Polygon->append(start);
}

CPolygon::~CPolygon()
{
}

void CPolygon::Draw(QPainter& painter)
{
	QPolygon t_Polygon = *m_Polygon;
	t_Polygon.append(end);
	painter.drawPolygon(t_Polygon);
}

void CPolygon::addPoint()
{
	m_Polygon->append(end);
}
