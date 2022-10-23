#pragma once

#include <QtGui>

class CWrapping
{
public:
	CWrapping();
	virtual ~CWrapping();
	void Draw(QPainter& paint, bool isdraw);
	virtual void ImageTransfrom(QImage *ptr_image, QPoint start_pos) = 0;
	void add_point(QPoint s, int weight, int height);
	void set_start(QPoint s);
	void set_end(QPoint e);


public:
	/*enum Type
	{
		kDefault = 0,
		kLine = 1,
		kRect = 2,
		kEllipse = 3,
		kPolygon = 4,
	};*/

protected:
	QPoint start;
	QPoint end;
	std::vector<std::pair<QPoint, QPoint>> matching_list;
};

