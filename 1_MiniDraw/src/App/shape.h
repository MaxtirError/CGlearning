#pragma once

#include <QtGui>

class CFigure
{
public:
	CFigure();
	virtual ~CFigure();
	virtual void Draw(QPainter& paint) = 0;
	virtual void addPoint() {};
	void set_start(QPoint s);
	void set_end(QPoint e);

public:
	enum Type
	{
		kDefault = 0,
		kLine = 1,
		kRect = 2,
		kEllipse = 3,
		kPolygon = 4,
	};

protected:
	QPoint start;
	QPoint end;
};

