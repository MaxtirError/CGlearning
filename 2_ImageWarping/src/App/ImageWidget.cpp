#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <iostream>

using std::cout;
using std::endl;

ImageWidget::ImageWidget(void)
{
	wrapping_tool = NULL;
	draw_status = false;
	ptr_image_ = new QImage();
	ptr_image_backup_ = new QImage();
}


ImageWidget::~ImageWidget(void)
{
}

void ImageWidget::paintEvent(QPaintEvent *paintevent)
{
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);

	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	image_start_position.setX((width() - ptr_image_->width()) / 2);
	image_start_position.setY((height() - ptr_image_->height()) / 2);
	QRect rect = QRect( (width()-ptr_image_->width())/2, (height()-ptr_image_->height())/2, ptr_image_->width(), ptr_image_->height());
	painter.drawImage(rect, *ptr_image_);

	painter.setPen(QPen(QBrush(Qt::red), 10, Qt::SolidLine));

	if (wrapping_tool != NULL) {
		wrapping_tool->Draw(painter, draw_status);
		update();
	}

	painter.end();
}


void ImageWidget::mousePressEvent(QMouseEvent* event)
{
	if (Qt::LeftButton == event->button())
	{
		if (wrapping_tool != NULL)
		{
			draw_status = true;
			start_point_ = end_point_ = event->pos();
			wrapping_tool->set_start(start_point_);
			wrapping_tool->set_end(end_point_);
		}
	}
	update();
}

void ImageWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (draw_status && wrapping_tool != NULL)
	{
		end_point_ = event->pos();
		wrapping_tool->set_end(end_point_);
	}
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (wrapping_tool != NULL)
	{
		draw_status = false;
		wrapping_tool->add_point(image_start_position, ptr_image_->width(), ptr_image_-> height());
	}
}

void ImageWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (wrapping_tool != NULL)
	{
		draw_status = false;
		wrapping_tool->ImageTransfrom(ptr_image_, image_start_position);
	}
	update();
}

void ImageWidget::Open()
{
	// Open file
	QString fileName = QFileDialog::getOpenFileName(this, tr("Read Image"), ".", tr("Images(*.bmp *.png *.jpg)"));

	// Load file
	if (!fileName.isEmpty())
	{
		ptr_image_->load(fileName);
		*(ptr_image_backup_) = *(ptr_image_);
	}

	//ptr_image_->invertPixels(QImage::InvertRgb);
	//*(ptr_image_) = ptr_image_->mirrored(true, true);
	//*(ptr_image_) = ptr_image_->rgbSwapped();
	cout<<"image size: "<<ptr_image_->width()<<' '<<ptr_image_->height()<<endl;
	update();
	wrapping_tool = NULL;
}

void ImageWidget::Save()
{
	SaveAs();
	wrapping_tool = NULL;
}

void ImageWidget::SaveAs()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images(*.bmp *.png *.jpg)"));
	if (filename.isNull())
	{
		return;
	}	

	ptr_image_->save(filename);
	wrapping_tool = NULL;
}

void ImageWidget::Invert()
{
	for (int i=0; i<ptr_image_->width(); i++)
	{
		for (int j=0; j<ptr_image_->height(); j++)
		{
			QRgb color = ptr_image_->pixel(i, j);
			ptr_image_->setPixel(i, j, qRgb(255-qRed(color), 255-qGreen(color), 255-qBlue(color)) );
		}
	}

	// equivalent member function of class QImage
	// ptr_image_->invertPixels(QImage::InvertRgb);
	update();
	wrapping_tool = NULL;
}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical)
{
	QImage image_tmp(*(ptr_image_));
	int width = ptr_image_->width();
	int height = ptr_image_->height();

	if (ishorizontal)
	{
		if (isvertical)
		{
			for (int i=0; i<width; i++)
			{
				for (int j=0; j<height; j++)
				{
					ptr_image_->setPixel(i, j, image_tmp.pixel(width-1-i, height-1-j));
				}
			}
		} 
		else			//仅水平翻转			
		{
			for (int i=0; i<width; i++)
			{
				for (int j=0; j<height; j++)
				{
					ptr_image_->setPixel(i, j, image_tmp.pixel(width-1-i, j));
				}
			}
		}
		
	}
	else
	{
		if (isvertical)		//仅垂直翻转
		{
			for (int i=0; i<width; i++)
			{
				for (int j=0; j<height; j++)
				{
					ptr_image_->setPixel(i, j, image_tmp.pixel(i, height-1-j));
				}
			}
		}
	}

	// equivalent member function of class QImage
	//*(ptr_image_) = ptr_image_->mirrored(true, true);
	update();
	wrapping_tool = NULL;
}

void ImageWidget::TurnGray()
{
	for (int i=0; i<ptr_image_->width(); i++)
	{
		for (int j=0; j<ptr_image_->height(); j++)
		{
			QRgb color = ptr_image_->pixel(i, j);
			int gray_value = (qRed(color)+qGreen(color)+qBlue(color))/3;
			ptr_image_->setPixel(i, j, qRgb(gray_value, gray_value, gray_value) );
		}
	}

	update();
	wrapping_tool = NULL;
}

void ImageWidget::Restore()
{
	*(ptr_image_) = *(ptr_image_backup_);
	update();
	wrapping_tool = NULL;
}

void ImageWidget::IDW()
{
	wrapping_tool = new CIDW();
}


void ImageWidget::RBF()
{
	wrapping_tool = new CRBF();
}