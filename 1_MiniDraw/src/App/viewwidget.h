#pragma once

#include <ui_viewwidget.h>

#include "Shape.h"
#include "Line.h"
#include "Rect.h"
#include "Ellipse.h"
#include "Polygon.h"

#include <qevent.h>
#include <qpainter.h>
#include <QWidget>

#include <vector>

class ViewWidget : public QWidget
{
	Q_OBJECT

public:
	ViewWidget(QWidget* parent = 0);
	~ViewWidget();

private:
	Ui::ViewWidget ui;

private:
	bool draw_status_;
	QPoint start_point_;
	QPoint end_point_;
	CFigure::Type type_;
	CFigure* shape_;
	std::vector<CFigure*> shape_list_;


public:
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);

public:
	void paintEvent(QPaintEvent*);
signals:
public slots:
	void setLine();
	void setRect();
	void setEllipse();
	void setPolygon();

};
