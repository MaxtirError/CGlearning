#pragma once
#include "Linescanner.h"
#include <QtGui>

class CPolygon {
public:
	CPolygon(QPoint start);
	~CPolygon();

	void set_start(QPoint s);
	void set_end(QPoint e);
	void Draw(QPainter& painter);
	ScannerRegion Get_Inside();
	void addPoint();

private:
	QPolygon *m_Polygon;
	QPoint start;
	QPoint end;
};
