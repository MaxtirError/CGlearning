#include "viewwidget.h"

ViewWidget::ViewWidget(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	draw_status_ = false;
	shape_ = NULL;
	type_ = CFigure::kDefault;
}

ViewWidget::~ViewWidget()
{
}

void ViewWidget::setLine()
{
	type_ = CFigure::kLine;
}

void ViewWidget::setRect()
{
	type_ = CFigure::kRect;
}

void ViewWidget::setEllipse()
{
	type_ = CFigure::kEllipse;
}

void ViewWidget::setPolygon()
{
	type_ = CFigure::kPolygon;
}

void ViewWidget::mousePressEvent(QMouseEvent* event)
{
	if (Qt::LeftButton == event->button())
	{
		switch (type_)
		{
		case CFigure::kLine:
			shape_ = new CLine();
			break;
		case CFigure::kDefault:
			break;

		case CFigure::kRect:
			shape_ = new CRect();
			break;
		case CFigure::kEllipse:
			shape_ = new CEllipse();
			break;
		case CFigure::kPolygon:
			if (shape_ != NULL)
				shape_->addPoint();
			else {
				shape_ = new CPolygon(event->pos());
				setMouseTracking(true);
			}
			break;
		}
		if (shape_ != NULL)
		{
			draw_status_ = true;
			start_point_ = end_point_ = event->pos();
			shape_->set_start(start_point_);
			shape_->set_end(end_point_);
		}
	}
	update();
}

void ViewWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (draw_status_ && shape_ != NULL)
	{
		end_point_ = event->pos();
		shape_->set_end(end_point_);
	}
}

void ViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (shape_ != NULL)
	{
		if (type_ != CFigure::kPolygon) {
			draw_status_ = false;
			shape_list_.push_back(shape_);
			shape_ = NULL;
		}
	}
}

void ViewWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (shape_ != NULL && type_ == CFigure::kPolygon)
	{
		draw_status_ = false;
		shape_list_.push_back(shape_);
		shape_ = NULL;
		setMouseTracking(false);
	}
}

void ViewWidget::paintEvent(QPaintEvent*)
{
	QPainter painter(this);

	for (int i = 0; i < shape_list_.size(); i++)
	{
		shape_list_[i]->Draw(painter);
	}

	if (shape_ != NULL) {
		shape_->Draw(painter);
	}

	update();
}