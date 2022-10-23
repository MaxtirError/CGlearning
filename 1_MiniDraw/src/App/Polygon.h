#pragma once

#include "Shape.h"

class CPolygon : public CFigure {
public:
	CPolygon(QPoint start);
	~CPolygon();

	void Draw(QPainter& painter);
	void addPoint();

private:
	QPolygon *m_Polygon;
};
