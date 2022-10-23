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

ScannerRegion CPolygon::Get_Inside() 
{
	LineScanner Scanner;
	for (auto p : *m_Polygon)
	{
		Scanner.add_point(p);
	}
	return Scanner.Scanning();
}

void CPolygon::addPoint()
{
	m_Polygon->append(end);
	printf("%d %d\n", end.rx(), end.ry());
}

void CPolygon::set_start(QPoint s)
{
	start = s;
}

void CPolygon::set_end(QPoint e)
{
	end = e;
}
